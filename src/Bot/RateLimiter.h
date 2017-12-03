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

#ifndef __BOT__RATELIMITER__
#define __BOT__RATELIMITER__

#include <string>

/*
* RateLimiter messaggi, serve a evitare un eccessivo uso del bot e il blocco delle richieste da parte di telegram
*
* Utilizza un thread interno come timer al fine di avere timer dedicati e che non sono influenzati da altre operazioni
*/
namespace RateLimiter
{

  /*
  * Inizializza il rate limiter
  */
  void init();

  /*
  * Aggiungi/Incrementa il contatore del rate limiter per l'id della chat passato
  * *Thread safe
  *
  * @chat_id: id della chat da aggiungere al rate limiter
  */
  void UpdateChatStatus(const std::string& chat_id);

  /*
  * Ottieni lo stato della chat
  * *Thread safe
  *
  * @chat_id: id della chat da controllare
  *
  * @returns: true se la chat ha sforato il limite di richieste/sec
  */
  bool isRateLimited(const std::string& chat_id);

  /*
  * Ottieni il numero di volte che la chat e' stata usata entro il tempo del rate limit
  * *Thread safe
  *
  * @chat_id: id della chat di cui si vogliono le informazioni
  *
  * @returns: il numero di volte in cui la chat e' stata usata
  */
  int getRateLimitCount(const std::string& chat_id);


  /*
  * Pulisci il conteiner delle chat limitate
  * *Thread safe
  */
  void clear_container();

  /*
  * Conrolla che chat_rates abbia la chiave specificata
  *
  * @chat_id: id della chat da controllare
  *
  * @returns: true se la chat gia esiste
  */
  bool rate_container_has_chat(const std::string& chat_id);

  /*
  * Crea una chiave nella map con l'id della chat specificato e 1 come n utilizzi
  *
  * @chat_id: id della chat da controllare
  */
  void rate_container_add_chat(const std::string& chat_id);

  /*
  * Funzione avviata nel thread interno che serve per liberare la lista di chat limitate ogni rate_limit_time
  */
  void thread_clear_container();

}

#endif
