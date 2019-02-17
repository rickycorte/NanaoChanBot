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

#ifndef HIKARIBACKEND_REQUEST_HPP
#define HIKARIBACKEND_REQUEST_HPP

#include <string>
#include <map>

namespace RickyCorte
{
namespace Http
{
    /**
     * An incoming http request
     */
    class Request
    {
    public:

        typedef std::map<std::string, char *> HeaderOptions;

        Request(const char *req_string, size_t len);

        /**
         * Request types supported
         */
        enum class RequestType {GET = 1, POST, PUT, DELETE }; /* we don't support all methods right now */

        /**
         * HTTP parser errors
         */
        enum HttpErrors {
            HTTP_BUFFER_ERROR = -2,
            HTTP_UNPARSED = -1,
            HTTP_BROKEN_HEADER = 1,
            HTTP_BROKEN_OPTION,
            HTTP_FORMAT_ERROR,
            HTTP_UNEXPECTED_TOKEN,
            HTTP_METHOD_NOT_SUPPORTED,
            HTTP_UNEXPECTED_PAYLOAD,
            HTTP_NOT_SUPPORTED_VERSION
        };

        /**
         * Returns true if the request is valid
         * @return
         */
        bool IsValid() const;

        /**
         * Returns parse error code
         * 0 with no error
         * @return
         */
        int GetErrorCode() const;

        /**
         * Returns request type
         * eg: POST
         * @return
         */
        RequestType GetType() const;

        /**
         * Return url path
         * example: www.test.com/home returns /home
         * @return
         */
        std::string GetPath() const;

        /**
         * Return header option value if present
         * If missing return ""
         *
         * @param option option name
         * @return
         */
        std::string GetHeader(const std::string& option) const;

        /**
         * Return the whole header option map
         * @return
         */
        const HeaderOptions &GetHeaderOptions() const;

        /**
         * Return request body if any
         *
         * @return
         */
        std::string GetBody() const;

        virtual ~Request();

        /**
         * Translates error code to a human readable error
         * @param error_code
         * @return
         */
        static std::string ErrorCodeToString(int error_code);

        std::string getErrorMessage() const { return  error_message; }

    private:
        /**
         * Parse first line of http header
         *
         * @param line_start
         * @return
         */
        inline bool parse_first_header_line(char *line_start);

        /**
         * Clean ALL resources used by the class!
         */
        inline void clean_up();

        char *_request_string;
        size_t _request_size;

        char *_path;
        RequestType _req_type;
        HeaderOptions _header_options;
        char *_body;

        int _error_code;

        std::string error_message;
    };

}
}

#endif //HIKARIBACKEND_REQUEST_HPP
