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
#define WATCH_INTERNAL_ACCESS
#include "common/setup_before.h"
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include "common/field_sizes.h"
#include "common/list.h"
#include "account.h"
#include "connection.h"
#include "common/eventlog.h"
#include "common/util.h"
#include "message.h"
#include "watch.h"
#include "common/setup_after.h"

static int identify_watch_function(char const * funcstr);
static int watch_func_add(t_connection * c, char const * nick);
static int watch_func_del(t_connection * c, char const * nick);
static int watch_func_add_all(t_connection * c);
static int watch_func_del_all(t_connection * c);
static int watch_del_pair(t_connection * owner, t_account * who);
static int watch_add_pair(t_connection * owner, t_account * who);
static int watch_is_pair(t_connection * c, t_account * a);
static int watch_del_all(t_connection * c);

static t_list * watchlist_head = NULL;


extern int watchlist_create(void)
{
    if (!(watchlist_head = list_create()))
        return -1;

    return 0;
}


extern int watchlist_destroy(void)
{
    t_elem *       curr;
    t_watch_pair * pair;
    
    if (watchlist_head)
    {
	LIST_TRAVERSE(watchlist_head,curr)
	{
	    pair = elem_get_data(curr);
	    if (!pair) /* should not happen */
	    {
		eventlog(eventlog_level_error,"watchlist_destroy","watchlist contains NULL item");
		continue;
	    }
	    if (list_remove_elem(watchlist_head,curr)<0)
        	eventlog(eventlog_level_error,"watchlist_destroy","could not remove item from list");
	    free(pair);
	}
	if (list_destroy(watchlist_head)<0)
	    return -1;
	watchlist_head = NULL;
    }
    
    return 0;
}


extern int watchlist_load(t_connection * c)
{
    t_account *		owner;
    t_account *		who;
    char *		watchp;
    char *		watch;
    char const *	twatch;
    char *		ttok;
    
    if (!c)
    {
		eventlog(eventlog_level_error,"watchlist_load","got NULL connection");
		return -1;
    }
    
    if (!(owner = conn_get_account(c)))
    {
		eventlog(eventlog_level_error,"watchlist_load","got NULL owner");
		return -1;
    }
    if (!(twatch = account_get_watch(owner)))
    {
		eventlog(eventlog_level_error,"warchlist_load","got NULL twatch");
		return -1;
    }

    if (strlen(twatch)<1)
    {
		account_unget_watch(twatch);
		return 0;
    }
    if (!(watch = strdup(twatch)))
    {
		eventlog(eventlog_level_error,"watchlist_load","could not allocate memory for watch");
		account_unget_watch(twatch);
		return -1;
    }
    account_unget_watch(twatch);

    watchp = watch;
    while ((ttok = strsep(&watch,",")))
    {
		if ((who = accountlist_find_account(ttok)))
	    	watch_add_pair(c,who);
		else
		    eventlog(eventlog_level_warn,"watchlist_load","got NULL who");
    }
    free(watchp);

    return 0;
}


extern int watchlist_save(t_connection * c)
{
    t_elem const * curr;
    t_watch_pair * pair;
    char *         watch;
    char const *   tname;
    int            first;
    char * 	   watchp;
		
    
    if (!c)
    {
        eventlog(eventlog_level_error,"watchlist_save","got null connection");
        return -1;
    }

    if (!(watch = strdup("")))
    {
		eventlog(eventlog_level_error,"watchlist_save","not enought memory to initialize watch");
		return -1;
    }

    first = 1;
    LIST_TRAVERSE_CONST(watchlist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair) /* shoud not happen */
	{
	    eventlog(eventlog_level_error,"watchlist_save","watchlist contains NULL item");
	    return -1;
	}
	if (c == pair->owner)
	{
	    if (!(tname = account_get_name(pair->who)))
	    {
			eventlog(eventlog_level_error,"watchlist_save","could not get account name");
	        continue;
	    }
	    if (first) 
	    {
			watchp = watch;
			if (!(watch = realloc(watch,strlen(tname)+1)))
			{
		    	eventlog(eventlog_level_error,"watchlist_save","could not allocate memory for watch");
			    account_unget_name(tname);
			    free(watchp);
			    return -1;
			}
			sprintf(watch,"%s",tname);
			first = 0;
		}
	    else
	    {
		watchp = watch;
		if (!(watch = realloc(watch,strlen(watch)+1+strlen(tname)+1))) /* old watch + "," nickname */
		{
		    eventlog(eventlog_level_error,"watchlist_save","could not reallocate memory for twatch");
		    account_unget_name(tname);
		    free(watchp);
		    return -1;
		}
		sprintf(watch,"%s,%s",watch,tname);
	    }
	    account_unget_name(tname);
	}
    }

    if (account_set_strattr(conn_get_account(c),"BNET\\acct\\watch",watch)<0)
    {
	eventlog(eventlog_level_error,"watchlist_save","could not set strattr");
	free(watch);
	return -1;
    }
    free(watch);
    if (watch_del_all(c)<0)
	eventlog(eventlog_level_error,"watchlist_save","could not purge watchlist");

    return 0;
}
	    

