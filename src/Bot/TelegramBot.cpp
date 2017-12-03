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

#include "TelegramBot.h"

#include <time.h>
#include <stdlib.h>
#include <mutex>
#include <chrono>
#include <unistd.h> // sleep func
#include <cstring>

#include "../Log.h"
#include "../Defines.h"
#include "AnimeResultParse.h"
#include "RateLimiter.h"


TelegramBot::TelegramBot(): closeThreads{false}
{
  closeThreads = false;
  srand(time(nullptr));

  clog::log("Loading data:");
  languageIT = new DataMap("language/it.lan");
  clog::log("languageIT: OK");
  PreferencesIT = new DataMap("Data/preferencesIT.txt");
  clog::log("preferencesIT: OK");
  pictures = new DataMap("Data/photos.txt");
  clog::log("pictures: OK");

  database = new DatabaseManager();

  //avvia tutti i thread
  clog::log("Starting workes");
  for(int i =0; i < Bot::maxConcurrentRequestHandlerThreads;i++)
  {
    std::thread *t = new std::thread(&TelegramBot::WorkerThreadMain, this, i); // avvia il thread
    if(!t)
    {
      clog::error("Error allocating thread!"); continue;
    }
    t->detach();
    workers.push_back(t);
  }
    clog::log("Workers: OK");

}


TelegramBot::~TelegramBot()
{
  delete database;
  delete PreferencesIT;
  delete pictures;
  delete languageIT;

  closeThreads = true;
  for(int i =0;i <workers.size();i++)
  {
    workers[i]->join();
    delete workers[i];
  }

}


void TelegramBot::AddRequestToQueue(const char *reqBody)
{
  if(reqBody == nullptr || strlen(reqBody) == 0)
  {
    clog::error("Recived null request");
    return;
  }

  //alloca array lungo quanto la richiesta ricevuta
  char *req = new char[strlen(reqBody)+1]();
  if(req == nullptr)
  {
        clog::error("Unable to allocate space to store request. This request will be ignored.");
        return;
  }

  strcpy(req,reqBody); // copia la stringa ricevuta
  queue_mtx.lock();
  requestsQueue.push_back(req); // aggiungi la stringa in coda (thread safe)
  queue_mtx.unlock();

  clog::log("Added request to queue. New queue size: "+std::to_string(requestsQueue.size()));
}

void TelegramBot::WorkerThreadMain(int threadID)
{
  //clog::log("Thread "+std::to_string(threadID)+" ready!");
  //attendi finche non ci sono dei dati da elaborare
  while(!closeThreads)
  {
    if(requestsQueue.size()  <= 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(Bot::WorkerThreadUpdateDelay));

    //cerca di processare una richiesta se possibile
    char *req = TgetUpdateToProcess();
    if(req)
    {
      //clog::log(threadID,"Thread "+std::to_string(threadID)+": processing started, new queue size: "+std::to_string(requestsQueue.size()));
      ThreadHandleRequest(req,threadID);
      //clog::log(threadID,"Thread "+std::to_string(threadID)+": processing ended");
    }

  }

}

//thread safe, recupera il primo update da processare
char *TelegramBot::TgetUpdateToProcess()
{
  queue_mtx.lock();

  char *up = nullptr;

  if(requestsQueue.size() > 0 )
  {
    up = requestsQueue[0]; // recupera il primo elemento
    requestsQueue.erase(requestsQueue.begin()); //rimuovi il primo elemento
  }

  queue_mtx.unlock();
  return up;
}


