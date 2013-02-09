/*
 * Copyright (C) 2002,2003 JEBs@shbe.net
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
 * This file must remain in d2cs cause the tricky usage
 * of common sources by d2cs & d2dbs and their header files
 * like "d2cs_d2gs_character.h" & "setup.h"
 *
 * HISTORY
 * =======
 * JE20030418 - Changed bone headed WHERE clause
 * JE20030413 - Removed wrong "escape" solution and changed selection meachnism
 *              from "LIKE" to "="
 * JE20030330 - Huge security whole with unescaped wildcard chars ("_" & "%") fixed
 * JE20030214 - Fixed 2 tiny memory leaks
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
# endif
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#include <errno.h>
#include "compat/pdir.h"
#include "compat/memcpy.h"
#include "common/eventlog.h"
#include "d2charfile.h"
#include "prefs.h"
#include "d2cs_d2gs_character.h"
#include <mysql.h>
#include "common/db_mysql.h"
#include "d2char_db_mysql.h"
#include "common/setup_after.h"

/* Build these vars static to save lot "init" and "connect" calls.
   This prevents the online change to a differnt DB but who does this */
static t_db_result	loadinfo = {0},		/* MySQL headers aren't correct initialized */
					loaddata = {0};

static MYSQL		saveinfo = {0},
					savedata = {0};

static int 			db_server_connect_done = 0;		/* Info logging & table update checking */


static unsigned int db_table_check(char const * db_table)
{
	/* Returns:	0 on error
				1 if everything is ok
				2 if no table exist
				n if table update from version x needed (future)
	 */
	t_db_result		mycon = {0};
    MYSQL_FIELD 	*field;
    unsigned int    i,
                    cfn,
                    tfn;

    eventlog(eventlog_level_trace,__FUNCTION__,"checking character table");
    if (!db_connect(&mycon.mysql)) return 0;

    /* Check if we have the right version */
    if (!(mycon.sql = malloc(strlen("SHOW TABLE STATUS LIKE ''")+strlen(db_table)+1))) {
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to allocate memory for query");
    	return 0;
    }
    sprintf(mycon.sql,"SHOW TABLE STATUS LIKE '%s'",db_table);
    if (mysql_query(&mycon.mysql, mycon.sql))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not check character table (%d:%s)",mysql_errno(&mycon.mysql),mysql_error(&mycon.mysql));
        mysql_free_result(mycon.res);
        return 0;
    }
    if (!(mycon.res = mysql_use_result(&mycon.mysql)))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not get query result (%d:%s)",mysql_errno(&mycon.mysql),mysql_error(&mycon.mysql));
        mysql_free_result(mycon.res);
        return 0;
    }

    /* Analyse field num of type and comment field */
    i=0;
    while((field = mysql_fetch_field(mycon.res)))
    {
        if(!strncasecmp(field->name,"Type",strlen(field->name))) tfn=i;
        if(!strncasecmp(field->name,"Comment",strlen(field->name))) cfn=i;
        i++;
    }
    if(!tfn)
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not find 'Type' field which is needed for validation");
        mysql_free_result(mycon.res);
        return 0;
    }
    if(!cfn)
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not find 'Comment' field which is needed for version checking");
        mysql_free_result(mycon.res);
        return 0;
    }

    /* Get row and check content of "Type" and "Comment" field */
    if ((mycon.row = mysql_fetch_row(mycon.res)))
    {
        if(strncasecmp(mycon.row[tfn],"MyISAM",6))
        {
            eventlog(eventlog_level_error,__FUNCTION__,"wrong table type (\"%s\" != \"MyISAM\")",mycon.row[tfn]);
            mysql_free_result(mycon.res);
            return 0;
        }
        if(strncasecmp(mycon.row[cfn],"d2character v0.1",16))
        {
            eventlog(eventlog_level_error,__FUNCTION__,"wrong table version (\"%s\" != \"d2character v0.1\")",mycon.row[cfn]);
            mysql_free_result(mycon.res);
            return 0;
        }
    } else {
    	/* Got now row, this means table don't exists */
	    mysql_free_result(mycon.res);
     	return 2;
    }
    mysql_free_result(mycon.res);
    return 1;
}


