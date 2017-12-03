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

#ifndef __DBMANAGER__
#define  __DBMANAGER__

#include <string>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

//classe che gestisce le connessioni e le query a mondodb
class DatabaseManager
{
  public:
    DatabaseManager();
    ~DatabaseManager();
    //recupoera il json di un anime presente del database.
    //query per titolo
    std::string GetAnimeInfos(const std::string& title);

  private:
    mongocxx::client *client;
    mongocxx::collection AnimeCollection;

    bool isRunning;

};

#endif //__DBMANAGER__
