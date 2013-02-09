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

/*
 * HISTORY
 * =======
 * JE20030413 - Removed function "db_escape_wildcard_chars"
 * JE20030330 - Added function "db_escape_wildcard_chars"
 * JE20021025 - Finished redraw and optimization
 * JE20021019 - Initial version 0.1 adapted from earlier version
 *				used in bnetd/db_mysql
 */

#ifdef WITH_MYSQL
#include "common/setup_before.h"
#include "common/eventlog.h"
#include <mysql.h>
#include "common/db_mysql.h"
#include "common/setup_after.h"

static int db_client_init_done = 0;


extern unsigned int db_init(MYSQL *mysql)
{
	if (!mysql_init(mysql))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not initialize MySQL (insufficient memory)");
		return 0;
	}

	if(!db_client_init_done)
	{
		db_client_init_done = 1;
		eventlog(eventlog_level_info, __FUNCTION__, "initialized MySQL (client version %s)", mysql_get_client_info());
	}
	return 1;
}


extern void db_free_result(MYSQL_RES * res)
{
	mysql_free_result(res);
}


extern void db_close(MYSQL *mysql)
{
	eventlog(eventlog_level_trace, __FUNCTION__,"closing MySQL");
	mysql_close(mysql);
}

#endif  /* WITH_MYSQL */

