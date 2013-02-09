/*
 * Copyright (C) 2001  Erik Latoshek [forester] (laterk@inbox.lv)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef __WINMAIN_H__
#define __WINMAIN_H__

#include <stdarg.h>

#define JUST_NEED_TYPES
#include "common/eventlog.h"
#undef JUST_NEED_TYPES

int extern gui_printf(const char *format, ...);
int extern gui_lprintf(t_eventlog_level l, const char *format, ...);
int extern gui_lvprintf(t_eventlog_level l, const char *format, va_list);

#endif