extern unsigned int db_connect(MYSQL *mysql)
{
	if (mysql->server_status == 2) return 1;

	if (!db_init(mysql)) return 0;

	eventlog(eventlog_level_trace, __FUNCTION__, "connecting to MySQL");
	if (!mysql_real_connect(mysql,prefs_get_db_host(),prefs_get_db_user(),prefs_get_db_pw(),prefs_get_db_name(),prefs_get_db_port(), 0, CLIENT_COMPRESS))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not connect to MySQL \"%d:%s\"",mysql_errno(mysql),mysql_error(mysql));
		return 0;
	}

	if(db_server_connect_done) return 1;

	db_server_connect_done = 1;
	eventlog(eventlog_level_info, __FUNCTION__, "connected to MySQL (server version %s)", mysql_get_server_info(mysql));

	/* Return with table_check status */
	return db_table_check(prefs_get_db_char_table());
}


extern unsigned int db_d2char_loadinfo(char const * db_table,char const * account,char const * charname,char const * realmname,t_d2charinfo_file * charinfo,unsigned int charinfo_size)
{
    unsigned long *	fieldlengths;
    unsigned int	size;

	/* This shall allow us to reconnect, after a disconnected MySQL link */
	if (!db_connect(&loadinfo.mysql)) return 0;

	if (!strlen(realmname)) {
		eventlog(eventlog_level_error, __FUNCTION__, "got no realm name");
		return 0;
	}

	if (!(loadinfo.sql = malloc(strlen("SELECT charinfo FROM  WHERE accname=\'\' AND charname=\'\' AND realm=\'\' AND recage=0")+strlen(db_table)+strlen(account)+strlen(charname)+strlen(realmname)+1)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for query");
		free(loadinfo.sql);
		return 0;
	}
	sprintf(loadinfo.sql, "SELECT charinfo FROM %s WHERE accname=\'%s\' AND charname=\'%s\' AND realm=\'%s\' AND recage=0",db_table,account,charname,realmname);
	if(mysql_query(&loadinfo.mysql,loadinfo.sql))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "error selecting character \"%s(*%s)@%s\" (%d:%s)",charname,account,realmname,mysql_errno(&loadinfo.mysql),mysql_error(&loadinfo.mysql));
		free(loadinfo.sql);
		return 0;
	}
	free(loadinfo.sql);

	if(!(loadinfo.res = mysql_use_result(&loadinfo.mysql)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "result error in character \"%s(*%s)@%s\" (%d:%s)",charname,account,realmname,mysql_errno(&loadinfo.mysql),mysql_error(&loadinfo.mysql));
		return 0;
	}

	if(!(loadinfo.row = mysql_fetch_row(loadinfo.res)))
	{
		/* No record in result set
			Don't output an error message cause this function is used to check
			if an character already exists when creating a new one */
		return 0;
	}

	/* We've to check if data fits into given bufsize cause d2dbs only gives an charpointer and not a real struct pointer */
	fieldlengths = mysql_fetch_lengths(loadinfo.res);
	if(!fieldlengths[0])
	{
		eventlog(eventlog_level_error, __FUNCTION__, "read zero charinfo size for character \"%s(*%s)@%s\"",charname,account,realmname);
		return 0;
	}
	if(fieldlengths[0] > charinfo_size)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "charinfo record is larger than given buffer size (%lu > %u) for character \"%s(*%s)@%s\"",(unsigned long) fieldlengths[0],charinfo_size,charname,account,realmname);
		return 0;
	}

	/* Copy whole result (struct) to struct pointed by given function parameter
		Only works on ANSI compiler, but i guess we all use such compiler */
	*charinfo = *(t_d2charinfo_file *) loadinfo.row[0];

	size=(unsigned int) fieldlengths[0];
	mysql_free_result(loadinfo.res);

	eventlog(eventlog_level_trace, __FUNCTION__, "successfully read info record \"%s(*%s)@%s\"",charinfo->header.charname,charinfo->header.account,charinfo->header.realmname);
	return size;
}


