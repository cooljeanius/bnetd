/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_REALM_TYPES
#define INCLUDED_REALM_TYPES

typedef struct realm
#ifdef REALM_INTERNAL_ACCESS
{
    char const *   name;
    char const *   description;
    unsigned int   sessionnum;
    unsigned int   active;
    unsigned int   ip;
    unsigned short port;
    unsigned int   player_number;
    unsigned int   game_number;
}
#endif
t_realm;

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_REALM_PROTOS
#define INCLUDED_REALM_PROTOS

#define JUST_NEED_TYPES
#include "common/list.h"
#include "connection.h"
#undef JUST_NEED_TYPES

extern char const * realm_get_name(t_realm const * realm) PURE_ATTR();
extern char const * realm_get_description(t_realm const * realm) PURE_ATTR();
extern unsigned short realm_get_port(t_realm const * realm) PURE_ATTR();
extern unsigned int realm_get_ip(t_realm const * realm) PURE_ATTR();
extern int realm_set_name(t_realm * realm, char const * name);
extern unsigned int realm_get_active(t_realm const * realm);
extern unsigned int realm_get_player_number(t_realm const * realm);
extern int realm_add_player_number(t_realm * realm, int number);
extern unsigned int realm_get_game_number(t_realm const * realm);
extern int realm_add_game_number(t_realm * realm, int number);
extern int realm_set_active(t_realm * realm, unsigned int active);
extern int realm_active(t_realm * realm, t_connection * c);
extern int realm_deactive(t_realm * realm);

extern int realmlist_create(char const * filename);
extern int realmlist_destroy(void);
extern t_realm * realmlist_find_realm(char const * realmname);
extern t_realm * realmlist_find_realm_by_ip(unsigned long ip); /* ??? */
extern t_list * realmlist(void);

#endif
#endif
