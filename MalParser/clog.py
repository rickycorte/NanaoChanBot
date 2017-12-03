
###################################################################################
#
#    NanaoChanBot
#    Copyright (C) 2017  RickyCorte
#    https://github.com/rickycorte
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
###################################################################################

from time import gmtime, strftime
from datetime import datetime
import os

def writeLog(prefix, message):
    cd = datetime.now().date()
    msg = prefix+" ["+ cd.strftime("%d/%m/%Y") + " "+  strftime("%H:%M:%S", gmtime()) +" GTM] "+ message
    print(msg)

    if not os.path.exists("log"):
        os.makedirs("log")

    of = open("log/log_"+ cd.strftime("%d_%m_%Y")+".log", "a")
    of.write(msg+"\n")
    of.close()



def log(message):
    writeLog("[LOG]",message)

def warn(message):
    writeLog("[WARN]",message)

def error(message):
    writeLog("[ERROR]",message)
