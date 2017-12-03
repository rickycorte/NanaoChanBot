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

#include "Update.h"

#include <cctype>
#include <sstream>
#include <cstring>

#include "rapidjson/pointer.h"

#include "../Defines.h"
#include "../Log.h"
#include "Stats.h"
#include "Connection.h"
#include "RateLimiter.h"


Update::Update(const char *json,int threadID) :
    skip_trigger_search{false}, is_command{false}, is_message_splitted{false}, contains_data{false}, is_longer_than_limit{false},
      has_parse_errors{false}, post_time{0}, processing_time{0}, group_member_join{false}, group_member_quit{false}, worker_thread_id{threadID},
      message_data{nullptr}, is_reply{false}, trigger_match{nullptr}
{
  processing_start_time = std::chrono::high_resolution_clock::now(); // recupera il tempo a cui inizia a processare
  rapidjson::Document doc;

  if(!json)
  {
    clog::error(worker_thread_id,"Invalid json!");
    has_parse_errors = true;
    return;
  }

  try
  {
    //parsa json con rapidjson
    doc.Parse(json);
    //controlla per errori del parse
    if(doc.HasParseError())
    {
      clog::error(worker_thread_id,"Input json has errors. Aborting parse.");
      clog::error(json);
      clog::error(Bot::StartUpHeaderLine);
      has_parse_errors = true;
      return;

    }

    rapidjson::Value *ptr = rapidjson::GetValueByPointer(doc,"/message");

    //recupera il contenuto utile al bot dal json
    if(ptr)
    {

      ptr = rapidjson::GetValueByPointer(doc,"/message/text");
      if(ptr)
      {
          message = ptr->GetString();
          //riduci il messaggio al limite massimo caratteri, gli altri vengono ignorati
          //serve per risparmiare risorse nei parse seguenti
          if(message.size() > Bot::maxMessageCharacters)
          {
            is_longer_than_limit = true;

            message = message.substr(0,Bot::maxMessageCharacters);
          }

          // to lower della stringa del messaggio
          Helper::tolower(message);
      }

      ptr = rapidjson::GetValueByPointer(doc,"/message/chat/id");
      if(ptr) chat_id = ptr->GetInt64();

      ptr = rapidjson::GetValueByPointer(doc,"/message/chat/type");
      if(ptr) update_type = ptr->GetString();

      ptr = rapidjson::GetValueByPointer(doc,"/message/message_id");
      if(ptr) message_id= ptr->GetInt();

      ptr = rapidjson::GetValueByPointer(doc,"/message/from/first_name");
      if(ptr) sender_name = ptr->GetString();

      ptr = rapidjson::GetValueByPointer(doc,"/message/from/username");
      if(ptr) sender_username = ptr->GetString();

      //recupera nome chat
      if(!isPrivateChat())
      {
        ptr = rapidjson::GetValueByPointer(doc,"/message/chat/title");
        if(ptr) chat_name = ptr->GetString();
        ptr = rapidjson::GetValueByPointer(doc,"/message/chat/username");
        if(ptr) { chat_name+=" @"; chat_name += ptr->GetString(); }

      }

      ptr = rapidjson::GetValueByPointer(doc,"/message/new_chat_member");
      if(ptr) group_member_join = true;

      ptr = rapidjson::GetValueByPointer(doc,"/message/left_chat_member");
      if(ptr) group_member_quit = true;

      ptr = rapidjson::GetValueByPointer(doc,"/message/reply_to_message/from/username");
      if(ptr)
      {
        is_reply = ( strcmp(ptr->GetString(), "NanaoChanBot") == 0 )? true : false;
      }

      //cerca eventuali comandi
      SearchCommand();

      //rimuovi ? alla fine
      int markPos = message.find_last_of("?");
      if(markPos != std::string::npos)
      {
        message.replace(markPos,1,"");
      }

      //cerca file e tag solo in messaggi che io invio al bot in chat privata
      if(isFromDeveloper() && isPrivateChat())
      {
          SearchForFileIDAndTag(doc);
      }

    }
    else
    {
      clog::error(worker_thread_id,"No message detected");
      has_parse_errors = true;
      return;
    }
  }
  catch(...)
  {
    clog::error(worker_thread_id,"Expection raised during json lookup. Invalid Update!");
    has_parse_errors = true;
  }

}

Update::~Update()
{

  if(message_data)
  {
    delete message_data;
  }

  if(trigger_match)
  {
    delete trigger_match;
  }

}


