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
#include "d2cs_d2gs_character.h"

extern unsigned int db_connect(MYSQL *mysql);
extern unsigned int db_d2char_loadinfo(char const * db_table,char const * account,char const * charname,char const * realmname,t_d2charinfo_file * charinfo,unsigned int charinfo_size);
extern unsigned int db_d2char_saveinfo(char const * db_table,char const * account,char const * charname,char const * realmname,t_d2charinfo_file * charinfo,unsigned int charinfo_size);
extern unsigned int db_d2char_loaddata(char const * db_table,char const * account,char const * charname,char const * realmname,unsigned char * chardata, unsigned int chardata_size);
extern unsigned int db_d2char_savedata(char const * db_table,char const * account,char const * charname,char const * realmname,unsigned char * chardata, unsigned int chardata_size);
extern unsigned int	db_d2char_delete(char const * db_table,char const * account,char const * charname,char const * realmname);
extern unsigned int db_d2char_create(char const * db_table,char const * account,char const * charname,t_d2charinfo_file * charinfo,char * chardata, unsigned int chardata_size);

