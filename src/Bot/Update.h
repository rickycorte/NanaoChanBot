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

#ifndef __BOT_UPDATE__
#define __BOT_UPDATE__

#include <string>
#include <cstdint>
#include <vector>
#include <chrono>

#include "rapidjson/document.h"
#include "../Helper.h"

/**********************************************************
* Un singolo update inviato da telegram
***********************************************************/
class Update
{
public:
  //Blocca la possibilita' di copiare gli update
  Update(const Update& up)  = delete;

  //crea un update a partire da un json
  Update(const char *json, int threadID);

  ~Update();


  /*******************************************************************
  *
  * Informazioni update
  *
  ********************************************************************/

  //resitituisce true se e' una chat privata
  bool isPrivateChat() const { return update_type == "private"; }
  //resitituisce se e' un gruppo o super gruppo
  bool isGroupChat() const { return update_type == "group" || update_type == "supergroup"; }
  //restituisce se e' un canale
  bool isChannel() const { return update_type == "channel"; }


  //controlla che questo update provenga da me
  bool isFromDeveloper() const { return getSenderUsername() == "xdevily"; }

  //true se e' un messaggio contenente unicamente il nome del bot
  bool isMessageName() const
  {
    return message == "nanao" || message == "nanao," || message == "nanao." || message == "nanao?" || message == "nanao!" || message == "@nanaochanbot";
  }

  //resituisce se e' entrato una persona nel gruppo (bot compreso)
  bool GroupHasNewMember() { return group_member_join; };
  //restituisce se e' uscito qualcuno dal gruppo (bot compreso)
  bool GroupHasMemberQuit();

  //indica se l'update va processato o meno
  bool SkipTriggerSearch() const { return skip_trigger_search; }

  //true se questo e' un comando
  bool isCommand() const { return is_command; }

  bool isLongMessage() const {return is_longer_than_limit; }

  //e' una risposta a un messaggio di nanao?
  bool isReply() const { return is_reply; }

  //il messaggio contiene dei dati da salvare su file
  bool hasData() const { return contains_data; }

  //questo messaggio non contiene errori ed e' utilizzabile nel parse
  bool isOk() const { return !has_parse_errors; }

  //true se si e' superato il limite di messaggi
  bool isRateLimited() const;

  //true se questo messaggio e' il primo a eccedere il rate limit
  bool shouldSendRateLimitWarning() const;

  std::string getChatID() const { return std::to_string(chat_id);  }


  /*******************************************************************
  *
  * Funzioni per ottenere dati dall'update
  *
  ********************************************************************/

  //resitituisce il testo del messaggio contenuto in questo update
  std::string getMessage() const { return message; }

  //resitituisce il nome o l'username a seconda di cio' che e' disponibile
  std::string getSenderName() const { return (sender_name != "")? sender_name : sender_username; }
  //restituisce l'username se possibile altrimenti ""
  std::string getSenderUsername() const { return sender_username; }
  //recupera tutti gli username (@user) contenuti nel messaggio
  std::vector<std::string> getUsernamesInMessage(int startIndex = 0) const;


  //restituisce il tempo passato dalla creazione di questo oggetto in ms
  double getProcessingTimeSinceStart() const;
  //restituisce il valore di tempo impiegato sino ad arrivare al momento precedente al post della risposta
  double getRequestProcessingTime() const { return processing_time; }
  //restituisce il tempo impiegato ad effettuate il post della risposta
  double getPostTime() const { return post_time; }

  //tipi di dati recuperati dal messaggio
  enum dataType {Sticker, Photo, Audio};

  //restituisci il i dati trovati nel messaggio e il loro tipo
  Helper::parameterBlock *getFileData(dataType *dt);


  /*******************************************************************
  *
  * Funzioni per inviare una risposta
  *
  ********************************************************************/

  //invia un messaggio alla chat
  void SendMessage(const std::string& message);
  //invia un messaggio alla chat come replica al corrente
  void SendReply(const std::string& message);
  //manda una foto alla chat
  void SendPhoto(const std::string&  id);
  //manda uno sticker alla chat
  void SendSticker(const std::string&  id) const;
  //manda un audio alla chat
  void SendAudio(const std::string& id) const;


