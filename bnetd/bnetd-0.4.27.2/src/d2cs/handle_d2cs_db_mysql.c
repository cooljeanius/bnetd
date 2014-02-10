/* handle_d2cs_db_mysql.c
 * Copyright (C) 2002, 2003  JEBs@shbe.net
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

/*
 * HISTORY
 * =======
 * JE20030418 - Changed bone headed WHERE clause
 * JE20030413 - Removed wrong "escape" solution and changed selection meachnism
 *              from "LIKE" to "="
 * JE20021024 - Initial version 0.1
 */

#ifdef WITH_MYSQL                       /* Whole file */
#include "common/setup_before.h"
#include "setup.h"
#ifdef HAVE_STDDEF_H
# include <stddef.h>
#else
# ifndef NULL
#  define NULL ((void *)0)
# endif /* !NULL */
#endif /* HAVE_STDDEF_H */
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#include "prefs.h"
#include "common/eventlog.h"
#include "common/db_mysql.h"
#include "common/setup_after.h"

static int db_server_connect_done = 0; /* Info logging & table update checking */


static unsigned int db_connect(MYSQL *mysql)
{
	if (mysql->server_status == 2) return 1;

	if (!db_init(mysql)) return 0;

	eventlog(eventlog_level_trace, __FUNCTION__, "connecting to MySQL");
	if (!mysql_real_connect(mysql,prefs_get_db_host(),prefs_get_db_user(),prefs_get_db_pw(),prefs_get_db_name(),prefs_get_db_port(), 0, CLIENT_COMPRESS))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not connect to MySQL \"%d:%s\"",mysql_errno(mysql),mysql_error(mysql));
		return 0;
	}

	if(db_server_connect_done)
		return 1;

	db_server_connect_done = 1;
	eventlog(eventlog_level_info, __FUNCTION__, "connected to MySQL (server version %s)", mysql_get_server_info(mysql));
	return 1;
}

extern void db_d2char_acccharlist (char const * db_table,char const * account,char const * realmname,t_db_result * myresult)
{
	/* This shall allow us to reconnect, after a disconnected MySQL link */
	if (!db_connect(&myresult->mysql)) return;

	if (!(myresult->sql = malloc(strlen("SELECT charname FROM  WHERE accname=\'\' AND realm=\'\' AND recage=0")+strlen(db_table)+strlen(account)+strlen(realmname)+1)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for query");
		free(myresult->sql);
		return;
	}
	sprintf(myresult->sql, "SELECT charname FROM %s WHERE accname=\'%s\' AND realm=\'%s\' AND recage=0",db_table,account,realmname);
	if(mysql_query(&myresult->mysql,myresult->sql))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "error selecting characters for \"*%s@%s\" (%d:%s)",account,realmname,mysql_errno(&myresult->mysql),mysql_error(&myresult->mysql));
		free(myresult->sql);
		return;
	}
	free(myresult->sql);

	if(!(myresult->res = mysql_store_result(&myresult->mysql)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "result error in selecting characters for \"*%s@%s\" (%d:%s)",account,realmname,mysql_errno(&myresult->mysql),mysql_error(&myresult->mysql));
		return;
	}
	eventlog(eventlog_level_trace, __FUNCTION__, "got a query result of %lu characters for \"*%s@%s\"", (unsigned long) mysql_num_rows(myresult->res),account,realmname);
}


extern char const * db_d2char_accchargetnext(t_db_result * myresult)
{
	if(!(myresult->row = mysql_fetch_row(myresult->res)))
	{
		return '\0';
	}
	return myresult->row[0];
}

#else
typedef int handle_d2cs_db_mysql_c_filenotempty; /* make ISO standard happy */
#endif  /* WITH_MYSQL */

/* EOF */
