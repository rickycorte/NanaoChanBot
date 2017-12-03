
###################################################################################
#
#    NanaoChanBot
#    Copyright (C) 2017  RickyCorte
#    https://github.com/rickycorte
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
###################################################################################

from pymongo import MongoClient
import urllib
import clog

#crea connessione al database
client = MongoClient("mongodb credentials");
AnimeDB = client.Nanao.Anime;

def addAnimeToDB(anime):
    AnimeDB.update({ "malID" : anime["malID"] },anime, True)
    clog.log("Added/Updated "+str(anime.get("title"))+" to database")
