/*
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
#ifdef WITH_MYSQL
#include "common/setup_before.h"
#include "common/eventlog.h"
#include "prefs.h"
#include <mysql.h>
#include "common/setup_after.h"

/* Used for easier initialization, more clear logfiles, and conversion routines (FlatFile2DB) */
static int db_client_init_done = 0,
           db_server_connect_done = 0;


static unsigned int db_init(MYSQL *mysql)
{
    if (!mysql_init(mysql))
    {
    eventlog(eventlog_level_error,"db_mysql_init","could not initialize MySQL (insufficient memory)");
    return 0;
    }

    if(!db_client_init_done)
    {
        db_client_init_done = 1;
        eventlog(eventlog_level_info,"db_mysql_init","initialized MySQL (client version %s)", mysql_get_client_info());
    }

    return 1;
}

static void db_close(MYSQL *mysql)
{
    eventlog(eventlog_level_debug,"db_mysql_close","closing MySQL");
    mysql_close(mysql);
}

static unsigned int db_table_update(void)
{
    if(!db_account_table_update()) return 0;

    return 1;
}

extern unsigned int db_connect(MYSQL *mysql)
{
    /* Only needed if not already connected yet */
    if (mysql->server_status != 2)
    {
        /* If not connected already, init is needed too */
        if (!db_init(mysql)) return 0;

        eventlog(eventlog_level_debug,"db_mysql_connect","connecting to MySQL");

        if (!mysql_real_connect(mysql,prefs_get_db_host(),prefs_get_db_user(),prefs_get_db_pw(),prefs_get_db_name(),prefs_get_db_port(),0,CLIENT_COMPRESS))
        {
        eventlog(eventlog_level_error,"db_mysql_connect","could not connect to MySQL \"%d:%s\"",mysql_errno(mysql),mysql_error(mysql));
        return 0;
        }

        if(!db_server_connect_done)
        {
            db_server_connect_done = 1;
            eventlog(eventlog_level_info,"db_mysql_connect","connected to MySQL (server version %s)", mysql_get_server_info(mysql));
            /* Do some DB checks, updates, imports, ... */
            if(!db_table_update())
            {
                eventlog(eventlog_level_error,"db_mysql_connect","MySQL table update failed");
                return 0;
            }
        }
    }
    return 1;
}


#endif  /* WITH_MYSQL */