void Update::SendMessage(const std::string& message)
{
  processing_time = getProcessingTimeSinceStart();
  clog::log(worker_thread_id,"Processing before post: " + std::to_string(processing_time) +"ms");

  Connection::perform_telegram_request(Bot::sendMessageUrl,{ {"chat_id",std::to_string(chat_id)}, {"text",message} });
  //Helper::MakeTelegramPost(Bot::sendMessageUrl,{ {"chat_id",std::to_string(chat_id)}, {"text",message} });

  post_time = getProcessingTimeSinceStart() - processing_time;
  clog::log(worker_thread_id,"Sent message to: " + std::to_string(chat_id) + " (requester: "+getSenderName()+")" );
}


void Update::SendReply(const std::string& message)
{
  processing_time = getProcessingTimeSinceStart();
  clog::log(worker_thread_id,"Processing before post: " + std::to_string(processing_time) +"ms");

  Connection::perform_telegram_request(Bot::sendMessageUrl,{ {"chat_id",std::to_string(chat_id)}, {"text",message}, {"reply_to_message_id", std::to_string(message_id)} });

  post_time = getProcessingTimeSinceStart() - processing_time;
  clog::log(worker_thread_id,"Sent reply to: " + std::to_string(chat_id) + " (requester: "+getSenderName()+")" );
}


void Update::SendPhoto(const std::string&  id)
{
  processing_time = getProcessingTimeSinceStart();
  clog::log(worker_thread_id,"Processing before post: " + std::to_string(processing_time) +"ms");

  Connection::perform_telegram_request(Bot::sendPhotoUrl,{ {"chat_id",std::to_string(chat_id)}, {"photo",id} });

  post_time = getProcessingTimeSinceStart() - processing_time;
  clog::log(worker_thread_id,"Sent photo to: " + std::to_string(chat_id) + " (requester: "+getSenderName()+")" );
}


void Update::SendSticker(const std::string&  id) const
{
  Connection::perform_telegram_request(Bot::sendStickerUrl,{ {"chat_id",std::to_string(chat_id)}, {"sticker",id} });
  clog::log(worker_thread_id,"Sent sticker to: " + std::to_string(chat_id) + " (requester: "+getSenderName()+")" );
}

void Update::SendAudio(const std::string&  id) const
{
  Connection::perform_telegram_request(Bot::sendAudioUrl,{ {"chat_id",std::to_string(chat_id)}, {"audio",id} });
  clog::log(worker_thread_id,"Sent audio to: " + std::to_string(chat_id) + " (requester: "+getSenderName()+")" );
}


void Update::RemoveMessage()
{
  message.clear();
}


void Update::SearchForFileIDAndTag(rapidjson::Document& doc)
{
    static std::string fileID;
    static std::string tag;
    static dataType type;

    rapidjson::Value *ptr;

    bool found = (fileID == "")? false : true;

    //usa messaggio come tag se il precedente era un file
    if(found)
    {
      tag = getMessage();
    }


    clog::log(worker_thread_id,"Searching for file:");
    //cerca la prima foto
    if(!found)
    {
        ptr = rapidjson::GetValueByPointer(doc,"/message/photo");
        if(ptr)
        {

            type = dataType::Photo;

            fileID = (*ptr)[0]["file_id"].GetString();
            //prendi caption foto se presente
            ptr = rapidjson::GetValueByPointer(doc,"/message/caption");
            if(ptr)
              tag = ptr->GetString();

            found = true;
         }
    }
    //cerca sticker
    if(!found)
    {
        ptr = rapidjson::GetValueByPointer(doc,"/message/sticker/file_id");
        if(ptr)
        {
          type = dataType::Sticker;
          fileID = ptr->GetString();
          found = true;
        }
      }
    //cerca file audio
    if(!found)
    {
        ptr = rapidjson::GetValueByPointer(doc,"/message/audio/file_id");
        if(ptr)
        {
          type = dataType::Audio;
          fileID = ptr->GetString();
          found = true;
        }
    }

    //controlla se ha rovato un file e il tag interente, se manca chiedilo e attendi la risposta
    if(found)
    {
      skip_trigger_search = true;
      if(tag == "" || fileID == "")
      {
        SendReply("Non sono riuscita a trovare il tag, il prossimo messaggio sara' usato come tag");
      }
      else
      {
        message_data = new Helper::parameterBlock(tag,fileID);
        contains_data = true;
        message_data_type = type;

        //pulisci ram
        fileID = "";
        tag = "";
      }
    }
    else clog::log(worker_thread_id,"No file found");

}


void Update::SearchCommand()
{
  if(message.size() > 0 && message[0] == '/')
  {
      is_command = true;
      clog::log(worker_thread_id,"Detected command");
      return;
  }

  clog::log(worker_thread_id,"No command found");
}


