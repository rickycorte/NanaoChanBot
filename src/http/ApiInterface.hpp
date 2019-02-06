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

#ifndef HIKARIBACKEND_APIINTERFACE_HPP
#define HIKARIBACKEND_APIINTERFACE_HPP


#include "Request.hpp"
#include "Reply.hpp"

namespace RickyCorte
{
namespace Http
{
    class ApiInterface
    {
    public:
        /**
         * GET callback
         * @param req Http request received
         * @return Reply to send
         */
        virtual Reply onGET(const Request &req)
        {
            return Reply(400, "{\"error\":\"GET not allowed\"}");
        }

        /**
         * POST callback
         * @param req Http request received
         * @return Reply to send
         */
        virtual Reply onPOST(const Request &req)
        {
            return Reply(400, "{\"error\":\"POST not allowed\"}");
        }

        /**
         * PUT callback
         * @param req Http request received
         * @return Reply to send
         */
        virtual Reply onPUT(const Request &req)
        {
            return Reply(400, "{\"error\":\"PUT not allowed\"}");
        }

        /**
         * DELETE callback
         * @param req Http request received
         * @return Reply to send
         */
        virtual Reply onDELETE(const Request &req)
        {
            return Reply(400, "{\"error\":\"DELETE not allowed\"}");
        }


        /**
         * Dispatch the request to the right handler
         * @param req
         * @return
         */
        Reply DispatchRequest(const Request &req)
        {
            if(!req.IsValid()) return Reply(400, "Bad Request");
            switch(req.GetType())
            {
                case Request::RequestType::GET:
                    return onGET(req);

                case Request::RequestType::POST:
                    return onPOST(req);

                case Request::RequestType::PUT:
                    return onPUT(req);

                case Request::RequestType::DELETE:
                    return onDELETE(req);
            }
        }


    };
}
}

#endif //HIKARIBACKEND_APIINTERFACE_HPP