extern unsigned int db_d2char_saveinfo(char const * db_table,char const * account,char const * charname,char const * realmname,t_d2charinfo_file * charinfo,unsigned int charinfo_size)
{
	char 			* sql;
    unsigned int	esc_charinfo_size = charinfo_size*2+1;	/* Escapeing binary data needs in worst case 2*size + 1 real zero */
	char			esc_charinfo[esc_charinfo_size];

	/* This shall allow us to reconnect, after a disconnected MySQL link */
	if (!db_connect(&saveinfo)) return 0;

	if (!(sql = malloc(strlen("UPDATE  SET mtime=NOW()+0,charinfo=\'\' WHERE accname=\'\' AND charname=\'\' AND realm=\'\' AND recage=0")+strlen(db_table)+esc_charinfo_size+strlen(account)+strlen(charname)+strlen(realmname)+1)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for query");
		free(sql);
		return 0;
	}

	/* Escape charinfo */
	if(!mysql_real_escape_string(&saveinfo, esc_charinfo, (char const *) charinfo, charinfo_size))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "charinfo escape failed");
		free(sql);
		return 0;
	}

	/* Build real sql command */
	sprintf(sql, "UPDATE %s SET mtime=NOW()+0,charinfo=\'%s\' WHERE accname=\'%s\' AND charname=\'%s\' AND realm=\'%s\' AND recage=0",db_table,esc_charinfo,account,charname,realmname);
	if(mysql_query(&saveinfo,sql))
	{
		eventlog(eventlog_level_error, __FUNCTION__,"save of character info \"%s(*%s)@%s\" failed (%d:%s)",charname,account,realmname,mysql_errno(&saveinfo),mysql_error(&saveinfo));
		free(sql);
		return 0;
	}
	free(sql);
	if(mysql_affected_rows(&saveinfo)<=0)
	{
		eventlog(eventlog_level_error, __FUNCTION__,"zero character info records \"%s(*%s)@%s\" updated (%d:%s)",charname,account,realmname,mysql_errno(&saveinfo),mysql_error(&saveinfo));
		free(sql);
		return 0;
	}
	eventlog(eventlog_level_trace, __FUNCTION__, "successfully updated character info \"%s(*%s)@%s\"",charname,account,realmname);
	/* d2dbs wants returned the written charinfo size. Not really, but may be somewhen */
	return charinfo_size;
}


extern unsigned int db_d2char_loaddata(char const * db_table,char const * account,char const * charname,char const * realmname,unsigned char * chardata, unsigned int chardata_size)
{
    unsigned long	* fieldlengths;
    unsigned int	size;

	/* This shall allow us to reconnect, after a disconnected MySQL link */
	if (!db_connect(&loaddata.mysql)) return 0;

	if (!(loaddata.sql = malloc(strlen("SELECT chardata FROM  WHERE accname=\'\' AND charname=\'\' AND realm=\'\' AND recage=0")+strlen(db_table)+strlen(account)+strlen(charname)+strlen(realmname)+1)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for query");
		free(loaddata.sql);
		return 0;
	}
	sprintf(loaddata.sql, "SELECT chardata FROM %s WHERE accname=\'%s\' AND charname=\'%s\' AND realm=\'%s\' AND recage=0",db_table,account,charname,realmname);

	if(mysql_query(&loaddata.mysql,loaddata.sql))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "error selecting character \"%s(*%s)@%s\" (%d:%s)",charname,account,realmname,mysql_errno(&loaddata.mysql),mysql_error(&loaddata.mysql));
		free(loaddata.sql);
		return 0;
	}
	free(loaddata.sql);

	if(!(loaddata.res = mysql_use_result(&loaddata.mysql)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "result error in character \"%s(*%s)@%s\" (%d:%s)",charname,account,realmname,mysql_errno(&loaddata.mysql),mysql_error(&loaddata.mysql));
		return 0;
	}

	if(!(loaddata.row = mysql_fetch_row(loaddata.res)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "record not found for character \"%s(*%s)@%s\"",charname,account,realmname);
		return 0;
	}

	fieldlengths = mysql_fetch_lengths(loaddata.res);
	if(!fieldlengths[0])
	{
		eventlog(eventlog_level_error, __FUNCTION__, "read zero chardata size for character \"%s(*%s)@%s\"",charname,account,realmname);
		return 0;
	}
	if(fieldlengths[0] > chardata_size)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "chardata record is larger than buffer size (%lu > %u) for character \"%s(*%s)@%s\"",fieldlengths[0],chardata_size,charname,account,realmname);
		return 0;
	}

	memcpy(chardata, loaddata.row[0], (unsigned long) fieldlengths[0]);
	size=(unsigned int) fieldlengths[0];
	mysql_free_result(loaddata.res);
	eventlog(eventlog_level_trace, __FUNCTION__, "successfully read data record \"%s(*%s)@%s\"",charname,account,realmname);
	return size;
}


