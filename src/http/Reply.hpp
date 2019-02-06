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


#ifndef HIKARIBACKEND_REPLY_HPP
#define HIKARIBACKEND_REPLY_HPP

#include <string>
#include <map>
#include <vector>

#define REPLY_HTTP_VERSION "1.1"

namespace RickyCorte
{
 namespace Http
 {
     /**
      * Represent a http reply to send
      */
     class Reply
     {
     public:

         typedef std::map<std::string, std::string> HeaderOptions;

         /**
          * Create a http reply
          *
          * @param code http code
          */
         Reply(int code);

         /**
          * Create a http reply
          *
          * @param code http code
          * @param body reply body
          */
         Reply(int code, const std::string& body);

         /**
          * Create a http reply
          *
          * @param code http code
          * @param body reply body
          * @param options header options
          */
         Reply(int code, const std::string& body, const HeaderOptions& options);

         /**
          * Set a header option
          * Note: headers like connection, lenght, server, date can't be set manually
          *
          * @param header option name
          * @param value option value
          */
         void SetHeader(const std::string& header, const std::string& value);

         /**
          * Set request body
          *
          * @param body
          */
         void SetBody(const std::string body);

         /**
          * Return ready to send http reply
          *
          * @return
          */
         std::string Dump();
     private:

         /**
          * Returnes the associated string to a http code
          * ex: 200 -> ok
          *
          * @param code http code to translate
          * @return code string
          */
         std::string translateHttpCode(int code);

         /**
          * Return current server time formatted as string
          *
          * @return
          */
         std::string getServerTime();


         /**
          * Internal header setter function that doesn't check for duplicates or whatever
          * Just replace or create
          *
          * @param header option name
          * @param value option value
          */
         void setHeaderI(const std::string& header, const std::string& value);


         int _code;
         HeaderOptions _options;
         std::string _body;

         const std::vector<std::string> LOCKED_HEADER_OPTIONS{"Date","Server","Connection","Content-Length"};
     };
 }

}

#endif //HIKARIBACKEND_REPLY_HPP
