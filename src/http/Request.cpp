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

#include "Request.hpp"

#include <cstring>

namespace RickyCorte::Http
{


    Request::Request(const char *req_string, size_t len)
            : _request_string{nullptr},_request_size{len + 1}, _path{nullptr},_body{nullptr}, _error_code{HTTP_UNPARSED}
    {
        _request_string = new char[_request_size];
        if(!_request_string)
        {
            _error_code = HTTP_BUFFER_ERROR;
            return;
        }
        memcpy(_request_string, req_string, len);
        _request_string[len] = '\0';                        // add null terminator to string


        int line_number = 0;
        char *previous_line_start = _request_string;
        int last_two_dots = -1;

        for(size_t i = 0; i < _request_size; i++)
        {
            if(_request_string[i] == '\0')                  // unexpected end of line in the middle of the request
            {
                _error_code = HTTP_UNEXPECTED_TOKEN;
                clean_up();
                return;
            }

            // remember last ':', we need this to keep the options parse fast
            // we take a bit more time to prevent false positives like "Host: localhost::8080"
            if(_request_string[i] == ':' && _request_size >  i+1 && _request_string[i+1] == ' ')
                last_two_dots = i;

            // we found a new line that could be followed by another header or by header terminators

            if(_request_string[i] == '\n')
            {
                // we check that the previous item is a \r because we replaced it earlier
                // we also check for range just to make sure we don't mess up
                if(i + 2 > _request_size || i < 1 || _request_string[i - 1] != '\r')
                {
                    _error_code = HTTP_FORMAT_ERROR;
                    clean_up();
                    return;
                }

                _request_string[i-1] = '\0';                // replace \r with \0

                // chose parser based on line number
                if(line_number == 0)
                {
                    // check that the first line of header is valid
                    if (!parse_first_header_line(previous_line_start))
                    {
                        // the above function sets error code by itself
                        clean_up();
                        return;
                    }
                }
                else
                {
                    // check if ':' is in the current line
                    if(last_two_dots >= 0 && previous_line_start - _request_string < last_two_dots)
                    {
                        _request_string[last_two_dots] = '\0'; // add terminator to split option name and value
                        // add header option to map
                        _header_options[std::string(previous_line_start)] = _request_string + last_two_dots + 2; // skip space after :
                    }
                    else
                    {
                        _error_code = HTTP_BROKEN_OPTION;
                        clean_up();
                        return;                        // broken header option
                    }
                }
                line_number++;
                previous_line_start = _request_string + i + 1;


                if(_request_string[i+2] == '\n')       // we found body because we have a double new line terminator!
                {
                    if(i + 3 < _request_size)               // range check before doing a dangerous operation
                        _body = _request_string + i + 3;    // (+3 -> 2 terminators and 1 body element)
                    break;                                  // stop parse, we found everything
                }

            }
        }

        if(_req_type == RequestType::GET)
        {
            if(!_body && strcmp(_body,"") != 0) // GET operations are not supposed to have a payload
            {
                _error_code = HTTP_UNEXPECTED_PAYLOAD;
                return;
            }
        }

        _error_code = 0;
    }

    bool Request::IsValid() const
    {
        return _error_code == 0;
    }

    Request::RequestType Request::GetType() const
    {
        return _req_type;
    }

    std::string Request::GetPath() const
    {
        if(!_path) return "";

        return std::string(_path);
    }

    std::string Request::GetHeader(const std::string &option) const
    {
        if(_header_options.find(option) != _header_options.end())
        {
            return std::string(_header_options.at(option));
        }

        return "";
    }


    std::string Request::GetBody() const
    {
        if(!_body) return "";

        return std::string(_body);
    }

    Request::~Request()
    {
       clean_up();
    }

    bool Request::parse_first_header_line(char *line_start)
    {
        if(!line_start)
        {
            _error_code = HTTP_UNPARSED;
            return false;
        }

        //RC_DEBUG("Line 1: ", line_start);
        char *protocol = nullptr;
        size_t len = strlen(line_start);

        size_t space_count = 0;
        for(size_t i = 0; i < len; i++)
        {
            if(line_start[i] == ' ')
            {
                space_count++;
                line_start[i] = '\0';       // replace space with terminator so we can grab individual parts
                if(space_count == 1)
                    _path = line_start + i + 1;
                if(space_count == 2)
                    protocol = line_start + i + 1;
                // the first line can't contain more than 3 spaces
                if(space_count > 2 )
                {
                    _error_code = HTTP_BROKEN_HEADER;
                    return false;
                }
            }
        }

        if(!_path || !protocol)
        {
            _error_code = HTTP_BROKEN_HEADER;
            return false;
        }

        // we only support http 1.1
        if(strcmp(protocol, "HTTP/1.1") != 0)
        {
            _error_code = HTTP_NOT_SUPPORTED_VERSION;
            return false;
        }

        if(strcmp(line_start, "GET") == 0) _req_type = RequestType::GET;
        else if(strcmp(line_start, "POST") == 0) _req_type = RequestType::POST;
        else if(strcmp(line_start, "PUT") == 0) _req_type = RequestType::PUT;
        else if(strcmp(line_start, "DELETE") == 0) _req_type = RequestType::DELETE;
        else
        {
            _error_code = HTTP_METHOD_NOT_SUPPORTED;
            return false; // broken protocol
        }

        return true;
    }


    const Request::HeaderOptions &Request::GetHeaderOptions() const
    {
        return _header_options;
    }

    void Request::clean_up()
    {
        if(_request_string)
        {
            delete[] _request_string;
            _request_string = nullptr;
        }
        _request_size = 0;
        _path = nullptr;
        _header_options.clear();
        _body = nullptr;
    }

    int Request::GetErrorCode() const
    {
        return _error_code;
    }

    std::string Request::ErrorCodeToString(int error_code)
    {
        std::string msg = "";
        switch (error_code)
        {
            case 0:
                break;
            case HTTP_BUFFER_ERROR:
                // NEVER GIVE the client sensible information about server resources
                msg = "Sorry, we can't handle your request right now!";
                break;
            case HTTP_UNPARSED:
                msg = "Something when wrong parsing your request";
                break;
            case HTTP_BROKEN_HEADER:
                msg = "Header error";
                break;
            case HTTP_BROKEN_OPTION:
                msg = "Options error";
                break;
            case HTTP_FORMAT_ERROR:
                msg = "Formatting error";
                break;
            case HTTP_UNEXPECTED_TOKEN:
                msg = "Unexpected token found in header";
                break;
            case HTTP_METHOD_NOT_SUPPORTED:
                msg = "Method not supported";
                break;
            case HTTP_UNEXPECTED_PAYLOAD:
                msg = "Unexpected payload, check your method";
                break;
            case HTTP_NOT_SUPPORTED_VERSION:
                msg = "Http version version not supported. Please use version 1.1";
                break;
            default:
                msg = "Error code "+ std::to_string(error_code) + " is not defined";
                break;
        }
        return msg;
    }


}