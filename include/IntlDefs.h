/*
RealTimeBattle, a robot programming game for Unix
Copyright (C) 1998-2000  Erik Ouchterlony and Ragnar Ouchterlony

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __INTLDEFS__
#define __INTLDEFS__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(str)                  dgettext("RealTimeBattle",str)
# ifdef gettext_noop
#  define N_(str)                gettext_noop(str)
# else
#  define N_(str)                (str)
# endif
#else
# define _(str)                  (str)
# define N_(str)                 (str)
# define gettext(str)            (str)
# define dgettext(dom,str)       (str)
# define dcgettext(dom,str,type) (str)
# define textdomain(str)         (str)
# define bindtextdomain(dom,dir) (dom)
#endif

#endif