extern int watchlist_notify_event(t_account * who, t_watch_event event)
{
    t_elem const * curr;
    t_watch_pair * pair;
    char const *   tname;
    char           msgtemp[MAX_MESSAGE_LEN];
    
    if (!who)
    {
	eventlog(eventlog_level_error,"watchlist_notify_event","got NULL who");
	return -1;
    }

    if (!(tname = account_get_name(who)))
    {
	eventlog(eventlog_level_error,"watchlist_notify_event","could not get account name");
	return -1;
    }

    switch (event)
    {
    case watch_event_login:
	sprintf(msgtemp,"%.64s has logged in.",tname);
	break;
    case watch_event_logout:
	sprintf(msgtemp,"%.64s has logged out.",tname);
	break;
    default:
	eventlog(eventlog_level_error,"watchlist_notify_event","got unknown event %u",(unsigned int)event);
	account_unget_name(tname);
	return -1;
    }
    account_unget_name(tname);
    
    LIST_TRAVERSE_CONST(watchlist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair) /* should not happen */
	{
	    eventlog(eventlog_level_error,"watchlist_notify_event","watchlist contains NULL item");
	    return -1;
	}
	if (pair->owner && (pair->who==who || !pair->who))
	    message_send_text(pair->owner,message_type_info,pair->owner,msgtemp);
    }
    
    return 0;
}


extern int handle_watch_command(t_connection * c, char const * text)
{

    int i;

    for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
    for (; text[i]==' '; i++);

    switch(identify_watch_function(text))
    {
	case WATCH_FUNC_WATCH:
	    watch_func_add(c,&text[i]);
	    break;
	case WATCH_FUNC_UNWATCH:
	    watch_func_del(c,&text[i]);
	    break;
	case WATCH_FUNC_WATCHALL:
	    watch_func_add_all(c);
	    break;
	case WATCH_FUNC_UNWATCHALL:
	    watch_func_del_all(c);
	    break;
	default:
	    message_send_text(c,message_type_info,c,"The command is incorrect.");
	    eventlog(eventlog_level_error,"handle_watch_comand","got unknown function");
    }

    return 0;
}


