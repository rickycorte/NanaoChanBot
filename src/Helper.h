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

#ifndef __HELPER__
#define __HELPER__

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

/**********************************************************
* Funzioni helper
***********************************************************/
namespace Helper
{

  /**********************************************************
  * risposte HTTP
  ***********************************************************/

  //crea una risposta http con l'header e il messaggio indicato
  //non chiude il socket
  static void sendHttpRequest(int sock, const std::string &head, const std::string &message);

  //invia http200 al socket
  //il socket non viene chiuso
  void SendHttp200(int sock);
  //invia http404 al socket
  //il socket non viene chiudo
  void SendHttpAuthErr(int sock);

  /**********************************************************
  * helper class usata per i file
  ***********************************************************/

  //rappresenta un parametro di un link con il suo valore
  class parameterBlock
  {
    public:
      parameterBlock(std::string p, std::string v) : param(p), value(v)
      {}

      std::string getParameter() const { return param; }
      std::string getValue() const { return value; }

    private:
      std::string param;
      std::string value;
  };


  /**********************************************************
  * string helper
  ***********************************************************/

  //crea un json {"name":"data"}
  std::string createShortJson(const std::string& name, const std::string& data);

  //sostituisci una parte di stringa con un altra
  std::string StrReplace(std::string target, const std::string& itmToReplace, const std::string& replacer);

  //tolower della stringa passata come parametro
  //rimuove caratteri non supportati come &
  void tolower(std::string& strRef);

  //carica in un vettore il testo letto dal file passato come parametro
  std::vector<std::string> LoadTextLibrary(const char *path);

  //recupera solo il json in una richiesta ricevuta
  std::string getJson(std::string body);

  //converti in stringa un numero con n precisione di decimali
  template <typename T>
  std::string to_string_with_precision(const T a_value, const int n = 0)
  {
      std::ostringstream out;
      out << std::fixed <<std::setprecision(n) << a_value;
      return out.str();
  }

  inline const char * const BoolToString(bool b)
  {
    return b ? "true" : "false";
  }

}

#endif // __HELPER__
