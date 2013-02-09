/*
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
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
#define CONNECTION_INTERNAL_ACCESS
#include "common/setup_before.h"
#ifdef WITH_BITS
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#include <errno.h>
#include "compat/strerror.h"
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "connection.h"
#include "bits.h"
#include "bits_ext.h"
#include "common/list.h"
#include "common/setup_after.h"

extern int create_bits_ext(t_connection *c, t_bits_connection_type type) {
	t_bits_connection_extension * e;
	if (c == NULL) {
		eventlog(eventlog_level_error,"create_bits_ext","got NULL connection");
		return -1;
	}
	e = malloc(sizeof(t_bits_connection_extension));
	if (!e) {
		eventlog(eventlog_level_error,"create_bits_ext","malloc failed: %s",strerror(errno));
		return -1;
	}
	e->type	= type;
	if (bits_master) {
		e->myaddr = BITS_ADDR_MASTER;
	} else {
		if (!bits_uplink_connection->bits) {
			e->myaddr = BITS_ADDR_PEER;
		} else {
			e->myaddr = bits_uplink_connection->bits->myaddr;
		}
	}
	e->va_locklist = list_create();
	e->channellist = list_create();
	e->peeraddr = 0;
	c->bits = e;
	return 0;
}

extern void destroy_bits_ext(t_connection *c)
{
    t_elem * curr;

    if (!c) {
	eventlog(eventlog_level_error,"destroy_bits_ext","got NULL connection");
	return;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"destroy_bits_ext","got connection without BITS extension");
	return;
    }
        
    LIST_TRAVERSE(c->bits->va_locklist,curr) {
        t_va_locklist_entry * e = elem_get_data(curr);
        
	list_remove_elem(c->bits->va_locklist,curr);
	free(e);
    }
    list_purge(c->bits->va_locklist);
    list_destroy(c->bits->va_locklist);

    LIST_TRAVERSE(c->bits->channellist,curr) {
        t_bits_channellist_entry * e = elem_get_data(curr);
        
	list_remove_elem(c->bits->channellist,curr);
	free(e);
    }
    list_purge(c->bits->channellist);
    list_destroy(c->bits->channellist);

    free(c->bits);
    return;
}


extern t_va_locklist_entry * bits_va_locklist_byname(t_connection * c, char const * name)
{
    t_elem * curr;
    
    if (!c) {
	eventlog(eventlog_level_error,"bits_va_locklist_byname","got NULL connection");
	return NULL;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_va_locklist_byname","got NULL bits connection extension");
	return NULL;
    }
    if (!name) {
	eventlog(eventlog_level_error,"bits_va_locklist_byname","got NULL name");
	return NULL;
    }
    LIST_TRAVERSE(c->bits->va_locklist,curr) {
        t_va_locklist_entry *e = elem_get_data(curr);
        
	if (strcasecmp((char *)&e->username,name)==0)
	    return e;
    }
    return NULL;
}

extern t_va_locklist_entry * bits_va_locklist_byuid(t_connection * c, int uid)
{
    t_elem * curr;
    
    if (!c) {
	eventlog(eventlog_level_error,"bits_va_locklist_byuid","got NULL connection");
	return NULL;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_va_locklist_byuid","got NULL bits connection extension");
	return NULL;
    }
    LIST_TRAVERSE(c->bits->va_locklist,curr) {
        t_va_locklist_entry *e = elem_get_data(curr);

	if (uid == e->uid)
	    return e;
    }
    return NULL;
}

extern int bits_va_locklist_add(t_connection * c, const char *name, int uid) {
    t_va_locklist_entry *e;

    if (!c) {
	eventlog(eventlog_level_error,"bits_va_locklist_add","got NULL connection");
	return -1;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_va_locklist_add","got NULL bits connection extension");
	return -1;
    }
    if (!name) {
	eventlog(eventlog_level_error,"bits_va_locklist_add","got NULL name");
	return -1;
    }
    if (bits_va_locklist_byname(c,name)) return 0;
    if (bits_va_locklist_byuid(c,uid)) return 0;

    e = malloc(sizeof(t_va_locklist_entry));
    if (!(e = malloc(sizeof(t_va_locklist_entry)))) {
	eventlog(eventlog_level_error,"bits_va_locklist_add","malloc failed: %s",strerror(errno));	
	return -1;
    }
    e->uid = uid;
    strncpy(e->username,name,sizeof(e->username)-1);
    e->username[sizeof(e->username)-1] = '\0'; /* must terminate because strncpy() doesn't guarantee it */
    list_append_data(c->bits->va_locklist,e);
    return 0;
}

