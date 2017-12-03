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

import urllib2
import base64
import re
import clog

#crea richiesta post per le api mal al fine di cercare l id
def malPost(name):
    url = "https://myanimelist.net/api/anime/search.xml?q="
    url+= urllib2.quote(name)
    req = urllib2.Request(url)
    log = base64.b64encode('%s:%s' % ("username","pwd"))
    req.add_header("Authorization", "Basic %s" % log)
    res = urllib2.urlopen(req)
    if res.getcode() == int(204) :
        clog.error( "Unable to find specified anime ("+ name +"), aborting operation")
        return None

    return res.read()

#restituisce il contentuto del tag xml seguente all index
def getItemAfterIndex(str, index):
    beg = str.find(">", index)+1
    end = str.find("<", beg)
    return str[beg:end].strip().replace("\n","").lower()

#recupera tutti i ceontenuti dei tag xml dopo gli index
def getAllItems(str, indexes, forceSingle = False):
    res = []
    i = 0
    while i < len(indexes):
        res.append(getItemAfterIndex(str,indexes[i]))
        #salta il tag di chiusura
        if not forceSingle:
            i += 2
        else:
            i += 1
    return res


def getIDByName(name):

    xml = malPost(name)
    if xml is None:
        return None

    #cerca tutti i titoli e l'id
    titles =  getAllItems(xml, [m.start() for m in re.finditer("title", xml)] )
    ids = getAllItems(xml, [m.start() for m in re.finditer("<id", xml)] , True)
    syn = getAllItems(xml, [m.start() for m in re.finditer("synonyms", xml)])
    enTitles = getAllItems(xml, [m.start() for m in re.finditer("english", xml)])

    name = name.lower()

    i = 0
    while i  < len(titles):
        if name in titles[i] or name in enTitles[i]:
            clog.log (name + " (" +titles[i]+")"+": "+ids[i])
            return ids[i]
        i += 1

    i = 0
    while i < len(titles):
        if name in syn[i]:
            print "2 "+name + " (" +titles[i]+")"+": "+ids[i]
            return ids[i]
        i+=1

    return None
