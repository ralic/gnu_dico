#  This file is part of GNU Dico.
#  Copyright (C) 2008, 2009, 2010 Wojciech Polak
#
#  GNU Dico is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3, or (at your option)
#  any later version.
#
#  GNU Dico is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>.

from django.conf import settings
from django.shortcuts import render_to_response
from django.utils.translation import ugettext as _

import re
import hashlib
import memcache
import socket
import dicoclient
from wit import wiki2html

def index (request):
    page = {}
    selects = {}
    mtc = {}
    result = {}
    markup_style = 'default'
    port = 2628

    sid = request.COOKIES.get ('dicoweb_sid', '')
    request.session.set_expiry (0)

    accept_lang = request.META.get ('HTTP_ACCEPT_LANGUAGE', '').split (',')
    for i, lang in enumerate (accept_lang):
        accept_lang[i] = lang.split (';')[0]

    if 'server' in request.session:
        server = request.session['server']
    else:
        server = settings.DICT_SERVERS[0]
    server = request.GET.get ('server', server)
    if not server in settings.DICT_SERVERS:
        server = settings.DICT_SERVERS[0]
    request.session['server'] = server;

    if len (settings.DICT_SERVERS) > 1:
        selects['sv'] = HtmlOptions (settings.DICT_SERVERS, server)

    key = hashlib.md5 ("%s/%s" % (sid, server.encode ('ascii',
                                                      'backslashreplace')))
    sid = key.hexdigest ()

    type = 'search'
    if 'define' in request.GET:
        type = 'define'
    else:
        type = 'search'

    database = request.GET.get ('db', '*')
    strategy = request.GET.get ('strategy', '.')

    mc = memcache.Client (settings.MEMCACHE_SERVERS)

    key_databases = str ('dicoweb/databases/' + server)
    key_strategies = str ('dicoweb/strategies/' + server)

    databases = mc.get (key_databases)
    strategies = mc.get (key_strategies)

    if server.find (':') != -1:
        s = server.split (':', 1)
        server = s[0]
        port = int (s[1])

    if not databases or not strategies:
        dc = dicoclient.DicoClient ()
        try:
            dc.open (server, port)
            dc.timeout = settings.DICT_TIMEOUT
            databases = dc.show_databases ()['databases']
            strategies = dc.show_strategies ()['strategies']
            dc.close ()
        except (socket.timeout, socket.error, dicoclient.DicoNotConnectedError):
            return render_to_response ('index.html', { 'selects': selects })

        mc.set (key_databases, databases, time=86400)
        mc.set (key_strategies, strategies, time=86400)

    for s in strategies:
        s[1] = _(s[1])
    databases.insert (0, ['!', _('First match')])
    databases.insert (0, ['*', _('All')])
    strategies.insert (0, ['.', _('Default')])

    selects['db'] = HtmlOptions (databases, database)
    selects['st'] = HtmlOptions (strategies, strategy)

    q = request.GET.get ('q', '')

    if 'q' in request.GET and q != '':
        langkey = '*';
        if database == '*': langkey = ','.join (accept_lang)

        key = hashlib.md5 ("%s:%d/%s/%s/%s/%s/%s" %
                           (server, port, langkey, type, database, strategy,
                            q.encode ('ascii', 'backslashreplace')))
        key = key.hexdigest ()
        result = mc.get ("dicoweb/" + key)

        if not result:
            try:
                dc = dicoclient.DicoClient ()
                dc.timeout = settings.DICT_TIMEOUT
                dc.open (server, port)
                dc.option ('MIME')

                if database == '*' and 'lang' in dc.server_capas:
                    dc.option ('LANG', ': ' + ' '.join (accept_lang))
                if 'markup-wiki' in dc.server_capas:
                    if dc.option ('MARKUP', 'wiki'):
                        markup_style = 'wiki'

                if database == 'dbinfo':
                    result = dc.show_info (q)
                elif type == 'define':
                    result = dc.define (database, q)
                else:
                    result = dc.match (database, strategy, q)
                dc.close ()

                result['markup_style'] = markup_style
                mc.set ("dicoweb/" + key, result, time=3600)

            except (socket.timeout, socket.error,
                    dicoclient.DicoNotConnectedError):
                return render_to_response ('index.html',
                                           { 'selects': selects })

        # get last match results
        if sid and type == 'search':
            mc.set ("dicoweb/%s/last_match" % sid, key, time=3600)
        else:
            key = mc.get ("dicoweb/%s/last_match" % sid)
        if key != None: mtc = mc.get ("dicoweb/" + key)

        mtc['dbnames'] = {}
        if mtc.has_key ('matches'):
            for m in mtc['matches']:
                for d in databases:
                    if d[0] == m:
                        mtc['dbnames'][m] = d[1]
                        break

        if database == 'dbinfo': q = ''
        if q != '': page['title'] = q + ' - '

    if result.has_key ('definitions'):
        rx1 = re.compile ('{+(.*?)}+', re.DOTALL)
        for df in result['definitions']:
            if df.has_key ('content-type') \
                    and df['content-type'].startswith ('text/x-wiki'):
                lang = df['x-wiki-language'] \
                    if df.has_key ('x-wiki-language') else 'en'
                wikiparser = wiki2html.HtmlWiktionaryMarkup (text=df['desc'],
                                                             html_base='?q=',
                                                             lang=lang)
                wikiparser.parse ()
                df['desc'] = str (wikiparser)
                df['format_html'] = True
            else:
                df['desc'] = re.sub ('_(.*?)_', '<b>\\1</b>', df['desc'])
                df['desc'] = re.sub (rx1, __subs1, df['desc'])

    return render_to_response ('index.html', { 'page': page, 'q': q,
                                               'mtc': mtc, 'result': result,
                                               'selects': selects })

def __subs1 (match):
    s = re.sub (r' +', ' ', match.group (1))
    return '<a href="?q=%s" title="Search for %s">%s</a>' \
        % (s.replace ('\n', ''), s.replace ('\n', ''), s)

class HtmlOptions:
    def __init__ (self, lst=[], value=''):
        self.lst = lst
        self.value = value
    def html (self):
        buf = []
        for opt in self.lst:
            if len (opt) == 2:
                if opt[0] == self.value:
                    buf.append ('<option value="%s" selected="selected">%s</option>' % (opt[0], opt[1]))
                else:
                    buf.append ('<option value="%s">%s</option>' % (opt[0],
                                                                    opt[1]))
            else:
                if opt == self.value:
                    buf.append ('<option value="%s" selected="selected">%s</option>' % (opt, opt))
                else:
                    buf.append ('<option value="%s">%s</option>' % (opt, opt))
        return '\n'.join (buf)
