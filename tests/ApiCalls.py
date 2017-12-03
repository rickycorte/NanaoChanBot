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

import urllib
import time



i = 0
add = 0

while i < 100:
	start_time = time.time()
	f = urllib.urlopen("https://api.telegram.org/token/sendMessage?chat_id=80145337&text="+str(i) )
	ms = (time.time() - start_time) *1000

	add += ms
	print("[%s]--- RTT: %s ms ---" % (i,ms))
	print f.read()
	i+=1	
print("Done avg[%s requests]: %s ms" %  ( i, (add/100) ) )