extern int bits_va_locklist_del(t_connection * c, t_va_locklist_entry * e) {
    if (!c) {
	eventlog(eventlog_level_error,"bits_va_locklist_del","got NULL connection");
	return -1;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_va_locklist_del","got NULL bits connection extension");
	return -1;
    }
    if (!e) {
	eventlog(eventlog_level_error,"bits_va_locklist_del","got NULL locklist entry");
	return -1;
    }
    list_remove_data(c->bits->va_locklist,e);
    list_purge(c->bits->va_locklist);
    return 0;
}

extern t_connection * bits_va_locklist_is_locked_by(int uid)
{
    t_connection *c;
    t_elem const * curr;

    LIST_TRAVERSE_CONST(connlist(),curr)
    {
	c = elem_get_data(curr);
	if (conn_get_class(c) == conn_class_bits) {
	    if (bits_va_locklist_byuid(c,uid)) return c;
	}
    }
    return NULL;
}

extern int bits_ext_channellist_add(t_connection * c, int channelid)
{
    t_bits_channellist_entry * e;

    if (!c) {
	eventlog(eventlog_level_error,"bits_ext_channellist_add","got NULL connection");
	return -1;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_ext_channellist_add","bits connection extension is NULL");
	return -1;
    }
    if (!(e = malloc(sizeof(t_bits_channellist_entry)))) {
	eventlog(eventlog_level_error,"bits_ext_channellist_add","malloc failed: %s",strerror(errno));
	return -1;
    }
    e->channelid = channelid;
    list_append_data(c->bits->channellist,e);
    return 0;
}

extern int bits_ext_channellist_del(t_connection * c, int channelid)
{
    t_bits_channellist_entry * e;

    e = bits_ext_channellist_find(c,channelid);
    if (e) {
	list_remove_data(c->bits->channellist,e);
	list_purge(c->bits->channellist);
	return 0;	
    } else {
	eventlog(eventlog_level_error,"bits_ext_channellist_del","channelid #%u not found in channellist",channelid);
	return -1;
    }
}

extern t_bits_channellist_entry * bits_ext_channellist_find(t_connection * c, int channelid)
{
    t_elem * curr;

    if (!c) {
	eventlog(eventlog_level_error,"bits_ext_channellist_del","got NULL connection");
	return NULL;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_ext_channellist_del","bits connection extension is NULL");
	return NULL;
    }
    LIST_TRAVERSE(c->bits->channellist,curr) {
        t_bits_channellist_entry * e = elem_get_data(curr);
        
	if (e->channelid == channelid)
	    return e;
    }
    return NULL;
}

extern t_connection * bits_ext_channellist_is_needed(int channelid)
{
    t_connection *c;
    t_elem const * curr;

    LIST_TRAVERSE_CONST(connlist(),curr)
    {
	c = elem_get_data(curr);
	if (conn_get_class(c) == conn_class_bits) {
	    if (bits_ext_channellist_find(c,channelid))
		return c;
	}
    }
    return NULL;
}

extern int bits_ext_set_peeraddr(t_connection * c, t_uint16 peeraddr)
{
    if (!c) {
	eventlog(eventlog_level_error,"bits_ext_set_peeraddr","got NULL connection");
	return -1;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_ext_set_peeraddr","got NULL BITS connection extension");
	return -1;
    }
    c->bits->peeraddr = peeraddr;
    return 0;
}

extern t_uint16 bits_ext_get_peeraddr(t_connection * c)
{
    if (!c) {
	eventlog(eventlog_level_error,"bits_ext_set_peeraddr","got NULL connection");
	return 0;
    }
    if (!c->bits) {
	eventlog(eventlog_level_error,"bits_ext_set_peeraddr","got NULL BITS connection extension");
	return 0;
    }
    return c->bits->peeraddr;
}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif

