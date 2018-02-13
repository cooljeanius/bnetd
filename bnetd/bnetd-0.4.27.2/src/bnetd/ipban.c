/*
 * Copyright (C) 2000  Gediminas (gugini@fortas.ktu.lt)
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
#define IPBAN_INTERNAL_ACCESS
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
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include "compat/strchr.h"
#include "compat/strdup.h"
#include "compat/strcasecmp.h"
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
#include <ctype.h>
#include "common/eventlog.h"
#include "common/util.h"
#include "connection.h"
#include "message.h"
#include "prefs.h"
#include "common/list.h"
#include "ipban.h"
#include "common/setup_after.h"

static int identify_ipban_function(const char * funcstr);
static int ipban_func_del(t_connection * c, char const * cp);
static int ipban_func_list(t_connection * c);
static int ipban_func_check(t_connection * c, char const * cp);
static int ipban_unload_entry(t_ipban_entry * e);
static int ipban_identical_entry(t_ipban_entry * e1, t_ipban_entry * e2);
static t_ipban_entry * ipban_str_to_ipban_entry(char const * cp);
static char * ipban_entry_to_str(t_ipban_entry const * entry);
static unsigned long ipban_str_to_ulong(char const * ipaddr);
static int ipban_could_be_exact_ip_str(char const * str);
static int ipban_could_be_ip_str(char const * ipstr);
static void ipban_usage(t_connection * c);

static t_list * ipbanlist_head = NULL;
static time_t lastchecktime = 0;

extern int ipbanlist_create(void)
{
    if (!(ipbanlist_head = list_create()))
	return -1;

    return 0;
}


extern int ipbanlist_destroy(void)
{
    t_elem *		curr;
    t_ipban_entry *	entry;

    if (ipbanlist_head)
    {
	LIST_TRAVERSE(ipbanlist_head,curr)
	{
	    entry = elem_get_data(curr);
	    if (!entry) /* should not happen */
	    {
		eventlog(eventlog_level_error,"ipbanlist_destroy","ipbanlist contains NULL item");
		continue;
	    }
	    if (list_remove_elem(ipbanlist_head,curr)<0)
		eventlog(eventlog_level_error,"ipbanlist_destroy","could not remove item from list");
	    ipban_unload_entry(entry);
	}
	if (list_destroy(ipbanlist_head)<0)
	    return -1;
	ipbanlist_head = NULL;
    }

    return 0;
}


