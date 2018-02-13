/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Gediminas (gediminas_lt@mailexcite.com)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2000  Dizzy (dizzy@roedu.net)
 * Copyright (C) 2000  Onlyer (onlyer@263.net)
 * Copyright (C) 2002  Bart³omiej Butyn (bartek@milc.com.pl)
 * Copyright (C) 2002  Joerg Ebeling (jebs@shbe.net)
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
#include "common/setup_before.h"
#include <stdio.h>
#ifdef HAVE_STDDEF_H
# include <stddef.h>
#else
# ifndef NULL
#  define NULL ((void *)0)
# endif
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#include "compat/strtoul.h"
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include "compat/strdup.h"
#include "compat/strcasecmp.h"
#include <ctype.h>
#include <errno.h>
#include "compat/strerror.h"
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_SYS_TYPES
# include <sys/types.h>
#endif
#include "compat/strftime.h"
#include "message.h"
#include "common/tag.h"
#include "connection.h"
#include "channel.h"
#include "game.h"
#include "common/util.h"
#include "common/version.h"
#include "account.h"
#include "server.h"
#include "prefs.h"
#include "common/eventlog.h"
#include "ladder.h"
#include "timer.h"
#include "common/bnettime.h"
#include "common/addr.h"
#include "gametrans.h"
#include "helpfile.h"
#include "mail.h"
#include "common/bnethash.h"
#include "runprog.h"
#include "common/list.h"
#include "common/proginfo.h"
#include "alias_command.h"
#include "realm.h"
#include "ipban.h"
#include "watch.h"
#include "friends.h"
#ifdef WITH_BITS
# include "bits.h"
# include "bits_query.h"
# include "bits_va.h"
# include "bits_packet.h"
# include "bits_login.h"
# include "bits_chat.h"
# include "bits_game.h"
#endif
#include "command.h"
#include "common/setup_after.h"


static char const * bnclass_get_str(unsigned int class);
static void do_whisper(t_connection * user_c, char const * dest, char const * text);
static void do_whois(t_connection * c, char const * dest);
static void user_timer_cb(t_connection * c, time_t now, t_timer_data str);


static char const * bnclass_get_str(unsigned int class)
{
    switch (class)
    {
    case PLAYERINFO_DRTL_CLASS_WARRIOR:
	return "warrior";
    case PLAYERINFO_DRTL_CLASS_ROGUE:
	return "rogue";
    case PLAYERINFO_DRTL_CLASS_SORCERER:
	return "sorcerer";
    default:
	return "unknown";
    }
}


static void do_whisper(t_connection * user_c, char const * dest, char const * text)
{
    t_connection * dest_c;
    char           temp[MAX_MESSAGE_LEN];
    char const *   tname;
    char const *   dest_name;


    if (!(dest_c = connlist_find_connection_by_name(dest,conn_get_realmname(user_c)))) /* Yoss: could here be conn_get_username()? */
                                                                                       /* JEBs: No, D2 needs a whisper via Charname ! */
    {
#ifndef WITH_BITS
	message_send_text(user_c,message_type_error,user_c,"That user is not logged on.");
	return;
#else
	bits_chat_user((tname = conn_get_username(user_c)),dest,message_type_whisper,conn_get_latency(user_c),conn_get_flags(user_c),text);
	conn_unget_username(user_c,tname);
	return;
#endif
    }

    dest_name = conn_get_username(dest_c);
    if (conn_get_dndstr(dest_c))
    {
        snprintf(temp, sizeof(temp), "%.64s is unavailable (%.128s)",dest_name,conn_get_dndstr(dest_c));
	conn_unget_username(dest_c,dest_name);
        message_send_text(user_c,message_type_info,user_c,temp);
        return;
    }

    message_send_text(user_c,message_type_whisperack,dest_c,text);

    if (conn_get_awaystr(dest_c))
    {
        snprintf(temp, sizeof(temp), "%.64s is away (%.128s)",dest_name,conn_get_awaystr(dest_c));
        message_send_text(user_c,message_type_info,user_c,temp);
    }
    if (conn_check_ignoring(dest_c,(tname = conn_get_username(user_c))))
    {
	snprintf(temp, sizeof(temp), "%.64s ignores you.",dest_name);
	conn_unget_username(user_c,tname);
	conn_unget_username(dest_c,dest_name);
	message_send_text(user_c,message_type_info,user_c,temp);
	return;
    }

    message_send_text(dest_c,message_type_whisper,user_c,text);

    if ((tname = conn_get_username(user_c)))
    {
        char username[1+MAX_USER_NAME]; /* '*' + username (including NULL) */

	if (strlen(tname)<MAX_USER_NAME)
	{
            sprintf(username,"*%s",tname);
	    conn_set_lastsender(dest_c,username);
	}
	conn_unget_username(dest_c,tname);
    }
    conn_unget_username(dest_c,dest_name);
}


static void do_whois(t_connection * c, char const * dest)
{
    t_connection *    dest_c;
    char              namepart[136]; /* 64 + " (" + 64 + ")" + NUL */
    char              temp[MAX_MESSAGE_LEN];
    char const *      verb;
    t_game const *    game;
    t_channel const * channel;

    if (!(dest_c = connlist_find_connection_by_name(dest,conn_get_realmname(c))))
    {
	message_send_text(c,message_type_error,c,"That user is not logged on.");
	return;
    }

    if (c==dest_c)
    {
	strcpy(namepart,"You");
	verb = "are";
    }
    else
    {
	char const * tname;

	sprintf(namepart,"%.64s",(tname = conn_get_username(dest_c)));
	conn_unget_username(dest_c,tname);
	verb = "is";
    }

    if ((game = conn_get_game(dest_c)))
    {
	if (strcmp(game_get_pass(game),"")==0)
#ifdef ACCT_DYN_LOAD
	    snprintf(temp, sizeof(temp), "%s %s currently in game \"%.64s\".",
		    namepart,
		    verb,
		    game_get_name(game));
#else

	    snprintf(temp, sizeof(temp), "%s %s logged on from account "UID_FORMAT", and %s currently in game \"%.49s\".",
		    namepart,
		    verb,
		    conn_get_userid(dest_c),
		    verb,
		    game_get_name(game));
#endif
	else
#ifdef ACCT_DYN_LOAD
	    snprintf(temp, sizeof(temp), "%s %s currently in private game \"%.64s\".",
		    namepart,
		    verb,
		    game_get_name(game));
#else
	    snprintf(temp, sizeof(temp), "%s %s logged on from account "UID_FORMAT", and %s currently in private game \"%.41s\".",
		    namepart,
		    verb,
		    conn_get_userid(dest_c),
		    verb,
		    game_get_name(game));
#endif
    }
    else if ((channel = conn_get_channel(dest_c)))
    {
	if (channel_get_permanent(channel)==1)
#ifdef ACCT_DYN_LOAD
            snprintf(temp, sizeof(temp), "%s %s currently in channel \"%.64s\".",
		    namepart,
		    verb,
		    channel_get_name(channel));
#else
	    snprintf(temp, sizeof(temp), "%s %s logged on from account "UID_FORMAT", and %s currently in channel \"%.38s\".",
		    namepart,
		    verb,
		    conn_get_userid(dest_c),
		    verb,
		    channel_get_name(channel));
#endif
	else
#ifdef ACCT_DYN_LOAD
            snprintf(temp, sizeof(temp), "%s %s currently in private channel \"%.64s\".",
		    namepart,
		    verb,
		    channel_get_name(channel));
#else
	    snprintf(temp, sizeof(temp), "%s %s logged on from account "UID_FORMAT", and %s currently in private channel \"%.38s\".",
		    namepart,
		    verb,
		    conn_get_userid(dest_c),
		    verb,
		    channel_get_name(channel));
#endif
    }
    else
#ifdef ACCT_DYN_LOAD
	snprintf(temp, sizeof(temp), "%s %s logged on.",
		namepart,
		verb);
#else
	snprintf(temp, sizeof(temp), "%s %s logged on from account "UID_FORMAT".",
		namepart,
		verb,
		conn_get_userid(dest_c));
#endif
    message_send_text(c,message_type_info,c,temp);
    if (game && strcmp(game_get_pass(game),"")!=0)
	message_send_text(c,message_type_info,c,"(This game is password protected.)");

    if (conn_get_dndstr(dest_c))
    {
        snprintf(temp, sizeof(temp), "%s %s refusing messages (%.94s)",
		namepart,
		verb,
		conn_get_dndstr(dest_c));
	message_send_text(c,message_type_info,c,temp);
    }
    else
        if (conn_get_awaystr(dest_c))
        {
            snprintf(temp, sizeof(temp), "%s away (%.111s)",
		    namepart,
		    conn_get_awaystr(dest_c));
	    message_send_text(c,message_type_info,c,temp);
        }
}


static void user_timer_cb(t_connection * c, time_t now, t_timer_data str)
{
    if (!c)
    {
	eventlog(eventlog_level_error,"user_timer_cb","got NULL connection");
	return;
    }
    if (!str.p)
    {
	eventlog(eventlog_level_error,"user_timer_cb","got NULL str");
	return;
    }

    if (now!=(time_t)0) /* zero means user logged out before expiration */
	message_send_text(c,message_type_info,c,str.p);
    free(str.p);
}


