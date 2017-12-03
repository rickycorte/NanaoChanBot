#!/usr/bin/env python
# -*- coding: utf-8 -*-


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

import argparse
import sys
import malHelper
import json
import os
import urllib2
import dbManager
import clog
import sys

#recupera il contentuto tra due separatori
def getHtmlContent(htm, open = ">", close = "<"):
    beg = htm.find(open)
    if beg == int(-1): #evita parse inutuli se non trovi l'inizio
        return "null"

    beg += len(open)
    end = htm.find(close, beg)

    #print(open[:10] + " beg: "+str(beg)+" end: "+str(end))

    if end == int(-1):
        return "null"

    return htm[beg:end].strip().replace('"','').replace("&amp;","and")

#crea un array contente i vari blocchi
def getArrayFromHtml(htm, separator):
    parts = htm.split(separator)
    arr = []
    for p in parts:
        if p:
            rs = getHtmlContent(p)
            if rs:
                arr.append(rs)
    return arr

#parse della pagina
def ParsePageOfID(id):
    dic = {}
    dic["malID"] = id;

    #scarica html
    req = urllib2.Request("https://myanimelist.net/anime/"+id)
    htm = ""
    try:
        res = urllib2.urlopen(req)
        htm = res.read()
        #tieni solo il contenuto per velocizzare il parse
        html = getHtmlContent(htm, '<div id=contentWrapper','<!-- end of contentWrapper' )
    except Exception as e:
        clog.error("Request error, cant find page")
        return None

    #parser core
    dic["title"] = getHtmlContent(htm ,'<span itemprop="name">')
    dic["title_low"] = dic["title"].lower() #usato per la ricerca nel db
    dic["entitle"] = getHtmlContent(htm ,'<span class="dark_text">English:</span>')
    dic["entitle_low"] = dic["entitle"].lower() #usato per la ricerca nel db
    dic["synonyms"] = getHtmlContent(htm ,'<span class="dark_text">Synonyms:</span>')
    dic["episodes"] = getHtmlContent(htm ,'<span class="dark_text">Episodes:</span>')
    dic["status"] = getHtmlContent(htm ,'<span class="dark_text">Status:</span>')
    dic["duration"] = getHtmlContent(htm ,'<span class="dark_text">Duration:</span>')
    dic["rating"] = getHtmlContent(htm ,'<span class="dark_text">Rating:</span>')
    dic["source"] = getHtmlContent(htm ,'<span class="dark_text">Source:</span>')
    dic["score"] = getHtmlContent(htm ,'<span class="dark_text">Source:</span>')
    dic["score"] = getHtmlContent(htm ,'<span itemprop="ratingValue">')
    dic["votes"] = getHtmlContent(htm ,'<span itemprop="ratingCount">')

    dic["type"] = getHtmlContent( getHtmlContent(htm,'<span class="dark_text">Type:</span>','</div>') ) #la doppia funzione serve per rimuovere il link
    dic["season"] = getHtmlContent( getHtmlContent(htm,'<span class="dark_text">Premiered:</span>','</div>') ) #la doppia funzione serve per rimuovere il link

    dic["genres"] = getArrayFromHtml( getHtmlContent(htm ,'<span class="dark_text">Genres:</span>','</div>'), ",")
    dic["adaptation"] = getArrayFromHtml(getHtmlContent(htm ,'<td nowrap="" valign="top" class="ar fw-n borderClass">Adaptation:</td><td width="100%" class="borderClass">','</tr>'),",")
    dic["prequel"] = getArrayFromHtml(getHtmlContent(htm ,'<td nowrap="" valign="top" class="ar fw-n borderClass">Prequel:</td><td width="100%" class="borderClass">','</tr>'),",")
    dic["sequel"] = getArrayFromHtml(getHtmlContent(htm ,'<td nowrap="" valign="top" class="ar fw-n borderClass">Sequel:</td><td width="100%" class="borderClass">','</tr>'),",")

    clog.log("Parsed "+id)
    return dic

#salva il json, solo debugfro
def saveJson(name, content):
    if content is None:
        return

    if not os.path.exists("test"):
        os.makedirs("test")
    fl = open("test/"+name,"w")
    json.dump(content,fl,ensure_ascii=False)
    fl.close()


def ParseByIds(ids):
    parts = ids.split("|")
    for p in parts:
        if p:
          res = ParsePageOfID(p)
          dbManager.addAnimeToDB(res)
          #saveJson(p+".json", res)
    clog.log("Id parse completed")



def ParseByNames(names):
    parts = names.split("|")
    ids = ""
    for p in parts:
        if p:
            res = malHelper.getIDByName(p.strip())
            if res is not None:
                ids += res + "|"
    if not "|" in ids:
        clog.log("No Animes");
        sys.exit(1)

    ParseByIds(ids)



parser = argparse.ArgumentParser()
#recupera un lista di nomi/id separati dal , passata come parametro
parser.add_argument('-n', action="store",dest="names", help="pass a title list separated with |")
parser.add_argument('-i', action="store", dest= "ids", help="pass a mal id list separated with |")
parser.add_argument('-f', action="store", dest="file", help="grab name list from file")

args = parser.parse_args()

if args.names:
    ParseByNames(args.names)
if args.ids:
    ParseByIds(args.ids)
if args.file:
    ParseByNames(open(args.file,'r').read())

if not args.names and not args.ids:
    clog.error("This script must have parameters to be run!")
    sys.exit()