static int identify_watch_function(char const * funcstr)
{
    if (strstart(funcstr,"/watch")==0)
	return WATCH_FUNC_WATCH;
    if (strstart(funcstr,"/unwatch")==0)
	return WATCH_FUNC_UNWATCH;
    if (strstart(funcstr,"/watchall")==0)
	return WATCH_FUNC_WATCHALL;
    if (strstart(funcstr,"/unwatchall")==0)
	return WATCH_FUNC_UNWATCHALL;

    return WATCH_FUNC_UNKNOWN;
}

	
static int watch_func_add(t_connection * c, char const * nick)
{
    t_account *  owner;
    t_account *  who;
    char         tmsg[MAX_MESSAGE_LEN];
    unsigned int i;
    int          remove_sep;
    t_elem const * curr;
    t_watch_pair * pair;
    char const * whonick;
    unsigned int counter;
    
    if (!(owner = conn_get_account(c)))
    {
        eventlog(eventlog_level_error,"watch_func_add","got NULL owner");
        return -1;
    }

    if (nick[0]=='\0')
    {
	message_send_text(c,message_type_info,c,"People on your watch list:");
	i = 0;
	remove_sep = 0;
	
	counter = 0;
	LIST_TRAVERSE_CONST(watchlist_head,curr)
	{
	    pair = elem_get_data(curr);
	    if (!pair) /* should not happen */
	    {
                eventlog(eventlog_level_error,"watch_func_add","watchlist contains NULL item");
                return -1;
            }
	    if (c == pair->owner)
	    {
		counter++;
		if (!(whonick = account_get_name(pair->who)))
		{
		    eventlog(eventlog_level_error,"watch_func_add","could not get account name");
		    continue;
		}
	        if (connlist_find_connection_by_accountname(whonick))
		{
		    if (i+3+strlen(whonick)+2+1 > (conn_get_class(c) == conn_class_bot ? MAX_MESSAGE_LEN - 13 : MAX_MESSAGE_LEN))
		    {
			if (remove_sep)
			    tmsg[i-2] = '\0';
		        message_send_text(c,message_type_info,c,tmsg);
			i = 0;
			remove_sep = 0;
		    }
		    sprintf(&tmsg[i],"(*)%.64s, ",whonick);
		    i += strlen(&tmsg[i]);
		    remove_sep = 1;
		}
		else
		{
		    if (i+strlen(whonick)+2+1 > (conn_get_class(c) == conn_class_bot ? MAX_MESSAGE_LEN - 13 : MAX_MESSAGE_LEN))
		    {
			if (remove_sep)
		    	    tmsg[i-2] = '\0';
			message_send_text(c,message_type_info,c,tmsg);
			i = 0;
			remove_sep = 0;
		    }
		    sprintf(&tmsg[i],"%.64s, ",whonick);
		    i += strlen(&tmsg[i]);
		    remove_sep = 1;
		}
		account_unget_name(whonick);
	    }
	}

	if (i>0)
	{
	    if (remove_sep)
	        tmsg[i-2] = '\0';
	    message_send_text(c,message_type_info,c,tmsg);
	}
	if (counter == 0)
	    message_send_text(c,message_type_info,c,"-No One-");
    	return 0;
    }

    if (!(who = accountlist_find_account(nick)))
    {
        message_send_text(c,message_type_info,c,"That user does not exist.");
        return 0;
    }
    if (owner == who)
    {
	message_send_text(c,message_type_info,c,"You don't know when you're on bnet?");
	return 0;
    }
    if (watch_is_pair(c,who)==0)
    {
	message_send_text(c,message_type_info,c,"You are already watching this user");
	return 0;
    }
    if (watch_add_pair(c,who)<0)
        message_send_text(c,message_type_error,c,"Add to watch list failed.");
    else
    {
    	sprintf(tmsg,"User %.64s added to your watch list.",nick);
	message_send_text(c,message_type_info,c,tmsg);
    }
    
    return 0;
}


static int watch_func_add_all(t_connection * c)
{    
    if (watch_add_pair(c,NULL)<0)
	message_send_text(c,message_type_error,c,"Add to watch list failed.");
    else
        message_send_text(c,message_type_info,c,"All users added to your watch list.");

    return 0;
}


static int watch_func_del(t_connection * c, char const * nick)
{
    t_account *  account;
    char         tmsg[MAX_MESSAGE_LEN];

    if (nick[0]=='\0')
    {
        message_send_text(c,message_type_info,c,"Who do you want to unwatch?");
        return 0;
    }
    if (!(account = accountlist_find_account(nick)))
    {
        message_send_text(c,message_type_info,c,"That user does not exist.");
        return 0;
    }
    if (watch_del_pair(c,account)<0)
            message_send_text(c,message_type_error,c,"Removal from watch list failed.");
    else
    {
        sprintf(tmsg,"User %.64s removed from your watch list.",nick);
        message_send_text(c,message_type_info,c,tmsg);
    }

    return 0;
}


static int watch_func_del_all(t_connection * c)
{
    if (watch_del_pair(c,NULL)<0)
        message_send_text(c,message_type_error,c,"Removal from watch list failed.");
    else
        message_send_text(c,message_type_info,c,"All users removed from your watch list.");

    return 0;
}


