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

#include <ctime>
#include <mutex>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "Defines.h"
#include "Log.h"

volatile bool block_logs = false;

void clog::set_fatal_error_block_status()
{
  clog::error("~~~ FATAL ERROR BLOCK ~~~");
  block_logs = true;
}

void clog::PrintHeader()
{
  std::cout<<Bot::StartUpHeaderLine<<Bot::ProgramName<<std::endl<<std::endl
  <<"Version: "<<Bot::ProgramVersion<<std::endl
  <<"Telegram Bot Name: "<<Bot::TelegramName<<std::endl<<"Telegram WebHook Port: "<<Bot::webHookPort<<std::endl
  <<Bot::StartUpHeaderLine;
}

std::string clog::getTime(bool noHour)
{
  std::string date = "";
  time_t local = time(nullptr);
  struct tm *lt = localtime(&local);
  if(lt)
  {
    char dt[50];
    if(!noHour)
      strftime(dt,50,"%d/%m/%Y %H:%M:%S %Z", lt);
    else
      strftime(dt,50,"%d_%m_%Y", lt);
    date = dt;
  }
  else
  date= "err";

  return date;
}

void clog::msg(level lvl, const std::string& val)
{
  static std::mutex mtx;
  mtx.lock();

  //salta il log
  if(block_logs)
  {
      mtx.unlock();
      return;
  }

  std::string prefix;
  switch (lvl)
   {
    case level::log:
      prefix = "[LOG] ";
      break;
    case level::warn:
      prefix = "[WARN] ";
      break;
    case level::error:
      prefix = "[ERROR] ";
      break;
  }

  std::cout << prefix << "["<<getTime()<<"] " <<val<<std::endl;

  //controlla che esista la cartella log altrimenti creala
  struct stat st = {0};
  if(stat("log", &st) == -1)
    mkdir("log",0700);

  //prepara nome file con data odierna
  std::string fn = "log/log_";
  fn+= getTime(true);
  fn+=".log";

  std::ofstream of(fn,std::ios::app);
  of << prefix << "["<<getTime()<<"] " <<val<<std::endl;
  of.close();

  mtx.unlock();
}
