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
#define FRIENDS_INTERNAL_ACCESS
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
#include "compat/strcasecmp.h"
#include "connection.h"
#include "account.h"
#include "game.h"
#include "channel.h"
#include "common/eventlog.h"
#include "message.h"
#include "friends.h"
#include "common/setup_after.h"

static int identify_friends_function(const char * funcstr);
static void friends_func_add(t_connection * c, char const * nick, t_friends_event events);
static void friends_func_del(t_connection * owner, char const * nick, t_friends_event events);
static void friends_func_list(t_connection * c, int all);
static void friends_func_msg(t_connection * c, char const * message);
static int friends_is_friend(t_connection * c, t_account * a);
static int friends_is_mutual_friend(t_connection * c1, t_account * a2);
static int friends_add_events(t_connection * owner, t_account * who, t_friends_event events);
static int friends_del_events(t_connection * owner, t_account * who, t_friends_event events);
static t_friends_event friends_identify_event(char const * eventstr);
static t_friends_event friends_str_to_event(t_connection * c, char const * eventstr);
static char * friends_event_to_str(t_friends_event event);
static int friends_del_all(t_connection * c);
static void friends_usage(t_connection * c);

static t_list * friendslist_head=NULL;


extern int friendslist_create(void)
{
    if (!(friendslist_head = list_create()))
	return -1;

    return 0;
}


extern int friendslist_destroy(void)
{
    t_elem *		curr;
    t_friends_pair * 	pair;

    if (friendslist_head)
    {
	LIST_TRAVERSE(friendslist_head,curr)
	{
	    pair = elem_get_data(curr);
	    if (!pair)
	    {
		eventlog(eventlog_level_error,"friendslist_destroy","friendslist contains NULL item.");
		continue;
	    }
	    if (list_remove_elem(friendslist_head,curr)<0)
		eventlog(eventlog_level_error,"friendslist_destroy","could not remove item from list");
	    free(pair);
	}

	if (list_destroy(friendslist_head)<0)
	    return -1;
	friendslist_head = NULL;
    }

    return 0;
}


extern int friendslist_load(t_connection * c)
{
    t_account *		owner;
    t_account *		who;
    char *		friends;
    char *		friendsp;
    char const * 	tfriends;
    char *       	ttok;
    char		nick[MAX_NICK_LEN];

    if (!c)
    {
	eventlog(eventlog_level_error,"friendslist_load","got NULL connection");
	return -1;
    }

    if (!(owner = conn_get_account(c)))
    {
        eventlog(eventlog_level_error,"friendslist_load","got NULL owner");
	return -1;
    }
    if (!(tfriends = account_get_friends(owner)))
    {
	eventlog(eventlog_level_error,"friendslist_load","got NULL tfriends");
        return -1;
    }
    if (strlen(tfriends)<1)
    {
	account_unget_friends(tfriends);
	return 0;
    }
    if (!(friends = strdup(tfriends)))
    {
	eventlog(eventlog_level_error,"friendslist_load","could not allocate memory for tfriends");
	account_unget_friends(tfriends);
	return -1;
    }
    account_unget_friends(tfriends);

    friendsp = friends;
    while ((ttok = strsep(&friends,",")))
    {
	snprintf(nick, sizeof(nick), "%.47s", ttok);
	if (!(ttok = strsep(&friends,",")))
	{
	    eventlog(eventlog_level_error,"friendslist_load","corrupted friends list for account: %s",nick);
	    continue;
	}
	if (!(who = accountlist_find_account(nick)))
	{
	    eventlog(eventlog_level_warn,"friendslist_load","got NULL who");
	    continue;
	}
	friends_add_events(c,who,friends_str_to_event(c,ttok));
    }
    free(friendsp);

    return 0;
}