  /*******************************************************************
  *
  * Funzioni per elaborare il contenuto del messaggio dell'update
  *
  ********************************************************************/

  //spezza il messaggio in parole e elimina le versione normale per risparmiare ram
  void SplitAndRemoveMessage();

  //restituisce il numero di parole nel messaggio
  int wordCount() const { return message_words.size(); };

  //restituisci tutte le parole dopo un index
  std::string getWordsAfterIndex(int index) const;
  //restituisci tutte le parole dopo la parola specificata
  std::string getWordsAfterWord(const std::string& word);
  //restituisci tutte le parole dopo la prima parola trovata
  std::string getWordsAfterFirstMatch(const std::vector<std::string>& wordArray);


  //ottieni l'index della prima parola trovata nel messaggio
  int getWordIndex(const std::vector<std::string>& checkers);
  //controlla che nel messaggio ci sia una parola della lista di checkers
  bool hasWord(const std::vector<std::string>& checkers);
  //controlla che nel messaggio ci sia una parola di ogni lista specificata
  bool hasWordsInArrays(const std::vector<std::vector<std::string>>& checkers);

  //cancella il messaggio interno
  void RemoveMessage();

  //cancella una parola dalla lista
  //eliminata solo la prima parola trovata
  void RemoveWord(const std::string& w);

  class match
  {
    public:
      match(int ind, std::string *mathString): index{ind}
      {
        string_matched = mathString;
      }

      int getIndex() const { return index; }
      std::string *getWord() const { return string_matched; }

    private:
      int index;
      std::string *string_matched;

  };

  match *getTriggerMatchResult() const { return trigger_match; }

  /*******************************************************************
  *
  * Funzioni per ottenere dati dall'update
  *
  ********************************************************************/

  //logga tutte le informazioni che hai tranne il messaggio
  void LogUpdateInformations();

private:

  //spezza il messaggio in parole e rimuovi quelle superflue come gli articoli
  std::vector<std::string> splitMessageAndRemoveUselessParts();

  //cerca se nel messaggio e' presente un file e recupera id + tag per poterlo usare in seguito
  void SearchForFileIDAndTag(rapidjson::Document& doc);

  //controlla che il messaggo ricevuto sia un comando (es /start) o meno
  void SearchCommand();

  /*******************************************************************
  *
  * Dati
  *
  ********************************************************************/

  //username di chi ha inviato il messaggio (puo essere "")
  std::string sender_username;
  //nome di chi ha inviato il messaggio
  std::string sender_name;
  //id della chat
  std::int64_t chat_id;
  //tipo di chat da cui proviene il messaggio
  std::string update_type;
  //id del messaggio
  int message_id;
  //messaggio non modificato
  std::string message;
  //lista di parole che compongono il messaggio
  std::vector<std::string> message_words;
  //nome e username del gruppo o del canale da cui proviene il messaggio
  std::string chat_name;

  //questo messaggio deve essere processato o meno - e' un domando / - contiene dei dati tipo immagine
  //- supera il limite di caratteri consentito - il messaggio e' stato diviso in parole
  bool skip_trigger_search, is_command, contains_data, is_longer_than_limit, is_message_splitted;
  //ci sono stati errori nel parse iniziale del messaggio
  bool has_parse_errors;
  bool group_member_join ,group_member_quit;
  //questo messaggio e' una risposta a un messaggio del bot
  bool is_reply;

  //id thread che si becca il processing di questo update
  int worker_thread_id;

  //posizione dove e' stato trovato l'ultimo trigger
  match *trigger_match;

  //tempo processing
  double processing_time, post_time;

  //dati immagini/ecc solo chat privata devs
  Helper::parameterBlock *message_data;
  dataType message_data_type;

  std::chrono::high_resolution_clock::time_point processing_start_time;

};

#endif // __BOT_UPDATE__
