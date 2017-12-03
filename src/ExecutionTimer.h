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

#ifndef __EXEC_TIMER
#define __EXEC_TIMER

namespace ExecutionTimer
{
  /*
  * Avvia il timer
  */
  void setup();

  /*
  * Aggiunge una funzione ai callback chiamati ogni minuto
  *
  * @ptr: puntatore alla funzione da aggiungere ai callback
  *
  * @return: id del callback appena aggiunto
  */
  int addCallback_MinuteTimer( void (*ptr)() );

  /*
  * Aggiunge una funzione ai callback chiamati ogni smezzanotte
  *
  * @ptr: puntatore alla funzione da aggiungere ai callback
  *
  * @return: id del callback appena aggiunto
  */
  int addCallback_MidnightTimer( void (*ptr)() );

}

#endif //__EXEC_TIMER