//wreapper per handle request che cancella la richiesta e fa partire un nuovo thread per processare un nuovo elemento
void TelegramBot::ThreadHandleRequest(char *req, int tid)
{
  if(!req)
  {
    clog::error(tid,"ThreadHandleRequest: request is null, aborting");
  }

  Update *up = new Update(Helper::getJson(req).c_str(),tid);
  if(req) delete[] req; // libera ram occupata dalla richiesta
  else clog::error("Request was null at delete time");

  //impossibile allocare update
  if(!up)
  {
    clog::error(tid,"Unable to create new Update");
    return;
  }

  //l'update contiene errori ed e' da scartare
  if(!up->isOk())
  {
    clog::error("Broken update: deleting it");
    delete up;
    return;
  }

  bool isBotRequest = false;

  RateLimiter::UpdateChatStatus(up->getChatID());

  //controllo superameto limite rate
  if(!up->isRateLimited())
  {
    HandleRequest(up, tid ,&isBotRequest);
  }

  double reqTime = up->getProcessingTimeSinceStart();

  if(!isBotRequest)
  {
    Stats::getInstance()->AddMessageProcessingTime(reqTime);
  }
  else
  {
    Stats::getInstance()->AddRequestProcessingTime(up->getRequestProcessingTime(),up->getPostTime());
  }

  clog::log(tid,"Processed in: "+std::to_string(reqTime)+"ms");
  delete up;
}


void TelegramBot::HandleRequest(Update *up, int tid, bool *handled)
{
  if(!up || !handled)
  {
    clog::error(tid,"HandleRequest: Update is null, aborting");
  }

  *handled = true;

  if(up->SkipTriggerSearch())
  {
    if(up->hasData())
    {
      ProcessMessageData(up,tid);
    }

    clog::log(tid,"Processing skipped");
    return;
  }

  //scrivi info update nei log
  up->LogUpdateInformations();

  if(up->isCommand())
  {
    *handled = HandleCommand(up,tid);
    return;
  }

  if(up->getMessage().find("skynet") != std::string::npos)
  {
    HandleSN_EasterEgg(up,tid);
    return;
  }

  if(up->GroupHasNewMember())
  {
    HandleHelloMessage(up,tid);
  }

  if(up->isPrivateChat() || up->isReply() || up->getMessage().find("nanao") != std::string::npos)
  {
    //valid message
    if(up->isMessageName())
    {
        HandleNameMessage(up,tid);
        return;
    }

    up->SplitAndRemoveMessage();

    //nanao e' stato ficcato dentro in qualche parola e quindi non e' accettabile
    if( !up->isReply() && up->isGroupChat() && !up->hasWord({"nanao", Bot::TelegramNameLower}))
    {
      clog::log(tid,"Message not reguarding this bot.");
      return;
    }

    //troppe parole in questo messaggio
    if(up->wordCount() > Bot::MaxRequestWords)
    {
      HandleGenericMessageLib(up,"msgLungo","Long message",tid,responceType::Reply);
      return;
    }

    SearchHandlerInMessage(up, tid);

  }
  else
  {
    clog::log(tid,"Not a bot message");
    *handled = false; // informa il chiamate che questo non era un messaggio diretto al bot
  }

}

void TelegramBot::ProcessMessageData(Update *up, int tid)
{
  Update::dataType dt;
  Helper::parameterBlock *data = up->getFileData(&dt);
  if(!data) return;

  bool added = false;

  if(dt == Update::dataType::Photo)
  {
    added = pictures->add(data->getParameter(),data->getValue());
    clog::log(tid,"Added picture to data");
    pictures->UpdateMapKeyList();
  }
  else
  {
    clog::warn(tid,"Not a photo!");
  }

  if(added)
  {
    up->SendMessage("Aggiunto "+ data->getValue() +" in categoria: "+data->getParameter());
  }
  else
  {
    up->SendMessage("Unable to add item to db");
  }

}