extern int friendslist_save(t_connection * c)
{
    char *		friends;
    t_elem const *	curr;
    t_friends_pair *	pair;
    char const *	tname;
    int			first;
    char *		events;
    char * 		tptr;

    if (!c)
    {
	eventlog(eventlog_level_error,"friendslist_save","got NULL connection");
	return -1;
    }

    if (!(friends = strdup("")))
    {
	eventlog(eventlog_level_error,"friendslist_save","not enought memory for initialize friends");
	return -1;
    }

    first = 1;
    LIST_TRAVERSE_CONST(friendslist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair) /* should not happen */
	{
	    eventlog(eventlog_level_error,"friendslist_save","friendslist contain NULL item");
	    return -1;
	}
        if (c == pair->owner)
	{
	    if (!(tname = account_get_name(pair->who)))
	    {
		eventlog(eventlog_level_error,"friendslist_save","could not get account name");
		continue;
	    }
            if (!(events = friends_event_to_str(pair->what)))
	    {
	        eventlog(eventlog_level_warn,"friendslist_save","got NULL event string");
		account_unget_name(tname);
	        continue;
	    }
	    tptr = friends;
	    if (first)
	    {
		if (!(friends = realloc(friends,strlen(tname)+1+strlen(events)+1)))
		{
		    eventlog(eventlog_level_error,"friends_save_attr","could not allocate memory for friends");
		    account_unget_name(tname);
		    free(events);
		    free(tptr);
		    return -1;
		}
		sprintf(friends,"%s,%s",tname,events);
		first = 0;
	    }
	    else
	    {
			if (!(friends = realloc(friends,strlen(friends)+1+strlen(tname)+1+strlen(events)+1)))
			{
			    eventlog(eventlog_level_error,"friends_save_attr","could not reallocate memory for friends");
			    account_unget_name(tname);
			    free(events);
			    free(tptr);
		    	return -1;
			}
			sprintf(friends,"%s,%s,%s",friends,tname,events);
	    }
	    account_unget_name(tname);
	    free(events);
	}
    }
    if (friends_del_all(c)<0)
		eventlog(eventlog_level_error,"friendslist_save","could not purge friendslist");
    if (account_set_strattr(conn_get_account(c),"BNET\\acct\\friends",friends)<0)
    {
		eventlog(eventlog_level_error,"friendslist_save","could not set strattr");
		free(friends);
		return -1;
    }
    free(friends);
    return 0;
}


extern int friends_notify_event(t_account * who, t_friends_event event, char const * info)
{
    t_elem const * 	curr;
    t_friends_pair * 	pair;
    char const * 	tname;
    char 		tmsg[MAX_MESSAGE_LEN];
    t_channel *		channel;
    t_game *		game;

    if (!who)
    {
	eventlog(eventlog_level_error,"friends_notify_event","got NULL who");
	return -1;
    }

    if (!(tname = account_get_name(who)))
    {
        eventlog(eventlog_level_error,"friends_notify_event","got NULL tname");
        return -1;
    }

    LIST_TRAVERSE_CONST(friendslist_head,curr)
    {
        pair = elem_get_data(curr);
        if (!pair) /* should not happen */
	{
	    eventlog(eventlog_level_error,"friends_notify_event","friendslist contains NULL item");
	    return -1;
	}
	if (pair->owner && (pair->who==who) && (pair->what&event))
	{
	    switch (event)
	    {
	    case friends_event_login:
		sprintf(tmsg,"Your friend %.64s has logged in.",tname);
		break;
	    case friends_event_logout:
		sprintf(tmsg,"Your friend %.64s has logged out.",tname);
	        break;
	    case friends_event_create_game:
		game = gamelist_find_game(info,game_type_all); /* FIXME: add checking */
		if (strcasecmp(game_get_pass(game),"")==0)
		    sprintf(tmsg,"Your friend %.64s has created game %.64s.",tname,info);
		else
		    if (friends_is_mutual_friend(pair->owner,who)==1)
			sprintf(tmsg,"Your friend %.64s has created private game %.64s.",tname,info);
		    else
			sprintf(tmsg,"Your friend %.64s has created private game",tname);
		break;
	    case friends_event_join_game:
		game = gamelist_find_game(info,game_type_all); /* FIXME: add checking */
		if (strcasecmp(game_get_pass(game),"")==0)
		    sprintf(tmsg,"Your friend %.64s has joined game %.64s.",tname,info);
		else
		    if (friends_is_mutual_friend(pair->owner,who)==1)
			sprintf(tmsg,"Your friend %.64s has joined private game %.64s.",tname,info);
		    else
			sprintf(tmsg,"Your friend %.64s has joined private game",tname);
		break;
	    case friends_event_finished_game:
		sprintf(tmsg,"Your friend %.64s has finished game.",tname);
		break;
	    case friends_event_join_channel:
		channel = channellist_find_channel_by_fullname(info); /* FIXME: add checking */
		if (channel_get_permanent(channel)==1)
		    sprintf(tmsg,"Your friend %.64s has joined channel %.64s.",tname,info);
		else
		    if (friends_is_mutual_friend(pair->owner,who)==1)
			sprintf(tmsg,"Your friend %.64s has joined private channel %.64s.",tname,info);
		    else
			sprintf(tmsg,"Your friend %.64s has joined private channel",tname);
		break;
	    default:
		account_unget_name(tname);
		eventlog(eventlog_level_error,"friends_notify_event","got unknown event %u",(unsigned int)event);
		return -1;
	    }
	    message_send_text(pair->owner,message_type_info,pair->owner,tmsg);
	}
    }
    account_unget_name(tname);


    return 0;
}


