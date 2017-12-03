
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

#include "AnimeResultParse.h"

#include "rapidjson/pointer.h"
#include "../Log.h"
#include "../Helper.h"
#include "../Defines.h"

//parsa il json e crea la risposta a una richiesta info anime
//refactor di sta immondizia
std::string AnimeResultParse::getAnimeResponceFromJson(const std::string& json)
{
  rapidjson::Document d;
  d.Parse(json.c_str());
  if(d.HasParseError())
  {
    clog::error("Input json has errors. Aborting parse.");
    clog::error(json);
    clog::error(Bot::StartUpHeaderLine);
    return "";
  }

  //prepara la risposta
  std::string resp = AnimeInfoHeaderIT;
  std::string temp;

  rapidjson::Value *ptr;

  //aggiunge titolo
  fillStringWithJsonData(&d,resp,"title",nullptr);

  //aggiungi il tipo se presente
  fillStringWithJsonData(&d,resp,"type",
        [](std::string& temp)
        {
          if(temp != "TV" && temp != "Movie" && temp != "Special") { temp = " "+temp; return;}
          if(temp == "TV") { temp = "a serie TV"; return;}
          if(temp == "Movie" ) { temp = " Film"; return;}
          if(temp == "Special") { temp = "o "+temp; return;}
        } );

  //aggiunti episodi e durata
  //non uso la funzione per fare il controllo perche' mi serve poi il valore del puntatore e mi evito di fare ricerche inutili
  ptr = rapidjson::GetValueByPointer(d,"/episodes");
  temp = ptr->GetString();
  if(ptr && temp != "Unknown")
  {
    resp += " ";
    resp += AnimeInfoEpisodeIT;
    fillStringWithJsonData(&d,resp,"episodes", nullptr);
    fillStringWithJsonData(&d, resp, "duration", nullptr);
    //decidi come terminare la scritta episod
    try
    {
      if(std::stoi(ptr->GetString()) > 1)
        resp = Helper::StrReplace(resp,"{s}", "i");
      else
        resp = Helper::StrReplace(resp,"{s}", "o");
    }catch(...)
    {
      clog::error("Errore: impossibile convertire dati in int");
      clog::error(ptr->GetString());
    }
  }

  //aggiungi score e voto
  if(jsonHadFieldAndIsNot(&d,"score","N/A"))
  {
    resp += " ";
    resp += AnimeInfoVoteIT;
    fillStringWithJsonData(&d,resp,"score",nullptr);
    fillStringWithJsonData(&d,resp,"votes",nullptr);
  }

    //status
    fillStringWithJsonData(&d, resp, "status", [](std::string& data) { data = " ("+data+")"; } );

    //season
    if(jsonHadFieldAndIsNot(&d, "season","null"))
    {
      resp = Helper::StrReplace(resp,"{seasBox}",AnimeInfoSeasonIT);
      fillStringWithJsonData(&d, resp, "season", nullptr);
    }
    else resp = Helper::StrReplace(resp,"{seasBox}", "");

    //pegi
    resp+=AnimePegiIT;
    fillStringWithJsonData(&d,resp,"rating",nullptr);

    //generi
    resp+=AnimeGenresIT;
    fillStringWithJsonData(&d,resp,"genres",nullptr);

    //lista prequel
    if(jsonHadFieldAndIsNot(&d, "prequel","null"))
    {
      resp+=AnimePrequelIT;
      fillStringWithJsonData(&d,resp,"prequel",nullptr);
    }

    //lista sequel
    if(jsonHadFieldAndIsNot(&d, "sequel","null"))
    {
      resp+=AnimeSequelIT;
      fillStringWithJsonData(&d,resp,"sequel",nullptr);
    }

  //aggiungi id mal
  resp += "\n";
  resp += AnimeInfoMALIT;
  fillStringWithJsonData(&d,resp,"malID",nullptr);

  return resp;
}

bool AnimeResultParse::jsonHadFieldAndIsNot(rapidjson::Document *doc, const std::string& search, const std::string& errorData)
{
  if(!doc) return false;
  char st[25];
  std::strcpy(st, ("/"+search).c_str() ) ;
  rapidjson::Value *ptr = rapidjson::GetValueByPointer(*doc, st );

  if(!ptr) return false;

  //controlli diversi per array e singolo elemento
  if(!ptr->IsArray())
  {
    std::string data = ptr->GetString();
    return ! (data == errorData);
  }
  else
  {
    //controlla che un elemento sia diverso dal dato di errore
    std::string data;
    for(rapidjson::SizeType i =0; i < ptr->Size();i++)
    {
      data = (*ptr)[i].GetString();
      if( data != errorData)
        return true;
    }
    return false;
  }

}

std::string AnimeResultParse::jsonTransformArray(rapidjson::Value *Dataptr, const std::string& separator, int maxItems)
{
  if(!Dataptr) return "";
  if(!Dataptr->IsArray()) return Dataptr->GetString();

  std::string resp, tmp;
  unsigned sz = Dataptr->Size();
  //imposta limite massimo di elementi letti
  if(maxItems != 0 && sz > maxItems ) sz = maxItems;
  //clog::log("sz: " +std::to_string(sz)+" dataSz: "+ std::to_string(Dataptr->Size()) + " maxItems "+ std::to_string(maxItems));
  for(rapidjson::SizeType i = 0; i < sz; i++)
  {
    //clog::log("Index "+std::to_string(i));
    tmp = (*Dataptr)[i].GetString();
    if(tmp == "null") continue; // dati invalidi
    //clog::log(tmp);

    resp += tmp;
    if(i != sz-1) resp += separator;
  }

  return resp;

}

void AnimeResultParse::fillStringWithJsonData(rapidjson::Document *doc, std::string& target, const std::string& searchTerm, void (*foundCallback)(std::string& data))
{
  if(!doc) return;
  std::string data;
  //converti la stringa in modo che rapidjson non caghi la minchia
  char st[25];
  std::strcpy(st, ("/"+searchTerm).c_str() ) ;
  rapidjson::Value *ptr = rapidjson::GetValueByPointer(*doc, st );

  if(ptr)
  {
    if(ptr->IsArray())
      data = jsonTransformArray(ptr,", ");
    else
      data = ptr->GetString();

    if(data != "null" && data != "N/A" && data != "Unknown")
    {
      if(foundCallback)
        foundCallback(data);
      target = Helper::StrReplace(target,"{"+searchTerm+"}", data);
    }
    else target = Helper::StrReplace(target,"{"+searchTerm+"}", "");

  }

}
