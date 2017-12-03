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

#ifndef __BOT_CONNECTION
#define __BOT_CONNECTION

#include <string>
#include <vector>
#include <chrono>

#include <openssl/ssl.h>

#include "../Defines.h"

namespace Connection
{

  typedef std::chrono::high_resolution_clock Clock;

  /**********************************************************************
  * Classi
  ***********************************************************************/

  /*
  * Classe che rappresenta una connessione ssl aperta
  */
  class connection_data
  {
    public:
      connection_data(int sock, SSL *conn, SSL_CTX *ctx)
      {
        connection = conn;
        context = ctx;
        socket = sock;
        last_use_time = Clock::now();
      }

      /*
      * Controlla se la connessione e' troppo vecchia per essere usata
      *
      *@returns: true se la connessione e' in timeout e quindi non deve essere usata
      */
      bool isTimeOut()
      {
        return std::chrono::duration_cast<std::chrono::minutes>(Clock::now() - last_use_time).count() >= Bot::connection_timeout_in_minutes;
      }

      /*
      * Aggiorna il tempo in cui e' stata usata l'ultima volta questa connessione
      */
      void update_last_used_time()
      {
        last_use_time = Clock::now();
      }

    SSL *connection;
    SSL_CTX *context;
    int socket;

    Clock::time_point last_use_time;
  };


  /*
  * Classe che rappresenta un parametro http
  */
  class http_parameter
  {
    public:
      http_parameter(std::string nam, std::string val) : name(nam), value(val)
      {}

      std::string name;
      std::string value;

      /*
      * Restituisce il parametro formattato per essere usato in una richiesta
      *
      * @first_elemet: e' il primo parametro dell'url? se si usa ? al posto di &
      *
      * @returns: il parametro formattato: "&name=value"
      */
      std::string getFormattedParameter(bool first_element) const;
  };


  /**********************************************************************
  * Funzioni
  ***********************************************************************/

  /*
  * Inizializza i componenti necessari per eseguire le connessioni ad altri server
  * * Va chiamata prima di procedere ad usare una qualsiasi delle funzioni di connection
  */
  void init();

  /*
  * Crea e connetti un socket verso la porta 443 dell'host specificato
  * Esegue autonomamente il lookup del dns
  *
  * @hostname_to_resolve: dominio da cui ricagare l'ip. Es: api.telegram.org
  *
  * @returns: id socket aperto e connesso, se il numero e' < 0 impossibile aprire connessione
  */
  int createSocket(const char *hostname_to_resolve);

  /*
  * Crea e connette un socket al una connessione critografata
  *
  * @sock: id del sock su cui effettuale la conversione a ssl
  *
  * @returns: la connessione ssl aperta sottoforma di connection_data. nullptr se la connessione fallisce.
  *   * In caso di fallimento questa funzione dealloca tutte le risorse usate e chiude il socket passato come parametro
  */
  connection_data *createSSLConnection(int sock);

  /*
  * Recupera una connessione pronta all'uso dalla pool
  * In caso non vi sia una connessione disponibile ne apre una nuova
  * * Thread safe (solo recupero dati pool)
  *
  * @returns: la connessione aperta da usare
  */
  connection_data *get_connection_from_pool();

  /*
  * Recupera una connessione pronta all'uso dalla pool
  * In caso non vi sia una connessione disponibile ne apre una nuova
  * * Thread safe
  *
  * @conn: connessione da aggiungere alla pool
  *
  * @returns: la connessione aperta da usare
  */
  void add_connection_to_pool(connection_data *conn);

  /*
  * Esegue una richiesta http alla connessione passata come parametro e chiama il callback con la risposta ricevuta
  *
  * @conn: connessione da usare per la richiesta http
  * @buffer: stringa contente la richiesta http
  * @callback: funzione da chiamare con il risultato ricevuto dalla richiesta. Es funzione: void result_parse(const std::string &result)
  */
  void perform_http_request(connection_data *conn, const std::string& buffer, void (*callback)(const std::string &result));

  /*
  * Esegue una richiesta http a una connessione aperta nei confronti di telegram.
  * Se nessuna connesione e' disponibile questa funzione ne apre una nuova
  *
  * @url_path: connessione da usare per la richiesta http
  * @parameters: vettore di parametri da usare per la richiesta http, vengonono aggiunti come: &nome=valore
  */
  void perform_telegram_request(const std::string &url_path, std::vector<http_parameter> parameters);

  /*
  * Callback per le chiamate api a telegram
  * Scrive se la richiesta e' andata a buon fine, altrimenti segnala l'errore
  *
  * @result: stringa risultato ricevuta dal server
  */
  void telegram_callback(const std::string &result);

  /*
  * Libera le risorse allocate alla connessione ssl e chiudi il socket corrispondente
  * * Dealloca anche conn
  *
  * @conn: connessione da chiudere
  */
  void freeConnection(connection_data *conn);

  /*
  * Url encode stinga
  *
  * @input: stringa da passare nell'url encode
  *
  * @returns: stringa in url encode
  */
  std::string url_encode(const std::string &input);

  /*
  * Printa nei log tutti i parametri passati
  *
  * @parameters: parametri richiesta http
  */
  void log_parameters(const std::vector<http_parameter> &parameters);


}

#endif