extern int handle_friends_command(t_connection * c, char const * text)
{
    char 	subcommand[MAX_FUNC_LEN];
    char	nick[MAX_NICK_LEN];
    char	events[MAX_EVENT_STR_LEN];
    char	msg[MAX_MESSAGE_LEN - 22 - 64 - 1 ]; /* max message lenght - "Message from friend %.64s: " - '\0' */
    int		i,j;

    for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
    for (; text[i]==' '; i++);

    for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get subcommand */
        if (j<sizeof(subcommand)-1) subcommand[j++] = text[i];
    subcommand[j] = '\0';
    for (; text[i]==' '; i++);

    if ((strcasecmp(subcommand,"m")==0) || (strcasecmp(subcommand,"msg")==0))
    {
	for (j=0; text[i]!='\0'; i++) /* get message */
	    if (j<sizeof(msg)-1) msg[j++] = text[i];
	msg[j] = '\0';
    }
    else
    {
        for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get nick */
	    if (j<sizeof(nick)-1) nick[j++] = text[i];
        nick[j] = '\0';
	for (; text[i]==' '; i++);

        for (j=0; text[i]!='\0'; i++) /* get events */
	    if (j<sizeof(events)-1) events[j++] = text[i];
        events[j] = '\0';
    }

    switch (identify_friends_function(subcommand))
    {
	case FRIENDS_FUNC_ADD:
	    friends_func_add(c,nick,friends_str_to_event(c,events));
	    break;
	case FRIENDS_FUNC_DEL:
	    friends_func_del(c,nick,friends_str_to_event(c,events));
	    break;
	case FRIENDS_FUNC_LIST_ALL:
	    friends_func_list(c,1);
	    break;
	case FRIENDS_FUNC_LIST:
	    friends_func_list(c,0);
	    break;
	case FRIENDS_FUNC_MSG:
	    friends_func_msg(c,msg);
	    break;
	case FRIENDS_FUNC_FLUSH:
	    if (friends_del_all(c)<0)
		message_send_text(c,message_type_error,c,"Error occured.");
	    else
		message_send_text(c,message_type_info,c,"List of friends flushed");
	    break;
	case FRIENDS_FUNC_HELP:
	    message_send_text(c,message_type_info,c,"The friends commands supports the following patterns.");
	    friends_usage(c);
	    break;
	default:
	    message_send_text(c,message_type_info,c,"The command is incorrect. Use one of the following patterns.");
	    friends_usage(c);
    }

    return 0;
}


static int identify_friends_function(const char * funcstr)
{
    if (strcasecmp(funcstr,"add")==0 || strcasecmp(funcstr,"a")==0)
	return FRIENDS_FUNC_ADD;
    if (strcasecmp(funcstr,"del")==0 || strcasecmp(funcstr,"d")==0)
	return FRIENDS_FUNC_DEL;
    if (strcasecmp(funcstr,"listall")==0 || strcasecmp(funcstr,"la")==0 || strcasecmp(funcstr,"")==0)
	return FRIENDS_FUNC_LIST_ALL;
    if (strcasecmp(funcstr,"list")==0 || strcasecmp(funcstr,"l")==0)
	return FRIENDS_FUNC_LIST;
    if (strcasecmp(funcstr,"msg")==0 || strcasecmp(funcstr,"m")==0)
	return FRIENDS_FUNC_MSG;
    if (strcasecmp(funcstr,"flush")==0)
	return FRIENDS_FUNC_FLUSH;
    if (strcasecmp(funcstr,"help")==0 || strcasecmp(funcstr,"h")==0)
	return FRIENDS_FUNC_HELP;

    return FRIENDS_FUNC_UNKNOWN;
}


