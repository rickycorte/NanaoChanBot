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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>
#include <thread>
#include <climits>
#include <mongocxx/instance.hpp>

//stacktrace
#include <execinfo.h>
#include <signal.h>

#include "Log.h"
#include "Defines.h"
#include "Helper.h"
#include "Bot/TelegramBot.h"
#include "Bot/Stats.h"

#include "ExecutionTimer.h"
#include "Bot/Connection.h"
#include "Bot/RateLimiter.h"


static TelegramBot *bot;
static int listenSocket;
static char *buffer;

static const int bufferSize = 8192;

volatile sig_atomic_t signaled = 0;

void HandleConnection(int sock)
{
  Stats::getInstance()->IncrementConnectionsNumber();
  //read data recivied in request
  memset(buffer, '\0', bufferSize );
  int sz = read(sock, buffer, bufferSize-1);
  *(buffer + sz) = '\0'; // assicurati che ci sia uno \0 a terminare la riga il prima possibile
  *(buffer + bufferSize - 1 ) = '\0'; //l'utimo carattere e' forzatamente uno \0 che termina la riga

  if(!buffer)
  {
    clog::error("NULL buffer");
    return;
  }
  //messaggio troppo lungo per essere processato
  //libera il buffer e manda indietro una richesta http200
  if(sz == bufferSize-1)
  {
    clog::log("Buffer is full!");
    //svuota la stream
    while( sz > 0)
    {
      sz = read(sock, buffer, bufferSize-1);
      clog::log("Clearing stream");
      clog::log(buffer);
    }

    Helper::SendHttp200(sock);
    clog::error("Ignored message longer than buffer");
    close(sock);
    return;
  }

  if(sz <= 1)
  {
    clog::error("Can't read data from socket.");
    return;
  }

  std::string body = buffer;

  //recupera ip originale e stampalo a schermo
  std::size_t fwb = body.find("X-Forwarded-For:");
  if(fwb != std::string::npos)
  {
    fwb+=16;
    std::size_t end = body.find('\n',fwb);

    std::string fip = "Original sender ip: ";
    fip += body.substr(fwb,end-fwb);
    clog::log(fip);
  }

  //controlla che la richiesta venga da telegram
  char *valid = strstr(buffer, Bot::TelegramToken);
  if(valid)
  {
    //informa subito telegram che l'hook e' stato ricevuto cosi da evitare duplicati
    Helper::SendHttp200(sock);
    clog::log("Request size: "+std::to_string(strlen(buffer)));
    bot->AddRequestToQueue( body.c_str() );

  }
  else
  {
    std::string lg = "Rejected request: invalid auth (req size:"+ std::to_string(strlen(buffer)) +")";
    clog::log(lg);
    clog::log(buffer);
    Helper::SendHttpAuthErr(sock);
  }
  close(sock);
}

void StartConnectionHander()
{
  clog::log("Starting connection handler");
  //prepara socket per ricevere webHook da Telegram
  struct sockaddr_in reciver;
  memset(&reciver, 0, sizeof(reciver));
  reciver.sin_family = AF_INET;
  reciver.sin_port = htons(atoi(Bot::webHookPort));
  reciver.sin_addr.s_addr = INADDR_ANY;

  //crea il socket e bindalo alla porta scelta
  listenSocket = socket(reciver.sin_family, SOCK_STREAM , 0);

  int sockOtp = 1;
  //evita che la porta rimanga in uso per un po dopo crash o chiusura
  setsockopt(listenSocket,SOL_SOCKET, SO_REUSEADDR, &sockOtp, sizeof(sockOtp));

  if( bind(listenSocket, (struct sockaddr *)&reciver , sizeof(reciver) ) < 0)
  {
    clog::error("Unable to bind this app to webHookPort");
    exit(1);
  }

  listen(listenSocket, Bot::maxConnections);

  //attendi le connessioni
  while (true)
  {
    //clog::log("Waiting for connections...");
    struct sockaddr_in client;
    unsigned int len = sizeof(client);

    //accetta connessione
    int newsock = accept(listenSocket, (struct sockaddr *)&client, &len);
    //imposta timeout
    struct timeval tv;
    tv.tv_sec = Bot::socketTimeOutSec; tv.tv_usec = Bot::socketTimeOutMilliSec;
    setsockopt(newsock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    //converti ip in stinga
    char ip[INET_ADDRSTRLEN];
    std::string lg = "Accepted connection from ip: ";
    lg += inet_ntoa(client.sin_addr);
    clog::log(lg);

    //controlla che la connesione sia valida
    if(newsock < 0 )
    {
      clog::warn("No connection.");
    }
    else
    {
      HandleConnection(newsock);
    }

  }
}

//scrivi lo stack trace del crash
void WriteStackTrace()
{
  void *array[100];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 100);

  // print out all the frames to stderr
  //std::string lg;
  char **traces = backtrace_symbols(array, size);

  //lg= "StackTrace:\n";
  for(int i=0;i<size;i++)
  {
    std::cout<<traces[i]<<std::endl;
    //lg += traces[i];
    //lg += "\n";
  }
  //std::cerr<<lg;
}


void sigHandler(int sig)
{
  clog::set_fatal_error_block_status();
  signaled = sig;
  WriteStackTrace();
}


void signalMessage(const char* msg)
{
  clog::error(msg);
  Connection::perform_telegram_request(Bot::sendMessageUrl,{ {"chat_id",std::to_string(-1001109568886)}, {"text", msg} });
}

int main(int argv, char *argc[])
{
  // registra handler per crash dovuti a puntatori errati
  signal(SIGSEGV, sigHandler);
  signal(SIGABRT, sigHandler);
  signal(SIGFPE, sigHandler);
  signal(SIGILL, sigHandler);
  signal(SIGINT, sigHandler);
  signal(SIGTERM, sigHandler);

  clog::PrintHeader();

  //crea connessione a mongodb
  clog::log("Creating mongocxx instance");
  auto instance =  mongocxx::instance();
  clog::log("Done");

  //one time allocation del buffer
  clog::log("Allocating socket buffer");
  buffer = new char[bufferSize];
  if(!buffer)
  {
    clog::error("Can't allocate socket buffer!");
    return 1;
  }
  clog::log("Done");

  //avvia il bot
  bot = new TelegramBot();
  if(!bot)
  {
    clog::error("Unable to create bot");
    return 1;
  }

  ExecutionTimer::setup();
  Connection::init();
  RateLimiter::init();

  //avvia l handler delle connessioni su un thread separato
  std::thread ch = std::thread(StartConnectionHander);

  //controlla per eventuali signal
  while(true)
  {
    if(signaled != 0) // ricevuto un signal
    {
      clog::error("Signal recived");
      switch (signaled)
      {
        case SIGABRT:
          signalMessage("Abort signal recived");
          break;
        case SIGFPE:
          signalMessage("Illegal float operation signal recived");
          //WriteStackTrace();
          break;
        case SIGILL:
         signalMessage("Illegal operation signal recived");
         //WriteStackTrace();
          break;
        case SIGINT:
          signalMessage("Interrupt signal recived");
          break;
        case SIGSEGV:
          signalMessage("Segmentation fault signal recived");
          //WriteStackTrace();
          break;
        case SIGTERM:
          signalMessage("Termination signal recived");
          break;
      }
      break;
    }

    sleep(1);

  }

  clog::log("Closing app");
  close(listenSocket); // chiudi socket
  delete[] buffer;   //dealloca buffer
  delete bot; //dealloba bot

  return 0;
}
