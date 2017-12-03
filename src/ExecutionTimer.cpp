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

#include "ExecutionTimer.h"

#include <vector>
#include <thread>
#include <mutex>
#include <cassert>
#include <chrono>

#include "Log.h"

namespace ExecutionTimer
{

  typedef std::chrono::high_resolution_clock Clock;

  //liste di callback
  static std::vector<void (*)()> minute_callbacks;
  static std::vector<void (*)()> midnight_callbacks;

  static std::mutex minute_list_mtx;
  static std::mutex midnight_list_mtx;

  static std::thread *timer;

  /*
  * Esegue tutti i callback registrati come minute_callbacks
  */
  void runMinuteCallbacks()
  {
    minute_list_mtx.lock();

    for(int i = 0; i < minute_callbacks.size(); i++)
    {
      minute_callbacks[i]();
    }

    minute_list_mtx.unlock();
  }

  /*
  * Esegue tutti i callback registrati come midnight callbacks
  */
  void runMidnightCallbacks()
  {
    midnight_list_mtx.lock();

    for(int i = 0; i < midnight_callbacks.size(); i++)
    {
      midnight_callbacks[i]();
    }

    midnight_list_mtx.unlock();
  }

  /*
  * Funzione timer eseguita dal thread
  */
  void thread_timer()
  {
    struct tm *t;
    time_t local;
    int hour;

    auto start = Clock::now();

    while(true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      //aspetta il passare di un minuto
      if(std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - start).count() >= 60)
      {
        start = Clock::now(); // aggiorna prima il tempo di partenza cosi non viene influenzato dalle operazioni seguenti

        runMinuteCallbacks();

        //controlla che sia mezza notte
        local = time(nullptr);
        t = localtime(&local);
        if(t->tm_hour == 0 && t->tm_min == 0) // mezzanotte
        {
          runMidnightCallbacks();
        }

     }

    }

  }


  void setup()
  {
    timer = new std::thread(&thread_timer);
    if(!timer)
    {
      clog::error("Unable to create timer thread!");
      assert(nullptr); //blocca il programma perche e' impossibile avviare una parte di esso
    }
    timer->detach();
    clog::log("Execution timer ready");
  }


  int addCallback_MinuteTimer( void (*ptr)() )
  {
    minute_list_mtx.lock();
    //controlla che non sia un puntatore nullo
    if(!ptr)
    {
          minute_list_mtx.unlock();
          return -1;
    }
    //aggiungi il puntatore nullo alla fine della lista
    minute_callbacks.push_back(ptr);
    int id = minute_callbacks.size()-1; // calcola l'id come ultima pos

    minute_list_mtx.unlock();

    return id;
  }


  int addCallback_MidnightTimer( void (*ptr)() )
  {
    midnight_list_mtx.lock();
    //controlla che non sia un puntatore nullo
    if(!ptr)
    {
          midnight_list_mtx.unlock();
          return -1;
    }
    //aggiungi il puntatore nullo alla fine della lista
    midnight_callbacks.push_back(ptr);
    int id = midnight_callbacks.size()-1; // calcola l'id come ultima pos

    midnight_list_mtx.unlock();

    return id;
  }

}