extern int ipbanlist_load(char const * filename)
{
    FILE *		fp;
    char *		buff;
    char *		ip;
    char *		timestr;
    char *		nickstr;
    char *		reason;
    unsigned int	currline;
    unsigned int	endtime;

    if (!filename)
    {
        eventlog(eventlog_level_error,"ipbanlist_load","got NULL filename");
	return -1;
    }

    if (!(fp = fopen(filename,"r")))
    {
        eventlog(eventlog_level_error,"ipbanlist_load","could not open banlist file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
	return -1;
    }

    for (currline=1; (buff = file_get_line(fp)); currline++)
    {
	ip = buff;

	/* eat whitespace in front */
	while (*ip=='\t' || *ip==' ') ip++;
	if (*ip=='\0' || *ip=='#')
	{
	    free(buff);
	    continue;
	}

	/* eat whitespace in back */
	while (ip[strlen(ip)-1]==' ' || ip[strlen(ip)-1]=='\t')
	    ip[strlen(ip)-1] = '\0';

	if (strchr(ip,' ') || strchr(ip,'\t'))
	{
	    timestr = ip;
	    while (*timestr!=' ' && *timestr!='\t') timestr++;
	    *timestr = '\0';
	    timestr++;
	    while (*timestr==' ' || *timestr=='\t') timestr++;
	}
	else
	    timestr = NULL;

	if (timestr)
	    if (strchr(timestr,' ') || strchr(timestr,'\t'))
	    {
		nickstr = timestr;
		while (*nickstr!=' ' && *nickstr!='\t') nickstr++;
		*nickstr = '\0';
		nickstr++;
		while (*nickstr==' ' || *nickstr=='\t') nickstr++;
	    }
	    else
		nickstr = NULL;
	else
	    nickstr = NULL;

	if (nickstr)
	    if (strchr(nickstr,' ') || strchr(nickstr,'\t'))
	    {
		reason = timestr;
		while (*reason!=' ' && *reason!='\t') reason++;
		*reason = '\0';
		reason++;
		while (*reason==' ' || *reason=='\t') reason++;
	    }
	    else
		reason = NULL;
	else
	    reason = NULL;


	if (!timestr)
	    endtime = 0;
	else
	    if (clockstr_to_seconds(timestr,&endtime)<0)
	    {
		eventlog(eventlog_level_error,"ipbanlist_load","could not convert to seconds: \"%s\". Banning pernamently.",timestr);
		endtime = 0;
	    }

	if (ipbanlist_add(NULL,ip,endtime,nickstr ? nickstr : "unknown",reason ? reason : "not given")!=0)
	{
	    free(buff);
	    eventlog(eventlog_level_warn,"ipbanlist_load","error in %.64s at line %u",filename,currline);
	    continue;
	}

	free(buff);
    }

    if (fclose(fp)<0)
        eventlog(eventlog_level_error,"ipbanlist_load","could not close banlist file \"%s\" after reading (fclose: %s)",filename,strerror(errno));

    return 0;
}


extern int ipbanlist_save(char const * filename)
{
    t_elem const *	curr;
    t_ipban_entry *	entry;
    FILE *		fp;
    char *		ipstr;
    char		line[1024];

    if (!filename)
    {
        eventlog(eventlog_level_error,"ipbanlist_save","got NULL filename");
	return -1;
    }

    if (!(fp = fopen(filename,"w")))
    {
        eventlog(eventlog_level_error,"ipbanlist_save","could not open banlist file \"%s\" for writing (fopen: %s)",filename,strerror(errno));
	return -1;
    }

    LIST_TRAVERSE_CONST(ipbanlist_head,curr)
    {
	entry = elem_get_data(curr);
	if (!entry)
	{
	    eventlog(eventlog_level_error,"ipbanlist_save","ipbanlist contains NULL element");
	    continue;
	}
	if (!(ipstr = ipban_entry_to_str(entry)))
	{
	    eventlog(eventlog_level_error,"ipbanlist_save","got NULL ipstr");
	    continue;
	}
	sprintf(line,"%s %lu %.64s %.128s\n",ipstr,entry->endtime,entry->admin,entry->reason);
	if (fwrite(line, strlen(line), 1, fp) <= 0)
	    eventlog(eventlog_level_error,"ipbanlist_save","could not write to banlist file (write: %s)",strerror(errno));
	free(ipstr);
    }

    if (fclose(fp)<0)
    {
        eventlog(eventlog_level_error,"ipbanlist_load","could not close banlist file \"%s\" after writing (fclose: %s)",filename,strerror(errno));
	return -1;
    }

    return 0;
}


extern int ipbanlist_check(char const * ipaddr)
{
    t_elem const *  curr;
    t_ipban_entry * entry;
    char *          whole;
    char const *    ip1;
    char const *    ip2;
    char const *    ip3;
    char const *    ip4;
    int		    counter;
    time_t	    now;

    if (!ipaddr)
    {
	eventlog(eventlog_level_warn,"ipban_check","got NULL ipaddr");
	return -1;
    }

    if (!(whole = strdup(ipaddr)))
    {
    	eventlog(eventlog_level_warn,"ipban_check","could not allocate memory to check ip against wildcard");
	return -1;
    }

    time(&now);
    eventlog(eventlog_level_debug,"ipban_check","lastcheck: %u, now: %u, now-lc: %u.",(unsigned)lastchecktime,(unsigned)now,(unsigned)(now-lastchecktime));

    if (now - lastchecktime >= prefs_get_ipban_check_int()) /* unsigned; no need to check prefs < 0 */
    {
	ipbanlist_unload_expired();
	lastchecktime = now;
    }

    ip1 = strtok(whole,".");
    ip2 = strtok(NULL,".");
    ip3 = strtok(NULL,".");
    ip4 = strtok(NULL,".");

    if (!ip1 || !ip2 || !ip3 || !ip4)
    {
	eventlog(eventlog_level_warn,"ipban_check","got bad IP address \"%s\"",ipaddr);
	free(whole);
	return -1;
    }

    eventlog(eventlog_level_debug,"ipban_check","checking %s.%s.%s.%s",ip1,ip2,ip3,ip4);

    counter = 0;
    LIST_TRAVERSE_CONST(ipbanlist_head,curr)
    {
	entry = elem_get_data(curr);
	if (!entry)
	{
	    eventlog(eventlog_level_error,"ipbanlist_check","ipbanlist contains NULL item");
	    return -1;
	}
	counter++;
	switch (entry->type)
	{
	case ipban_type_exact:
	    if (strcmp(entry->info1,ipaddr)==0)
	    {
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s matched exact %s",ipaddr,entry->info1);
		free(whole);
		return counter;
	    }
	    eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match exact %s",ipaddr,entry->info1);
	    continue;

	case ipban_type_wildcard:
	    if (strcmp(entry->info1,"*")!=0 && strcmp(ip1,entry->info1)!=0)
	    {
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match part 1 of wildcard %s.%s.%s.%s",ipaddr,entry->info1,entry->info2,entry->info3,entry->info4);
		continue;
	    }
	    if (strcmp(entry->info2,"*")!=0 && strcmp(ip2,entry->info2)!=0)
	    {
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match part 2 of wildcard %s.%s.%s.%s",ipaddr,entry->info1,entry->info2,entry->info3,entry->info4);
		continue;
	    }
	    if (strcmp(entry->info3,"*")!=0 && strcmp(ip3,entry->info3)!=0)
	    {
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match part 3 of wildcard %s.%s.%s.%s",ipaddr,entry->info1,entry->info2,entry->info3,entry->info4);
		continue;
	    }
	    if (strcmp(entry->info4,"*")!=0 && strcmp(ip4,entry->info4)!=0)
	    {
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match part 4 of wildcard %s.%s.%s.%s",ipaddr,entry->info1,entry->info2,entry->info3,entry->info4);
		continue;
	    }

	    eventlog(eventlog_level_debug,"ipbanlist_check","address %s matched wildcard %s.%s.%s.%s",ipaddr,entry->info1,entry->info2,entry->info3,entry->info4);
	    free(whole);
	    return counter;

	case ipban_type_range:
	    if ((ipban_str_to_ulong(ipaddr) >= ipban_str_to_ulong(entry->info1)) &&
		(ipban_str_to_ulong(ipaddr) <= ipban_str_to_ulong(entry->info2)))
	    {
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s matched range %s-%s",ipaddr,entry->info1,entry->info2);
		free(whole);
		return counter;
	    }
	    eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match range %s-%s",ipaddr,entry->info1,entry->info2);
	    continue;

	case ipban_type_netmask:
	    {
		unsigned long	lip1;
		unsigned long	lip2;
		unsigned long	netmask;

		if (!(lip1 = ipban_str_to_ulong(ipaddr)))
		    return -1;
		if (!(lip2 = ipban_str_to_ulong(entry->info1)))
		    return -1;
		if (!(netmask = ipban_str_to_ulong(entry->info2)))
		    return -1;

		lip1 = lip1 & netmask;
		lip2 = lip2 & netmask;
		if (lip1 == lip2)
		{
		    eventlog(eventlog_level_debug,"ipbanlist_check","address %s matched netmask %s/%s",ipaddr,entry->info1,entry->info2);
		    free(whole);
		    return counter;
		}
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match netmask %s/%s",ipaddr,entry->info1,entry->info2);
		continue;
	    }

	case ipban_type_prefix:
	    {
		unsigned long	lip1;
		unsigned long	lip2;
		int		prefix;

		if (!(lip1 = ipban_str_to_ulong(ipaddr)))
		    return -1;
		if (!(lip2 = ipban_str_to_ulong(entry->info1)))
		    return -1;
		prefix = atoi(entry->info2);

		lip1 = lip1 >> (32 - prefix);
		lip2 = lip2 >> (32 - prefix);
		if (lip1 == lip2)
		{
		    eventlog(eventlog_level_debug,"ipbanlist_check","address %s matched prefix %s/%s",ipaddr,entry->info1,entry->info2);
		    free(whole);
		    return counter;
		}
		eventlog(eventlog_level_debug,"ipbanlist_check","address %s does not match prefix %s/%s",ipaddr,entry->info1,entry->info2);
		continue;
	    }
	default:  /* unknown type */
	    eventlog(eventlog_level_warn,"ipbanlist_check","found bad ban type %d",(int)entry->type);
	}
    }

    free(whole);

    return 0;
}


extern int ipbanlist_add(t_connection * c, char const * cp, time_t endtime, char const * adminname, char const * reason)
{
    t_ipban_entry *	entry;
    char		tstr[MAX_MESSAGE_LEN];
    char const *	ctname;
    const char *	tname;

    if (!(entry = ipban_str_to_ipban_entry(cp)))
    {
	if (c)
	    message_send_text(c,message_type_error,c,"Bad IP.");
        eventlog(eventlog_level_error,"ipbanlist_add","could not convert to t_ipban_entry: \"%s\"",cp);
        return -1;
    }

    if (c)
	if (!(ctname = conn_get_username(c)))
	{
	    eventlog(eventlog_level_error,"ipbanlist_add","could not get username from connection");
	    if (adminname)
	    {
		if (!(tname = strdup(adminname)))
		{
		    eventlog(eventlog_level_error,"ipbanlist_add","could not allocate memory for tname");
		    if (c)
			message_send_text(c,message_type_error,c,"Not enought memory!");
		    return -1;
		}
	    }
	    else
		if (!(tname = strdup("unknown")))
		{
		    eventlog(eventlog_level_error,"ipbanlist_add","could not allocate memory for tname");
		    if (c)
			message_send_text(c,message_type_error,c,"Not enought memory!");
		    return -1;
		}
	}
	else
	{
	    if (!(tname = strdup(ctname)))
	    {
	        eventlog(eventlog_level_error,"ipbanlist_add","could not allocate memory for tname");
	        if (c)
		    message_send_text(c,message_type_error,c,"Not enought memory!");
		return -1;
	    }
	    conn_unget_username(c,ctname);
	}
    else
	if (adminname)
	{
	    if (!(tname = strdup(adminname)))
	    {
	        eventlog(eventlog_level_error,"ipbanlist_add","could not allocate memory for tname");
	        if (c)
		    message_send_text(c,message_type_error,c,"Not enought memory!");
		return -1;
	    }
	}
	else
	    tname = "unknown";
    entry->admin = (char *)tname;
    entry->endtime = endtime;
    if (!(entry->reason = strdup(reason)))
    {
	eventlog(eventlog_level_error,"ipbanlist_add","could not allocate momory for entry->reason");
	if (c)
	    message_send_text(c,message_type_error,c,"Not enought memory!");
	free((void *)tname);
	return -1;
    }

    if (c)
    {
	time_t		now;

	time(&now);
	if (endtime == 0)
	{
            sprintf(tstr,"%s banned permamently by %.64s: %.128s",cp,tname,reason);
            eventlog(eventlog_level_info,"ipbanlist_add",tstr);
            message_send_admins(c,message_type_info,tstr);
	    sprintf(tstr,"%s banned permamently.",cp);
	    message_send_text(c,message_type_info,c,tstr);
	}
	else
	{
            snprintf(tstr, sizeof(tstr), "%s banned for %.48s by %.64s: %.124s",
		     cp, seconds_to_timestr(entry->endtime - now), tname,
		     reason);
            eventlog(eventlog_level_info,"ipbanlist_add",tstr);
            message_send_admins(c,message_type_info,tstr);
	    sprintf(tstr,"%s banned for %.48s.",cp,seconds_to_timestr(entry->endtime - now));
	    message_send_text(c,message_type_info,c,tstr);
	}
    }

    if (list_append_data(ipbanlist_head,entry)<0)
    {
	ipban_unload_entry(entry);
	if (c)
	    message_send_text(c,message_type_error,c,"Could not append data to list.");
	eventlog(eventlog_level_error,"ipbanlist_add","could not append entry");
	return -1;
    }

    return 0;
}


extern int ipbanlist_unload_expired(void)
{
    t_elem *		curr;
    t_ipban_entry * 	entry;
    time_t		now;

    time(&now);
    LIST_TRAVERSE(ipbanlist_head,curr)
    {
	entry = elem_get_data(curr);
	if (!entry)
	{
	    eventlog(eventlog_level_error,"ipbanlist_unload_expired","ipbanlist_contains NULL element");
	    return -1;
	}
	if ((entry->endtime - now <= 0) && (entry->endtime != 0))
	{
	    eventlog(eventlog_level_debug,"ipbanlist_unload_expired","removing item: %s",entry->info1);
	    if (list_remove_elem(ipbanlist_head,curr)<0)
		eventlog(eventlog_level_error,"ipbanlist_unload_expired","could not remove item");
	    else
		ipban_unload_entry(entry);
	}
    }
    list_purge(ipbanlist_head);

    return 0;
}

extern time_t ipbanlist_str_to_time_t(t_connection * c, char const * timestr)
{

    unsigned int	bmin;
    char		minstr[MAX_TIME_STR];
    int			i;
    char		tstr[MAX_MESSAGE_LEN];
    time_t		now;

    for (i=0; isdigit(timestr[i]) && i<sizeof(minstr)-1; i++)
	minstr[i] = timestr[i];
    minstr[i] = '\0';

    if (timestr[i]!='\0')
	if (c)
	{
	    if (strlen(minstr)<1)
		message_send_text(c,message_type_info,c,"There was an error in time.");
	    else
	    {
		sprintf(tstr,"There was an error in time. Banning only for: %s minutes.",minstr);
	        message_send_text(c,message_type_info,c,tstr);
	    }
	}

    if (clockstr_to_seconds(minstr,&bmin)<0) /* it thinks these are seconds but we treat them as minutes */
    {
        eventlog(eventlog_level_error,"ipbanlist_str_to_time_t","could not convert to minutes: \"%s\"",timestr);
        return -1;
    }
    if (bmin == 0)
    	return 0;
    else
    {
	time(&now);
	return now + bmin*60;
    }
}


extern int handle_ipban_command(t_connection * c, char const * text)
{
    char		subcommand[MAX_FUNC_LEN];
    char		ipstr[MAX_IP_STR];
    char		timestr[MAX_TIME_STR];
    char 		reason[MAX_REASON_LEN];
    unsigned int 	i,j;

    for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip command */
    for (; text[i]==' '; i++);

    for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get subcommand */
	if (j<sizeof(subcommand)-1) subcommand[j++] = text[i];
    subcommand[j] = '\0';
    for (; text[i]==' '; i++);

    for (j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get ip address */
	if (j<sizeof(ipstr)-1) ipstr[j++] = text[i];
    ipstr[j] = '\0';
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

    switch (identify_ipban_function(subcommand))
    {
	case IPBAN_FUNC_ADD:
	    ipbanlist_add(c,ipstr,ipbanlist_str_to_time_t(c,timestr),NULL,reason);
	    break;
	case IPBAN_FUNC_DEL:
	    ipban_func_del(c,ipstr);
	    break;
	case IPBAN_FUNC_LIST:
	    ipban_func_list(c);
	    break;
	case IPBAN_FUNC_CHECK:
	    ipban_func_check(c,ipstr);
	    break;
	case IPBAN_FUNC_HELP:
	    message_send_text(c,message_type_info,c,"The ipban command supports the following patterns.");
	    ipban_usage(c);
	    break;
	default:
	    message_send_text(c,message_type_info,c,"The command is incorect. Use one of the following patterns.");
	    ipban_usage(c);
    }

    return 0;
}


static int identify_ipban_function(const char * funcstr)
{
    if (strcasecmp(funcstr,"add")==0 || strcasecmp(funcstr,"a")==0)
	return IPBAN_FUNC_ADD;
    if (strcasecmp(funcstr,"del")==0 || strcasecmp(funcstr,"d")==0)
	return IPBAN_FUNC_DEL;
    if (strcasecmp(funcstr,"list")==0 || strcasecmp(funcstr,"l")==0 || strcmp(funcstr,"")==0)
	return IPBAN_FUNC_LIST;
    if (strcasecmp(funcstr,"check")==0 || strcasecmp(funcstr,"c")==0)
	return IPBAN_FUNC_CHECK;
    if (strcasecmp(funcstr,"help")==0 || strcasecmp(funcstr,"h")==0)
	return IPBAN_FUNC_HELP;

    return IPBAN_FUNC_UNKNOWN;
}


static int ipban_func_del(t_connection * c, char const * cp)
{
    t_ipban_entry *	to_delete;
    int			to_delete_nmbr;
    t_ipban_entry *	entry;
    t_elem *		curr;
    unsigned int	counter;
    char		tstr[MAX_MESSAGE_LEN];

    counter = 0;
    if (strchr(cp,'.') || strchr(cp,'/') || strchr(cp,'*'))
    {
	if (!(to_delete = ipban_str_to_ipban_entry(cp)))
	{
	    message_send_text(c,message_type_error,c,"Illegal IP entry.");
	    return -1;
	}
	LIST_TRAVERSE(ipbanlist_head,curr)
	{
	    entry = elem_get_data(curr);
	    if (!entry)
	    {
		eventlog(eventlog_level_error,"ipban_func_del","ipbanlist contains NULL item");
		return -1;
	    }
	    if (ipban_identical_entry(to_delete,entry))
	    {
		counter++;
		if (list_remove_elem(ipbanlist_head,curr)<0)
		    eventlog(eventlog_level_error,"ipbanlist_unload_expired","could not remove item");
		else
		    ipban_unload_entry(entry);
	    }
	}

	ipban_unload_entry(to_delete);
	list_purge(ipbanlist_head);
	if (counter == 0)
	{
	    message_send_text(c,message_type_error,c,"No matching entry.");
	    return -1;
	}
	else
	{
	    if (counter == 1)
		sprintf(tstr,"Entry deleted.");
	    else
		sprintf(tstr,"Deleted %u entries.",counter);
	    message_send_text(c,message_type_info,c,tstr);
	    return 0;
	}
    }

    to_delete_nmbr = atoi(cp);
    if (to_delete_nmbr <= 0)
    {
	message_send_text(c,message_type_error,c,"Wrong entry number.");
	return -1;
    }
    LIST_TRAVERSE(ipbanlist_head,curr)
    {
	if (to_delete_nmbr == ++counter)
	{
	    entry = elem_get_data(curr);
	    if (!entry)
	    {
		eventlog(eventlog_level_error,"ipban_func_del","ipbanlist contains NULL item");
		return -1;
	    }
	    if (list_remove_elem(ipbanlist_head,curr)<0)
	        eventlog(eventlog_level_error,"ipbanlist_unload_expired","could not remove item");
	    else
	    {
	        ipban_unload_entry(entry);
		message_send_text(c,message_type_info,c,"Entry deleted.");
	    }
	}
    }

    list_purge(ipbanlist_head);
    if (to_delete_nmbr > counter)
    {
	sprintf(tstr,"There are only %u entries.",counter);
	message_send_text(c,message_type_error,c,tstr);
	return -1;
    }

    return 0;
}


static int ipban_func_list(t_connection * c)
{
    t_elem const *	curr;
    t_ipban_entry * 	entry;
    char		tstr[MAX_MESSAGE_LEN];
    unsigned int	counter;
    char	 	timestr[50];
    time_t		now;
    char *		ipstr;

    time(&now);
    counter = 0;
    message_send_text(c,message_type_info,c,"Banned IPs:");
    LIST_TRAVERSE_CONST(ipbanlist_head,curr)
    {
	entry = elem_get_data(curr);
	if (!entry)
	{
	    eventlog(eventlog_level_error,"ipban_func_list","ipbanlist contains NULL item");
	    return -1;
	}
	counter++;
	if (entry->endtime == 0)
	    sprintf(timestr,"perm");
	else
	    sprintf(timestr,"%.48s",seconds_to_timestr(entry->endtime - now));

	if (!(ipstr = ipban_entry_to_str(entry)))
	{
	    eventlog(eventlog_level_error,"ipban_func_list","could not convert entry to string");
	    continue;
	}
	snprintf(tstr, sizeof(tstr), "%u: %s (%s) by: %.64s reason: %.120s",
		 counter, ipstr, timestr, entry->admin, entry->reason);
	message_send_text(c,message_type_info,c,tstr);
	free(ipstr);
    }

    if (counter == 0)
	message_send_text(c,message_type_info,c,"none");
    return 0;
}


static int ipban_func_check(t_connection * c, char const * cp)
{
    int		res;
    char	entry[MAX_MESSAGE_LEN];

    res = ipbanlist_check(cp);
    switch (res)
    {
	case 0:
	    message_send_text(c,message_type_info,c,"IP not banned.");
	    break;
	case -1:
	    message_send_text(c,message_type_error,c,"Error occured.");
	    break;
	default:
	    sprintf(entry,"IP banned by rule #%i.",res);
	    message_send_text(c,message_type_info,c,entry);
    }

    return 0;
}


static int ipban_identical_entry(t_ipban_entry * e1, t_ipban_entry * e2)
{
    switch (e2->type)
    {
	case ipban_type_exact:
	    if (strcmp(e1->info1,e2->info1)==0)
		return 1;
	    break;
	case ipban_type_range:
	    if (strcmp(e1->info1,e2->info1)==0 &&
		strcmp(e1->info2,e2->info2)==0)
		return 1;
	    break;
	case ipban_type_wildcard:
	    if (strcmp(e1->info1,e2->info1)==0 &&
		strcmp(e1->info2,e2->info2)==0 &&
		strcmp(e1->info3,e2->info3)==0 &&
		strcmp(e1->info4,e2->info4)==0)
		return 1;
	    break;
	case ipban_type_netmask:
	    if (strcmp(e1->info1,e2->info1)==0 &&
		strcmp(e1->info2,e2->info2)==0)
		return 1;
	    break;
	case ipban_type_prefix:
	    if (strcmp(e1->info1,e2->info1)==0 &&
		strcmp(e1->info2,e2->info2)==0)
		return 1;
	    break;
        default:  /* unknown type */
	    eventlog(eventlog_level_warn,"ipbanlist_identical_entry","found bad ban type %d",(int)e2->type);
    }

    return 0;
}


static int ipban_unload_entry(t_ipban_entry * e)
{
    switch (e->type)
    {
    	case ipban_type_exact:
	case ipban_type_wildcard:
	    if (e->info1)
		free(e->info1);
	    break;
	case ipban_type_range:
	case ipban_type_netmask:
	case ipban_type_prefix:
	    if (e->info1)
		free(e->info1);
	    if (e->info2)
		free(e->info2);
	    break;
	default:  /* unknown type */
    	    eventlog(eventlog_level_warn,"ipbanlist_unload","found bad ban type %d",(int)e->type);
	    return -1;
    }
    free(e->admin);
    free(e->reason);
    free(e);
    return 0;
}


static t_ipban_entry * ipban_str_to_ipban_entry(char const * ipstr)
{
    char *          matched;
    char *          whole;
    char *          cp;
    t_ipban_entry * entry;

    if (!(entry = malloc(sizeof(t_ipban_entry))))
    {
        eventlog(eventlog_level_error,"ipban_str_to_t_ipban_entry","could not allocate memory for entry %s",ipstr);
        return NULL;
    }
    if (!ipstr)
    {
	eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","got NULL IP");
	free(entry);
	return NULL;
    }
    if (ipstr[0] == '\0')
    {
        eventlog(eventlog_level_warn,"ipban_str_to_ipban_entry","got empty IP string");
	free(entry);
        return NULL;
    }
    if (!(cp = strdup(ipstr)))
    {
	eventlog(eventlog_level_warn,"ipban_str_to_ipban_entry","could not allocate memory for cp");
	free(entry);
	return NULL;
    }

    if (ipban_could_be_ip_str(cp)==0)
    {
	eventlog(eventlog_level_debug,"ipban_str_to_ipban_entry","string: \"%.32s\" can not be valid IP",cp);
	free(cp);
	free(entry);
	return NULL;
    }
    if ((matched = strchr(cp,'-'))) /* range */
    {
        entry->type = ipban_type_range;
	eventlog(eventlog_level_debug,"ipban_str_to_ipban_entry","entry: %s matched as ipban_type_range",cp);
        matched[0] = '\0';
        if (!(entry->info1 = strdup(cp))) /* start of range */
        {
	    eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","could not allocate memory for info on entry %s",cp);
	    free(entry);
	    free(cp);
	    return NULL;
	}
	if (!(entry->info2 = strdup(&matched[1]))) /* end of range */
	{
	    eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","could not allocate memory for info on entry %s",cp);
	    free(entry->info1);
	    free(entry);
	    free(cp);
	    return NULL;
	}
	entry->info3 = NULL; /* clear unused elements so debugging is nicer */
	entry->info4 = NULL;
    }
    else
        if (strchr(cp,'*')) /* wildcard */
        {
	    entry->type = ipban_type_wildcard;
	    eventlog(eventlog_level_debug,"ipban_str_to_ipban_entry","entry: %s matched as ipban_type_wildcard",cp);

	    /* only free() info1! */
	    if (!(whole = strdup(cp)))
	    {
	        eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","could not allocate memory for info on entry \"%s\"",cp);
	        free(entry);
		free(cp);
	        return NULL;
	    }
	    entry->info1 = strtok(whole,".");
	    entry->info2 = strtok(NULL,".");
	    entry->info3 = strtok(NULL,".");
	    entry->info4 = strtok(NULL,".");
	    if (!entry->info4) /* not enough dots */
	    {
	        eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","wildcard entry \"%s\" does not contain all four octets",cp);
	        free(entry->info1);
	        free(entry);
		free(cp);
	        return NULL;
	    }
	}
	else
	    if ((matched = strchr(cp,'/'))) /* netmask or prefix */
	    {
		if (strchr(&matched[1],'.'))
		{
		    entry->type = ipban_type_netmask;
		    eventlog(eventlog_level_debug,"ipban_str_to_ipban_entry","entry: %s matched as ipban_type_netmask",cp);
		}
		else
		{
		    entry->type = ipban_type_prefix;
		    eventlog(eventlog_level_debug,"ipban_str_to_ipban_entry","entry: %s matched as ipban_type_prefix",cp);
		}

		matched[0] = '\0';
		if (!(entry->info1 = strdup(cp)))
		{
		    eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","could not allocate memory for info on entry \"%s\"",cp);
	    	    free(entry);
		    free(cp);
		    return NULL;
		}
		if (!(entry->info2 = strdup(&matched[1])))
		{
		    eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","could not allocate memory for info on entry \"%s\"",cp);
		    free(entry->info1);
		    free(entry);
		    free(cp);
		    return NULL;
		}
		entry->info3 = NULL; /* clear unused elements so debugging is nicer */
		entry->info4 = NULL;
	    }
	    else /* exact */
	    {
		entry->type = ipban_type_exact;
		eventlog(eventlog_level_debug,"ipban_str_to_ipban_entry","entry: %s matched as ipban_type_exact",cp);

		if (!(entry->info1 = strdup(cp)))
		{
		    eventlog(eventlog_level_error,"ipban_str_to_ipban_entry","could not allocate memory for info on entry \"%s\"",cp);
		    free(entry);
		    free(cp);
		    return NULL;
		}
		entry->info2 = NULL; /* clear unused elements so debugging is nicer */
		entry->info3 = NULL;
		entry->info4 = NULL;
	    }
    free(cp);

    return entry;
}


