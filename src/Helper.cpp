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
#include <ctime>
#include <curl/curl.h>
#include <fstream>
#include <mutex>
#include <chrono>
#include <thread>

#include "Helper.h"
#include "Log.h"
#include "Defines.h"
#include "ExecutionTimer.h"

namespace Helper
{

void sendHttpRequest(int sock, const std::string &head, const std::string &message)
{
  std::string resp = head + "\n";

  resp+= "Date: ";
  //crete time string
  char dt[50];
  time_t t = time(nullptr);
  strftime(dt,50,"%a, %d %b %Y %H:%M:%S %Z", gmtime(&t));
  resp+= dt;
  resp += "\n";

  resp+= "Server: custom\n";
  resp+= "Content-Type: text/html\n";
  resp+= "Content-Lenght: "+std::to_string(message.size())+"\n";
  resp += "Connection: close\n\n";
  resp += message;

  send(sock, resp.c_str(), resp.size() ,0);
  clog::log(head + " sent to socket");
}

//send back a http 200 to request to inform that the request has been recived
void SendHttp200(int sock)
{
  sendHttpRequest(sock,"HTTP/1.1 200 OK","ok");
}

void SendHttpAuthErr(int sock)
{
  sendHttpRequest(sock,"HTTP/1.1 404 Not Found","404 NOT FOUND");
}

/**********************************************************
* Helper stringhe
***********************************************************/

std::string StrReplace(std::string target, const std::string& itmToReplace, const std::string& replacer)
{
  size_t pos = target.find(itmToReplace);
  // stringa non trovata, non c'e nulla da sostituire
  if(pos == std::string::npos)
  {
    return target;
  }

  target.replace(pos, itmToReplace.size(), replacer);
  return target;

}


void tolower(std::string& strRef)
{
  for(int i = 0; i < strRef.size(); i++)
  {
    strRef[i] = std::tolower(strRef[i]);
    if(strRef[i] == '&') strRef[i] = ' ';
  }
}

std::vector<std::string> LoadTextLibrary(const char *path)
{
  std::vector<std::string> items;

  std::ifstream in(path);
  if(in.fail())
  {
    clog::error("Unable to load text lib:");
    clog::error(path);
    in.close();
    return items;
  }

  std::string line;
  while(getline(in,line))
  {
    items.push_back(line);
  }
  in.close();

  std::string mes = "Loaded "+std::to_string(items.size())+" from: "+ path;
  clog::log(mes);

  return items;
}

//trova il json (prima {) nella stringa passata
std::string getJson(std::string body)
  {
    //trova l inizio del json
    std::size_t pos = body.find("\n{\"");
    if(pos == std::string::npos)
    {
      clog::error("No json detected in body");
      return "";
    }
    body = body.substr(pos+1);

    return body;
  }

std::string createShortJson(const std::string& name, const std::string& data)
{
  std::string res = "{\""+name+"\":\""+data+"\"}";
  return res;
}

}
