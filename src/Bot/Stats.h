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

#ifndef __STATS__
#define __STATS__

#include <string>

#include "../Log.h"

//TODO: resettare stats se si sta per superare lalore max
class Stats
{
public:
    static Stats *getInstance()
    {
      if(s == nullptr)
        s = new Stats;

      return s;
    }


    //incrementa il numero di connessioni ricevute
    //  * thread safe
    void IncrementConnectionsNumber();

    //aggiungi il tempo impiegato a processare il messaggio
    //  *incrementa il numero di messaggi ricevuti
    //  * thread safe
    void AddMessageProcessingTime(double tm);

    //aggiunge il tempo di processing, di post alle stats
    // *il tempo di processing puo' essere reindirizzato alla media di richieste pesanti che richiedono diverse operazioni
    // *questa funzione incrementa il numero di richieste ricevute e dei messaggi ricevuti
    // *thread safe
    void AddRequestProcessingTime(double processing, double post, bool isHeavy = false);

    //calcola il tempo di processing medio di tutte le richieste
    double calculateAverageProcessingTime();
    //calcola il tempo di processing medio delle sole richieste del bot
    double calculateAverageBotRequestsProcessingTime();
    //calcola il tempo medio impiegato a mandare dati a telegram
    double calculateAverageRoundTripTime();
    //stima tempo di coda in caso di intasamento (tempo risposta medio / numero thread)
    double estimateQueueTime();
    //stima il tempo di risposta medio
    double estimateAverageResponceTime();

    double getMessagePerMinute() {return LastMinuteMessages; }
    double getRequestsPerMinute() {return LastMinuteBotRequests; }
    double getConnectonNumber() {return ConnectionsRecived-1; } //-1 perche se no i dati tengono in conto di una richiesta non ancora elaborata
    double getMessageCount() { return MessagesRecived; }
    double getRequestCount() {return BotRequestsRecived; }

    //restituisce il numero di messaggi che sono stati ricevuti ma non sono indirizzati al bot
    double getChatMessagesThatAreNotRequestsCount() { return MessagesRecived - BotRequestsRecived; }

    //restituise il messaggio contente le stats di nanao in base al template passato come parametro
    std::string getStatsMessage(const std::string& base);

private:
  Stats();
  Stats(Stats const&) = delete;
  void operator =(Stats const&) = delete;

  static Stats *s;

  //media
  double avg(double sum, double count) { return sum/count; }

  //salva i dati del giorno in una riga su file cosi da poter essere letti
  static void SaveDayDataToFile();

  //resetta i valori delle richieste al minuto
  static void ResetRequestsPerMinute();
  //resetta tutte le statistiche della giornata
  static void ResetDayStats();

  //aggiungi il tempo impegato a processare una richiesta al bot
  void AddBotRequestProcessingTime(double tm);
  //aggiungi il tempo impegato a inviare la risposta alle api
  void AddRequestPostTime(double tm);

  //incrementa il numero di messaggi ricevuti da telegram
  void IncrementMessagesRecived();
  //incrementa richieste bot
  void IncrementBotRequests();

  //*****************************************************************
  //dati

  double ConnectionsRecived;
  double MessagesRecived;
  double BotRequestsRecived;

  double ProcessingTime; // tempo di processing di tutte le richieste ricevute
  double BotRequestsProcessingTime; // tempo di processing delle sole richieste del bot
  double SendTime;

  double LastMinuteBotRequests; // numero richieste al bot ricevute nell'ultimo minuto
  double LastMinuteMessages;  //numero messaggi ricevuti dal bot nell'ultimo minuto

};

#endif