static void friends_func_add(t_connection * owner, char const * nick, t_friends_event events)
{
    t_account *		who;
    t_account *		acc_owner;
    char		tstr[MAX_MESSAGE_LEN];

    if (!owner)
    {
	eventlog(eventlog_level_error,"friends_func_add","got NULL connection");
	return;
    }
    if (!nick)
    {
	eventlog(eventlog_level_error,"friends_func_add","got NULL nick");
	return;
    }

    if (!(acc_owner = conn_get_account(owner)))
    {
	eventlog(eventlog_level_error,"friends_func_add","got NULL owner account");
	return;
    }
    if (!(who = accountlist_find_account(nick)))
    {
	message_send_text(owner,message_type_info,owner,"That user does not exist.");
	return;
    }
    if (who == acc_owner)
    {
	message_send_text(owner,message_type_info,owner,"Do you like yourself so much?");
	return;
    }
    if (friends_add_events(owner,who,events)<0)
	message_send_text(owner,message_type_error,owner,"Add to friends list failed");
    else
    {
	snprintf(tstr, sizeof(tstr), "User %.64s added to your friends list.",nick);
	message_send_text(owner,message_type_info,owner,tstr);
    }
}


static void friends_func_del(t_connection * owner, char const * nick, t_friends_event events)
{
    t_account * 	account;
    int 		ret;

    if (!owner)
    {
	eventlog(eventlog_level_error,"friends_func_del","got NULL owner");
	return;
    }
    if (!nick)
    {
	eventlog(eventlog_level_error,"friensd_func_del","got NULL nick");
	return;
    }

    if (!(account = accountlist_find_account(nick)))
    {
	eventlog(eventlog_level_error,"friends_func_del","got NULL account");
	message_send_text(owner,message_type_error,owner,"No such user.");
	return;
    }
    ret = friends_del_events(owner,account,events);
    if (ret == 0)
	message_send_text(owner,message_type_info,owner,"Done.");
    else if (ret == 1)
	message_send_text(owner,message_type_info,owner,"That user is not on your friends list");
    else
	message_send_text(owner,message_type_info,owner,"Error occured.");
}


