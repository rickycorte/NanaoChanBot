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

/****************************************************
*
* Contiene tutte le funzioni per effettuare il parse di un anime ricevuto dal db
* e creare una risposta adattata a queste ultime
*
****************************************************/
#ifndef NANAO_BOT_ANIMERESULTPARSER_H
#define NANAO_BOT_ANIMERESULTPARSER_H

#include <string>
#include "rapidjson/document.h"

namespace AnimeResultParse
{

  //prepara il messaggio di risposta standard per una ricerca di anime
  std::string getAnimeResponceFromJson(const std::string& json);
  //prepara un blocco contentete una parte delle info dell'anime
  void fillStringWithJsonData(rapidjson::Document *doc, std::string& target, const std::string& searchTerm, void (*foundCallback)(std::string&) = nullptr);
  //controlla che il json abbia un parametro e che il suo valore non sia quello specificato
  bool jsonHadFieldAndIsNot(rapidjson::Document *doc, const std::string& search, const std::string& errorData);
  //recupera i valori di un array e inserisci in una stringa con l'apposito separatore
  std::string jsonTransformArray(rapidjson::Value *Dataptr, const std::string& separator, int maxItems = 0);

  const char *const AnimeInfoHeaderIT = "{title}:\ne' un{type}{status}{seasBox}.";
  const char *const AnimeInfoSeasonIT = " dalla season {season}";
  const char *const AnimeInfoVoteIT = "Ha una valutazione di {score} basata su {votes} voti.";
  const char *const AnimeInfoEpisodeIT = "E' composto da {episodes} episod{s} con la durata di {duration}";
  const char *const AnimeInfoMALIT = "Per piu info: https://myanimelist.net/anime/{malID}";
  const char *const AnimeGenresIT = "\nGeneri: {genres}.";
  const char *const AnimePegiIT = "\nRating: {rating}.";
  const char *const AnimePrequelIT = "\nPrequel: {prequel}.";
  const char *const AnimeSequelIT = "\nSequel: {sequel}.";
}

#endif //NANAO_BOT_ANIMERESULTPARSER_H
