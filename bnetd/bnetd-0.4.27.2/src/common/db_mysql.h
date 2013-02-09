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

#include <mysql.h>

typedef struct
{
    MYSQL     	mysql;	/* MySQL connection var isn't clearly inited */
    MYSQL_RES 	*res;
    MYSQL_ROW	row;
    char 		*sql;
} t_db_result;


extern unsigned int db_init(MYSQL *mysql);
extern void 		db_free_result(MYSQL_RES * res);
extern void 		db_close(MYSQL *mysql);