static char * ipban_entry_to_str(t_ipban_entry const * entry)
{
    char 	tstr[MAX_MESSAGE_LEN];
    char * 	str;

    switch (entry->type)
    {
        case ipban_type_exact:
    	    sprintf(tstr,"%s",entry->info1);
	    break;
	case ipban_type_wildcard:
	    sprintf(tstr,"%s.%s.%s.%s",entry->info1,entry->info2,entry->info3,entry->info4);
	    break;
	 case ipban_type_range:
	    sprintf(tstr,"%s-%s",entry->info1,entry->info2);
	    break;
	 case ipban_type_netmask:
	 case ipban_type_prefix:
	    sprintf(tstr,"%s/%s",entry->info1,entry->info2);
	    break;

	default: /* unknown type */
	    eventlog(eventlog_level_warn,"ipban_entry_to_str","found bad ban type %d",(int)entry->type);
	    return NULL;
    }
    if (!(str = strdup(tstr)))
    {
	eventlog(eventlog_level_error,"ipban_entry_to_str","could not allocate memmory for str");
	return NULL;
    }

    return str;
}

static unsigned long ipban_str_to_ulong(char const * ipaddr)
{
    unsigned long	lip;
    char *    		ip1;
    char *    		ip2;
    char *    		ip3;
    char *    		ip4;
    char *		tipaddr;

    if (!(tipaddr = strdup(ipaddr)))
    {
        eventlog(eventlog_level_warn,"ipban_ipstr_to_ulong","could not allocate memory for tipaddr");
	return 0;
    }
    ip1 = strtok(tipaddr,".");
    ip2 = strtok(NULL,".");
    ip3 = strtok(NULL,".");
    ip4 = strtok(NULL,".");
    lip = (atoi(ip1) << 24) + (atoi(ip2) << 16) + (atoi(ip3) << 8) + (atoi(ip4));

    free(tipaddr);

    return lip;
}


