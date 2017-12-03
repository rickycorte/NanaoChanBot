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


/**********************************************************
* Log to console with data
***********************************************************/
#ifndef __LOG__
#define __LOG__
#include <string>
#include <iostream>

namespace clog
{
  std::string getTime(bool noHour=false);

  enum class level { none, log, warn, error };

  //print a message on console with time and prefix
  void msg(level lvl, const std::string& val);

  /**********************************************************
  * Funzioni per i log
  ***********************************************************/


  inline void log(const std::string val)
  {
    msg(level::log, val);
  }


  inline void warn(const std::string val)
  {
    msg(level::warn, val);
  }

  inline void error(const std::string val)
  {
    msg(level::error, val);
  }


  inline void log(int tid, const std::string val)
  {
    msg(level::log, "["+ std::to_string(tid)+"] "+ val);
  }

  inline void warn(int tid, const std::string val)
  {
    msg(level::warn, "["+ std::to_string(tid)+"] "+ val);
  }

  inline void error(int tid, const std::string val)
  {
    msg(level::error, "["+ std::to_string(tid)+"] "+ val);
  }


  void PrintHeader();

  void set_fatal_error_block_status();

}

#endif //__LOG__
