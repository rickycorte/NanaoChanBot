/************************************************************************************
*
*    NanaoChanBot
*    Copyright (C) 2017  RickyCorte
*    https://github.com/rickycorte
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
************************************************************************************/

#include "Connection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>

#include <openssl/err.h>

#include "../Log.h"
#include "../Defines.h"
#include "../Helper.h"


namespace Connection
{

  static std::vector<connection_data *> connection_pool;
  static std::mutex pool_mtx;


  void init()
  {
    //inizializza openssl
    SSL_load_error_strings ();
    SSL_library_init ();

  }


  int createSocket(const char *hostname_to_resolve)
  {
    //prova a creare un socket
    int sock = socket(AF_INET, SOCK_STREAM, 0); /* O for socket() function choose the correct protocol based on the socket type. */

    if(sock <= 0)
    {
        clog::error("Unable to open ssl connection socket!");
        return -1;
    }

    //lookup dns telegram
    struct addrinfo *servinfo;
    if(getaddrinfo(hostname_to_resolve,"443",nullptr,&servinfo) != 0)
    {
        clog::error("DNS lookup failed for: " + std::string(hostname_to_resolve));
        return -2;
    }

    bool is_connected = false;
    //sfoglia i risultati
    for(auto entry = servinfo; entry != nullptr; entry = entry->ai_next)
    {
        //tenta la connessione con le varie possibilita
        //finche i server non accettano una richiesta
        if(connect(sock, entry->ai_addr, entry->ai_addrlen) == 0)
        {
          is_connected = true;
          break;
        }

    }
    //elimina dati dns inutili
    freeaddrinfo(servinfo);

    //controlla che ci si e' connessi a qualcosa
    if(!is_connected)
    {
      clog::error("Unable to connect to: "+ std::string(hostname_to_resolve) );
      return -3;
    }

    return sock;
  }



  connection_data *createSSLConnection(int sock)
  {
    SSL_CTX *ssl_ctx = SSL_CTX_new (SSLv23_client_method ());

    SSL *conn = SSL_new(ssl_ctx);
    SSL_set_fd(conn, sock);

    int err = SSL_connect(conn);

    //connessione fallina
    if (err != 1)
    {
      clog::error("Unable to connect ssl socket");
      //deallova tutte le risorse usate
      SSL_shutdown(conn);
      SSL_free (conn);
      SSL_CTX_free(ssl_ctx);
      close(sock);

      return nullptr;
    }

    return new connection_data(sock,conn,ssl_ctx);

  }


  connection_data *get_connection_from_pool()
  {
    pool_mtx.lock();

    //recupera una connessione nuova se possibile
    if(connection_pool.size() > 0)
    {
      //recupera la connessione
      connection_data *conn = connection_pool[0];
      connection_pool.erase(connection_pool.begin());

      //controlla che sia ancora possibile usarla
      if(SSL_get_shutdown(conn->connection) == 0 && !conn->isTimeOut())
      {
        //connesione buona
        pool_mtx.unlock();
        return conn;
      }
      else
      {
        clog::log("Connection closed: timeout.");
        //dealloca oggetti usati dalla connesione non piu valida
        freeConnection(conn);
      }

    }

    //crea una connessione nuova visto che non si puo usare una vecchia
    pool_mtx.unlock();

    //crea un socket
    int sock = createSocket(Bot::Telegram_Host);
    if(sock < 0) return nullptr;

    //crea connessione ssl
    connection_data *conn = createSSLConnection(sock);
    if(!conn)
    {
      close(sock);
      return nullptr;
    }

    return conn;
  }


  void add_connection_to_pool(connection_data *conn)
  {
    if(!conn) return;
    pool_mtx.lock();

    conn->update_last_used_time();
    connection_pool.push_back(conn);

    pool_mtx.unlock();
  }


  void perform_http_request(connection_data *conn, const std::string& buffer, void (*callback)(const std::string &result))
  {
    //TODO: aggiungere controllo al socket
    if(!conn)
    {
      clog::error("No connection provied! Can't make a http request");
      return;
    }
    //invia richiesta
    if( SSL_write(conn->connection, buffer.c_str(), buffer.size()) <= 0)
    {
      clog::error("Write error, aborting post");
      freeConnection(conn);
      return;
    }

    //leggi risposta
    char in_buffer[100];
    std::string responce;
    int len;
    do
    {
      len = SSL_read(conn->connection, in_buffer, 100);

      in_buffer[len] = '\0'; // assicurati che il buffer sia terminato sempre da uno \0
      if(len > 0)
      {
        responce += in_buffer;
      }

    } while (len == 100 && SSL_pending(conn->connection) != 0); // ripeti finche il buffer si riempie al 100% e che non ci sia nulla in attesa, altrimenti la connessione e' finita

    //chiama il callback con la risposta
    if(callback)
      callback(responce);

  }

  void perform_telegram_request(const std::string &url_path, std::vector<http_parameter> parameters)
  {
    //prepera la stringa per la richiesta
    std::string req = Bot::HTTP_Get_Template;
    req = Helper::StrReplace(req, "{host}", Bot::Telegram_Host);

    //prepara la path da usare nella richiesta http
    //TODO: url encode
    std::string path = Bot::Telegram_Base_Path;
    if(url_path != "") path += "/"+url_path;
    for(int i = 0 ; i < parameters.size(); i++)
    {
      if(parameters[i].name == "chat_id" && parameters[i].value == std::to_string(Bot::avoid_post_id))
      {
        log_parameters(parameters);
        return;
      }

      path += parameters[i].getFormattedParameter( i == 0 );
    }
    //imposta la path nella richiesta
    req = Helper::StrReplace(req,"{path}",path);

    connection_data *conn = get_connection_from_pool();

    //esegui il post alle api telegram
    perform_http_request(conn, req, &telegram_callback);
    //riaggiungi la connesione solo se valida
    if(conn)
      add_connection_to_pool(conn);
  }


  void telegram_callback(const std::string &result)
  {
    bool found = false;

    if(!found && result.find("\"ok\":true") != std::string::npos)
      { clog::log("Telegram API: ok");  found = true; }

    if(!found && result.find("\"ok\":false") != std::string::npos)
      {
        clog::error("Telegram API: error");
        //scrivi il json contenente l'errore
        clog::error( result.substr( result.find("{") -1 ) );
        found = true;
      }
    if(!found)
    {
      clog::error("Telegram API: unknown");
    }
  }


  void freeConnection(connection_data *conn)
  {
    if(!conn) return;

    SSL_shutdown(conn->connection);
    SSL_free (conn->connection);
    SSL_CTX_free(conn->context);
    close(conn->socket);

    delete conn;
  }


  std::string url_encode(const std::string &input)
  {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = input.begin(), n = input.end(); i != n; ++i)
    {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
  }


  std::string http_parameter::getFormattedParameter(bool first_element) const
  {
    if(first_element)
      return "?"+name+"="+url_encode(value);
    else
      return "&"+name+"="+url_encode(value);
  }



  void log_parameters(const std::vector<http_parameter> &parameters)
  {
    for(int i = 0; i < parameters.size(); i++)
    {
      clog::log(parameters[i].name+": "+parameters[i].value);
    }
  }


}
