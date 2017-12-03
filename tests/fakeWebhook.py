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

from socket import *

normal_message = '{"update_id":10000,"message":{"date":1441645532,"chat":{"last_name":"Test Lastname","id":80145337,"type": "{type}","first_name":"Test Firstname","username":"Testusername"},"message_id":1365,"from":{"last_name":"Test Lastname","id":1111111,"first_name":"Test Firstname","username":"Testusername"},"text":"{message}"}}'
broken_message = '{"update_id":10000,"message":{"date":1441645532,"chat":{"last_name":"Test Lastname","id":-1,"type": "{type}","first_name":"Test Firstname","username":"Testusername"},"message_id":1365,"from":{"last_name":"Test Lastname","id":1111111,"first_name":"Test Firstname","username":"Testusername"},"text":"{message}"'

#apre un socket verso l'ip locale e manda li una post con i dati passati da parametro
def sockSend(data):
        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect( ("127.0.0.1",80) )
        sock.send(data);
        sock.recv(1024)
        sock.close()

#fai una richiesta http con i dati specificati (aggiunge header http)
def makeHttpReq(body):

    body = "POST /token HTTP/1.1\nUser-Agent: NanaoTestScript\nContent-Type: text/json\nContent-Lenght:"+str(len(body))+"\n\n" + body

    sockSend(body)

#prepera il json per il post e crea la richista http
def post(ctype,message,template, id):

    template = template.replace("{type}", ctype)
    template = template.replace("{message}", (message + " *"+str(id)+"*") )

    makeHttpReq(template)

    print "["+str(id) + "] Posted '"+ message+"' as "+ ctype + " message"
    #print template

#inizio script
while True:
    text = raw_input("Scrivere il messaggio da mandare a nanao:\n p/g x{n_volte} b|msg\n p = privato  g = gruppo  b = messaggio corrotto  [r NON DISPONIBILE]\n-> ")
    parts = text.split("|") # fai in pezzi il messaggio

    if len(parts) < 2:
        parts.insert(0,"p")

    #scegli il tipo di chat
    ctype = ""
    if parts[0].find("p") != -1 or parts[0].find("r") != -1:
         ctype = "private"
    else:
        ctype = "group"

    #cerca se c'e' il numero di volte con cui eseguire  la richista
    times = 1
    if parts[0].find("x") != -1:
        times = int( parts[0].strip() [ parts[0].find("x") +1 :] )

    i = 0
    while i < times:
        try:
            #messaggio corretto
            if parts[0].find("b") == -1:
                post(ctype,parts[1],normal_message,i)
            else:
                post(ctype,parts[1],broken_message,i) #messaggio corrotto
        except:
            print("Errore durante una richiesta, operazione annullata.")
            break

        i += 1
    print "\n"