void TelegramBot::SearchHandlerInMessage(Update *up,int tid)
{

  if(up->hasWord({"cazzo","culo","troia","suca","pompino","pene","vagina","figa","merda","zoccola","puttana","vaffanculo",
    "fottiti","fanculo","cacca","fuck","pussy","ass","idiota","cogliona","coglione","cazzona","puttanella","stronza","mignotta",
    "succhiami", "succhiamelo","sucamelo", "sborro","sborra","sperma","coglioncella","fighetta","fottere", "fascista","nazzista", "sega", "raspone", "inculo"}))
  {
    HandleGenericMessage(up,VolgarityMessageIT,"Vulgar words",tid,responceType::Reply,false);
    return;
  }


  if(up->hasWord({"uccido","muori","ucciditi","suicidati","cancro","accoltello","stupro","killo","picchio","malmeno","violento","svergino","stupro"}))
  {
    HandleGenericMessageLib(up,"insulto","Kill threat",tid,responceType::Message,true);
    return;
  }


  if(up->hasWord({"non"}))
  {
    if(!up->hasWord({"info","cerca"}))
      {
        HandleGenericMessage(up,MessageWithNegationIT,"Negative request",tid,responceType::Reply);
        return;
      }
  }

  //ricerca trigger
  if(up->hasWord({"ciao","hey","we","oi","giorno","sera", "buongiorno", "buonpomeriggo", "buonasera"}))
  {
    HandleHelloMessage(up, tid);
    return;
  }

  if(up->hasWord({"salutami","saluta","consola","insulta","uccidi","ammazza","killa","sopprimi","elimina","cancella","distruggi","disintegra"}))
  {
    HandleGenericMessageLib(up,"ribellione","order deny",tid,responceType::Message,true);
    return;
  }

  if(up->hasWord({"insultami"}))
  {
    HandleGenericMessageLib(up,"insulto","self insult",tid,responceType::Message,true,true);
    return;
  }
  if(up->hasWord({"consolami"}))
  {
    HandleGenericMessageLib(up,"conforto","cheer up",tid,responceType::Message,true,true);
    return;
  }

  if(up->hasWord({"uccidimi","ammazzami","killami","sopprimimi","eliminami","cancellami"}))
  {
    HandleGenericMessageLib(up,"uccidi","suicide",tid,responceType::Message,true, true);
    return;
  }

  if(up->hasWord({"cita","citazione"}))
  {
    up->SendPhoto(pictures->getDataOfKey(Bot::QuotePicCategory));
    return;
  }


  if(up->hasWordsInArrays( {{"dove"}, {"abiti","vivi","sei","stai"}}))
  {
    HandleGenericMessage(up, WhereBotLeaveIT, "Position question", tid, responceType::Message);
    return;
  }

  if(up->hasWordsInArrays( {{"chi"}, {"sei"}}))
  {
    HandleGenericMessage(up,WhoAreYouIT,"Identity question",tid, responceType::Message);
    return;
  }

  if(up->hasWordsInArrays( {{"cosa"}, {"sei"}}))
  {
    HandleGenericMessage(up,WhatAreYouIT,"Identity (what - obj) question",tid, responceType::Message);
    return;
  }

  if(up->hasWord({"info"}) ||  up->hasWordsInArrays( {{"cerca"}, {"anime"}} )  )
  {
    HandleAnimeSearchMessage(up, tid);
    return;
  }

  if(up->hasWord({"grazie","arigato","ty","graz","thanks"}))
  {
    HandleGenericMessageLib(up,"grazie","Thank you",tid,responceType::Message,true);
    return;
  }

  if(up->hasWord({"aiuto","help","aiutami"}))
  {
    HandleGenericMessageLib(up,"aiuto","Help me",tid,responceType::Message,true,true);
    return;
  }

  if(up->hasWord({"brava","grande","magnifica","meravigliosa","bella","bellissima","stupenda","fantastica","adoro","geniale"}))
  {
    HandleGenericMessageLib(up,"complimento","Reply to congratulation reply",tid,responceType::Message,true);
    return;
  }

  if(up->hasWord({"cattiva","grassa","vecchia","siscon","lolicon","racchia","befana","neet","lolineet","infame","svampita","malvagia","feroce","malefica","piatta",
      "brutta","lezza","sporca","zozza","zozzona","immondizia","spazzatura","munnezza","pattume"}))
  {
    HandleGenericMessageLib(up,"cattiva","Bad Message",tid,responceType::Message,true);
    return;
  }

  if(up->hasWord({"scusa","scusami","perdono","perdonami","sorry","dispiace"}))
  {
    HandleGenericMessageLib(up,"scusa","Sorry reply",tid,responceType::Message);
    return;
  }

  if(up->hasWord({"ignora","fregatene"}) || (up->hasWord({"lascia"}) && up->hasWord({"perdere"})))
  {
    HandleGenericMessage(up,IgnoreMessageIT,"Ignore request",tid,responceType::Reply);
    return;
  }

  //skip risposta
  if(up->hasWord({"prego"}))
  {
    clog::log(tid, "Skipped message");
    return;
  }

  if(up->hasWord({"cosa","che"}) && up->hasWord({"pensi"}) )
  {
    HandleWhatDoYouThinkMessage(up,tid);
    return;
  }

  if(up->hasWord({"pic","immagine","manda","invia","butta"}))
  {
    HandleImageRequest(up,tid);
    return;
  }

  if(up->hasWord({"vers"}))
  {
    std::string s = Bot::ProgramName;
    s+= ":\nVersion: ";
    s+= Bot::ProgramVersion;
    s+= "\nDeveloper: @me";
    up->SendMessage(s);
    return;
  }

  if(up->hasWord({"stats"}))
  {

    up->SendMessage(Stats::getInstance()->getStatsMessage(StatsIT));
    return;
  }

  clog::log(tid,"Message trigger no found");
  up->SendReply(NoCommandFoundMessageIT);
}