/* who == NULL means anybody */
static int watch_add_pair(t_connection * owner, t_account * who)
{
    t_elem const * curr;
    t_watch_pair * pair;
    
    if (!owner)
    {
		eventlog(eventlog_level_error,"watch_add_pair","got NULL owner");
		return -1;
    }
    
    LIST_TRAVERSE_CONST(watchlist_head,curr)
    {
		pair = elem_get_data(curr);
		if (!pair) /* should not happen */
		{
		    eventlog(eventlog_level_error,"watch_add_pair","watchlist contains NULL item");
		    return -1;
		}
		if (pair->owner==owner && pair->who==who)
		    return 0;
	    }
	    
	    if (!(pair = malloc(sizeof(t_watch_pair))))
	    {
			eventlog(eventlog_level_error,"watch_add_pair","could not allocate memory for pair");
			return -1;
	    }
	    pair->owner = owner;
	    pair->who   = who;
		
	    if (list_append_data(watchlist_head,pair)<0)
	    {
			free(pair);
			eventlog(eventlog_level_error,"watch_add_pair","could not append temp");
			return -1;
	    }
#ifdef ACCT_DYN_UNLOAD
		eventlog(eventlog_level_debug,"watch_add_pair","incerasing ref");
    	account_inc_ref(who);
#endif
    
    return 0;
}


/* who == NULL means anybody */
static int watch_del_pair(t_connection * owner, t_account * who)
{
    t_elem *       curr;
    t_watch_pair * pair;
    
    if (!owner)
    {
		eventlog(eventlog_level_error,"watch_del_pair","got NULL owner");
		return -1;
    }
    
    LIST_TRAVERSE(watchlist_head,curr)
    {
		pair = elem_get_data(curr);
		if (!pair) /* should not happen */
		{
		    eventlog(eventlog_level_error,"watch_del_pair","watchlist contains NULL item");
		    return -1;
		}
		if (pair->owner==owner && pair->who==who)
		{
		    if (list_remove_elem(watchlist_head,curr)<0)
		    {
		        eventlog(eventlog_level_error,"watch_del_pair","could not remove item");
		        pair->owner = NULL;
		    }
		    else
		        free(pair);
	    
	    	list_purge(watchlist_head);
#ifdef ACCT_DYN_UNLOAD
			eventlog(eventlog_level_debug,"watch_del_pair","decreasing ref");
		    account_dec_ref(who);
#endif
		    return 0;
		}
    }
    
    return -1; /* not found */
}


static int watch_is_pair(t_connection * c, t_account * a)
{
    t_elem const * curr;
    t_watch_pair * pair;
    
    LIST_TRAVERSE_CONST(watchlist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair) /* shold not happen */
	{
	    eventlog(eventlog_level_error,"watchlist_is_pair","watchlist contains NULL item");
	    return -1;
	}
	if ((pair->owner==c) && (pair->who==a))
	    return 0;
    }

    return 1;
}


static int watch_del_all(t_connection * c)
{
    t_elem *       curr;
    t_watch_pair * pair;
	t_account *		who;

    if (!c)
    {
		eventlog(eventlog_level_error,"watch_del_all","got NULL c");
		return -1;
    }

    LIST_TRAVERSE(watchlist_head,curr)
    {
		pair = elem_get_data(curr);
		if (!pair) /* should not happen */
		{
		    eventlog(eventlog_level_error,"watch_del_all","watchlist contains NULL item");
		    return -1;
		}
		if (pair->owner==c)
		{
			who = pair->who;
		    if (list_remove_elem(watchlist_head,curr)<0)
		    {
				eventlog(eventlog_level_error,"watch_del_all","could not remove item");
				pair->owner = NULL;
		    }
		    else
				free(pair);
#ifdef ACCT_DYN_UNLOAD
		    account_dec_ref(who);
#endif
		}
    }
    list_purge(watchlist_head);

    return 0;
}

#ifdef ACCT_DYN_UNLOAD
extern unsigned int watch_count_acc_ref(t_account const * account)
{
    t_watch_pair * pair;
	t_elem const * curr;
	unsigned int   counter;
			
	if (!account)
	{
		eventlog(eventlog_level_error,"watch_count_acc_ref","got NULL account");
		return 0;
	}
	
	counter = 0;
	LIST_TRAVERSE_CONST(watchlist_head,curr)
	{
		pair = elem_get_data(curr);
		if (pair->who == account)
			counter++;
	}
	return counter;
}
#endif