extern unsigned int db_d2char_savedata(char const * db_table,char const * account,char const * charname,char const * realmname,unsigned char * chardata, unsigned int chardata_size)
{
	char 			* sql;
	unsigned int	esc_chardata_size = chardata_size*2+1;		/* Escapeing binary data needs in worst case 2*size + 1 */
	char			esc_chardata[esc_chardata_size];

	/* This shall allow us to reconnect, after a disconnected MySQL link */
	if (!db_connect(&savedata)) return 0;

	/* Escape chardata */
	if(!mysql_real_escape_string(&savedata,esc_chardata,chardata,chardata_size))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "chardata escape failed");
		free(sql);
		return 0;
	}

	/* Build and do sql */
	if (!(sql = malloc(strlen("UPDATE  SET mtime=NOW()+0,chardata=\'\' WHERE accname=\'\' AND charname=\'\' AND realm=\'\' AND recage=0")+strlen(db_table)+esc_chardata_size+strlen(account)+strlen(charname)+strlen(realmname)+1)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for query");
		free(sql);
		return 0;
	}
	sprintf(sql, "UPDATE %s SET mtime=NOW()+0,chardata=\'%s\' WHERE accname=\'%s\' AND charname=\'%s\' AND realm=\'%s\' AND recage=0",db_table,esc_chardata,account,charname,realmname);
	if(mysql_query(&savedata,sql))
	{
		eventlog(eventlog_level_error, __FUNCTION__,"save of character \"%s(*%s)@%s\" failed (%d:%s)",charname,account,realmname,mysql_errno(&savedata),mysql_error(&savedata));
		free(sql);
		return 0;
	}
	free(sql);
	if(mysql_affected_rows(&savedata)<=0)
	{
		eventlog(eventlog_level_error, __FUNCTION__,"zero character data records \"%s(*%s)@%s\" updated (%d:%s)",charname,account,realmname,mysql_errno(&savedata),mysql_error(&savedata));
		free(sql);
		return 0;
	}
	eventlog(eventlog_level_trace, __FUNCTION__, "successfully updated character data \"%s(*%s)@%s\"",charname,account,realmname);
	return chardata_size;
}


extern unsigned int db_d2char_delete(char const * db_table,char const * account, char const * charname, char const * realmname)
{
	MYSQL	mysql = {0};    /* MySQL headers aren't correct initialized */
	char 	* sql;

	/* This shall allow us to reconnect, after a disconnected MySQL link */
	if (!db_connect(&mysql)) return 0;

	if (!strlen(realmname)) {
		eventlog(eventlog_level_error, __FUNCTION__, "got no realm name ");
		return 0;
	}

	if (!(sql = malloc(strlen("DELETE FROM  WHERE accname=\'\' AND charname=\'\' AND realm=\'\' AND recage=0")+strlen(db_table)+strlen(account)+strlen(charname)+strlen(realmname)+1)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for query");
		free(sql);
		return 0;
	}
	sprintf(sql, "DELETE FROM %s WHERE accname=\'%s\' AND charname=\'%s\' AND realm=\'%s\' AND recage=0",db_table,account,charname,realmname);
	if(mysql_query(&mysql,sql))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "error deleting character \"%s(*%s)@%s\" (%d:%s)",charname,account,realmname,mysql_errno(&mysql),mysql_error(&mysql));
		free(sql);
		return 0;
	}
	free(sql);
    if(mysql_affected_rows(&mysql)!=0)
	{
		eventlog(eventlog_level_error, __FUNCTION__,"zero data records for \"%s(*%s)@%s\" deleted (%d:%s)",charname,account,realmname,mysql_errno(&mysql),mysql_error(&mysql));
		return 0;
	}
	eventlog(eventlog_level_trace, __FUNCTION__, "successfully deleted record \"%s(*%s)@%s\"",charname,account,realmname);
	return 1;
}