static void friends_func_list(t_connection * c, int all)
{
    t_elem const * 	curr;
    t_friends_pair * 	pair;
    char 		tmsg[MAX_MESSAGE_LEN];
    char		tstr[MAX_MESSAGE_LEN];
    char const *	nick;
    char *		event;
    unsigned int	counter;
    int			isany;
    t_connection *	tc;
    t_game const *	game;
    t_channel const *	channel;

    if (!c)
    {
	eventlog(eventlog_level_error,"friends_func_list","got NULL connection");
	return;
    }
    counter = 0;
    isany = 0;

    if (all)
	message_send_text(c,message_type_info,c,"Your friends are:");
    else
	message_send_text(c,message_type_info,c,"Your logged friends are:");

    LIST_TRAVERSE_CONST(friendslist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair)
	{
	    eventlog(eventlog_level_error,"friends_func_list","friendslist contains NULL item");
	    return;
	}
	if (pair->owner == c)
	{
	    if (!(nick = account_get_name(pair->who)))
	    {
		eventlog(eventlog_level_error,"friends_func_list","got NULL nick");
		continue;
	    }
	    event = friends_event_to_str(pair->what);
	    tc = connlist_find_connection_by_accountname(nick);
	    if (all)
	    {
		if (!tc)
		{
		    if (friends_is_mutual_friend(c,pair->who)==1)
			sprintf(tmsg,"%u: %.64s [mutual, offline] (%s)",++counter,nick,event);
		    else
			sprintf(tmsg,"%u: %.64s [offline] (%s)",++counter,nick,event);
		    message_send_text(c,message_type_info,c,tmsg);
		    isany = 1;
		}
	    }
	    else
		if (!tc)
		    counter++;
	    if (tc)
	    {
		if ((game = conn_get_game(tc)))
		    if (strcmp(game_get_pass(game),"")==0)
			snprintf(tstr, sizeof(tstr), "in game \"%.64s\"",game_get_name(game));
		    else
			if (friends_is_mutual_friend(c,pair->who)==1)
			    snprintf(tstr, sizeof(tstr), "in private game \"%.64s\"",game_get_name(game));
			else
			    snprintf(tstr, sizeof(tstr), "in private game");
		else if ((channel = conn_get_channel(tc)))
		    if (channel_get_permanent(channel)==1)
			snprintf(tstr, sizeof(tstr), "in channel \"%.64s\"",channel_get_name(channel));
		    else
			if (friends_is_mutual_friend(c,pair->who)==1)
			    snprintf(tstr, sizeof(tstr), "in private channel \"%.64s\"",channel_get_name(channel));
			else
			    snprintf(tstr, sizeof(tstr), "in private channel");
		else
		    snprintf(tstr, sizeof(tstr), "not in game nor in channel"); /* FIXME: so where? Could be at score table after game, but anywhere else? How to check it? */

		if (friends_is_mutual_friend(c,pair->who)==1)
	            sprintf(tmsg,"%u: %.64s [mutual, %.128s] (%s)",++counter,nick,tstr,event);
		else
		    sprintf(tmsg,"%u: %.64s [%.128s] (%s)",++counter,nick,tstr,event);
	        message_send_text(c,message_type_info,c,tmsg);
		isany = 1;
	    }
	    free(event);
	    account_unget_name(nick);
	}
    }

    if (isany==0)
	message_send_text(c,message_type_info,c,"-No One-");
}


static void friends_func_msg(t_connection * c, char const * message)
{
    t_elem const *	curr;
    t_friends_pair *	pair;
    char		msg[MAX_MESSAGE_LEN];
    t_account *		account;
    t_connection *	tc;
    t_connection *	lc;
    unsigned int	counter;
    char const *	tname;

    if (!c)
    {
	eventlog(eventlog_level_error,"friends_func_msg","got NULL connection");
	return;
    }
    if (!message)
    {
	eventlog(eventlog_level_error,"friends_func_msg","got null message");
	return;
    }

    if (!(account = conn_get_account(c)))
    {
	eventlog(eventlog_level_error,"friends_func_msg","got NULL account");
	return;
    }
    if (!(tname = account_get_name(account)))
    {
	eventlog(eventlog_level_error,"account_func_msg","got NULL tname");
	return;
    }

    snprintf(msg, sizeof(msg), "Message from your friend %.60s: %s", tname,
	     message);
    account_unget_name(tname);
    counter = 0;
    lc = NULL;

    LIST_TRAVERSE_CONST(friendslist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair) /* should not happen */
	{
	    eventlog(eventlog_level_error,"friends_func_msg","friendlist contains NULL item");
	    return;
	}
	if (!(tname = account_get_name(pair->who)))
	{
	    eventlog(eventlog_level_error,"account_func_msg","got NULL tname");
	    continue;
	}
	if ((tc = connlist_find_connection_by_accountname(tname)))
	    if (c == pair->owner && friends_is_friend(tc,account)==1)
	        if (!(conn_get_dndstr(tc)))
	        {
		    message_send_text(tc,message_type_info,tc,msg);
		    counter++;
		    lc = tc;
		}
	account_unget_name(tname);
    }
    if (counter == 0)
	message_send_text(c,message_type_info,c,"No one have recived your message.");
    else
    {
	if (counter == 1)
	{
	    if (!(tname = conn_get_username(lc)))
	    {
		eventlog(eventlog_level_error,"friends_func_msg","conn_get_username return NULL");
		sprintf(msg,"1 user have recived your message.");
	    }
	    else
	    {
		sprintf(msg,"%.64s have recived your message.",tname);
		conn_unget_username(lc,tname);
	    }
	}
	else
	    sprintf(msg,"%u users have recived your message.",counter);

	message_send_text(c,message_type_info,c,msg);
    }
}