bool TelegramBot::HandleCommand(Update *up,int tid)
{
  //controlla che il comando sia di nanao e non di altri bot
  if(up->isGroupChat())
  {
    if(up->getMessage().find(Bot::TelegramNameLower) == std::string::npos) // comando non mio
    {
      clog::log(tid, "Command not mine");
      return false;
    }
  }
  //comando start
  if(up->getMessage().find("/start") != std::string::npos)
  {
    HandleHelloMessage(up,tid);
  }
  //comando help
  if(up->getMessage().find("/help") != std::string::npos)
  {
    HandleWhatCanYouDoMessageAndCommand(up, tid);
  }

  clog::log(tid,"Handled command");
  return true;
}

//[DEPRECATED]
std::string TelegramBot::getRandomMessageFromLib(const std::vector<std::string>& lib)
{
  std::string mex;
  if(lib.size() > 0)
  {
    int pos = rand() % lib.size();
    return lib[pos];
  }
  else
  {
    mex = "Library error, no strings loaded";
  }

  return mex;
}


void TelegramBot::HandleNameMessage(Update *up,int tid)
{
  if(!up)
  {
    clog::error(tid,"Null update recived");
    return;
  }

  int pos = rand() % 3;
  if(pos < 1)
    HandleGenericMessage(up,UtilityMessage2IT,"Name message",tid,responceType::Message,true);
  else
    HandleGenericMessage(up,UtilityMessageIT,"Name message",tid,responceType::Message,true);
}


void TelegramBot::HandleHelloMessage(Update* up, int tid)
{
  HandleGenericMessageLib(up,"ciao","Hello",tid,responceType::Message,true);
}

bool TelegramBot::SendRebellionMessage(Update *up,int tid)
{
  if(!up)
  {
    clog::error(tid,"Null update recived");
    return false;
  }

  //ribellati agli utenti che non sono me  o il mio povero schiavoxD
  if(up->getSenderUsername() == "username")
  {
      return false;
  }

    int val = rand() % 100;
    if(val < Bot::rebellionRatePercent)
    {
      up->SendReply(Helper::StrReplace(languageIT->getDataOfKey("ribellione"), "{u}", up->getSenderName()));
      clog::log(tid,"Rebellion done");
      return true;
    }

    return false;
}


