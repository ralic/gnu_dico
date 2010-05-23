#  This file is part of GNU Dico.
#  Copyright (C) 2008, 2009 Wojciech Polak
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

from django.template import Library

register = Library ()

def media_prefix ():
    """Return the string contained in the setting MEDIA_URL."""
    try:
        from django.conf import settings
    except ImportError:
        return ''
    return settings.MEDIA_URL

media_prefix = register.simple_tag (media_prefix)