/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_WATCH_TYPES
#define INCLUDED_WATCH_TYPES

#ifdef WATCH_INTERNAL_ACCESS

#ifdef JUST_NEED_TYPES
# include "account.h"
# include "connection.h"
#else
# define JUST_NEED_TYPES
# include "account.h"
# include "connection.h"
# undef JUST_NEED_TYPES
#endif

#define WATCH_FUNC_WATCH 1
#define WATCH_FUNC_UNWATCH 2
#define WATCH_FUNC_WATCHALL 3
#define WATCH_FUNC_UNWATCHALL 4
#define WATCH_FUNC_LIST 5
#define WATCH_FUNC_UNKNOWN 6

#endif

typedef enum
{
    watch_event_login=1,
    watch_event_logout=2
} t_watch_event;

#ifdef WATCH_INTERNAL_ACCESS
typedef struct
{
    t_connection * owner; /* who to notify */
    t_account *    who;   /* when this account login or logout */
} t_watch_pair;
#endif

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_WATCH_PROTOS
#define INCLUDED_WATCH_PROTOS

#define JUST_NEED_TYPES
#include "account.h"
#include "connection.h"
#undef JUST_NEED_TYPES

extern int watchlist_create(void);
extern int watchlist_destroy(void);
extern int watchlist_load(t_connection * c);
extern int watchlist_save(t_connection * c);
extern int watchlist_notify_event(t_account * who, t_watch_event event);
extern int handle_watch_command(t_connection * c, char const * text);
#ifdef ACCT_DYN_UNLOAD
extern unsigned int watch_count_acc_ref(t_account const * account);
#endif

#endif
#endif