std::vector<std::string> Update::splitMessageAndRemoveUselessParts()
{
  std::vector<std::string> res;
  std::stringstream ss(message);
  std::string word;
  while(ss>>word)
  {

    if(word == "nanao," || word == "nanao." || word == "nanao?" || word == "nanao!")
    {
      res.push_back("nanao");
      continue;
    }

    if(word != "di" && word != "a" && word != "da" && word != "in" && word != "con" && word != "su"
     && word != "per" && word != "tra" && word != "fra" && word != "il" && word != "lo" && word != "le"
     && word != "i" && word != "gli" && word != "dello" && word != "della" && word != "delle" && word != "degli" && word != "dei" && word != "del" && word != "un" && word != "una" )
     {
       res.push_back(word);
     }
  }

  return res;
}


void Update::SplitAndRemoveMessage()
{
  if(!is_message_splitted)
  {
    message_words = splitMessageAndRemoveUselessParts();
    RemoveMessage();
    is_message_splitted = true;
  }
}

int Update::getWordIndex(const std::vector<std::string>& checkers)
{
  //cerca l'index della prima corrispondenza
  for(int i =0; i< message_words.size(); i++)
  {
    for(int j = 0; j < checkers.size(); j++)
    {

      if(message_words[i] == checkers[j])
      {
        //evita un memory leack. Istanzia sempre una nuova classe con l'ultimo match riscontrato
        if(trigger_match) delete trigger_match;
        trigger_match = new match(i, &message_words[i]);

        return i;
      }

    }
  }

  return -1;
}


bool Update::hasWord(const std::vector<std::string>& checkers)
{
  return ( getWordIndex(checkers) == -1 )? false : true;
}


bool Update::hasWordsInArrays(const std::vector<std::vector<std::string>>& checkers)
{
  for(int i = 0; i < checkers.size(); i++ )
  {
    if(!hasWord(checkers[i])) //un array non ha trovato corrispondenze
      return false;
  }
  return true;
}


std::vector<std::string> Update::getUsernamesInMessage(int startIndex) const
{
  std::vector<std::string> res;
  for(int i = startIndex; i < message_words.size(); i++)
  {
     //cerca gli username
     if(message_words[i].find("@") != std::string::npos)
     {
       res.push_back(message_words[i]);
     }
  }

  return res;
}


//restituisci tutte le parole dopo un index
std::string Update::getWordsAfterIndex(int index) const
{
  std::string res;
  //recupera il nome dell'anime dal messaggio
  for(int i = index+1; i < message_words.size();i++)
  {
    res += message_words[i];
    if( i != message_words.size()-1)
      res+=" ";
  }

  return res;
}


//restituisci tutte le parole dopo la parola specificata
std::string Update::getWordsAfterWord(const std::string& word)
{
  return getWordsAfterIndex(getWordIndex( {word} ));
}


//restituisci tutte le parole dopo la prima parola trovata
std::string Update::getWordsAfterFirstMatch(const std::vector<std::string>& wordArray)
{
  return getWordsAfterIndex(getWordIndex( wordArray ));
}

void Update::RemoveWord(const std::string& w)
{
  int wIndex = 0;
  for(int i=0; i<message_words.size();i++)
  {
    if(message_words[i] == w)
    {
      wIndex = i;
      break;
    }
  }

  message_words.erase(message_words.begin() + wIndex);

}

Helper::parameterBlock *Update::getFileData(dataType *dt)
{
  if(dt != nullptr)
  {
    *dt = message_data_type;
  }
  return message_data;
}

double Update::getProcessingTimeSinceStart() const
{
  std::chrono::high_resolution_clock::time_point cTime =  std::chrono::high_resolution_clock::now();
  std::chrono::duration<double,std::milli> time_span = cTime - processing_start_time;
  return time_span.count();
}

void Update::LogUpdateInformations()
{
    clog::log(worker_thread_id,"Update info: sender: " + sender_name + " (@" + sender_username + ") chat: "
      + std::to_string(chat_id) + " ("+ chat_name +")" + " messageID: " + std::to_string(message_id) + " chatType: " + update_type
      + " isCommand: " + Helper::BoolToString(is_command) + " isLongerThanLimit: " + Helper::BoolToString(is_longer_than_limit)+ " isReply: "+ Helper::BoolToString(is_reply));
}


bool Update::isRateLimited() const
{
  return RateLimiter::isRateLimited(std::to_string(chat_id));
}


//true se questo messaggio e' il primo a eccedere il rate limit
bool Update::shouldSendRateLimitWarning() const
{
  return RateLimiter::getRateLimitCount(std::to_string(chat_id)) == Bot::rate_limit_message_count+1;
}
