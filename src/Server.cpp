/*
 * Hikari Backend
 *
 * Copyright (C) 2018 RickyCorte
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Server.hpp"

#include <rickycorte/Logging.hpp>

#include <cstring>
#include <chrono>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include "http/Reply.hpp"
#include "http/Request.hpp"


namespace RickyCorte
{

    Server::Server(int bind_port)
    :_server_port{bind_port}, _exit_loop{false}
    {
        _epoll_fd = epoll_create1(0);
        if(_epoll_fd == -1)
        {
            RC_CRITICAL("Can't create epoll file descriptor");
            exit(1);
        }
    }

    void Server::Run()
    {
        _server_fd = bind_server_port(_server_port);


        struct epoll_event *events = (epoll_event *)calloc(EPOLL_MAX_EVENTS, sizeof(epoll_event));

        if(listen(_server_fd, MAX_CONNECTIONS))
        {
            RC_CRITICAL("Unable to listen to server socket");
            exit(4);
        }


        if(!add_to_epoll(_server_fd, _epoll_fd))
        {
            RC_CRITICAL("Unable to add server fd to epoll");
            exit(5);
        }

        RC_INFO("Running http server on port: ", _server_port);

        while (!_exit_loop)
        {
            int ev_count = epoll_wait(_epoll_fd, events, EPOLL_MAX_EVENTS, 1000);
            for(int i = 0; i < ev_count; i++)
            {
                if(check_epoll_error( &events[i] )) continue; //skip iteration on error

                //check for new pending connections
                if(events[i].data.fd == _server_fd)
                {
                    //accept all new connections
                    int new_conn = -1;
                    do
                    {
                        new_conn = accept_connection(_server_fd);
                        if(new_conn > -1 && !add_to_epoll(new_conn, _epoll_fd))
                        {
                            RC_WARNING("Unable to add connection ", new_conn," to epoll, ignoring it");
                            close(new_conn);
                        }
                    }
                    while(new_conn != -1);
                }
                else
                {
                    handle_socket_event(&events[i]);
                }

            }

        }

        free(events);
    }


    void Server::Dispose()
    {
        close(_epoll_fd);
        close(_server_fd);
        for(auto itr = _api_endpoints.begin(); itr != _api_endpoints.end(); itr++)
        {
            delete itr->second;
        }
        _api_endpoints.clear();
    }


    int Server::bind_server_port(int port)
    {
        struct sockaddr_in reciver;
        memset(&reciver, 0, sizeof(reciver));
        reciver.sin_family = AF_INET;
        reciver.sin_port = htons(port);

        //create listen socket
        int listenSocket = socket(reciver.sin_family, SOCK_STREAM , 0);

        int sockOtp = 1;
        //enable socket reuse before os timeout
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &sockOtp, sizeof(sockOtp));

        if( bind(listenSocket, (struct sockaddr *)&reciver , sizeof(reciver) ) < 0)
        {
            RC_CRITICAL("Unable to bind server on port: ", port);
            exit(2);
        }

        //set notblocking
        if(fcntl(listenSocket, F_SETFL, O_NONBLOCK))
        {
            RC_CRITICAL("Can't set listen socket to nonblock mode");
            exit(3);
        }

        return listenSocket;
    }


    int Server::accept_connection(int server_fd)
    {
        struct sockaddr in_addr;
        socklen_t in_len = sizeof(in_addr);


        RC_DEBUG("server sock: ", server_fd);
        int new_conn = accept4(server_fd, &in_addr, &in_len, SOCK_NONBLOCK);

        //print new connection informations
        if(new_conn != -1)
        {
            char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
            if(getnameinfo(&in_addr, in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV) == 0)
            {
                RC_INFO("Accepted connection ", new_conn, " from ", hbuf,":",sbuf);
            }
        }
        else RC_DEBUG("Connection error: "," ", errno ," ", strerror(errno));

        return new_conn;
    }


    bool Server::check_epoll_error(epoll_event *event)
    {
        if(!event) return true;

        if ((event->events & EPOLLERR) || (event->events & EPOLLHUP) ||
            (!(event->events & EPOLLIN)))
        {
            // An error has occured on this fd
            RC_ERROR("Error in socket fd: ", event->data.fd, ". Error type: ",event->events, " Error number: ", errno);
            fprintf(stderr, "epoll error. events=%u\n", event->events);
            close(event->data.fd);
            return true;
        }

        if(event->events & EPOLLRDHUP)
        {
            RC_WARNING("Detected socket close on fd: ", event->data.fd, ". Closing socket");
            return true;
        }

        return false;
    }


    void Server::handle_socket_event(epoll_event *event)
    {

        std::string req_string;
        char *buffer = new char[READ_BUFFER_SIZE];
        if(!buffer)
        {
            RC_CRITICAL("Unable to allocate read buffer!");
            return;
        }

        //read all available data
        int read_sz = -1, ready_to_read_sz = -1;
        do
        {
            memset(buffer, '\0', READ_BUFFER_SIZE);
            read_sz = read(event->data.fd, buffer, READ_BUFFER_SIZE -1);

            ioctl(event->data.fd, FIONREAD, &ready_to_read_sz);

            if(read_sz > 0)
            {
                req_string += buffer;
            }
            else
            {
                RC_INFO("Socket ", event->data.fd, " has been closed");
                close(event->data.fd);
                return;
            }

        }
        while(ready_to_read_sz > 0);

        // let's check request size
        if(req_string.size() > MAX_HTTP_REQUEST_SIZE)
        {
            // we just lie about our max request size
            // most of the clients will just trust us and don't overload the server
            req_string = Http::Reply(413, "Payload size too large! Max size is: "
                                          + std::to_string(MAX_HTTP_REQUEST_SIZE / 2)).Dump();
        }
        else
        {
            using namespace std::chrono;

            high_resolution_clock::time_point start_tm = high_resolution_clock::now();

            //RC_DEBUG("Got request: ", req_string);
            auto req = Http::Request(req_string.c_str(), req_string.size());

            auto duration = duration_cast<microseconds>( high_resolution_clock::now() - start_tm ).count();
            RC_DEBUG("Parse time: ", duration,"us");


            // check if valid and path is present
            if(req.IsValid())
            {
                if(_api_endpoints.find(req.GetPath()) != _api_endpoints.end())
                {
                    auto rep = _api_endpoints[req.GetPath()]->DispatchRequest(req);
                    rep.SetHeader("Content-Type", "application/json");
                    req_string = rep.Dump();
                }
                else
                    req_string = Http::Reply(404, "Not found").Dump();
            }
            else
            {
                req_string = Http::Reply(400, Http::Request::ErrorCodeToString(req.GetErrorCode())+" "+ req.getErrorMessage()).Dump();
            }
        }

        //RC_DEBUG("Computed reply: ", req_string);
        if(write(event->data.fd, req_string.c_str(), req_string.size()) <= 0)
        {
            RC_ERROR("Error writing on socket: ", event->data.fd);
        }

        //TODO: sto coso lo mandiamo in timeout al posto di chiuderlo eh :3
        delete[] buffer;
    }


    bool Server::add_to_epoll(int target_socket, int epoll_fd)
    {
        epoll_event event;

        event.data.fd = target_socket;
        event.events = EPOLLIN | EPOLLET;

        return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_socket, &event) == 0;
    }

    void Server::AddApiInterface(std::string path, Http::ApiInterface *api)
    {
        if(!api) return;

        _api_endpoints[path] = api;
    }

}