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


#include "Reply.hpp"

namespace RickyCorte::Http
{

    Reply::Reply(int code)
    {
        _code = code;
    }

    Reply::Reply(int code, const std::string &body)
    {
        _code = code;
        _body = body;
    }

    Reply::Reply(int code, const std::string &body, const Reply::HeaderOptions& options)
    {
        _code = code;
        _body = body;
        _options = options;
    }

    void Reply::SetHeader(const std::string &header, const std::string &value)
    {
        for(auto i =0; i < LOCKED_HEADER_OPTIONS.size(); i++)
            if(header == LOCKED_HEADER_OPTIONS[i])
                return;

        setHeaderI(header, value);
    }

    void Reply::SetBody(const std::string body)
    {
        _body = body;
    }

    std::string Reply::Dump()
    {
        // add forced headers
        setHeaderI("Server", "Hikari Backend");
        setHeaderI("Date", getServerTime());
        setHeaderI("Connection", "Keep-Alive");
        setHeaderI("Content-Length", std::to_string(_body.length()));

        std::string reply = "HTTP/";
        reply += REPLY_HTTP_VERSION;
        reply += " " + std::to_string(_code) + " " + translateHttpCode(_code) + "\r\n";
        for(auto itr = _options.begin(); itr != _options.end(); itr++)
        {
            reply += itr->first + ": " + itr->second + "\r\n";
        }
        reply += "\r\n";
        reply += _body;


        return reply;
    }

    std::string Reply::translateHttpCode(int code)
    {
        std::string text = "";
        switch(code)
        {
            case 100:
                text = "Continue";
                break;
            case 101:
                text = "Switching Protocol";
                break;
            case 200:
                text = "OK";
                break;
            case 204:
                text = "No Content";
                break;
            case 301:
                text = "Moved Permanently";
                break;
            case 302:
                text = "Found";
                break;
            case 400:
                text = "Bad Request";
                break;
            case 401:
                text = "Unauthorized";
                break;
            case 403:
                text = "Forbidden";
                break;
            case 404:
                text = "Not Found";
                break;
            case 405:
                text = "Method Not Allowed";
                break;
            case 408:
                text = "Request Timeout";
                break;
            case 411:
                text = "Length Required";
                break;
            case 413:
                text = "Payload Too Large";
                break;
            case 429:
                text = "Too Many Requests";
                break;
            case 500:
                text = "Internal Server Error";
                break;
            case 501:
                text = "Not Implemented";
                break;
            case 502:
                text = "Bad Gateway";
                break;
            case 503:
                text = "Service Unavailable";
                break;
            case 505:
                text = "HTTP Version Not Supported";
                break;

            default:
                text = "-1";
        }

        return text;
    }

    std::string Reply::getServerTime()
    {
        char dt[50];
        time_t t = time(nullptr);
        strftime(dt,50,"%a, %d %b %Y %H:%M:%S %Z", gmtime(&t));

        return dt;
    }

    void Reply::setHeaderI(const std::string &header, const std::string &value)
    {
        _options[header] = value;
    }
}