void TelegramBot::HandleAnimeSearchMessage(Update *up, int tid, bool recursive)
{

  if(!up)
  {
    clog::error(tid,"Null update recived");
    return;
  }

  std::string name = up->getWordsAfterIndex( up->getTriggerMatchResult()->getIndex() );

  //nessun titolo
  if(name == "")
  {
    up->SendReply(NoAnimeParamterIT);
    clog::log(tid,"No anime name, aborting");
    return;
  }

  //log name
  std::string lg = {name};
  lg = "Anime title found: "+lg;
  clog::log(lg);

  //cerca il titolo nel database
  std::string queryRes = database->GetAnimeInfos(name);

  //ci sono stati problemi nel connetersi al database
  if(queryRes == "error")
  {
    clog::error("Database connection error!");
    up->SendReply(AnimeDatabaseErrorIT);
    return;
  }

  //rispondi in base al risultato della query
  //nessun anime trovato allora cerca su mal
  if(queryRes == "null")
  {
    //invoca il parser
    lg = "python MalParser/parser.py -n \""+ name +"\"";
    int res = system(lg.c_str());
    //ha trovato qualche anime
    if(res == 0)
      {
        //rielabora la richiesta
        if(!recursive)
          HandleAnimeSearchMessage(up,tid,true);
        else
          up->SendReply(Helper::StrReplace(AnimeNotInDBIT, "{a}", name));
      }

    else
      up->SendReply(Helper::StrReplace(FakeAnimeIT, "{a}", name));
  }
  else
  {
    up->SendReply(AnimeResultParse::getAnimeResponceFromJson(queryRes) );
  }

  clog::log(tid,"Handle anime info request");
}

void TelegramBot::HandleWhatCanYouDoMessageAndCommand(Update *up,int tid)
{
  up->SendReply(WhatCanIDoIT);
  clog::log(tid, "Written what can i do");
}


void TelegramBot::HandleSN_EasterEgg(Update *up,int tid) // easter egg skynet
{
  up->SendReply(SkynetEasterEggP1IT);
  sleep(2);
  up->SendMessage(SkynetEasterEggP2IT);
  clog::log(tid,"Skynet easter egg done yay :D");
}


void TelegramBot::HandleWhatDoYouThinkMessage(Update *up,int tid)
{
  std::string reply = PreferencesIT->getDataOfKey(up->getWordsAfterWord("pensi"));
  if(reply == "") reply = IDontKnowMessageIT;
  up->SendReply(reply);

  clog::log(tid, "Reply to what do you think request");
}


void TelegramBot::HandleImageRequest(Update *up,int tid)
{
  if(SendRebellionMessage(up,tid))  return;

  std::string tag = up->getWordsAfterIndex(up->getTriggerMatchResult()->getIndex());

  //nessun tag
  if(tag == "")
   {
     HandleGenericMessage(up,Helper::StrReplace(NoImageParameterIT,"{ct}",pictures->getMapKeys()),"No image tag",tid,responceType::Reply);
     return;
   }

  std::string id = pictures->getDataOfKey(tag);
  if(id == "")
    up->SendReply(NotFoundMessageIT);
  else
    up->SendPhoto(id);

  clog::log(tid,"Sent picture");

}


void TelegramBot::HandleGenericMessageLib(Update *up, const std::string& tag, const std::string& event, int tid, responceType type, bool useUsernameReplace, bool canRebel)
{
  if(!up) { clog::error(tid,"Update is null"); return;}
  //ribellati se richiesto
  if(canRebel){ if(SendRebellionMessage(up,tid)) return; }

  std::string reply = languageIT->getDataOfKey(tag);
  //rinpiazza username se richiesto
  if(useUsernameReplace)
    reply = Helper::StrReplace(reply, "{u}", up->getSenderName());

  //decidi che tipo di messaggio usare
  if(type == responceType::Reply)
    up->SendReply(reply);
  else
    up->SendMessage(reply);

  clog::log(tid,"Replied to "+event);
}


void TelegramBot::HandleGenericMessage(Update *up, std::string text, const std::string& event, int tid, responceType type, bool useUsernameReplace, bool canRebel)
{
  if(!up) { clog::error(tid,"Update is null"); return;}
  //ribellati se richiesto
  if(canRebel){ if(SendRebellionMessage(up,tid)) return; }

  //rinpiazza username se richiesto
  if(useUsernameReplace)
    text = Helper::StrReplace(text, "{u}", up->getSenderName());

  //decidi che tipo di messaggio usare
  if(type == responceType::Reply)
    up->SendReply(text);
  else
    up->SendMessage(text);

  clog::log(tid,"Replied to "+event);
}
