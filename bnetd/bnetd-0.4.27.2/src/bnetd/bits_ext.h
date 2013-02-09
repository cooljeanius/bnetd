/*
 * Copyright (C) 2000  Marco Ziech (mmz@gmx.net)
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

#ifdef WITH_BITS

#ifndef INCLUDED_BITS_EXT_TYPES
#define INCLUDED_BITS_EXT_TYPES

#ifndef JUST_NEED_TYPES
# define JUST_NEED_TYPES
# include "common/list.h"
# include "common/bnethash.h"
# undef JUST_NEED_TYPES
#else
# include "common/list.h"
# include "common/bnethash.h"
#endif

typedef struct t_va_locklist_entry {
	char username[MAX_USER_NAME+1]; /* FIXME: Is this "+1" necesary? */
	int uid;
} t_va_locklist_entry;

typedef struct t_bits_channellist_entry {
	int channelid;
} t_bits_channellist_entry;

typedef enum {
	bits_to_master, /* connection to uplink server */
	bits_to_slave,  /* connection to other bits clients */
	bits_loop       /* bits loopback connection */
} t_bits_connection_type;

typedef struct {
    t_bits_connection_type  type;
    t_uint16                myaddr; /* FIXME: Hmmm ... remove for the sake of redundancy :) */
    t_uint16		        peeraddr;
    t_list     		      * va_locklist;
    t_list		          * channellist;
} t_bits_connection_extension;

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_BITS_EXT_PROTOS
#define INCLUDED_BITS_EXT_PROTOS

#define JUST_NEED_TYPES
#include "connection.h"
#undef JUST_NEED_TYPES

extern int create_bits_ext(t_connection *c, t_bits_connection_type type);
extern void destroy_bits_ext(t_connection *c);
extern t_va_locklist_entry * bits_va_locklist_byname(t_connection * c, char const * name);
extern t_va_locklist_entry * bits_va_locklist_byuid(t_connection * c, int uid);
extern int bits_va_locklist_add(t_connection * c, const char *name, int uid);
extern int bits_va_locklist_del(t_connection * c, t_va_locklist_entry * e);
extern t_connection * bits_va_locklist_is_locked_by(int uid);

extern int bits_ext_channellist_add(t_connection * c, int channelid);
extern int bits_ext_channellist_del(t_connection * c, int channelid);
extern t_bits_channellist_entry * bits_ext_channellist_find(t_connection * c, int channelid);
extern t_connection * bits_ext_channellist_is_needed(int channelid);

extern int bits_ext_set_peeraddr(t_connection * c, t_uint16 peeraddr);
extern t_uint16 bits_ext_get_peeraddr(t_connection * c);

#endif
#endif

#endif
