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

#include "DatabaseManager.h"
#include "Log.h"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

//apri una connessione con la collection anime
DatabaseManager::DatabaseManager()
{
  isRunning = false;
  clog::log("Connecting to database");
  client = new mongocxx::client({"mongo credentials"});
  if(!client)
  {
    clog::error("Unable to create database connection");
    return;
  }

  AnimeCollection = (*client)["Nanao"]["Anime"];

  clog::log("Database ready");
  isRunning = true;

}

DatabaseManager::~DatabaseManager()
{
  delete client;

  clog::log("Closing database connection");
}

std::string DatabaseManager::GetAnimeInfos(const std::string& title)
{
  if(!isRunning)
  {
    clog::error("Database not connected, rejected operation");
    return "null";
  }

  clog::log("Searching anime");
  try
  {
    //cerca per titolo (usa regex: titolo specificato.*)
    mongocxx::stdx::optional<bsoncxx::document::value> maybe_result = AnimeCollection.find_one(document{} << "$or" << open_array
      << open_document<< "title_low" << open_document << "$regex" << ("^" + title) << close_document <<close_document
      << open_document << "entitle_low" << open_document << "$regex" << ("^" + title) << close_document << close_document
      << close_array << finalize );

    //trovato un anime nel database
    if(maybe_result)
    {
      std::string lg = "Found entry for name: ";
      lg += title;
      clog::log(lg);

      //restituisci il risultato come json
      return bsoncxx::to_json(*maybe_result);
    }
    else
    {
      //nessun anime trovato
      std::string lg = "Unable to find: ";
      lg += title;
      clog::log(lg);

      return "null";
    }

  }
  catch(...) // errore connessione al database
  {
    return "error";
  }

}