extern unsigned int db_d2char_create(char const * db_table,char const * account,char const * charname,t_d2charinfo_file * charinfo,char * chardata, unsigned int chardata_size)
{
	MYSQL			mysql = {0};    /* MySQL headers aren't correct initialized */
	char 			* sql;
	unsigned int	charinfo_size = sizeof(*charinfo);
	unsigned int	esc_charinfo_size = charinfo_size*2+1;	/* Escapeing binary data needs in worst case 2*size + 1 real zero */
	char			esc_charinfo[esc_charinfo_size];
	unsigned int	esc_chardata_size = chardata_size*2+1;
	char			esc_chardata[esc_chardata_size];

	/* This shall allow us to reconnect, after a disconnected MySQL link */
	if (!db_connect(&mysql)) return 0;

	if (!strlen(charinfo->header.realmname)) {
		eventlog(eventlog_level_error, __FUNCTION__, "got no realm name");
		db_close(&mysql);
		return 0;
	}

	if (!(sql = malloc(strlen("INSERT  (charname,accname,realm,mtime,ctime,btime,recage,charinfo,chardata) VALUES ('','','',FROM_UNIXTIME()+0,FROM_UNIXTIME()+0,NOW()+0,0,'','')")+strlen(db_table)+strlen(charname)+strlen(account)+strlen(charinfo->header.realmname)+10+10+esc_charinfo_size+esc_chardata_size+1)))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for query");
		free(sql);
		db_close(&mysql);
		return 0;
	}

	/* Escape charinfo and chardata */
	if(!mysql_real_escape_string(&mysql, esc_charinfo, (char const *) charinfo, charinfo_size))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "charinfo escape failed");
		free(sql);
		db_close(&mysql);
		return 0;
	}
	if(!mysql_real_escape_string(&mysql, esc_chardata, chardata, chardata_size))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "chardata escape failed");
		free(sql);
		db_close(&mysql);
		return 0;
	}

	/* Build real sql command */
	sprintf(sql, "INSERT %s (charname,accname,realm,mtime,ctime,btime,recage,charinfo,chardata) VALUES (\'%s\',\'%s\',\'%s\',FROM_UNIXTIME(%lu)+0,FROM_UNIXTIME(%u)+0,NOW()+0,0,\'%s\',\'%s\')",db_table,charname,account,charinfo->header.realmname,bn_int_get(charinfo->header.last_time),bn_int_get(charinfo->header.create_time),esc_charinfo,esc_chardata);
	if(mysql_query(&mysql,sql))
	{
		eventlog(eventlog_level_error, __FUNCTION__,"creation of character \"%s(*%s)@%s\" failed (%d:%s)",
					charname, account, charinfo->header.realmname, mysql_errno(&mysql),mysql_error(&mysql));
		free(sql);
		db_close(&mysql);
		return 0;
	}
	free(sql);
	if(mysql_affected_rows(&mysql)<=0)
	{
		eventlog(eventlog_level_error, __FUNCTION__,"zero data records \"%s(*%s)@%s\" inserted (%d:%s)",charname,account,charinfo->header.realmname,mysql_errno(&mysql),mysql_error(&mysql));
		free(sql);
		db_close(&mysql);
		return 0;
	}
	eventlog(eventlog_level_trace, __FUNCTION__, "successfully inserted record \"%s(*%s)@%s\"", charinfo->header.charname, charinfo->header.account, charinfo->header.realmname);
	db_close(&mysql);
	return 1;
}


#endif  /* WITH_MYSQL */

