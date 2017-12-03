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

#include "DataMap.h"
#include "Log.h"

#include <fstream>
#include <iostream>
#include <cstdio>

DataMap::DataMap(const std::string& fileToUse)
{
  //ricordati che file hai aperto
  fileName = fileToUse;
  //apri il file
  std::ifstream in(fileToUse);
  if(in.fail())
  {
    clog::error("Unable to load lib:");
    clog::error(fileToUse);
    in.close();
    return;
  }

  //dati interni da printare per capire quante cose sono state lette
  int mapKeys = 0;
  int TotalTokens = 0;

  std::string line;
  while(getline(in,line))
  {
    if(line.size() < 5) continue; // la riga e' troppo corta per poter essere letta

    size_t separatorPos = line.find(" | ");
    if(separatorPos == std::string::npos || line == "\n") continue; // errore riga non valida

    std::string tag = line.substr(0, separatorPos); //ottieni il tag prima del separatore
    std::string token = line.erase(0, separatorPos + 3);  //ottieni i dati dopo il separatore

    //crea il nuovo vettore da mappare se non esiste
    if(!hasKey(tag))
    {
      CreateMapKey(tag,token); // inserici un vettore associato al tag letto nella map
      mapKeys++;
      TotalTokens++;
    }
    else // aggiungi elemento al vettore corrispondente che e' gia presente nella map
    {
      data[tag].push_back(token);
      TotalTokens++;
    }
  }
  in.close();

  clog::log("DataMap ("+fileName+"): read "+std::to_string(mapKeys)+" keys with a total of "+std::to_string(TotalTokens)+" items");

  UpdateMapKeyList();
}

bool DataMap::hasKey(const std::string& key) const
{
  return data.find(key) != data.end();
}


void DataMap::DebugWriteAllData() const
{
  for(auto it = data.cbegin(); it != data.cend();it++)
  {
    std::cout<<it->first<<": "<<std::endl;
    for(int j = 0; j < it->second.size();j++)
    {
      std::cout<<it->second[j]<<" | ";
    }

    std::cout<<std::endl<<std::endl;
  }
}


const std::string& DataMap::getDataOfKey(const std::string& key, int index)
{
  if(hasKey(key))
  {
    if(index < 0) // index sbagliato quindi randomizza
      index = rand() % data[key].size();

    if(index >= data[key].size()) // out of range
      return nullData;

    return data[key][index];

  }
  else
    return nullData;
}


void DataMap::CreateMapKey(const std::string& key, const std::string& token)
{
  data.insert(std::pair<std::string, std::vector<std::string> >(key,{token}));
}


void DataMap::AddDataToMapKey(const std::string& key, const std::string& token)
{
  if(hasKey(key))
    data[key].push_back(token);
  else
    CreateMapKey(key,token);
}


bool DataMap::add(const std::string& key, const std::string& token)
{
  if(hasKey(key))
  {
    //controlla duplicati
    for(int i = 0;i < data[key].size();i++)
    {
      if(data[key][i] == token) // duplicato trovato
        {
          clog::warn("Duplicate found for: ["+key+":"+token+"], ignoring it");
          return false;
        }
    }

  }


  // se arrivi qui vuol dire che non hai trovato duplicati
  AddDataToMapKey(key, token);
  //scrivi la modifica nel file
  std::ofstream os(fileName,std::ofstream::app);
  if(os.fail())
  {
    clog::error("Unable update lib:");
    clog::error(fileName);
    os.close();
    return false;
  }

  os<<key<<" | "<<token<<std::endl;
  os.close();
  return true;
}


bool DataMap::remove(const std::string& key)
{
  if(hasKey(key))
  {
    data.erase(key);
    return WriteDataToFile();
  }

  return false;
}


bool DataMap::remove(const std::string& key,const std::string& token)
{
  if(hasKey(key))
  {
    int pos = -1;
    for(int i =0;i<data[key].size();i++)
    {
      if(data[key][i] == token)
      {
        pos = i;
        break;
      }
    }

    if(pos >= 0 ) //trovato un elemento
     {
        data[key].erase(data[key].begin()+pos);
        return WriteDataToFile();
     }
  }

  return false;
}


bool DataMap::WriteDataToFile()
{

  //scrivi un file temporaneo con tutti i dati aggiornati
  std::ofstream of(fileName+"_wrt",std::ofstream::out | std::ofstream::trunc);// apri il file per la scrittura e cancella tutto quello che e' gia presente
  if(of.fail())
  {
    clog::error("Unable to update lib:");
    clog::error(fileName+"_wrt");
    of.close();
    return false;
  }

  for(auto it = data.cbegin(); it != data.cend();it++)
  {
    for(int j = 0; j < it->second.size();j++)
    {
      of<<it->first<<" | "<<it->second[j]<<std::endl;
    }
  }
  of.close();

  //cancella file attuale
  if(std::remove(fileName.c_str()) != 0)
    clog::error("Unable to delete: "+ fileName);
  //cambia il nome del file appena scritto in quello originale cosi da essere usato al prossimo avvio
  if(std::rename( (fileName +"_wrt").c_str() ,fileName.c_str()) != 0)
    clog::error("Unable to rename: "+ fileName+"_fwd");

  return true;
}

void DataMap::UpdateMapKeyList()
{
  mapKeys = "";
  auto end = data.end();
  end--; // ultimo el

  for(auto it = data.cbegin(); it != data.cend();it++)
  {
    mapKeys += it->first;
    if(it !=  end ) // non e' l'ultimo elemento
      mapKeys += ", ";
  }
  clog::log("Updated key list");
}

//restituisci un puntatore alla lista di chiavi come stringa intera
const std::string &DataMap::getMapKeys()
{
  return mapKeys;
}
