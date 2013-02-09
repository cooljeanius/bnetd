/* 
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2002  Bart³omiej Butyn (bartek@milc.com.pl)
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
#ifndef INCLUDED_FRIENDS_TYPES
#define INCLUDED_FRIENDS_TYPES

#ifdef FRIENDS_INTERNAL_ACCESS

#define MAX_FUNC_LEN 20
#define MAX_EVENT_STR_LEN 20
#define FRIENDS_ALL_EVENTS (friends_event_login | friends_event_logout | friends_event_create_game | friends_event_join_game | friends_event_finished_game | friends_event_join_channel)
#define FRIENDS_NO_EVENTS (!FRIENDS_ALL_EVENTS)

#define FRIENDS_FUNC_ADD 1
#define FRIENDS_FUNC_DEL 2
#define FRIENDS_FUNC_FLUSH 3
#define FRIENDS_FUNC_PROMOTE 4
#define FRIENDS_FUNC_DEMOTE 5
#define FRIENDS_FUNC_MSG 6
#define FRIENDS_FUNC_LIST 7
#define FRIENDS_FUNC_LIST_ALL 8
#define FRIENDS_FUNC_HELP 9
#define FRIENDS_FUNC_UNKNOWN 10

#ifdef JUST_NEED_TYPES
# include "connection.h"
#else
# define JUST_NEED_TYPES
# include "connection.h"
# undef JUST_NEED_TYPES
#endif


#endif

typedef enum
{
    friends_event_login = 1,
    friends_event_logout = 2,
    friends_event_create_game = 4,
    friends_event_join_game = 8,
    friends_event_finished_game = 16,
    friends_event_join_channel = 32
} t_friends_event;

#ifdef FRIENDS_INTERNAL_ACCESS
typedef struct
{
    t_connection *  owner;
    t_account *     who;
    t_friends_event what;
} t_friends_pair;
#endif

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_FRIENDS_PROTOS
#define INCLUDED_FRIENDS_PROTOS

#define JUST_NEED_TYPES
#include "account.h"
#include "connection.h"
#undef JUST_NEED_TYPES

extern int friendslist_create(void);
extern int friendslist_destroy(void);
extern int friendslist_load(t_connection * c);
extern int friendslist_save(t_connection * c);
extern int friends_notify_event(t_account * who, t_friends_event event, char const * info);
extern int handle_friends_command(t_connection * t, char const * text);
#ifdef ACCT_DYN_UNLOAD
extern unsigned int friends_count_acc_ref(t_account const * account);
#endif


#endif
#endif
