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

#ifndef __DATAMAP__
#define __DATAMAP__
#include <map>
#include <string>
#include <vector>

//libreria contente dati sottoforma di std::map <tag, data>
class DataMap
{
public:
  //inizializza la classe coi dati contenuti nel file specificato
  DataMap(const std::string& fileToUse);
  //controlla che sia prensente la key specificata tra i dati
  bool hasKey(const std::string& key) const;
  //resitituisce un puntatore ai dati corrispondenti alla key
  //index -1 = random
  const std::string& getDataOfKey(const std::string& key, int index = -1);
  //aggiungi un elemento ai dati e salvalo anche sul file corrispondente, restituisce se l'operazione e' andata a buon fine o meno
  //ignora i dati duplicati
  bool add(const std::string& key, const std::string& token);
  //rimuovi tutti gli elmenti di un tag dai dati in ram e dal file, restituisce se l'operazione e' andata a buon fine o meno
  bool remove(const std::string& key);
  //rimuovi un elemento dai dati in ram e dal file, restituisce se l'operazione e' andata a buon fine o meno
  bool remove(const std::string& key,const std::string& token);

  void DebugWriteAllData() const;

  //aggiorna il vettore che contiene i nomi delle key della map
  void UpdateMapKeyList();
  //restituisci un puntatore alla lista di chiavi che non deve essere modificata o cancellata
  const std::string &getMapKeys();

private:

  //scrivi tutti i dati nella map sul file sovrascrivendo il suo contenuto
  bool WriteDataToFile();
  //crea un nuovo elemento della map e inserisci un vettore con l'elemento token
  void CreateMapKey(const std::string& key, const std::string& token);
  //aggiunti l'elemento token al vettore corrispondente a key
  void AddDataToMapKey(const std::string& key, const std::string& token);

  std::map<std::string, std::vector<std::string>> data;
  std::string fileName;
  std::string nullData;

  std::string mapKeys;

};

#endif
