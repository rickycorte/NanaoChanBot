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

#ifndef __BOT__
#define __BOT__

#include <vector>
#include <string>
#include <thread>
#include <mutex>

#include "Update.h"
#include "../Helper.h"
#include "../DataMap.h"
#include "Stats.h"

class TelegramBot
{
public:

  TelegramBot();
  ~TelegramBot();

  //crea e aggiungi un update alla coda
  void AddRequestToQueue(const char *reqBody);

private:

  //handle a bot update e cerca tra i vari trigger una corrispondenza
  void HandleRequest(Update *up, int tid, bool *handled);

  // esegui un comando tipo /start
  bool HandleCommand(Update *up, int tid);

  //[DEPRECATED] Restituisce un messaggio a caso della libreria passata come parametro
  std::string getRandomMessageFromLib(const std::vector<std::string>& lib);

  //aggiunge i file che mando al bot ai vari container
  void ProcessMessageData(Update *up, int tid);

  /*********************************************************************
  * Handler messaggi
  *********************************************************************/

  //wrapper per tutti gli handler di nanao
  void SearchHandlerInMessage(Update *up, int tid);

  //rispondi a un messaggio contenente solo il nome
  void HandleNameMessage(Update *up, int tid);
  //rispondi a un saluto
  void HandleHelloMessage(Update *up, int tid);
  //ribellati a una richiesta discorsiva con x percentale
  //restituisce true se si ribella altrimenti false
  bool SendRebellionMessage(Update *up, int tid);
  //ricerca anime standard per titolo
  void HandleAnimeSearchMessage(Update *up, int tid, bool recursive = false);

  //risponde alle domande cosa ne pensi di
  void HandleWhatDoYouThinkMessage(Update *up, int tid);

  //manda un immagine con un tag
  //puo ribellarsi
  void HandleImageRequest(Update *up, int tid);

  //tipo di risposta da inviare: messaggio normale o risposta a un messaggio (quote)
  enum class responceType { Message, Reply };

  //risposta generica con una libreria di testi
  //replacer opzionale
  void HandleGenericMessageLib(Update *up, const std::string& tag, const std::string& event, int tid, responceType type, bool useUsernameReplace = false, bool canRebel = false);
  //risposta generica con una testo singolo
  //replacer opzionale
  void HandleGenericMessage(Update *up, std::string text, const std::string& event, int tid, responceType type, bool useUsernameReplace = false, bool canRebel = false);

  // easter egg skynet (usato come suo nome)
  void HandleSN_EasterEgg(Update *up, int tid);

  /*********************************************************************
  * Handler comandi
  *********************************************************************/

  //rispondi al comando /help
  void HandleWhatCanYouDoMessageAndCommand(Update *up, int tid);

  /*********************************************************************
  * Thread
  *********************************************************************/

  std::vector<std::thread *> workers;

  volatile bool closeThreads;

  //lista delle richeiste da processare
  std::vector<char *> requestsQueue;

  //mantiene aperto il thread in attessa che la posizione della lista corrispondente all'id sia riempita con dei dati
  void WorkerThreadMain(int threadID);

  //thread safe, recupera il primo update da processare
  char *TgetUpdateToProcess();
  //wreapper per handle request che cancella la richiesta e fa partire un nuovo thread per processare un nuovo elemento
  void ThreadHandleRequest(char *req, int tid);

  //mutex che evita accesso multiplo simultaneo a lista richeiste da processare
  std::mutex queue_mtx;


  /*********************************************************************
  * Data
  *********************************************************************/

  //mappe di risposte e dati
  DataMap *PreferencesIT;


  DataMap *pictures;

  //librerie di testo contenenti i messaggi del bot
  DataMap *languageIT;

  //frasi costanti usate dal bot
  const char *const UtilityMessageIT = "{u} come posso esserti utile?";
  const char *const NoCommandFoundMessageIT = "Spiegati meglio baka";
  const char *const DenyInsultMessageIT= "*linguaccia*";
  const char *const DenySelfInsultIT = "Mi hai preso per una scema?";
  const char *const UtilityMessage2IT = "Si sono io";
  const char *const VolgarityMessageIT = "Non parlo con un aho volgare come te, se smetti di dire queste cose forse ti perdono baka";
  const char *const WhereBotLeaveIT = "Io vivo nel cloud in una comoda casa senza finestre";
  const char *const WhoAreYouIT = "Sono la tipica ragazza che adora anime, computer e non esiste";
  const char *const WhatAreYouIT = "Non sono un oggetto stupido baka, sono una persona proprio come te";

  const char *const AnimeNotInDBIT = "Mi spiace, ma non sono riuscita a trovare nulla"; //{a} per titolo anime
  const char *const FakeAnimeIT = "Stupido baka quello non e' un anime!"; //{a} per titolo anime
  const char *const WhatCanIDoIT = "Fammi pensare... non me lo ricordo baaaaka!";


  const char *const SkynetEasterEggP1IT = "Stavate parlando di me?";
  const char *const SkynetEasterEggP2IT = "Che scema che sono, a volte mi dimentico di chiamarmi Nanao :3";

  const char *const IgnoreMessageIT = "Baka sono io a decidere chi o cosa ignorare";
  const char *const MessageWithNegationIT = "Non dirmi cosa non devo fare stupido baka!";
  const char *const IDontKnowMessageIT = "Non saprei :3";
  const char *const NotFoundMessageIT = "Non ho nulla del genere :3";

  const char *const StatsIT = "Connessioni: {c}\nMessaggi ricevuti: {m}\nRichieste ricevute: {r}\n\nMessaggi al minuto: {mm}\nRichieste al minuto: {rm}\n\nTempo processing messaggi: {pm}ms\nTempo processing richieste: {pr}ms\n\nRTT Telegram: {rtt}ms\nTempo di risposta stimato: {q}ms";
  const char *const NoImageParameterIT = "Baka la prossima volta ricordati di dirmi una categoria dell'immagine! Per ora ho {ct}";
  const char *const NoAnimeParamterIT = "Se vuoi che ti cerchi delle informazioni di un anime devi dirmi il titolo, stupido baka!";

  const char *const AnimeDatabaseErrorIT = "Mi spiace, al momento non posso aiutarti.";

};

#endif //__BOT__
