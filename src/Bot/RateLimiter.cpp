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

#include "RateLimiter.h"

#include <map>
#include <mutex>
#include <thread>
#include <chrono>

#include "../Defines.h"
#include "../Log.h"

namespace RateLimiter
{

  /*
  * Rate limiter container: chat_id - n utilizzi chat
  */
  static std::map<std::string,int> chat_rates;

  static std::mutex chat_rates_mtx;

  static std::thread *timer;



  bool rate_container_has_chat(const std::string& chat_id)
  {
    return chat_rates.find(chat_id) != chat_rates.end();
  }


  void rate_container_add_chat(const std::string& chat_id)
  {
    chat_rates.insert(std::pair<std::string, int >(chat_id,1));
  }


  void clear_container()
  {
    chat_rates_mtx.lock();

    chat_rates.clear();
    chat_rates_mtx.unlock();
  }

  /*
  * Funzione avviata nel thread interno che serve per liberare la lista di chat limitate ogni rate_limit_time
  */
  void thread_clear_container()
  {
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(Bot::rate_limit_time));
      clear_container();
    }
  }

  void init()
  {
    timer = new std::thread(&thread_clear_container);
    if(!timer)
    {
      clog::error("Can't allocate thread for RateLimiter");
      return;
    }

    timer->detach();
    clog::log("Rate limiter ready");
  }


  void UpdateChatStatus(const std::string& chat_id)
  {
    chat_rates_mtx.lock();

    //controlla se la chat e' presente nel limiter
    if(!rate_container_has_chat(chat_id))
    {
      //aggiungi nuova chat
      rate_container_add_chat(chat_id);
    }
    else // gia presente, aggiorna il numero di rate
    {
      chat_rates[chat_id] += 1;
    }

    chat_rates_mtx.unlock();
  }


  int getRateLimitCount(const std::string& chat_id)
  {
    chat_rates_mtx.lock();

    int res = 0;
    if(rate_container_has_chat(chat_id))
      res = chat_rates[chat_id];

    chat_rates_mtx.unlock();

    return res;
  }


  bool isRateLimited(const std::string& chat_id)
  {
    return getRateLimitCount(chat_id) > Bot::rate_limit_message_count;
  }

}