extern int handle_command(t_connection * c,  char const * text)
{
#if 0
    if (strstart(text,"/nt")==0) /* for testing time conversion */
    {
        unsigned int i;
	t_bnettime   bnt;

        for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
        for (; text[i]==' '; i++); /* skip spaces */

	bnettime_set_str(&bnt,&text[i]);
        message_send_text(c,message_type_info,c,&text[i]);

	account_set_normal_last_time(conn_get_account(c),conn_get_clienttag(c),bnt);

        return 0;
    }
#endif
    if (strstart(text,"/me")==0)
    {
	t_channel const * channel;
	unsigned int      i;

        if (!(channel = conn_get_channel(c)))
	{
	    message_send_text(c,message_type_error,c,"You are not in a channel.");
	    return 0;
	}

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++); /* skip spaces */

	if (!conn_quota_exceeded(c,&text[i]))
	    channel_message_send(channel,message_type_emote,c,&text[i]);
	return 0;
    }
    if (strstart(text,"/msg")==0 ||
	strstart(text,"/whisper")==0 ||
	strstart(text,"/w")==0 ||
	strstart(text,"/m")==0)
    {
	char         dest[MAX_NICK_LEN];
	unsigned int i,j;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

	if (text[i]=='\0')
	{
	    message_send_text(c,message_type_error,c,"What do you want to say?");
	    return 0;
	}

	do_whisper(c,dest,&text[i]);

	return 0;
    }
    if (strstart(text,"/status")==0 || strstart(text,"/users")==0)
    {
	char msgtemp[MAX_MESSAGE_LEN];

	snprintf(msgtemp, sizeof(msgtemp), "There are currently %d users online, in %d games and %d channels.",
		connlist_login_get_length(),
		gamelist_get_length(),
		channellist_get_length());
	message_send_text(c,message_type_info,c,msgtemp);

	return 0;
    }
    if (strstart(text,"/who")==0)
    {
	char                 msgtemp[MAX_MESSAGE_LEN];
#ifndef WITH_BITS
	t_connection const * conn;
#else
	t_bits_channelmember const * conn;
#endif
	t_channel const *    channel;
	unsigned int         i;
#ifndef WITH_BITS
	char const *         tname;
#endif

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (!(channel = channellist_find_channel_by_name(&text[i],conn_get_country(c),conn_get_realmname(c))))
	{
	    message_send_text(c,message_type_error,c,"That channel does not exist.");
	    message_send_text(c,message_type_error,c,"(If you are trying to search for a user, use the /whois command.)");
	    return 0;
	}
	if (channel_check_banning(channel,c)==1)
	{
	    message_send_text(c,message_type_error,c,"You are banned from that channel.");
	    return 0;
	}

	snprintf(msgtemp, sizeof(msgtemp), "Users in channel %.64s:",&text[i]);
	i = strlen(msgtemp);
	for (conn=channel_get_first(channel); conn; conn=channel_get_next())
	{
#ifndef WITH_BITS
	    if (i+strlen((tname = conn_get_username(conn)))+2>sizeof(msgtemp)) /* " ", name, '\0' */
	    {
		message_send_text(c,message_type_info,c,msgtemp);
		i = 0;
	    }
	    sprintf(&msgtemp[i]," %s",tname);
	    conn_unget_username(conn,tname);
	    i += strlen(&msgtemp[i]);
#else
	    char const * name = bits_loginlist_get_name_bysessionid(conn->sessionid);

	    if (!name) {
		eventlog(eventlog_level_error,"handle_command","FIXME: user without name");
		continue;
	    }
	    if (i+strlen(name)+2>sizeof(msgtemp)) /* " ", name, '\0' */
	    {
		message_send_text(c,message_type_info,c,msgtemp);
		i = 0;
	    }
	    sprintf(&msgtemp[i]," %s",name);
	    i += strlen(&msgtemp[i]);
#endif
	}
	if (i>0)
	    message_send_text(c,message_type_info,c,msgtemp);

	return 0;
    }
    if (strstart(text,"/whois")==0 || strstart(text,"/whereis")==0 || strstart(text,"/where")==0)
    {
	unsigned int i;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	do_whois(c,&text[i]);

	return 0;
    }
    if (strstart(text,"/whoami")==0)
    {
	char const * tname;

	if (!(tname = conn_get_username(c)))
	{
	    message_send_text(c,message_type_error,c,"Unable to obtain your account name.");
	    return 0;
	}

	do_whois(c,tname);
	conn_unget_username(c,tname);

	return 0;
    }
    if (strstart(text,"/announce")==0)
    {
	char         msgtemp[MAX_MESSAGE_LEN];
	unsigned int i;
	char const * tname;
	t_message *  message;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
	    account_get_auth_announce(conn_get_account(c))!=1) /* default to false */
	{
	    message_send_text(c,message_type_info,c,"You do not have permission to use this command.");
	    return 0;
	}

	snprintf(msgtemp, sizeof(msgtemp), "Announcement from %.64s: %.128s",(tname = conn_get_username(c)),&text[i]);
	eventlog(eventlog_level_command,"handle_command","[%d] %.64s announced: %.128s", conn_get_socket(c), tname,&text[i]);
	conn_unget_username(c,tname);
	if (!(message = message_create(message_type_broadcast,c,NULL,msgtemp)))
	    message_send_text(c,message_type_info,c,"Could not broadcast message.");
	else
	{
	    if (message_send_all(message)<0)
		message_send_text(c,message_type_info,c,"Could not broadcast message.");
	    message_destroy(message);
	}

	return 0;
    }
    if (strstart(text,"/beep")==0)
    {
	message_send_text(c,message_type_info,c,"Audible notification on."); /* FIXME: actually do something */
	return 0; /* FIXME: these only affect CHAT clients... I think they prevent ^G from being sent */
    }
    if (strstart(text,"/nobeep")==0)
    {
	message_send_text(c,message_type_info,c,"Audible notification off."); /* FIXME: actually do something */
	return 0;
    }
    if (strstart(text,"/version")==0)
    {
	message_send_text(c,message_type_info,c,"BNETD "BNETD_VERSION);

	return 0;
    }
    if (strstart(text,"/copyright")==0 || strstart(text,"/warranty")==0 || strstart(text,"/license")==0)
    {
	static char const * const info[] =
	{
	    " Copyright (C) 1998,1999,2000,2001,2002  See source for details",
	    " ",
	    " This program is free software; you can redistribute it and/or",
	    " modify it under the terms of the GNU General Public License",
	    " as published by the Free Software Foundation; either version 2",
	    " of the License, or (at your option) any later version.",
	    " ",
	    " This program is distributed in the hope that it will be useful,",
	    " but WITHOUT ANY WARRANTY; without even the implied warranty of",
	    " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
	    " GNU General Public License for more details.",
	    " ",
	    " You should have received a copy of the GNU General Public License",
	    " along with this program; if not, write to the Free Software",
	    " Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.",
	    NULL
	};
	unsigned int i;

	for (i=0; info[i]; i++)
		message_send_text(c,message_type_info,c,info[i]);

	return 0;
    }
    if (strstart(text,"/uptime")==0)
    {
	char msgtemp[MAX_MESSAGE_LEN];

	snprintf(msgtemp, sizeof(msgtemp), "Uptime: %s",seconds_to_timestr(server_get_uptime()));
	message_send_text(c,message_type_info,c,msgtemp);

	return 0;
    }
    if (strstart(text,"/stats")==0 || strstart(text,"/astat")==0)
    {
	char         msgtemp[MAX_MESSAGE_LEN];
	char         dest[MAX_NICK_LEN];
	unsigned int i,j;
	t_account *  account;
	char const * clienttag;
	char const * tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

	if (dest[0]=='\0') /* if no argument, stat ourself */
	{
	    strcpy(dest,(tname = conn_get_username(c)));
	    conn_unget_username(c,tname);
	}

#ifdef WITH_BITS
	if (bits_va_command_with_account_name(c,text,dest))
	    return 0;
#endif

	if (!(account = accountlist_find_account(dest)))
	{
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#ifdef WITH_BITS
	if (account_is_invalid(account)) {
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#endif

	if (text[i]!='\0')
	    clienttag = &text[i];
	else if (!(clienttag = conn_get_clienttag(c)))
	{
	    message_send_text(c,message_type_error,c,"Unable to determine client game.");
	    return 0;
	}

	if (strlen(clienttag)!=4)
	{
	    snprintf(msgtemp, sizeof(msgtemp), "You must supply a user name and a valid program ID. (Program ID \"%.32s\" is invalid.)",clienttag);
	    message_send_text(c,message_type_error,c,msgtemp);
	    message_send_text(c,message_type_error,c,"Example: /stats joe STAR");
	    return 0;
	}

	if (strcasecmp(clienttag,CLIENTTAG_BNCHATBOT)==0)
	{
	    message_send_text(c,message_type_error,c,"This game does not support win/loss records.");
	    message_send_text(c,message_type_error,c,"You must supply a user name and a valid program ID.");
	    message_send_text(c,message_type_error,c,"Example: /stats joe STAR");
	    return 0;
	}
	else if (strcasecmp(clienttag,CLIENTTAG_DIABLORTL)==0 ||
	         strcasecmp(clienttag,CLIENTTAG_DIABLOSHR)==0)
	{
	    snprintf(msgtemp, sizeof(msgtemp), "%.64s's record:",(tname = account_get_name(account)));
	    account_unget_name(tname);
	    message_send_text(c,message_type_info,c,msgtemp);

	    snprintf(msgtemp, sizeof(msgtemp), "level: %u",account_get_normal_level(account,clienttag));
	    message_send_text(c,message_type_info,c,msgtemp);

	    snprintf(msgtemp, sizeof(msgtemp), "class: %.16s",bnclass_get_str(account_get_normal_class(account,clienttag)));

	    message_send_text(c,message_type_info,c,msgtemp);

	    snprintf(msgtemp, sizeof(msgtemp), "stats: %u str  %u mag  %u dex  %u vit  %u gld",
		    account_get_normal_strength(account,clienttag),
		    account_get_normal_magic(account,clienttag),
		    account_get_normal_dexterity(account,clienttag),
		    account_get_normal_vitality(account,clienttag),
		    account_get_normal_gold(account,clienttag));
	    message_send_text(c,message_type_info,c,msgtemp);

	    snprintf(msgtemp, sizeof(msgtemp), "Diablo kills: %u",account_get_normal_diablo_kills(account,clienttag));
	    message_send_text(c,message_type_info,c,msgtemp);
	}
	else if (strcasecmp(clienttag,CLIENTTAG_WARCIIBNE)==0)
	{
	    snprintf(msgtemp, sizeof(msgtemp), "%.64s's record:",(tname = account_get_name(account)));
	    account_unget_name(tname);
	    message_send_text(c,message_type_info,c,msgtemp);

#ifdef GAMEI_SERVER
	    if (account_get_ladder_rating(account,clienttag,ladder_id_normal)>0)
	    snprintf(msgtemp, sizeof(msgtemp), "%u points (%u-%u-%u)",
		account_get_ladder_rating(account,clienttag,ladder_id_ironman),
	        account_get_ladder_wins(account,clienttag,ladder_id_ironman),
	        account_get_ladder_losses(account,clienttag,ladder_id_ironman),
	        account_get_ladder_disconnects(account,clienttag,ladder_id_normal));
	    else
		strcpy(msgtemp,"1000 points (0-0-0)");

#else
	    snprintf(msgtemp, sizeof(msgtemp), "Normal games: %u-%u-%u",
		    account_get_normal_wins(account,clienttag),
		    account_get_normal_losses(account,clienttag),
		    account_get_normal_disconnects(account,clienttag));
	    message_send_text(c,message_type_info,c,msgtemp);
	    if (account_get_ladder_rating(account,clienttag,ladder_id_normal)>0)
		snprintf(msgtemp, sizeof(msgtemp), "Ladder games: %u-%u-%u (rating %u)",
			account_get_ladder_wins(account,clienttag,ladder_id_normal),
			account_get_ladder_losses(account,clienttag,ladder_id_normal),
			account_get_ladder_disconnects(account,clienttag,ladder_id_normal),
			account_get_ladder_rating(account,clienttag,ladder_id_normal));
	    else
		strcpy(msgtemp,"Ladder games: 0-0-0");
#endif
	    message_send_text(c,message_type_info,c,msgtemp);

	    if (account_get_ladder_rating(account,clienttag,ladder_id_ironman)>0)
		snprintf(msgtemp, sizeof(msgtemp), "IronMan games: %u-%u-%u (rating %u)",
			account_get_ladder_wins(account,clienttag,ladder_id_ironman),
			account_get_ladder_losses(account,clienttag,ladder_id_ironman),
			account_get_ladder_disconnects(account,clienttag,ladder_id_ironman),
			account_get_ladder_rating(account,clienttag,ladder_id_ironman));
	    else
		strcpy(msgtemp,"IronMan games: 0-0-0");
	    message_send_text(c,message_type_info,c,msgtemp);
	}
	else
	{
	    snprintf(msgtemp, sizeof(msgtemp), "%.64s's record:",(tname = account_get_name(account)));
	    account_unget_name(tname);
	    message_send_text(c,message_type_info,c,msgtemp);

#ifdef GAMEI_SERVER
	    if (account_get_ladder_rating(account,clienttag,ladder_id_normal)>0)
	    snprintf(msgtemp, sizeof(msgtemp), "%u points (%u wins %u losses %u disconnects)",
		account_get_ladder_rating(account,clienttag,ladder_id_normal),
		account_get_ladder_wins(account,clienttag,ladder_id_normal),
    		account_get_ladder_losses(account,clienttag,ladder_id_normal),
            	account_get_ladder_disconnects(account,clienttag,ladder_id_normal));
	    else
		strcpy(msgtemp,"1000 points (0-0-0)");
#else
	    snprintf(msgtemp, sizeof(msgtemp), "Normal games: %u-%u-%u",
		    account_get_normal_wins(account,clienttag),
		    account_get_normal_losses(account,clienttag),
		    account_get_normal_disconnects(account,clienttag));
	    message_send_text(c,message_type_info,c,msgtemp);

	    if (account_get_ladder_rating(account,clienttag,ladder_id_normal)>0)
		snprintf(msgtemp, sizeof(msgtemp), "Ladder games: %u-%u-%u (rating %u)",
			account_get_ladder_wins(account,clienttag,ladder_id_normal),
			account_get_ladder_losses(account,clienttag,ladder_id_normal),
			account_get_ladder_disconnects(account,clienttag,ladder_id_normal),
			account_get_ladder_rating(account,clienttag,ladder_id_normal));
	    else
		strcpy(msgtemp,"Ladder games: 0-0-0");
#endif
	    message_send_text(c,message_type_info,c,msgtemp);
	}

	return 0;
    }
    if (strstart(text,"/time")==0)
    {
	char        msgtemp[MAX_MESSAGE_LEN];
	t_bnettime  btsystem;
	t_bnettime  btlocal;
	time_t      now;
	struct tm * tmnow;

	/* Battle.net time: Wed Jun 23 15:15:29 */
	now = time(NULL);
	if (!(tmnow = localtime(&now)))
	    strcpy(msgtemp,"BNETD time: ?");
	else
	    strftime(msgtemp,sizeof(msgtemp),"BNETD time: %a %b %d %H:%M:%S",tmnow);
	message_send_text(c,message_type_info,c,msgtemp);
	if (conn_get_class(c)==conn_class_bnet)
	{
	btsystem = bnettime();

	    btlocal = bnettime_add_tzbias(btsystem,conn_get_tzbias(c));
	    now = bnettime_to_time(btlocal);
	    if (!(tmnow = gmtime(&now)))
		strcpy(msgtemp,"Your local time: ?");
	    else
		strftime(msgtemp,sizeof(msgtemp),"Your local time: %a %b %d %H:%M:%S",tmnow);
	    message_send_text(c,message_type_info,c,msgtemp);
	}

	return 0;
    }
    if (strstart(text,"/channel")==0 || strstart(text,"/join")==0)
    {
    	unsigned int i;

	    for (i=0; text[i]!=' ' && text[i]!='\0'; i++) /* skip command */
        	for (; text[i]==' '; i++);

        if (text[i]=='\0')
	    {
	        message_send_text(c,message_type_error,c,"Please specify a channel.");
    	    return 0;
	    }

        if (conn_set_channel(c,&text[i],NULL)<0)
            conn_set_channel(c,CHANNEL_NAME_BANNED,NULL); /* should not fail */

    	return 0;
    }
    if (strstart(text,"/pchannel")==0 || strstart(text,"/pjoin")==0)
    {
    	char         channelname[strlen(text)]; /* There's nothing like an max_channel_name_len, so this kind is safe */
	    unsigned int i,j;

    	for (i=0; text[i]!=' ' && text[i]!='\0'; i++)  /* skip command */
	    for (; text[i]==' '; i++);
    	for (j=0; text[i]!=' ' && text[i]!='\0'; i++)   /* get channelname */
	        if (j<sizeof(channelname)-1) channelname[j++] = text[i];
    	channelname[j] = '\0';
	    for (; text[i]==' '; i++);  /* move to password if existant */

        if (channelname[0]=='\0')
        {
	        message_send_text(c,message_type_error,c,"Please specify a channel.");
    	    return 0;
        }

        if (text[i]=='\0')
        {
	        message_send_text(c,message_type_error,c,"You forgot the password.");
    	    return 0;
        }

        if (conn_set_channel(c,channelname,&text[i])<0)
            conn_set_channel(c,CHANNEL_NAME_BANNED,NULL); /* should not fail */

    	return 0;
    }
    if (strstart(text,"/rejoin")==0)
    {
    	const t_channel * channel;
	const char *temp;
    	const char *chname;
        const char *chpasswd = NULL;

        if (!(channel = conn_get_channel(c)))
    	{
	        message_send_text(c,message_type_error,c,"You are not in a channel.");
	        return 0;
    	}

	    if (!(temp = channel_get_name(channel)))
	        return 0;

    	/* we need to copy the channel name (and password) because we might remove the
	       last person (ourself) from the channel and cause the string
    	   to be freed in destroy_channel() */
    	if (!(chname = strdup(temp)))
	    {
        	eventlog(eventlog_level_error,__FUNCTION__,"malloc error while trying to duplicate channelname");
	        return 0;
	    }

        if (channel_get_password(channel))
            if (!(chpasswd = strdup(channel_get_password(channel))))
            {
            	eventlog(eventlog_level_error,__FUNCTION__,"malloc error while trying to duplicate channelpassword");
	            return 0;
            }

        if (conn_set_channel(c, chname, chpasswd) < 0)
            conn_set_channel(c, CHANNEL_NAME_BANNED, NULL); /* should not fail */

        free((void *)chname); /* avoid warning */
   	    free((void *)chpasswd); /* avoid warning */

    	return 0;
    }
    if (strstart(text,"/away")==0)
    {
	unsigned int i;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (text[i]=='\0') /* back to normal */
	{
	    message_send_text(c,message_type_info,c,"You are no longer marked as away.");
	    conn_set_awaystr(c,NULL);
	}
	else
	{
	    message_send_text(c,message_type_info,c,"You are now marked as being away.");
	    conn_set_awaystr(c,&text[i]);
	}

	return 0;
    }
    if (strstart(text,"/dnd")==0)
    {
	unsigned int i;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (text[i]=='\0') /* back to normal */
	{
	    message_send_text(c,message_type_info,c,"Do Not Disturb mode cancelled.");
	    conn_set_dndstr(c,NULL);
	}
	else
	{
	    message_send_text(c,message_type_info,c,"Do Not Disturb mode engaged.");
	    conn_set_dndstr(c,&text[i]);
	}

	return 0;
    }
    if (strstart(text,"/ignore")==0 || strstart(text,"/squelch")==0)
    {
	char         tmsg[MAX_MESSAGE_LEN];
	unsigned int i;
	t_account *  account;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	/* D2 puts * before username - FIXME: the client don't see it until
	 the player rejoins the channel */
	if (text[i]=='*')
	  i++;

	if (text[i]=='\0')
	{
	    conn_send_ignore(c);
	    return 0;
	}
	if (!(account = accountlist_find_account(&text[i])))
	{
	    message_send_text(c,message_type_error,c,"No such user.");
	    return 0;
	}
	if (conn_get_account(c)==account)
	{
	    message_send_text(c,message_type_error,c,"You can't squelch yourself.");
	    return 0;
	}
	if (conn_check_ignoring(c,&text[i])==1)
	{
	    sprintf(tmsg,"You are already ignoring %.64s.",&text[i]);
	    message_send_text(c,message_type_info,c,tmsg);
	    return 0;
	}
	if (conn_add_ignore(c,account)<0)
	    message_send_text(c,message_type_error,c,"Could not squelch user.");
	else
	{
	    char const * tname;

	    sprintf(tmsg,"%-.20s has been squelched.",(tname = account_get_name(account)));
	    account_unget_name(tname);
	    message_send_text(c,message_type_info,c,tmsg);
	}

	return 0;
    }
    if (strstart(text,"/unignore")==0 || strstart(text,"/unsquelch")==0)
    {
	unsigned int      i;
        t_account const * account;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	/* D2 puts * before username - FIXME: the client don't see it until
	 the player rejoins the channel */
	if (text[i]=='*')
	  i++;

	if (text[i]=='\0')
        {
	    message_send_text(c,message_type_info,c,"Who don't you want to ignore?");
	    return 0;
	}

        if (!(account = accountlist_find_account(&text[i])))
	{
	    message_send_text(c,message_type_info,c,"No such user.");
	    return 0;
	}

	if (conn_del_ignore(c,account)<0)
	    message_send_text(c,message_type_info,c,"User was not being ignored.");
	else
	    message_send_text(c,message_type_info,c,"No longer ignoring.");

	return 0;
    }
    if (strstart(text,"/designate")==0)
    {
	char           msgtemp[MAX_MESSAGE_LEN];
	unsigned int   i;
	t_channel *    channel;
        t_connection * noc;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

        if (!(channel = conn_get_channel(c)))
        {
            message_send_text(c,message_type_error,c,"This command can only be used inside a channel.");
	    return 0;
        }
	if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
            channel_get_operator(channel)!=c)
        {
            message_send_text(c,message_type_error,c,"You are not a channel operator.");
	    return 0;
        }
	if (text[i]=='\0')
        {
	    message_send_text(c,message_type_info,c,"Who do you want to designate?");
	    return 0;
	}

        if (!(noc = connlist_find_connection_by_accountname(&text[i])))
        {
            message_send_text(c,message_type_error,c,"That user is not logged in.");
	    return 0;
        }
        if (conn_get_channel(noc)!=channel)
        {
            message_send_text(c,message_type_error,c,"That user is not in this channel.");
            return 0;
        }

        if (channel_set_next_operator(channel,noc)<0)
            message_send_text(c,message_type_error,c,"Unable to designate that user.");
        else
        {
	    char const * tname;

            snprintf(msgtemp, sizeof(msgtemp), "%s will be the new operator when you resign.",(tname = conn_get_username(noc)));
	    conn_unget_username(noc,tname);
            message_send_text(c,message_type_info,c,msgtemp);
        }

        return 0;
    }
    if (strstart(text,"/resign")==0)
    {
	t_channel *    channel;

        if (!(channel = conn_get_channel(c)))
        {
            message_send_text(c,message_type_error,c,"This command can only be used inside a channel.");
	    return 0;
        }
        if (channel_get_operator(channel)!=c)
        {
            message_send_text(c,message_type_error,c,"This command is reserved for channel operators.");
	    return 0;
        }

        if (channel_choose_operator(channel,NULL)<0)
            message_send_text(c,message_type_error,c,"You are unable to resign.");
        else
            message_send_text(c,message_type_info,c,"You are no longer the operator.");

	return 0;
    }
    if (strstart(text,"/kick")==0)
    {
	char              msgtemp[MAX_MESSAGE_LEN];
	char              dest[MAX_NICK_LEN];
	unsigned int      i,j;
	t_channel const * channel;
        t_connection *    kuc;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

        if (!(channel = conn_get_channel(c)))
        {
            message_send_text(c,message_type_error,c,"This command can only be used inside a channel.");
	    return 0;
        }
	if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
            channel_get_operator(channel)!=c)
        {
            message_send_text(c,message_type_error,c,"You are not a channel operator.");
	    return 0;
        }
	if (dest[0]=='\0')
        {
	    message_send_text(c,message_type_info,c,"Who do you want to kick off the channel?");
	    return 0;
	}

        if (!(kuc = connlist_find_connection_by_accountname(dest)))
        {
            message_send_text(c,message_type_error,c,"That user is not logged in.");
	    return 0;
        }
        if (conn_get_channel(kuc)!=channel)
        {
	    message_send_text(c,message_type_error,c,"That user is not in this channel.");
	    return 0;
        }
	if (account_get_auth_admin(conn_get_account(kuc))==1 && account_get_auth_admin(conn_get_account(c))!=1)
	{
	    message_send_text(c,message_type_error,c,"You cannot kick administrators.");
	    return 0;
	}

        {
	    char const * tname1;
	    char const * tname2;

	    tname1 = account_get_name(conn_get_account(kuc));
	    tname2 = account_get_name(conn_get_account(c));
	    if (text[i]!='\0')
        	snprintf(msgtemp, sizeof(msgtemp), "%-.20s has been kicked by %-.20s (%s).",tname1,tname2?tname2:"unknown",&text[i]);
	    else
        	snprintf(msgtemp, sizeof(msgtemp), "%-.20s has been kicked by %-.20s.",tname1,tname2?tname2:"unknown");
	    if (tname2)
		account_unget_name(tname2);
	    if (tname1)
		account_unget_name(tname1);
    	    channel_message_log(channel,channel_loglevel_command,c,0,msgtemp);
            if (prefs_get_kick_notify())
	    {
		if (prefs_get_kick_notify_all())
		    channel_message_send(channel,message_type_info,c,msgtemp);
		else
		    channel_message_send_to_admins(channel,message_type_info,c,msgtemp);
	    }
        }
        conn_set_channel(kuc,CHANNEL_NAME_KICKED,NULL); /* should not fail */

        return 0;
    }
    if (strstart(text,"/ban")==0)
    {
	char           msgtemp[MAX_MESSAGE_LEN];
	char           dest[MAX_NICK_LEN];
	unsigned int   i,j;
	t_channel *    channel;
        t_connection * buc;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

        if (!(channel = conn_get_channel(c)))
        {
            message_send_text(c,message_type_error,c,"This command can only be used inside a channel.");
	    return 0;
        }
	if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
            channel_get_operator(channel)!=c)
        {
            message_send_text(c,message_type_error,c,"You are not a channel operator.");
	    return 0;
        }
	if (dest[0]=='\0')
        {
	    message_send_text(c,message_type_info,c,"Who do you want to ban from the channel?");
	    return 0;
	}

	{
	    t_account * account;

	    if (!(account = accountlist_find_account(dest)))
		message_send_text(c,message_type_info,c,"That account doesn't currently exist, banning anyway.");
	    else
		if (account_get_auth_admin(account)==1) /* default to false */
		{
	 	    message_send_text(c,message_type_error,c,"You cannot ban administrators.");
	 	    return 0;
       	 	}
	}

        if (channel_ban_user(channel,dest)<0)
	{
            snprintf(msgtemp, sizeof(msgtemp), "Unable to ban %-.20s.",dest);
            message_send_text(c,message_type_error,c,msgtemp);
	}
        else
        {
	    char const * tname;

	    tname = account_get_name(conn_get_account(c));
	    if (text[i]!='\0')
        	snprintf(msgtemp, sizeof(msgtemp), "%-.20s has been banned by %-.20s (%s).",dest,tname?tname:"unknown",&text[i]);
	    else
        	snprintf(msgtemp, sizeof(msgtemp), "%-.20s has been banned by %-.20s.",dest,tname?tname:"unknown");
	    if (tname)
		account_unget_name(tname);
    	    channel_message_log(channel,channel_loglevel_command,c,0,msgtemp);
            channel_message_send(channel,message_type_info,c,msgtemp);
        }
        if ((buc = connlist_find_connection_by_accountname(dest)) &&
            conn_get_channel(buc)==channel)
            conn_set_channel(buc,CHANNEL_NAME_BANNED,NULL);

        return 0;
    }
    if (strstart(text,"/unban")==0)
    {
	char         msgtemp[MAX_MESSAGE_LEN];
	t_channel *  channel;
	unsigned int i;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

        if (!(channel = conn_get_channel(c)))
        {
            message_send_text(c,message_type_error,c,"This command can only be used inside a channel.");
	    return 0;
        }
	if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
            channel_get_operator(channel)!=c)
        {
            message_send_text(c,message_type_error,c,"You are not a channel operator.");
	    return 0;
        }
	if (text[i]=='\0')
        {
	    message_send_text(c,message_type_info,c,"Who do you want to unban from the channel?");
	    return 0;
	}

        if (channel_unban_user(channel,&text[i])<0)
            message_send_text(c,message_type_error,c,"That user is not banned.");
        else
        {
            snprintf(msgtemp, sizeof(msgtemp), "%s is no longer banned from this channel.",&text[i]);
            message_send_text(c,message_type_info,c,msgtemp);
        }

        return 0;
    }
    if ((strstart(text,"/friends")==0) ||
	strstart(text,"/f")==0)
    {
	handle_friends_command(c,text);
	return 0;
    }

    if (prefs_get_extra_commands()==0)
    {
	message_send_text(c,message_type_error,c,"Unknown command.");
	eventlog(eventlog_level_debug,"handle_command","got unknown standard command \"%s\"",text);
	return 0;
    }

    /*******************************************************************/

    if (strstart(text,"/reply")==0 || strstart(text,"/r")==0)
    {
	unsigned int i;
	char const * dest;

	if (!(dest = conn_get_lastsender(c)))
        {
            message_send_text(c,message_type_error,c,"No one messaged you, use /m instead");
            return 0;
	}

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (text[i]=='\0')
	{
            message_send_text(c,message_type_error,c,"What do you want to reply?");
            return 0;
	}
	do_whisper(c,dest,&text[i]);
	return 0;
    }
    if (strstart(text,"/ann")==0) /* same as /announce */
    {
	char         msgtemp[MAX_MESSAGE_LEN];
	unsigned int i;
	char const * tname;
	t_message *  message;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
	    account_get_auth_announce(conn_get_account(c))!=1) /* default to false */
	{
	    message_send_text(c,message_type_info,c,"You do not have permission to use this command.");
	    return 0;
	}

	snprintf(msgtemp, sizeof(msgtemp), "Announcement from %.64s: %.128s",(tname = conn_get_username(c)),&text[i]);
	eventlog(eventlog_level_command,"handle_command","[%d] %.64s announced: %.128s", conn_get_socket(c), tname,&text[i]); /* KWS */
	conn_unget_username(c,tname);
	if (!(message = message_create(message_type_broadcast,c,NULL,msgtemp)))
	    message_send_text(c,message_type_info,c,"Could not broadcast message.");
	else
	{
	    if (message_send_all(message)<0)
		message_send_text(c,message_type_info,c,"Could not broadcast message.");
	    message_destroy(message);
	}

	return 0;
    }
     if (strstart(text,"/realmann")==0)
     {
        char         msgtemp[MAX_MESSAGE_LEN];
        unsigned int i;
        char const * realmname;
        char const * tname;
        t_connection * tc;
        t_elem const * curr;
        t_message    * message;

        for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
        for (; text[i]==' '; i++);

        if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
            account_get_auth_announce(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_info,c,"You do not have permission to use this command.");
            return 0;
        }
        if (!(realmname=conn_get_realmname(c))) {
            message_send_text(c,message_type_info,c,"You must join a realm first");
        }
        snprintf(msgtemp, sizeof(msgtemp), "Announcement from %.32s@%.32s: %.128s",(tname = conn_get_username(c)),realmname,&text[i]);
	eventlog(eventlog_level_command,"handle_command","[%d] %.64s announced: %.128s", conn_get_socket(c), tname,&text[i]); /* KWS */
        conn_unget_username(c,tname);
        if (!(message = message_create(message_type_broadcast,c,NULL,msgtemp)))
        {
            message_send_text(c,message_type_info,c,"Could not broadcast message.");
        }
        else
        {
            LIST_TRAVERSE_CONST(connlist(),curr)
            {
                 tc = elem_get_data(curr);
                 if (!tc)
                     continue;
                 if ((conn_get_realmname(tc))&&(strcasecmp(conn_get_realmname(tc),realmname)==0))
                 {
                       message_send(message,tc);
                 }
            }
       }
       return 0;
    }
    if (strstart(text,"/watch")==0)
    {
	handle_watch_command(c,text);
        return 0;
    }
    if (strstart(text,"/unwatch")==0)
    {
	handle_watch_command(c,text);
        return 0;
    }
    if (strstart(text,"/watchall")==0)
    {
	handle_watch_command(c,text);
        return 0;
    }
    if (strstart(text,"/unwatchall")==0)
    {
	handle_watch_command(c,text);
        return 0;
    }
    if (strstart(text,"/lusers")==0)
    {
	char           msgtemp[MAX_MESSAGE_LEN];
	t_channel *    channel;
        t_elem const * curr;
	char const *   banned;
	unsigned int   i;

	if (!(channel = conn_get_channel(c)))
        {
            message_send_text(c,message_type_error,c,"This command can only be used inside a channel.");
	    return 0;
        }

	strcpy(msgtemp,"Banned users:");
	i = strlen(msgtemp);
	LIST_TRAVERSE_CONST(channel_get_banlist(channel),curr)
	{
	    banned = elem_get_data(curr);
	    if (i+strlen(banned)+2>sizeof(msgtemp)) /* " ", name, '\0' */
	    {
		message_send_text(c,message_type_info,c,msgtemp);
		i = 0;
	    }
	    sprintf(&msgtemp[i]," %s",banned);
	    i += strlen(&msgtemp[i]);
	}
	if (i>0)
	    message_send_text(c,message_type_info,c,msgtemp);

	return 0;
    }
    if (strstart(text,"/news")==0)
    {
	char const * filename;
	FILE *       fp;

	if ((filename = prefs_get_newsfile()))
	    if ((fp = fopen(filename,"r")))
	    {
		message_send_file(c,fp);
		if (fclose(fp)<0)
		    eventlog(eventlog_level_error,"handle_command","could not close news file \"%s\" after reading (fclose: %s)",filename,strerror(errno));
	    }
	    else
	    {
		eventlog(eventlog_level_error,"handle_command","could not open news file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
                message_send_text(c,message_type_info,c,"No news today.");
	    }
	else
            message_send_text(c,message_type_info,c,"No news today.");

	return 0;
    }
    if (strstart(text,"/games")==0)
    {
	unsigned int   i;
	char           msgtemp[MAX_MESSAGE_LEN];
        t_elem const * curr;
#ifndef WITH_BITS
        t_game const * game;
#else
        t_bits_game const * bgame;
#endif
	char const *   tag;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (text[i]=='\0')
	{
	    tag = conn_get_clienttag(c);
	    message_send_text(c,message_type_info,c,"Currently accessable games:");
	}
	else if (strcmp(&text[i],"all")==0)
	{
	    tag = NULL;
	    message_send_text(c,message_type_info,c,"All current games:");
	}
	else
	{
	    tag = &text[i];
	    message_send_text(c,message_type_info,c,"Current games of that type:");
	}

	if (prefs_get_hide_addr() && account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
	    snprintf(msgtemp, sizeof(msgtemp), " ------name------ p -status- --------type--------- count");
	else
	    snprintf(msgtemp, sizeof(msgtemp), " ------name------ p -status- --------type--------- count --------addr--------");
	message_send_text(c,message_type_info,c,msgtemp);
#ifndef WITH_BITS
        LIST_TRAVERSE_CONST(gamelist(),curr)
#else
        LIST_TRAVERSE_CONST(bits_gamelist(),curr)
#endif
	{
#ifdef WITH_BITS
	    t_game * game;

	    bgame = elem_get_data(curr);
	    game = bits_game_create_temp(bits_game_get_id(bgame));
#else
	    game = elem_get_data(curr);
#endif
	    if ((!tag || !prefs_get_hide_pass_games() || strcmp(game_get_pass(game),"")==0) &&
		(!tag || strcasecmp(game_get_clienttag(game),tag)==0))
		{
		    if (prefs_get_hide_addr() && account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        	        snprintf(msgtemp, sizeof(msgtemp), " %-16.16s %1.1s %-8.8s %-21.21s %5u",
				game_get_name(game),
				strcmp(game_get_pass(game),"")==0 ? "n":"y",
				game_status_get_str(game_get_status(game)),
				game_type_get_str(game_get_type(game)),
				game_get_ref(game));
		    else
        	        snprintf(msgtemp, sizeof(msgtemp), " %-16.16s %1.1s %-8.8s %-21.21s %5u %s",
				game_get_name(game),
				strcmp(game_get_pass(game),"")==0 ? "n":"y",
				game_status_get_str(game_get_status(game)),
				game_type_get_str(game_get_type(game)),
				game_get_ref(game),
				addr_num_to_addr_str(game_get_addr(game),game_get_port(game)));
                    message_send_text(c,message_type_info,c,msgtemp);
		}
#ifdef WITH_BITS
	    bits_game_destroy_temp(game);
#endif
	}

	return 0;
    }
    if (strstart(text,"/channels")==0 || strstart(text,"/chs")==0)
    {
	unsigned int      i;
	char              msgtemp[MAX_MESSAGE_LEN];
	t_elem const *    curr;
        t_channel const * channel;
	t_connection *    opr;
	char const *      oprname;
	char const *      tag;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (text[i]=='\0')
	{
	    tag = conn_get_clienttag(c);
	    message_send_text(c,message_type_info,c,"Currently accessable channels:");
	}
	else if (strcmp(&text[i],"all")==0)
	{
	    tag = NULL;
	    message_send_text(c,message_type_info,c,"All current channels:");
	}
	else
	{
	    tag = &text[i];
	    message_send_text(c,message_type_info,c,"Current channels of that type:");
	}

	snprintf(msgtemp, sizeof(msgtemp), " ----------name---------- users ----operator----");
	message_send_text(c,message_type_info,c,msgtemp);
	LIST_TRAVERSE_CONST(channellist(),curr)
	{
	    channel = elem_get_data(curr);
	    if ((!tag || !prefs_get_hide_temp_channels() || channel_get_permanent(channel)) &&
		(!tag || !channel_get_clienttag(channel) ||
		 strcasecmp(channel_get_clienttag(channel),tag)==0))
	    {
		if ((opr = channel_get_operator(channel)))
		    oprname = conn_get_username(opr);
		else
		    oprname = NULL;
        	snprintf(msgtemp, sizeof(msgtemp), " %-24.24s %5u %-16.16s",
			channel_get_name(channel),
			channel_get_length(channel),
			oprname?oprname:"");
		if (oprname)
		    conn_unget_username(opr,oprname);
        	message_send_text(c,message_type_info,c,msgtemp);
            }
	}

	return 0;
    }
    if (strstart(text,"/addacct")==0)
    {
	unsigned int i,j;
	t_account  * temp;
	t_hash       passhash;
	char         username[MAX_USER_NAME];
	char         msgtemp[MAX_MESSAGE_LEN];
	char         pass[256];
	char const * tname;

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
	{
	    message_send_text(c,message_type_error,c,"This command is only enabled for admins.");
	    return 0;
	}

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++);
	for (; text[i]==' '; i++);

	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get username */
    	if (j<sizeof(username)-1) username[j++] = text[i];
	username[j] = '\0';

	for (; text[i]==' '; i++); /* skip spaces */
	for (j=0; text[i]!='\0'; i++) /* get pass (spaces are allowed) */
	    if (j<sizeof(pass)-1) pass[j++] = text[i];
	pass[j] = '\0';

	if (username[0]=='\0' || pass[0]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Command requires USER and PASS as arguments.");
	    return 0;
	}

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s addacct: username=%s, pass=%s", conn_get_socket(c), (tname = conn_get_username(c)),username,pass); /* KWS */
	conn_unget_username(c,tname);

	/* FIXME: truncate or err on too long password */
	for (i=0; i<strlen(pass); i++)
	    if (isupper((int)pass[i])) pass[i] = tolower((int)pass[i]);

	bnet_hash(&passhash,strlen(pass),pass);

	snprintf(msgtemp, sizeof(msgtemp), "Trying to add account \"%s\" with password \"%s\"",username,pass);
	message_send_text(c,message_type_info,c,msgtemp);

	snprintf(msgtemp, sizeof(msgtemp), "Hash is: %s",hash_get_str(passhash));
	message_send_text(c,message_type_info,c,msgtemp);

	if (!(temp = account_create(username,hash_get_str(passhash))))
	{
	    message_send_text(c,message_type_error,c,"Failed to create account!");
	    eventlog(eventlog_level_info,"handle_command","[%d] account \"%s\" not created by admin (failed)",conn_get_socket(c),username);
	    return 0;
	}
	if (!accountlist_add_account(temp))
	{
	    account_destroy(temp);
	    message_send_text(c,message_type_error,c,"Failed to insert account (already exists?)!");
	    eventlog(eventlog_level_info,"handle_command","[%d] account \"%s\" could not be created by admin (insert failed)",conn_get_socket(c),username);
        }
        else
	{
#ifdef ACCT_DYN_LOAD
	    snprintf(msgtemp, sizeof(msgtemp), "Account for %s created.",username);
#else
	    snprintf(msgtemp, sizeof(msgtemp), "Account "UID_FORMAT" created.",account_get_uid(temp));
#endif
	    message_send_text(c,message_type_info,c,msgtemp);
	    eventlog(eventlog_level_info,"handle_command","[%d] account \"%s\" created by admin",conn_get_socket(c),username);
        }
        return 0;
    }
    if (strstart(text,"/chpass")==0)
    {
	unsigned int i,j;
	t_account  * account;
	t_account  * temp;
	t_hash       passhash;
	char         msgtemp[MAX_MESSAGE_LEN];
	char         arg1[256];
	char         arg2[256];
	char const * username;
	char *       pass;
	char const * tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++);
	for (; text[i]==' '; i++);

	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get username/pass */
    	if (j<sizeof(arg1)-1) arg1[j++] = text[i];
	arg1[j] = '\0';

	for (; text[i]==' '; i++); /* skip spaces */
	for (j=0; text[i]!='\0'; i++) /* get pass (spaces are allowed) */
	    if (j<sizeof(arg2)-1) arg2[j++] = text[i];
	arg2[j] = '\0';

	if (arg2[0]=='\0')
	{
	    username = conn_get_username(c);
	    pass     = arg1;
	}
	else
	{
	    username = arg1;
	    pass     = arg2;
	}

	if (pass[0]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Command requires PASS argument.");
	    return 0;
	}

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s chpass: username=%s", conn_get_socket(c), (tname = conn_get_username(c)),username); /* KWS */
	conn_unget_username(c,tname);

	/* FIXME: truncate or err on too long password */
	for (i=0; i<strlen(pass); i++)
	    if (isupper((int)pass[i])) pass[i] = tolower((int)pass[i]);

	bnet_hash(&passhash,strlen(pass),pass);

	snprintf(msgtemp, sizeof(msgtemp), "Trying to change password for account \"%s\" to \"%s\"",username,pass);
	message_send_text(c,message_type_info,c,msgtemp);

	snprintf(msgtemp, sizeof(msgtemp), "Hash is: %s",hash_get_str(passhash));
	message_send_text(c,message_type_info,c,msgtemp);

	if (!(temp = accountlist_find_account(username)))
	{
	    message_send_text(c,message_type_error,c,"Account does not exist.");
	    if (username!=arg1)
		conn_unget_username(c,username);
	    return 0;
	}

	account = conn_get_account(c);

	if ((temp==account && account_get_auth_changepass(account)==0) || /* default to true */
	    (temp!=account && account_get_auth_admin(account)!=1)) /* default to false */
	{
	    eventlog(eventlog_level_info,"handle_command","[%d] password change for \"%s\" refused (no change access)",conn_get_socket(c),username);
	    if (username!=arg1)
		conn_unget_username(c,username);
	    message_send_text(c,message_type_error,c,"Only admins may change passwords for other accounts.");
	    return 0;
	}

	if (username!=arg1)
	    conn_unget_username(c,username);

	if (account_set_pass(temp,hash_get_str(passhash))<0)
	{
	    message_send_text(c,message_type_error,c,"Unable to set password.");
	    return 0;
	}

#ifdef ACCT_DYN_LOAD
	snprintf(msgtemp, sizeof(msgtemp), "Password for %s updated.",username);
#else
	snprintf(msgtemp, sizeof(msgtemp), "Password for account "UID_FORMAT" updated.",account_get_uid(temp));
#endif
	message_send_text(c,message_type_info,c,msgtemp);
        return 0;
    }
    if (strstart(text,"/connections")==0 || strstart(text,"/con")==0)
    {
	char           msgtemp[MAX_MESSAGE_LEN];
	t_elem const * curr;
	t_connection * conn;
	char           name[19];
        unsigned int   i; /* for loop */
        char const *   channel_name;
        char const *   game_name;

	if (!prefs_get_enable_conn_all() && account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_error,c,"This command is only enabled for admins.");
	    return 0;
        }

        message_send_text(c,message_type_info,c,"Current connections:");
        /* addon */
        for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
        for (; text[i]==' '; i++);

	if (text[i]=='\0')
	{
	    snprintf(msgtemp, sizeof(msgtemp), " -class -tag -----name------ -lat(ms)- ----channel---- --game--");
	    message_send_text(c,message_type_info,c,msgtemp);
	}
	else
	    if (strcmp(&text[i],"all")==0) /* print extended info */
	    {
	        if (prefs_get_hide_addr() && account_get_auth_admin(conn_get_account(c))!=1)
                    snprintf(msgtemp, sizeof(msgtemp), " -#- -class ----state--- -tag -----name------ -session-- -flag- -lat(ms)- ----channel---- --game--");
                else
	            snprintf(msgtemp, sizeof(msgtemp), " -#- -class ----state--- -tag -----name------ -session-- -flag- -lat(ms)- ----channel---- --game-- ---------addr--------");
		message_send_text(c,message_type_info,c,msgtemp);
	    }
	    else
	    {
                message_send_text(c,message_type_error,c,"Unknown option.");
		return 0;
	    }

        LIST_TRAVERSE_CONST(connlist(),curr)
        {
	    conn = elem_get_data(curr);
	    if (conn_get_account(conn))
	    {
		char const * tname;

		sprintf(name,"\"%.16s\"",(tname = conn_get_username(conn)));
		conn_unget_username(conn,tname);
	    }
	    else
		strcpy(name,"(none)");

	    if (conn_get_channel(conn)!=NULL)
                channel_name = channel_get_name(conn_get_channel(conn));
            else channel_name = "none";
            if (conn_get_game(conn)!=NULL)
                game_name = game_get_name(conn_get_game(conn));
            else game_name = "none";

            if (text[i]=='\0')
                snprintf(msgtemp, sizeof(msgtemp), " %-6.6s %4.4s %-16.16s %9u %-16.16s %-8.8s",
                        conn_class_get_str(conn_get_class(conn)),
                        conn_get_fake_clienttag(conn),
                        name,
                        conn_get_latency(conn),
                        channel_name,
                        game_name);
            else
	        if (prefs_get_hide_addr() && account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
                    snprintf(msgtemp, sizeof(msgtemp), " %3d %-6.6s %-12.12s %4.4s %-16.16s 0x%08x 0x%04x %9u %-16.16s %-8.8s",
                            conn_get_socket(conn),
                            conn_class_get_str(conn_get_class(conn)),
                            conn_state_get_str(conn_get_state(conn)),
                            conn_get_fake_clienttag(conn),
                            name,
                            conn_get_sessionkey(conn),
                            conn_get_flags(conn),
                            conn_get_latency(conn),
                            channel_name,
                            game_name);
  	        else
                    snprintf(msgtemp, sizeof(msgtemp), " %3d %-6.6s %-12.12s %4.4s %-16.16s 0x%08x 0x%04x %9u %-16.16s %-8.8s %s",
			    conn_get_socket(conn),
			    conn_class_get_str(conn_get_class(conn)),
			    conn_state_get_str(conn_get_state(conn)),
			    conn_get_fake_clienttag(conn),
			    name,
			    conn_get_sessionkey(conn),
			    conn_get_flags(conn),
			    conn_get_latency(conn),
                            channel_name,
                            game_name,
			    addr_num_to_addr_str(conn_get_addr(conn),conn_get_port(conn)));

            message_send_text(c,message_type_info,c,msgtemp);
        }

	return 0;
    }
    if (strstart(text,"/finger")==0)
    {
	char           dest[MAX_NICK_LEN];
	char           msgtemp[MAX_MESSAGE_LEN];
	unsigned int   i,j;
	t_account *    account;
        t_connection * conn;
	char const *   host;
	char *         tok;
	char const *   tname;
	char const *   tsex;
	char const *   tloc;
	char const *   tage;
	char const *   thost;
	char const *   tdesc;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

#ifdef WITH_BITS
	if (bits_va_command_with_account_name(c,text,dest))
	    return 0;
#endif

	if (!(account = accountlist_find_account(dest)))
	{
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#ifdef WITH_BITS
	if (account_is_invalid(account)) {
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#endif
#ifdef ACCT_DYN_LOAD
        snprintf(msgtemp, sizeof(msgtemp), "Login: %-16.16s           Sex: %.14s",
		(tname = account_get_name(account)),
		(tsex = account_get_sex(account)));
#else
	snprintf(msgtemp, sizeof(msgtemp), "Login: %-16.16s "UID_FORMAT" Sex: %.14s",
		(tname = account_get_name(account)),
		account_get_uid(account),
		(tsex = account_get_sex(account)));
#endif
	account_unget_name(tname);
	account_unget_sex(tsex);
        message_send_text(c,message_type_info,c,msgtemp);

        snprintf(msgtemp, sizeof(msgtemp), "Location: %-23.23s Age: %.14s",
		(tloc = account_get_loc(account)),
		(tage = account_get_age(account)));
	account_unget_loc(tloc);
	account_unget_age(tage);
        message_send_text(c,message_type_info,c,msgtemp);

        if (!(host = thost = account_get_ll_host(account)) ||
	    account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
	    host = "unknown";

        {
            time_t      then;
	    struct tm * tmthen;

            then = account_get_ll_time(account);
	    tmthen = localtime(&then); /* FIXME: determine user's timezone */
	    if (!(conn = connlist_find_connection_by_accountname(dest)))
		if (tmthen)
		    strftime(msgtemp,sizeof(msgtemp),"Last login %a %b %d %H:%M from ",tmthen);
		else
		    strcpy(msgtemp,"Last login ? from ");
	    else
		if (tmthen)
		    strftime(msgtemp,sizeof(msgtemp),"On since %a %b %d %H:%M from ",tmthen);
		else
		    strcpy(msgtemp,"On since ? from ");
        }
	strncat(msgtemp,host,32);
	if (thost)
	    account_unget_ll_host(thost);

	if (account_get_auth_admin(conn_get_account(c))==1 || prefs_get_hide_addr()==0)
	{
	    if (conn)
		snprintf(msgtemp, sizeof(msgtemp), "%s (%s)",msgtemp,addr_num_to_ip_str(conn_get_addr(conn)));
	    else
	    {
	      tname = account_get_ll_ip(account);
	      tname = tname;
	      snprintf(msgtemp, sizeof(msgtemp), "%s (%s)",msgtemp,tname ? tname : "unknown");
	      account_unget_ll_ip(tname);
	    }
	}
	message_send_text(c,message_type_info,c,msgtemp);

	if (conn)
	{
	    snprintf(msgtemp, sizeof(msgtemp), "Idle %s",seconds_to_timestr(conn_get_idletime(conn)));
 	    message_send_text(c,message_type_info,c,msgtemp);
	}

        strncpy(msgtemp,(tdesc = account_get_desc(account)),sizeof(msgtemp));
	msgtemp[sizeof(msgtemp)-1] = '\0';
	account_unget_desc(tdesc);
        for (tok=strtok(msgtemp,"\r\n"); tok; tok=strtok(NULL,"\r\n"))
	    message_send_text(c,message_type_info,c,tok);
        message_send_text(c,message_type_info,c,"");

	return 0;
    }
    if (strstart(text,"/operator")==0 || strstart(text,"/op")==0)
    {
	char                 msgtemp[MAX_MESSAGE_LEN];
	t_connection const * opr;
	t_channel const *    channel;

        if (!(channel = conn_get_channel(c)))
        {
            message_send_text(c,message_type_error,c,"This command can only be used inside a channel.");
	    return 0;
        }

	if (!(opr = channel_get_operator(channel)))
	    strcpy(msgtemp,"There is no operator.");
	else
	{
	    char const * tname;

	    snprintf(msgtemp, sizeof(msgtemp), "%.64s is the operator.",(tname = conn_get_username(opr)));
	    conn_unget_username(opr,tname);
	}
        message_send_text(c,message_type_info,c,msgtemp);
        return 0;
    }
    if (strstart(text,"/admins")==0)
    {
       char            msgtemp[MAX_MESSAGE_LEN];
       unsigned int    i;
       t_elem const *  curr;
       t_connection *  tc;
       char const *    nick;
       int             remove_sep;

       strcpy(msgtemp,"Currently logged on Administrators: ");
       i = strlen(msgtemp);
       remove_sep = 0;
       LIST_TRAVERSE_CONST(connlist(),curr)
       {
           tc = elem_get_data(curr);
           if (!tc)
               continue;
           if (account_get_auth_admin(conn_get_account(tc))==1)
           {
               if ((nick = conn_get_username(tc)))
               {
                   if (i+strlen(nick)+2>sizeof(msgtemp)) /* " ", name, '\0' */
                   {
		       if (remove_sep)
			  msgtemp[i-2] = '\0';
                       message_send_text(c,message_type_info,c,msgtemp);
                       i = 0;
		       remove_sep = 0;
                   }
                   sprintf(&msgtemp[i],"%.64s, ", nick);
                   i += strlen(&msgtemp[i]);
		   remove_sep = 1;
                   conn_unget_username(tc,nick);
               }
           }
       }
	if (i>0)
	{
	    if (remove_sep)
	        msgtemp[i-2] = '\0';
	    message_send_text(c,message_type_info,c,msgtemp);
	}
       return 0;
    }
    if (strstart(text,"/logout")==0 || strstart(text,"/quit")==0 || strstart(text,"/exit")==0)
    {
	message_send_text(c,message_type_info,c,"Connection closed.");
        conn_set_state(c,conn_state_destroy);
        return 0;
    }
    if (strstart(text,"/kill")==0)
    {
	unsigned int	i,j;
	t_connection *	user;
	char		usernick[MAX_NICK_LEN];
	char		timestr[MAX_TIME_STR];
	char		reason[MAX_REASON_LEN];

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get nick */
	    if (j<sizeof(usernick)-1) usernick[j++] = text[i];
	usernick[j]='\0';

	for (; text[i]==' '; i++);

	if (isdigit(text[i]))
	{
        for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get time */
            if (j<sizeof(timestr)-1) timestr[j++] = text[i];
        timestr[j] = '\0';
        for (; text[i]==' '; i++);
	}
	else
        sprintf(timestr,"0");

	for (j=0; text[i]!='\0'; i++) /* get reason */
    	    if (j<sizeof(reason)-1) reason[j++] = text[i];
	reason[j] = '\0';
	if (strcasecmp(reason,"") == 0)
	    sprintf(reason,"not given");

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
	    return 0;
        }
	if (usernick[0]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Which user do you want to kill?");
	    return 0;
	}

    /* JEBs20020922: Changed this from "...by_accountname" to "...by_name" to get the ability of
                     kill users by charname from within a chat- or botclient */
    if (!(user = connlist_find_connection_by_name(usernick,conn_get_realmname(c))))
	{
	    message_send_text(c,message_type_error,c,"That user is not logged in?");
	    return 0;
	}

	ipbanlist_add(c,addr_num_to_ip_str(conn_get_addr(user)),ipbanlist_str_to_time_t(c,timestr),NULL,reason);
        conn_set_state(user,conn_state_destroy);
        return 0;
    }
    if (strstart(text,"/killsession")==0)
    {
	unsigned int	i,j;
	t_connection *	user;
	char		session[16]; /* FIXME: max lenght? */
	char		timestr[MAX_TIME_STR];
	char		reason[MAX_REASON_LEN];

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get nick */
	    if (j<sizeof(session)-1) session[j++] = text[i];
	session[j]='\0';
	for (; text[i]==' '; i++);

        if (isdigit(text[i]))
	{
	    for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get time */
		if (j<sizeof(timestr)-1) timestr[j++] = text[i];
	    timestr[j] = '\0';
	    for (; text[i]==' '; i++);
	}
        else
	    sprintf(timestr,"0");

	for (j=0; text[i]!='\0'; i++) /* get reason */
	    if (j<sizeof(reason)-1) reason[j++] = text[i];
	reason[j] = '\0';
	if (strcasecmp(reason,"") == 0)
	    sprintf(reason,"not given");

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
	    return 0;
        }
	if (session[0]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Which session do you want to kill?");
	    return 0;
	}
	if (!isxdigit((int)session[0]))
	{
	    message_send_text(c,message_type_error,c,"That is not a valid session.");
	    return 0;
	}
	if (!(user = connlist_find_connection_by_sessionkey((unsigned int)strtoul(session,NULL,16))))
	{
            message_send_text(c,message_type_error,c,"That session does not exist.");
	    return 0;
	}

	ipbanlist_add(c,addr_num_to_ip_str(conn_get_addr(user)),ipbanlist_str_to_time_t(c,timestr),NULL,reason);
        conn_set_state(user,conn_state_destroy);
        return 0;
    }
    if (strstart(text,"/gameinfo")==0)
    {
	unsigned int   i;
	char           msgtemp[MAX_MESSAGE_LEN];
	t_game const * game;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (text[i]=='\0')
	{
	    if (!(game = conn_get_game(c)))
	    {
		message_send_text(c,message_type_error,c,"You are not in a game.");
		return 0;
	    }
	}
        else
	    if (!(game = gamelist_find_game(&text[i],game_type_all)))
	    {
		message_send_text(c,message_type_error,c,"That game does not exist.");
		return 0;
	    }

        snprintf(msgtemp, sizeof(msgtemp), "Name: %-20.20s    ID: "GAMEID_FORMAT" (%s)",game_get_name(game),game_get_id(game),strcmp(game_get_pass(game),"")==0?"public":"private");
        message_send_text(c,message_type_info,c,msgtemp);

	{
	    t_account *  owner;
	    char const * tname;
	    char const * namestr;

	    if (!(owner = conn_get_account(game_get_owner(game))))
	    {
		tname = NULL;
		namestr = "none";
	    }
	    else
		if (!(tname = account_get_name(owner)))
		    namestr = "unknown";
		else
		    namestr = tname;

            snprintf(msgtemp, sizeof(msgtemp), "Owner: %-20.20s",namestr);

            if (tname)
		account_unget_name(tname);
	}
        message_send_text(c,message_type_info,c,msgtemp);

	if (!prefs_get_hide_addr() || account_get_auth_admin(conn_get_account(c))==1) /* default to false */
	{
	    unsigned int   addr;
	    unsigned short port;
	    unsigned int   taddr;
	    unsigned short tport;

	    taddr=addr = game_get_addr(game);
	    tport=port = game_get_port(game);
	    gametrans_net(conn_get_addr(c),conn_get_port(c),conn_get_local_addr(c),conn_get_local_port(c),&taddr,&tport);

	    if (taddr==addr && tport==port)
        	snprintf(msgtemp, sizeof(msgtemp), "Address: %s",
		    addr_num_to_addr_str(addr,port));
	    else
        	snprintf(msgtemp, sizeof(msgtemp), "Address: %s (trans %s)",
		    addr_num_to_addr_str(addr,port),
		    addr_num_to_addr_str(taddr,tport));
	    message_send_text(c,message_type_info,c,msgtemp);
	}

        snprintf(msgtemp, sizeof(msgtemp), "Client: %4s (version %s, startver %u)",game_get_clienttag(game),vernum_to_verstr(game_get_version(game)),game_get_startver(game));
	message_send_text(c,message_type_info,c,msgtemp);

	{
	    time_t      gametime;
	    struct tm * gmgametime;

	    gametime = game_get_create_time(game);
	    if (!(gmgametime = gmtime(&gametime)))
		strcpy(msgtemp,"Created: ?");
	    else
		strftime(msgtemp,sizeof(msgtemp),"Created: "GAME_TIME_FORMAT,gmgametime);
	    message_send_text(c,message_type_info,c,msgtemp);

	    gametime = game_get_start_time(game);
	    if (gametime!=(time_t)0)
	    {
		if (!(gmgametime = gmtime(&gametime)))
		    strcpy(msgtemp,"Started: ?");
		else
		    strftime(msgtemp,sizeof(msgtemp),"Started: "GAME_TIME_FORMAT,gmgametime);
	    }
	    else
		strcpy(msgtemp,"Started: ");
            message_send_text(c,message_type_info,c,msgtemp);
	}

	snprintf(msgtemp, sizeof(msgtemp), "Status: %s",game_status_get_str(game_get_status(game)));
	message_send_text(c,message_type_info,c,msgtemp);

        snprintf(msgtemp, sizeof(msgtemp), "Type: %-20.20s",game_type_get_str(game_get_type(game)));
	message_send_text(c,message_type_info,c,msgtemp);

        snprintf(msgtemp, sizeof(msgtemp), "Speed: %s",game_speed_get_str(game_get_speed(game)));
	message_send_text(c,message_type_info,c,msgtemp);

        snprintf(msgtemp, sizeof(msgtemp), "Difficulty: %s",game_difficulty_get_str(game_get_difficulty(game)));
	message_send_text(c,message_type_info,c,msgtemp);

        snprintf(msgtemp, sizeof(msgtemp), "Option: %s",game_option_get_str(game_get_option(game)));
	message_send_text(c,message_type_info,c,msgtemp);

	{
	    char const * mapname;

	    if (!(mapname = game_get_mapname(game)))
		mapname = "unknown";
            snprintf(msgtemp, sizeof(msgtemp), "Map: %-20.20s",mapname);
	    message_send_text(c,message_type_info,c,msgtemp);
	}

        snprintf(msgtemp, sizeof(msgtemp), "Map Size: %ux%u",game_get_mapsize_x(game),game_get_mapsize_y(game));
	message_send_text(c,message_type_info,c,msgtemp);
        snprintf(msgtemp, sizeof(msgtemp), "Map Tileset: %s",game_tileset_get_str(game_get_tileset(game)));
	message_send_text(c,message_type_info,c,msgtemp);
        snprintf(msgtemp, sizeof(msgtemp), "Map Type: %s",game_maptype_get_str(game_get_maptype(game)));
	message_send_text(c,message_type_info,c,msgtemp);

        snprintf(msgtemp, sizeof(msgtemp), "Players: %u current, %u total, %u max",game_get_ref(game),game_get_count(game),game_get_maxplayers(game));
	message_send_text(c,message_type_info,c,msgtemp);

	{
	    char const * description;

	    if (!(description = game_get_description(game)))
		description = "";
	    snprintf(msgtemp, sizeof(msgtemp), "Description: %-20.20s",description);
	}

	{
	    char const * tplayers;

	    snprintf(msgtemp, sizeof(msgtemp), "Currently in game:%s",(tplayers = game_get_players(game)));
	    game_unget_players(tplayers);
	    message_send_text(c,message_type_info,c,msgtemp);
	}

	return 0;
    }
    if (strstart(text,"/ladderactivate")==0)
    {
	char const * tname;

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
	    return 0;
        }

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s ladderactivate", conn_get_socket(c), (tname = conn_get_username(c)));
	conn_unget_username(c,tname);

	ladderlist_make_all_active();
	message_send_text(c,message_type_info,c,"Copied current scores to active scores on all ladders.");
	return 0;
    }
    if (strstart(text,"/shutdown")==0)
    {
	char         dest[32];
	unsigned int i,j;
	unsigned int delay;
	char const * tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
	    return 0;
        }

	if (dest[0]=='\0')
	    delay = prefs_get_shutdown_delay();
	else
	    if (clockstr_to_seconds(dest,&delay)<0)
	    {
		message_send_text(c,message_type_error,c,"Invalid delay.");
		return 0;
	    }

	server_quit_delay(delay);

	if (delay)
	{
	    message_send_text(c,message_type_info,c,"You initialized the shutdown sequence.");
	    eventlog(eventlog_level_command,"handle_command","[%d] %.64s shutdown: %d second delay", conn_get_socket(c), (tname = conn_get_username(c)),delay);
	}
	else
	{
	    message_send_text(c,message_type_info,c,"You canceled the shutdown sequence.");
	    eventlog(eventlog_level_command,"handle_command","[%d] %.64s shutdown: canseled", conn_get_socket(c), (tname = conn_get_username(c)));
	}
	conn_unget_username(c,tname);

	return 0;
    }
    if (strstart(text,"/ladderinfo")==0)
    {
	char         dest[32];
	char         msgtemp[MAX_MESSAGE_LEN];
	unsigned int rank;
	unsigned int i,j;
	t_account *  account;
	char const * clienttag;
	char const * tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

	if (dest[0]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Which rank do you want ladder info for?");
	    return 0;
	}
	if (str_to_uint(dest,&rank)<0 || rank<1)
	{
	    message_send_text(c,message_type_error,c,"Invalid rank.");
	    return 0;
	}

	if (text[i]!='\0')
	    clienttag = &text[i];
	else if (!(clienttag = conn_get_clienttag(c)))
	{
	    message_send_text(c,message_type_error,c,"Unable to determine client game.");
	    return 0;
	}

	if (strlen(clienttag)!=4)
	{
	    snprintf(msgtemp, sizeof(msgtemp), "You must supply a rank and a valid program ID. (Program ID \"%.32s\" is invalid.)",clienttag);
	    message_send_text(c,message_type_error,c,msgtemp);
	    message_send_text(c,message_type_error,c,"Example: /ladderinfo 1 STAR");
	    return 0;
	}

	if (strcasecmp(clienttag,CLIENTTAG_STARCRAFT)==0)
	{
            if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_active,CLIENTTAG_STARCRAFT,ladder_id_normal)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Starcraft active  %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_active_wins(account,CLIENTTAG_STARCRAFT,ladder_id_normal),
			account_get_ladder_active_losses(account,CLIENTTAG_STARCRAFT,ladder_id_normal),
			account_get_ladder_active_disconnects(account,CLIENTTAG_STARCRAFT,ladder_id_normal),
			account_get_ladder_active_rating(account,CLIENTTAG_STARCRAFT,ladder_id_normal));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Starcraft active  %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);

	    if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_current,CLIENTTAG_STARCRAFT,ladder_id_normal)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Starcraft current %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_wins(account,CLIENTTAG_STARCRAFT,ladder_id_normal),
			account_get_ladder_losses(account,CLIENTTAG_STARCRAFT,ladder_id_normal),
			account_get_ladder_disconnects(account,CLIENTTAG_STARCRAFT,ladder_id_normal),
			account_get_ladder_rating(account,CLIENTTAG_STARCRAFT,ladder_id_normal));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Starcraft current %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);
	}
	else if (strcasecmp(clienttag,CLIENTTAG_BROODWARS)==0)
	{
	    if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_active,CLIENTTAG_BROODWARS,ladder_id_normal)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Brood War active  %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_active_wins(account,CLIENTTAG_BROODWARS,ladder_id_normal),
			account_get_ladder_active_losses(account,CLIENTTAG_BROODWARS,ladder_id_normal),
			account_get_ladder_active_disconnects(account,CLIENTTAG_BROODWARS,ladder_id_normal),
			account_get_ladder_active_rating(account,CLIENTTAG_BROODWARS,ladder_id_normal));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Brood War active  %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);

	    if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_current,CLIENTTAG_BROODWARS,ladder_id_normal)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Brood War current %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_wins(account,CLIENTTAG_BROODWARS,ladder_id_normal),
			account_get_ladder_losses(account,CLIENTTAG_BROODWARS,ladder_id_normal),
			account_get_ladder_disconnects(account,CLIENTTAG_BROODWARS,ladder_id_normal),
			account_get_ladder_rating(account,CLIENTTAG_BROODWARS,ladder_id_normal));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Brood War current %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);
	}
	else if (strcasecmp(clienttag,CLIENTTAG_WARCIIBNE)==0)
	{
	    if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_active,CLIENTTAG_WARCIIBNE,ladder_id_normal)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II standard active  %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_active_wins(account,CLIENTTAG_WARCIIBNE,ladder_id_normal),
			account_get_ladder_active_losses(account,CLIENTTAG_WARCIIBNE,ladder_id_normal),
			account_get_ladder_active_disconnects(account,CLIENTTAG_WARCIIBNE,ladder_id_normal),
			account_get_ladder_active_rating(account,CLIENTTAG_WARCIIBNE,ladder_id_normal));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II standard active  %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);

	    if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_active,CLIENTTAG_WARCIIBNE,ladder_id_ironman)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II IronMan active   %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_active_wins(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman),
			account_get_ladder_active_losses(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman),
			account_get_ladder_active_disconnects(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman),
			account_get_ladder_active_rating(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II IronMan active   %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);

	    if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_current,CLIENTTAG_WARCIIBNE,ladder_id_normal)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II standard current %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_wins(account,CLIENTTAG_WARCIIBNE,ladder_id_normal),
			account_get_ladder_losses(account,CLIENTTAG_WARCIIBNE,ladder_id_normal),
			account_get_ladder_disconnects(account,CLIENTTAG_WARCIIBNE,ladder_id_normal),
			account_get_ladder_rating(account,CLIENTTAG_WARCIIBNE,ladder_id_normal));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II standard current %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);

	    if ((account = ladder_get_account_by_rank(rank,ladder_sort_highestrated,ladder_time_current,CLIENTTAG_WARCIIBNE,ladder_id_ironman)))
	    {
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II IronMan current  %5u: %-20.20s %u/%u/%u rating %u",
			rank,
			(tname = account_get_name(account)),
			account_get_ladder_wins(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman),
			account_get_ladder_losses(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman),
			account_get_ladder_disconnects(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman),
			account_get_ladder_rating(account,CLIENTTAG_WARCIIBNE,ladder_id_ironman));
		account_unget_name(tname);
	    }
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Warcraft II IronMan current  %5u: <none>",rank);
	    message_send_text(c,message_type_info,c,msgtemp);
	}
	else
	{
	    message_send_text(c,message_type_error,c,"This game does not support win/loss records.");
	    message_send_text(c,message_type_error,c,"You must supply a rank and a valid program ID.");
	    message_send_text(c,message_type_error,c,"Example: /ladderinfo 1 STAR");
	}

	return 0;
    }
    if (strstart(text,"/timer")==0)
    {
	unsigned int i,j;
	unsigned int delta;
	char         deltastr[64];
	t_timer_data data;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get comm */
	    if (j<sizeof(deltastr)-1) deltastr[j++] = text[i];
	deltastr[j] = '\0';
	for (; text[i]==' '; i++);

	if (deltastr[0]=='\0')
	{
	    message_send_text(c,message_type_error,c,"How long do you want the timer to last?");
	    return 0;
	}

	if (clockstr_to_seconds(deltastr,&delta)<0)
	{
	    message_send_text(c,message_type_error,c,"Invalid duration.");
	    return 0;
	}

	if (text[i]=='\0')
	    data.p = strdup("Your timer has expired.");
	else
	    data.p = strdup(&text[i]);

	if (timerlist_add_timer(c,time(NULL)+(time_t)delta,user_timer_cb,data)<0)
	{
	    eventlog(eventlog_level_error,"handle_command","could not add timer");
	    free(data.p);
	    message_send_text(c,message_type_error,c,"Could not set timer.");
	}
	else
	{
	    char msgtemp[MAX_MESSAGE_LEN];

	    snprintf(msgtemp, sizeof(msgtemp), "Timer set for %s",seconds_to_timestr(delta));
	    message_send_text(c,message_type_info,c,msgtemp);
	}

	return 0;
    }
    if (strstart(text,"/netinfo")==0)
    {
	char           dest[MAX_USER_NAME];
	char           msgtemp[MAX_MESSAGE_LEN];
	unsigned int   i,j;
        t_connection * conn;
	char const *   host;
	char const *   thost;
	t_game const * game;
	unsigned int   addr;
	unsigned short port;
	unsigned int   taddr;
	unsigned short tport;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

	if (dest[0]=='\0')
	{
	    char const * tname;

	    strcpy(dest,(tname = conn_get_username(c)));
	    conn_unget_username(c,tname);
	}

	if (!(conn = connlist_find_connection_by_accountname(dest)))
	{
	    message_send_text(c,message_type_error,c,"That user is not logged on.");
	    return 0;
	}

	if (conn_get_account(conn)!=conn_get_account(c) &&
	    prefs_get_hide_addr() && account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
	{
	    message_send_text(c,message_type_error,c,"Address information for other users is only available to admins.");
	    return 0;
	}

	snprintf(msgtemp, sizeof(msgtemp), "Server TCP: %s (bind %s)",addr_num_to_addr_str(conn_get_real_local_addr(conn),conn_get_real_local_port(conn)),addr_num_to_addr_str(conn_get_local_addr(conn),conn_get_local_port(conn)));
	message_send_text(c,message_type_info,c,msgtemp);

        if (!(host=thost = account_get_ll_host(conn_get_account(conn))))
	    host = "unknown";
	snprintf(msgtemp, sizeof(msgtemp), "Client TCP: %s (%.32s)",addr_num_to_addr_str(conn_get_addr(conn),conn_get_port(conn)),host);
	if (thost)
	    account_unget_ll_host(thost);
	message_send_text(c,message_type_info,c,msgtemp);

	taddr=addr = conn_get_game_addr(conn);
	tport=port = conn_get_game_port(conn);
	gametrans_net(conn_get_addr(c),conn_get_port(c),addr,port,&taddr,&tport);

	if (taddr==addr && tport==port)
	    snprintf(msgtemp, sizeof(msgtemp), "Client UDP: %s",
		    addr_num_to_addr_str(addr,port));
	else
	    snprintf(msgtemp, sizeof(msgtemp), "Client UDP: %s (trans %s)",
		    addr_num_to_addr_str(addr,port),
		    addr_num_to_addr_str(taddr,tport));
	message_send_text(c,message_type_info,c,msgtemp);

	if ((game = conn_get_game(conn)))
	{
	    taddr=addr = game_get_addr(game);
	    tport=port = game_get_port(game);
	    gametrans_net(conn_get_addr(c),conn_get_port(c),addr,port,&taddr,&tport);

	    if (taddr==addr && tport==port)
		snprintf(msgtemp, sizeof(msgtemp), "Game UDP:  %s",
			addr_num_to_addr_str(addr,port));
	    else
		snprintf(msgtemp, sizeof(msgtemp), "Game UDP:  %s (trans %s)",
			addr_num_to_addr_str(addr,port),
			addr_num_to_addr_str(taddr,tport));
	}
	else
	    strcpy(msgtemp,"Game UDP:  none");
	message_send_text(c,message_type_info,c,msgtemp);

	return 0;
    }
    if (strstart(text,"/quota")==0)
    {
	char msgtemp[MAX_MESSAGE_LEN];

	snprintf(msgtemp, sizeof(msgtemp), "Your quota allows you to write %u lines per %u seconds.",prefs_get_quota_lines(),prefs_get_quota_time());
	message_send_text(c,message_type_info,c,msgtemp);
	snprintf(msgtemp, sizeof(msgtemp), "Long lines will be considered to wrap every %u characters.",prefs_get_quota_wrapline());
	message_send_text(c,message_type_info,c,msgtemp);
	snprintf(msgtemp, sizeof(msgtemp), "You are not allowed to send lines with more than %u characters.",prefs_get_quota_maxline());
	message_send_text(c,message_type_info,c,msgtemp);

	return 0;
    }
    if (strstart(text,"/lockacct")==0)
    {
	unsigned int   i;
	t_connection * user;
	t_account *    account;
	char const *   tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
	    return 0;
        }
	if (text[i]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Which user do you want to lock?");
	    return 0;
	}
#ifdef WITH_BITS
	if (bits_va_command_with_account_name(c,text,&text[i]))
	    return 0;
#endif

	if (!(account = accountlist_find_account(&text[i])))
	{
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#ifdef WITH_BITS
	if (account_is_invalid(account)) {
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#endif
	if ((user = connlist_find_connection_by_accountname(&text[i])))
	    message_send_text(user,message_type_info,user,"Your account has just been locked by admin.");

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s lockacct: %.128s", conn_get_socket(c), (tname = conn_get_username(c)),&text[i]);
	conn_unget_username(c,tname);

        account_set_auth_lock(account,1);
        message_send_text(c,message_type_error,c,"That user account is now locked.");
        return 0;
    }
    if (strstart(text,"/unlockacct")==0)
    {
	unsigned int   i;
	t_connection * user;
	t_account *    account;
	char const *   tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
        {
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
	    return 0;
        }
	if (text[i]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Which user do you want to unlock?");
	    return 0;
	}
#ifdef WITH_BITS
	if (bits_va_command_with_account_name(c,text,&text[i]))
	    return 0;
#endif
	if (!(account = accountlist_find_account(&text[i])))
	{
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#ifdef WITH_BITS
	if (account_is_invalid(account)) {
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#endif
	if ((user = connlist_find_connection_by_accountname(&text[i])))
	    message_send_text(user,message_type_info,user,"Your account has just been unlocked by admin.");

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s unlockacct: %.128s", conn_get_socket(c), (tname = conn_get_username(c)),&text[i]);
	conn_unget_username(c,tname);

        account_set_auth_lock(account,0);
        message_send_text(c,message_type_error,c,"That user account is now unlocked.");
        return 0;
    }
    if (strstart(text,"/fortune")==0)
    {
	char         msgtemp[MAX_MESSAGE_LEN];
	char const * fcmd;
	FILE *       pp;
	unsigned int len;
	unsigned int i;

	if (!(fcmd = prefs_get_fortunecmd()) || fcmd[0]=='\0')
	{
	    message_send_text(c,message_type_info,c,"Fortune not available.");
	    return 0;
	}

	if (!(pp = runprog_open(fcmd)))
	{
	    message_send_text(c,message_type_error,c,"Fortune failed.");
	    eventlog(eventlog_level_error,"handle_command","could not open fortune process \"%s\"",fcmd);
	    return 0;
	}

	while (fgets(msgtemp,sizeof(msgtemp),pp))
	{
	    len = strlen(msgtemp);
	    if (len>0 && msgtemp[len-1]=='\n') /* FIXME: what about \r? */
		msgtemp[len-1] = '\0';
	    for (i=0; i<len; i++)
		if (msgtemp[i]=='\t')
		    msgtemp[i] = ' ';

	    message_send_text(c,message_type_info,c,msgtemp);
	}

	if (runprog_close(pp)!=0)
	{
	    message_send_text(c,message_type_error,c,"Fortune failed.");
	    eventlog(eventlog_level_error,"handle_command","could not close fortune process");
	}
	return 0;
    }
    if (strstart(text,"/flag")==0)
    {
	char         msgtemp[MAX_MESSAGE_LEN];
	char         dest[32];
	unsigned int i,j;
	unsigned int newflag;
	char const * tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

	if (account_get_auth_admin(conn_get_account(c))!=1)
	{
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
            return 0;
	}
        if (dest[0]=='\0')
	{
	    message_send_text(c,message_type_error,c,"What flags do you want to set?");
	    return 0;
	}

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s flag: %.128s", conn_get_socket(c), (tname = conn_get_username(c)),dest);
	conn_unget_username(c,tname);

	newflag = strtoul(dest,NULL,0);
	conn_set_flags(c,newflag);

	snprintf(msgtemp, sizeof(msgtemp), "Flags set to 0x%08x.",newflag);
	message_send_text(c,message_type_info,c,msgtemp);
        return 0;
    }
    if (strstart(text,"/tag")==0)
    {
	char         msgtemp[MAX_MESSAGE_LEN];
	char         dest[8];
	unsigned int i,j;
	char const * tname;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);
	for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get dest */
	    if (j<sizeof(dest)-1) dest[j++] = text[i];
	dest[j] = '\0';
	for (; text[i]==' '; i++);

        if (account_get_auth_admin(conn_get_account(c))!=1)
	{
            message_send_text(c,message_type_error,c,"This command is reserved for admins.");
            return 0;
        }
        if (strlen(dest)!=4)
	{
            message_send_text(c,message_type_error,c,"Client tag should be four characters long.");
            return 0;
        }

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s tag: %.128s", conn_get_socket(c), (tname = conn_get_username(c)),dest);
	conn_unget_username(c,tname);

        conn_set_clienttag(c,dest);
	snprintf(msgtemp, sizeof(msgtemp), "Client tag set to %s.",dest);
        message_send_text(c,message_type_info,c,msgtemp);
        return 0;
    }

    if (strstart(text,"/bitsinfo")==0)
    {
#ifndef WITH_BITS
	message_send_text(c,message_type_info,c,"This BNETD server was compiled WITHOUT BITS support.");
#else
	char temp[MAX_MESSAGE_LEN];
	t_elem const * curr;
	int count;

	message_send_text(c,message_type_info,c,"This BNETD server was compiled WITH BITS support.");
	snprintf(temp, sizeof(temp), "Server address: 0x%04x",bits_get_myaddr());
	message_send_text(c,message_type_info,c,temp);
	if (bits_uplink_connection) {
	    message_send_text(c,message_type_info,c,"Server type: client/slave");
	} else {
	    message_send_text(c,message_type_info,c,"Server type: master");
	}
	message_send_text(c,message_type_info,c,"BITS routing table:");
	count = 0;
	LIST_TRAVERSE_CONST(bits_routing_table,curr) {
	    t_bits_routing_table_entry *e = elem_get_data(curr);
	    count++;

	    snprintf(temp, sizeof(temp), "Route %d: [%d] -> 0x%04x",count,conn_get_socket(e->conn),e->bits_addr);
	    message_send_text(c,message_type_info,c,temp);
	}
	if (bits_master) {
	    message_send_text(c,message_type_info,c,"BITS host list:");
	    LIST_TRAVERSE_CONST(bits_hostlist,curr) {
	    	t_bits_hostlist_entry *e = elem_get_data(curr);

		if (e->name)
		    if (strlen(e->name)>128)
		    	snprintf(temp, sizeof(temp), "[0x%04x] name=(too long)",e->address);
		    else
		    	snprintf(temp, sizeof(temp), "[0x%04x] name=\"%s\"",e->address,e->name);
		else {
		    eventlog(eventlog_level_error,"handle_command","corruption in bits_hostlist detected");
		    snprintf(temp, sizeof(temp), "[0x%04x] name=(null)",e->address);
		}
	    	message_send_text(c,message_type_info,c,temp);
	    }
	}
#endif
	return 0;
    }

    if (strstart(text,"/help")==0)
    {
	handle_help_command(c,text);
	return 0;
    }

    if (strstart(text,"/mail")==0)
    {
	handle_mail_command(c,text);
	return 0;
    }

    if (strstart(text,"/ipban")==0)
    {
        char const * tname;
	unsigned int i;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
	{
	    message_send_text(c,message_type_info,c,"You do not have permission to use this command.");
	    return 0;
	}

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s ipban %s", conn_get_socket(c), (tname = conn_get_username(c)),&text[i]);
	conn_unget_username(c,tname);

	handle_ipban_command(c,text);
	return 0;
    }

    if (strstart(text,"/set")==0)
    {
	t_account * account;
	unsigned int i,a,k,v;
	unsigned int j1;
        char const * tname;
        char * temp;

	if (!(temp = strdup(text)))
	{
	    eventlog(eventlog_level_error,"handle_command","could not allocate memory for temp (with /set)");
	    free(temp);
	    return 0;
	}
	if (account_get_auth_admin(conn_get_account(c))!=1) /* default to false */
	{
	    message_send_text(c,message_type_error,c,"This command is only enabled for admins.");
	    free(temp);
	    return 0;
	}

	for (i=0; temp[i]!=' ' && temp[i]!='\0'; i++); /* skip command */
	for (; temp[i]==' '; i++);

	if (temp[i]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Which account do you want to change?");
	    free(temp);
	    return 0;
	}

	a=i;
	for (; temp[i]!=' ' && temp[i]!='\0'; i++); /* skip command */
	temp[i] = '\0';
	i++;

#ifdef WITH_BITS
	if (bits_va_command_with_account_name(c,text,&temp[a]))
	{
	    free(temp);
	    return 0;
	}
#endif

	if (!(account = accountlist_find_account(&temp[a])))
	{
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    free(temp);
	    return 0;
	}
#ifdef WITH_BITS
	if (account_is_invalid(account))
	{
	    message_send_text(c,message_type_error,c,"Invalid user.");
	    return 0;
	}
#endif
	k=i;
	for (; temp[i]!=' ' && temp[i]!='\0'; i++); /* skip key */
	j1=i;
	for (; temp[i]==' '; i++);

	if (temp[i]=='\0')
	{
	    message_send_text(c,message_type_error,c,"Which key do you want to change?");
	    free(temp);
	    return 0;
	}

	temp[j1]='\0';
	v=i;

	if (temp[i]=='\0')
	{
	    message_send_text(c,message_type_error,c,"What value do you want to set?");
	    free(temp);
	    return 0;
	}

	eventlog(eventlog_level_command,"handle_command","[%d] %.64s set \"%s\" account \"%s\" key with \"%s\" value", conn_get_socket(c), (tname = conn_get_username(c)),&temp[a],&temp[k],&temp[v]);
	conn_unget_username(c,tname);

	if (strcmp(&temp[k],"BNET\\acct\\username")==0 || strcmp(&temp[k],"BNET\\acct\\passhash1")==0 || strcmp(&temp[k],"BNET\\acct\\userid")==0)
	    message_send_text(c,message_type_error,c,"Can't set this key");
	else if (account_set_strattr(account,&temp[k],&temp[v])<0)
	    message_send_text(c,message_type_error,c,"Unable to set key");
	else
	    message_send_text(c,message_type_error,c,"Key set succesfull");
	free(temp);

	return 0;
    }

    if (strstart(text,"/motd")==0)
    {
	char const * filename;
	FILE *       fp;

	if ((filename = prefs_get_motdfile()))
	    if ((fp = fopen(filename,"r")))
	    {
		message_send_file(c,fp);
		if (fclose(fp)<0)
		    eventlog(eventlog_level_error,"handle_command","could not close motd file \"%s\" after reading (fopen: %s)",filename,strerror(errno));
	    }
	    else
	    {
		eventlog(eventlog_level_error,"handle_command","could not open motd file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
		message_send_text(c,message_type_error,c,"Unable to open motd.");
	    }
	else
	    message_send_text(c,message_type_error,c,"No motd.");
	return 0;
    }

    if (strlen(text)>=2 && strncmp(text,"//",2)==0)
    {
	handle_alias_command(c,text);
	return 0;
    }

    if (strstart(text,"/topic")==0)
    {
	t_channel * channel;
	int         i;


	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (!(channel = conn_get_channel(c)))
	{
	    message_send_text(c,message_type_error,c,"Thic command can only be used inside a channel.");
	    return 0;
	}
	if (text[i]=='\0')
	    channel_send_topic(channel,c);
	else
	{
	    if (account_get_auth_admin(conn_get_account(c))!=1 && /* default to false */
	        channel_get_operator(channel)!=c)
	    {
	        message_send_text(c,message_type_error,c,"You are not a channel operator.");
		return 0;
	    }

	    if (channel_set_topic(channel,&text[i])<0)
	    {
		eventlog(eventlog_level_warn,"handle_command","Couldn't set topic.");
		message_send_text(c,message_type_error,c,"Couldn't set topic.");
	    }
	    else
		message_send_text(c,message_type_info,c,"Topic changed.");
	}
	return 0;
    }

    if (strstart(text,"/ping")==0)
    {
	char tstr[MAX_MESSAGE_LEN];
	int  i;

	for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
	for (; text[i]==' '; i++);

	if (text[i]=='\0')
	    sprintf(tstr,"Your latency: %ims",conn_get_latency(c));
	else
	{
	    t_connection * user;
	    char           usernick[MAX_NICK_LEN];
	    int            j;

	    for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get nick */
		if (j<sizeof(usernick)-1) usernick[j++] = text[i];
	    usernick[j]='\0';

	    if (!(user = connlist_find_connection_by_accountname(usernick)))
	    {
	        message_send_text(c,message_type_error,c,"That user is not logged in?");
	        return 0;
	    }
	    sprintf(tstr,"%.64s's latency: %ims",usernick,conn_get_latency(user));
	}

	message_send_text(c,message_type_info,c,tstr);
	return 0;
    }

#ifdef ACCT_DYN_UNLOAD
    if (strstart(text,"/logref")==0)  /* REMOVE IT XXXXXXXX */
    {
	accountlist_log_ref();
	return 0;
    }
#endif

    message_send_text(c,message_type_error,c,"Unknown command.");
    eventlog(eventlog_level_debug,"handle_command","got unknown command \"%s\"",text);
    return 0;
}