static int friends_is_friend(t_connection * c, t_account * a)
{
    t_elem const *	curr;
    t_friends_pair *	pair;

    if (!c)
    {
	eventlog(eventlog_level_error,"friends_is_friend","got NULL connection");
	return -1;
    }
    if (!a)
    {
	eventlog(eventlog_level_error,"friends_is_friend","got NULL account");
	return -1;
    }

    LIST_TRAVERSE_CONST(friendslist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair) /* should not happen */
	{
	    eventlog(eventlog_level_error,"friends_is_friend","friendlist contains NULL item");
	    return -1;
	}

	if ((c == pair->owner) && (a == pair->who))
	    return 1;
    }

    return 0;
}


static int friends_is_mutual_friend(t_connection * c1, t_account * a2)
{
    t_connection * c2;
    t_account * a1;
    char const * name1;
    char const * name2;
    char *	 friends;
    char *	 friendsp;
    char const * tfriends;
    char *	 ttok;


    if (!c1)
    {
	eventlog(eventlog_level_error,"friends_is_mutual_friend","got NULL connection");
	return -1;
    }
    if (!a2)
    {
	eventlog(eventlog_level_error,"friends_is_mutual_friend","got NULL account");
	return -1;
    }

    if (friends_is_friend(c1,a2)!=1)
	return 0;

    if (!(name2 = account_get_name(a2)))
    {
	eventlog(eventlog_level_error,"friends_is_mutual_friend","got NULL name2");
	return -1;
    }
    if (!(a1 = conn_get_account(c1)))
    {
	eventlog(eventlog_level_error,"friends_is_mutual_friend","got NULL a1");
	return -1;
    }

    if ((c2 = connlist_find_connection_by_accountname(name2)))
    {
	account_unget_name(name2);
	if (friends_is_friend(c2,a1)==1)
	    return 1;
	else
	    return 0;
    }
    else
    {
        account_unget_name(name2);
	if (!(tfriends = account_get_friends((a2))))
	{
	    eventlog(eventlog_level_error,"friends_is_mutual_friend","got NULL tfriends");
	    account_unget_name(name2);
	    return -1;
	}
	if (strlen(tfriends)<1)
	{
	    account_unget_friends(tfriends);
	    return 0;
	}
	if (!(friends = strdup(tfriends)))
	{
	    account_unget_friends(tfriends);
	    eventlog(eventlog_level_error,"friends_is_mutual_friend","could not allocate memory for friends");
	    return -1;
	}
	account_unget_friends(tfriends);
	if (!(name1 = account_get_name(a1)))
	{
	    eventlog(eventlog_level_error,"friends_is_mutual_friend","got NULL name1");
	    return -1;
	}


	friendsp = friends;
	while ((ttok = strsep(&friends,",")))
	{
	    if (strcasecmp(ttok,name1)==0)
	    {
		account_unget_name(name1);
		free(friendsp);
		return 1;
	    }
	    if (!(ttok = strsep(&friends,",")))
	    {
		eventlog(eventlog_level_error,"friendslist_load","corrupted friends list");
		continue;
	    }
	}
	account_unget_name(name1);
	free(friendsp);
    }

    return 0;
}


static int friends_add_events(t_connection * owner, t_account * who, t_friends_event events)
{
    t_elem const *	curr;
    t_friends_pair * 	pair;

    if (!owner)
    {
	eventlog(eventlog_level_error,"friends_add_events","got NULL owner");
	return -1;
    }
    if (!who)
    {
	eventlog(eventlog_level_error,"friends_add_events","got NULL who");
	return -1;
    }

    LIST_TRAVERSE_CONST(friendslist_head,curr)
    {
	pair = elem_get_data(curr);
	if (!pair) /* should not happen */
	{
	    eventlog(eventlog_level_error,"friends_add_events","friendlist contains NULL item");
	    return -1;
	}
	if (pair->owner==owner && pair->who==who)
	{
	    pair->what |= events;
	    return 0;
	}
    }
    if (!(pair = malloc(sizeof(t_friends_pair))))
    {
        eventlog(eventlog_level_error,"friends_add_events","could not allocate memory for pair");
	return -1;
    }
    pair->owner = owner;
    pair->who   = who;
    pair->what  = events;

    if (list_append_data(friendslist_head,pair)<0)
    {
		free(pair);
        eventlog(eventlog_level_error,"friends_add_events","could not apppend pair");
		return -1;
    }
#ifdef ACCT_DYN_UNLOAD
    account_inc_ref(who);
#endif

    return 0;
}


