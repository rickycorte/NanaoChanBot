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

#ifndef HIKARIBACKEND_ECHOAPI_HPP
#define HIKARIBACKEND_ECHOAPI_HPP

#include "http/ApiInterface.hpp"

namespace RickyCorte
{

    class EchoApi : public Http::ApiInterface
    {
    public:

        Http::Reply onGET(const Http::Request &req) override
        {

            std::string headers = std::to_string((int)req.GetType()) + " at " + req.GetPath() + "\nYour Headers:\n";
            for (auto itr = req.GetHeaderOptions().begin(); itr != req.GetHeaderOptions().end(); itr++)
            {
                headers += "\t" + itr->first + " -> " + itr->second + "\n";
            }

            return Http::Reply(200, headers + "Your body:\n!BEGIN!\n" + req.GetBody() + "\n!END!");
        }

    };
}


#endif //HIKARIBACKEND_ECHOAPI_HPP
