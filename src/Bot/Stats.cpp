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

#include "Stats.h"

#include <thread>
#include <chrono>
#include <ctime>
#include <mutex>

#include "../Helper.h"
#include "../Defines.h"
#include "../ExecutionTimer.h"

Stats *Stats::s = nullptr;

Stats::Stats()
{

  ConnectionsRecived = 0;
  MessagesRecived = 0;
  BotRequestsRecived = 0;

  ProcessingTime = 0;
  BotRequestsProcessingTime = 0;
  SendTime = 0;

  LastMinuteBotRequests = 0;
  LastMinuteMessages = 0;

  //aggiungi callback a timer
  ExecutionTimer::addCallback_MidnightTimer(&Stats::ResetDayStats);
  ExecutionTimer::addCallback_MinuteTimer(&ResetRequestsPerMinute);
}


//incrementa il numero di connessioni ricevute
void Stats::IncrementConnectionsNumber()
{
  static std::mutex mtx;
  mtx.lock();

  ConnectionsRecived++;

  mtx.unlock();
}

//incrementa il numero di messaggi ricevuti da telegram (rivolti o meno al bot)
void Stats::IncrementMessagesRecived()
{
  MessagesRecived++; //incrementa messaggi normali sempre
  LastMinuteMessages++; // incrementa msg ricevuti nell'ultimo minuto
}

void Stats::IncrementBotRequests()
{
  BotRequestsRecived++;
  LastMinuteBotRequests++;
  IncrementMessagesRecived();
}

//aggiungi il tempo impiegato a processare il messaggio
void Stats::AddMessageProcessingTime(double tm)
{
  static std::mutex mtx;
  mtx.lock();

  ProcessingTime += tm;
  IncrementMessagesRecived();
  //clog::log("MessageProcessing: Added "+std::to_string(tm)+" new time: "+std::to_string(ProcessingTime));

  mtx.unlock();
}

void Stats::AddBotRequestProcessingTime(double tm)
{
  BotRequestsProcessingTime += tm;
  //AddMessageProcessingTime(tm);
  //clog::log("RequestProcessing: Added "+std::to_string(tm)+" new time: "+std::to_string(BotRequestsProcessingTime));
}


//aggiungi il tempo impegato a inviare la risposta alle api
void Stats::AddRequestPostTime(double tm)
{
  SendTime += tm;
  //clog::log("SendTime: Added "+std::to_string(tm)+" new time: "+std::to_string(SendTime));
}

//TODO: aggiungere stats per richieste pesanti
void Stats::AddRequestProcessingTime(double processing, double post, bool isHeavy)
{
  static std::mutex mtx;
  mtx.lock();

  AddBotRequestProcessingTime(processing);
  AddRequestPostTime(post);
  IncrementBotRequests();

  mtx.unlock();
}

//calcola il tempo di processing medio di tutte le richieste
double Stats::calculateAverageProcessingTime()
{
  return avg(ProcessingTime, MessagesRecived);
}

//calcola il tempo di processing medio delle sole richieste del bot
double Stats::calculateAverageBotRequestsProcessingTime()
{
  return avg(BotRequestsProcessingTime, BotRequestsRecived);
}

double Stats::calculateAverageRoundTripTime()
{
  return avg(SendTime, BotRequestsRecived);
}

//stima il tempo di risposta medio
double Stats::estimateAverageResponceTime()
{
  return calculateAverageBotRequestsProcessingTime() + calculateAverageRoundTripTime();
}

//stima tempo di coda in caso di intasamento (tempo risposta medio / numero thread)
double Stats::estimateQueueTime()
{
  return estimateAverageResponceTime() * (1 + LastMinuteBotRequests / Bot::maxConcurrentRequestHandlerThreads) //tempo medio di risposta a richieste bot basato su thread e richieste/min
  + calculateAverageProcessingTime() * ( LastMinuteMessages / Bot::maxConcurrentRequestHandlerThreads ); // tempo medio di risposta a messaggi basato su thread e richieste
}

//salva i dati del giorno in una riga su file cosi da poter essere letti
void Stats::SaveDayDataToFile()
{
  //TODO
}

//resetta i valori delle richieste al minuto
void Stats::ResetRequestsPerMinute()
{
  getInstance()->LastMinuteMessages = 0;
  getInstance()->LastMinuteBotRequests = 0;
}

//resetta tutte le statistiche della giornata
void Stats::ResetDayStats()
{
  clog::log("Reset data for the new day!");

  getInstance()->ConnectionsRecived = 0;
  getInstance()->MessagesRecived = 0;
  getInstance()->BotRequestsRecived = 0;

  getInstance()->ProcessingTime = 0;
  getInstance()->BotRequestsProcessingTime = 0;
  getInstance()->SendTime = 0;
}

std::string Stats::getStatsMessage(const std::string& base)
{
  std::string r = Helper::StrReplace(base,"{c}",Helper::to_string_with_precision(Stats::getInstance()->getConnectonNumber()));
  r = Helper::StrReplace(r,"{m}",Helper::to_string_with_precision(Stats::getInstance()->getMessageCount()));
  r = Helper::StrReplace(r,"{r}",Helper::to_string_with_precision(Stats::getInstance()->getRequestCount()));
  r = Helper::StrReplace(r,"{mm}",Helper::to_string_with_precision(Stats::getInstance()->getMessagePerMinute()));
  r = Helper::StrReplace(r,"{rm}",Helper::to_string_with_precision(Stats::getInstance()->getRequestsPerMinute()));
  r = Helper::StrReplace(r,"{pm}",Helper::to_string_with_precision(Stats::getInstance()->calculateAverageProcessingTime(),3));
  r = Helper::StrReplace(r,"{pr}",Helper::to_string_with_precision(Stats::getInstance()->calculateAverageBotRequestsProcessingTime(),3));
  r = Helper::StrReplace(r,"{rtt}",Helper::to_string_with_precision(Stats::getInstance()->calculateAverageRoundTripTime(),3));
  r = Helper::StrReplace(r,"{q}",Helper::to_string_with_precision(Stats::getInstance()->estimateQueueTime(),3));

  return r;
}