static int friends_del_events(t_connection * owner, t_account * who, t_friends_event events)
{
    t_elem *		curr;
    t_friends_pair * 	pair;

    if (!owner)
    {
	eventlog(eventlog_level_error,"friendslist_del_events","got NULL owner.");
	return -1;
    }
    if (!who)
    {
	eventlog(eventlog_level_error,"friendslist_del_events","got NULL who.");
	return -1;
    }

    LIST_TRAVERSE(friendslist_head,curr)
    {
		pair = elem_get_data(curr);
		if (!pair) /* should not happen */
		{
		    eventlog(eventlog_level_error,"friends_del_events","friendslist contain NULL item");
		    return -1;
		}
		if (pair->owner==owner && pair->who==who)
		{
		    pair->what &= ~events;
		    if (pair->what==0)
		    {
				if (list_remove_elem(friendslist_head,curr)<0)
				{
			    	eventlog(eventlog_level_error,"friends_del_events","could not remove item");
				    pair->owner = NULL;
				}
				else
			    	free(pair);
		    }

#ifdef ACCT_DYN_UNLOAD
		    account_dec_ref(who);
#endif
		    list_purge(friendslist_head);
	    	return 0;
		}
    }

    return 1;
}


static t_friends_event friends_identify_event(char const * eventstr)
{
    t_friends_event	tevent;

    tevent = FRIENDS_NO_EVENTS;
    if (strcasecmp(eventstr,"li")==0) tevent = friends_event_login;
    else if (strcasecmp(eventstr,"lo")==0) tevent = friends_event_logout;
    else if (strcasecmp(eventstr,"l")==0) tevent = friends_event_logout | friends_event_login;
    else if (strcasecmp(eventstr,"gc")==0) tevent = friends_event_create_game;
    else if (strcasecmp(eventstr,"gi")==0) tevent = friends_event_join_game;
    else if (strcasecmp(eventstr,"go")==0) tevent = friends_event_finished_game;
    else if (strcasecmp(eventstr,"g")==0) tevent = friends_event_finished_game | friends_event_join_game;
    else if (strcasecmp(eventstr,"ci")==0) tevent = friends_event_join_channel;
    else if (strcasecmp(eventstr,"c")==0) tevent = friends_event_join_channel;

    return tevent;
}


static t_friends_event friends_str_to_event(t_connection * c, char const * eventstr)
{
    t_friends_event 	tevent;
    t_friends_event	ttevent;
    char *		teventstr;
    char *		peventstr;
    char *		ttok;
    char		tstr[MAX_MESSAGE_LEN];

    if (!c)
    {
	eventlog(eventlog_level_error,"friends_str_to_event","got NULL connection");
	return FRIENDS_NO_EVENTS;
    }
    if (!eventstr)
    {
	eventlog(eventlog_level_error,"friends_str_to_event","got NULL eventstr");
	return FRIENDS_NO_EVENTS;
    }

    if (eventstr[0]=='\0')
	return FRIENDS_ALL_EVENTS;

    tevent = FRIENDS_NO_EVENTS;
    teventstr = strdup(eventstr);
    peventstr = teventstr;
    while ((ttok = strsep(&teventstr," ")))
	if (strcasecmp(ttok,""))
	{
	    ttevent = friends_identify_event(ttok);
	    if (ttevent == FRIENDS_NO_EVENTS)
	    {
		if (c)
		{
		    snprintf(tstr, sizeof(tstr), "Unknown option: %s",ttok);
		    message_send_text(c,message_type_info,c,tstr);
		}
	    }
	    else
		tevent |= ttevent;
	}
    free(peventstr);

    return tevent;
}