static int ipban_could_be_exact_ip_str(char const * str)
{
    char * 	ipstr;
    char *	s;
    char * 	ttok;
    int 	i;

    if (!(ipstr = strdup(str)))
    {
	eventlog(eventlog_level_error,"ipban_could_be_exact_ip_str","could not allocate memmory for ipstr");
	return 0;
    }

    s = ipstr;
    for (i=0; i<4; i++)
    {
	ttok = strsep(&ipstr,".");
	if (!ttok || strlen(ttok)<1 || strlen(ttok)>3)
	{
	    free(s);
	    return 0;
	}
    }

    free(s);
    return 1;
}


static int ipban_could_be_ip_str(char const * str)
{
    char *	matched;
    char *	ipstr;
    int 	i;

    if (strlen(str)<7)
    {
	eventlog(eventlog_level_error,"ipban_could_be_ip_str","string too short");
	return 0;
    }
    for (i=0; i<strlen(str); i++)
	if (!isdigit(str[i]) && str[i]!='.' && str[i]!='*' && str[i]!='/' && str[i]!='-')
	{
	    eventlog(eventlog_level_debug,"ipban_could_be_ip_str","illegal character on position %i",i);
	    return 0;
	}

    if (!(ipstr = strdup(str)))
    {
	eventlog(eventlog_level_error,"ipban_could_be_ip_str","could not allocate memory for ipstr");
	return 0;
    }
    if ((matched = strchr(ipstr,'-')))
    {
	matched[0] = '\0';
	if ((ipban_could_be_exact_ip_str(ipstr)==0) ||
	    (ipban_could_be_exact_ip_str(&matched[1])==0))
	{
	    free(ipstr);
	    return 0;
	}
    }
    else if ((matched = strchr(ipstr,'*')))
    {
	if (ipban_could_be_exact_ip_str(ipstr)==0) /* FIXME: 123.123.1*.123 allowed */
	{
	    free(ipstr);
	    return 0;
	}
    }
    else if ((matched = strchr(ipstr,'/')))
    {
	matched[0] = '\0';
	if (strchr(&matched[1],'.'))
	{
	    if ((ipban_could_be_exact_ip_str(ipstr)==0) ||
		(ipban_could_be_exact_ip_str(&matched[1])==0))
	    {
		free(ipstr);
		return 0;
	    }
	}
        else
	{
    	    if (ipban_could_be_exact_ip_str(ipstr)==0)
	    {
		free(ipstr);
		return 0;
	    }
	    for (i=1; i<strlen(&matched[1]); i++)
		if (!isdigit(matched[i]))
		{
		    free(ipstr);
		    return 0;
		}
	    if (atoi(&matched[1])>32) /* can not be less than 0 because IP/-24 is matched as range */
	    {
		free(ipstr);
		return 0;
	    }
	}
    }
    else
    {
	if (ipban_could_be_exact_ip_str(ipstr)==0)
	{
	    free(ipstr);
	    return 0;
	}
    }
    free(ipstr);

    return 1;
}


static void ipban_usage(t_connection * c)
{
    message_send_text(c,message_type_info,c,"to print this information:");
    message_send_text(c,message_type_info,c,"    /ipban h[elp]");
    message_send_text(c,message_type_info,c,"to print all baned IPs");
    message_send_text(c,message_type_info,c,"    /ipban [l[ist]]");
    message_send_text(c,message_type_info,c,"to erase ban:");
    message_send_text(c,message_type_info,c,"    /ipban d[el] <IP|index num>");
    message_send_text(c,message_type_info,c,"    (IP have to be entry accepted in bnban)");
    message_send_text(c,message_type_info,c,"to add ban:");
    message_send_text(c,message_type_info,c,"    /ipban a[dd] IP [time] [reason]");
    message_send_text(c,message_type_info,c,"    (IP have to be entry accepted in bnban)");
    message_send_text(c,message_type_info,c,"to check is specified IP banned:");
    message_send_text(c,message_type_info,c,"    /ipban c[heck] IP");
}