static char * friends_event_to_str(t_friends_event event)
{
    char * 	tevent;
    char	tstr[MAX_EVENT_STR_LEN];

    strcpy(tstr,"");
    if (event & friends_event_login) snprintf(tstr, sizeof(tstr), "%.16s li",
					      tstr);
    if (event & friends_event_logout) snprintf(tstr, sizeof(tstr), "%.16s lo",
					       tstr);
    if (event & friends_event_create_game) snprintf(tstr, sizeof(tstr),
						    "%.16s gc", tstr);
    if (event & friends_event_join_game) snprintf(tstr, sizeof(tstr),
						  "%.16s gi", tstr);
    if (event & friends_event_finished_game) snprintf(tstr, sizeof(tstr),
						      "%.16s go", tstr);
    if (event & friends_event_join_channel) snprintf(tstr, sizeof(tstr),
						     "%.16s ci", tstr);
    tstr[strlen(tstr)] = '\0';
    if (!(tevent = strdup(&tstr[1])))
    {
	eventlog(eventlog_level_error,"friends_event_to_str","could not allocate memory for tevent");
	return NULL;
    }

    return tevent;
}


static int friends_del_all(t_connection * c)
{
    t_elem *       curr;
    t_friends_pair * pair;

    if (!c)
    {
	eventlog(eventlog_level_error,"friends_del_all","got NULL c");
	return -1;
    }

    LIST_TRAVERSE(friendslist_head,curr)
    {
		pair = elem_get_data(curr);
		if (!pair) /* should not happen */
		{
		    eventlog(eventlog_level_error,"friends_del_all","friendslist contains NULL item");
		    return -1;
		}
		if (pair->owner==c)
		{
		    if (list_remove_elem(friendslist_head,curr)<0)
		    {
				eventlog(eventlog_level_error,"friends_del_all","could not remove item");
				pair->owner = NULL;
		    }
		    else
			{
#ifdef ACCT_DYN_UNLOAD
		    	account_dec_ref(pair->who);
#endif
				free(pair);
			}
		}
    }
    list_purge(friendslist_head);

    return 0;
}


static void friends_usage(t_connection * c)
{
    message_send_text(c,message_type_info,c,"to add player to list of friends:");
    message_send_text(c,message_type_info,c,"/f[reinds] a[dd] <nick> [events]");
    message_send_text(c,message_type_info,c,"do remove player from list of friends");
    message_send_text(c,message_type_info,c,"/f[riends] d[el] <nick> [events]");
    message_send_text(c,message_type_info,c,"to list friends curently online");
    message_send_text(c,message_type_info,c,"/f[riends] l[ist]");
    message_send_text(c,message_type_info,c,"to list all friends:");
    message_send_text(c,message_type_info,c,"/f[riends] <la|listall>");
    message_send_text(c,message_type_info,c,"to send a message to mutual friends:");
    message_send_text(c,message_type_info,c,"/f[riends] m[sg] <message>");
    message_send_text(c,message_type_info,c,"to flush list of friends:");
    message_send_text(c,message_type_info,c,"/f[riends] flush");
    message_send_text(c,message_type_info,c,"to print this information:");
    message_send_text(c,message_type_info,c,"/f[riends] h[elp]");
    message_send_text(c,message_type_info,c,"");
    message_send_text(c,message_type_info,c,"events can be:");
    message_send_text(c,message_type_info,c,"li - log in");
    message_send_text(c,message_type_info,c,"lo - log out");
    message_send_text(c,message_type_info,c,"l - log in or out");
    message_send_text(c,message_type_info,c,"gc - create game");
    message_send_text(c,message_type_info,c,"gi - join game");
    message_send_text(c,message_type_info,c,"go - finish game");
    message_send_text(c,message_type_info,c,"g - join or leave game");
    message_send_text(c,message_type_info,c,"ci - join channel");
    message_send_text(c,message_type_info,c,"c - join channel");
    message_send_text(c,message_type_info,c,"default is all of them");
}

#ifdef ACCT_DYN_UNLOAD
extern unsigned int friends_count_acc_ref(t_account const * account)
{
    t_friends_pair * pair;
    t_elem const * 	 curr;
    unsigned int	 counter;

    if (!account)
    {
		eventlog(eventlog_level_error,"friends_count_acc_ref","got NULL account");
		return 0;
    }

    counter = 0;
    LIST_TRAVERSE_CONST(friendslist_head,curr)
    {
		pair = elem_get_data(curr);
		if (pair->who == account)
			counter++;
    }
    return counter;
}
#endif
