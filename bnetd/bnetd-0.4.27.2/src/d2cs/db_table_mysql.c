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
 * JE20021026 - Complete redraw and valgrind testing
 * JE20021026 - Initial version 0.1
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
#include "common/eventlog.h"
#include "prefs.h"
#include "d2cs_d2gs_character.h"
#include <mysql.h>
#include "common/db_mysql.h"
#include "d2char_db_mysql.h"
#include "common/setup_after.h"


static unsigned int db_table_create(char const * db_table);
static unsigned int db_table_import(char const * db_table);


extern unsigned int db_table_init(void)
{
	MYSQL			mysql = {0};
	unsigned int	dbstatus;

	dbstatus = db_connect(&mysql);
	if(dbstatus == 2)
	{
        if(!db_table_create(prefs_get_db_char_table())) return 0;
        if(!db_table_import(prefs_get_db_char_table())) return 0;
	} else if(dbstatus>2) {
	   	eventlog(eventlog_level_error,__FUNCTION__,"unknown character table version");
	    return 0;
	} else return dbstatus;
}


/* ********** Table creation and update code ********** */


static unsigned int db_table_create(char const * db_table)
{
	t_db_result		mycon = {0};

    if (!db_connect(&mycon.mysql)) return 0;
    if (!(mycon.sql = malloc(strlen("CREATE TABLE  (charname varchar(16) NOT NULL default \'0\',")+
			strlen("accname varchar(16) NOT NULL default \'\',realm varchar(32) NOT NULL default \'\',")+
      		strlen("mtime timestamp(14) NOT NULL,ctime timestamp(14) NOT NULL,btime timestamp(14) NOT NULL,")+
        	strlen("recage smallint(5) unsigned default \'0\',charinfo blob NOT NULL,chardata blob NOT NULL,")+
         	strlen("UNIQUE KEY RepKey(charname,realm,recage),KEY accname(accname),KEY charname(charname),")+
          	strlen("KEY realm(realm),KEY recage(recage)")+
          	strlen(") TYPE=MyISAM COMMENT=\'d2character v0.1\'")+strlen(db_table)+1)))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to allocate memory for query");
    	return -1;
    }
    sprintf(mycon.sql,"CREATE TABLE %s (charname varchar(16) NOT NULL default \'0\',accname varchar(16) NOT NULL default \'\',realm varchar(32) NOT NULL default \'\',mtime timestamp(14) NOT NULL,ctime timestamp(14) NOT NULL,btime timestamp(14) NOT NULL,recage smallint(5) unsigned default \'0\',charinfo blob NOT NULL,chardata blob NOT NULL,UNIQUE KEY RepKey(charname,realm,recage),KEY accname(accname),KEY charname(charname),KEY realm(realm),KEY recage(recage)) TYPE=MyISAM COMMENT=\'d2character v0.1\'",db_table);
    if (mysql_query(&mycon.mysql, mycon.sql))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not create character table (%d:%s)",mysql_errno(&mycon.mysql),mysql_error(&mycon.mysql));
        return 0;
    }
    eventlog(eventlog_level_info,__FUNCTION__,"new character table (version 0.1) created");
    return 1;
}


/* Quick and Dirty import of all character files (info & data) */
static unsigned int db_table_import(char const * db_table)
{
	t_pdir				* accdir,
						* chardir;
	char const			* account,
						* character;
	t_d2charinfo_file	charinfo;
	unsigned long		countaccs = 0,
						countchars = 0;
	char				* path,
						* file;
	unsigned int		size;
	unsigned char		chardata[MAX_SAVEFILE_SIZE];

    eventlog(eventlog_level_info,__FUNCTION__,"import character table from flat file characters");

    /* walk through all charinfo/accountdirs */
    if (!(accdir = p_opendir(prefs_get_charinfo_dir())))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"unable to open charinfo directory \"%s\" for reading (p_opendir: %s)",prefs_get_charinfo_dir(),strerror(errno));
        return 0;
    }
	while ((account=p_readdir(accdir))) {
		if (account[0]=='.') continue;

		if(d2char_check_acctname(account)<0) {
    	    eventlog(eventlog_level_error,__FUNCTION__,"account \"%s\" hasn't passed account name check. skipping ...",account);
	        continue;
    	}

		if (!(path=malloc(strlen(prefs_get_charinfo_dir())+1+strlen(account)+1))) {
			eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for account path");
			return 0;
		}
	    sprintf(path,"%s/%s",prefs_get_charinfo_dir(),account);

	    /* Walk through each account/charname */
	    if (!(chardir = p_opendir(path)))
	    {
    	    eventlog(eventlog_level_error,__FUNCTION__,"unable to open charinfo directory \"%s\" for reading (p_opendir: %s) skipping ...",path,strerror(errno));
	        continue;
    	}
		while ((character=p_readdir(chardir))) {
			if (character[0]=='.') continue;

            /* Read charinfo */
			if (!(file=malloc(strlen(path)+1+strlen(character)+1))) {
				eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charinfo path");
				free(path);
				return 0;
			}
		    sprintf(file,"%s/%s",path,character);

         	size = sizeof(t_d2charinfo_file);
			if (file_read(file,&charinfo,&size)<0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading charinfo file \"%s\" skipping ...",file);
				free(file);
				continue;
			}
			free(file);

			if (size!=sizeof(t_d2charinfo_file)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad charinfo file %s (length %d) skipping ...",file,size);
				continue;
			}
			if(d2charinfo_check(&charinfo)<0) {
				eventlog(eventlog_level_error, __FUNCTION__, "charinfo check of file %s failed ! skipping ...",file);
				continue;
			}
			
         	/* Read related chardata */
			str_to_lower(charinfo.header.charname);
			if(d2char_check_charname(charinfo.header.charname)<0) {
    		    eventlog(eventlog_level_error,__FUNCTION__,"character \"%s\" hasn't passed charname check. skipping ...",charinfo.header.charname);
	    	    continue;
	    	}
			if (!(file=malloc(strlen(prefs_get_charsave_dir())+1+strlen(charinfo.header.charname)+1))) {
				eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charsave path");
				free(file);
				free(path);
				return 0;
			}
		    sprintf(file,"%s/%s",prefs_get_charsave_dir(),charinfo.header.charname);

			size = MAX_SAVEFILE_SIZE;
			if (file_read(file,&chardata,&size)<0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading charsave file \"%s\" skipping ...",file);
				free(file);
				continue;
			}
			free(file);

			if (size>MAX_SAVEFILE_SIZE) {
				eventlog(eventlog_level_error, __FUNCTION__, "charsave file \"%s\" exceeds buffer size \"%u\". skipping ...",file,MAX_SAVEFILE_SIZE);
				continue;
			}

			/* Use the realmname from d2cs instead of the one used in the charinfo cause it could be happen,
				that someone wasn't in since the last realmname change */
		    sprintf(charinfo.header.realmname,"%s",prefs_get_realmname());
			if(!db_d2char_create(prefs_get_db_char_table(),charinfo.header.account,charinfo.header.charname,&charinfo,chardata,size))
				continue;			
		
         	eventlog(eventlog_level_debug,__FUNCTION__,"imported \"%s(*%s)@%s\"",charinfo.header.charname,charinfo.header.account,charinfo.header.realmname);
			countchars++;			
		}
	    if (p_closedir(chardir)<0)
		    eventlog(eventlog_level_error,__FUNCTION__,"unable to close character directory \"%s\" (p_closedir: %s)",path,strerror(errno));
		free(path);
		countaccs++;
    }

    if (p_closedir(accdir)<0)
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to close user directory \"%s\" (p_closedir: %s)",prefs_get_charinfo_dir(),strerror(errno));

    eventlog(eventlog_level_info,__FUNCTION__,"imported %lu characters form %lu accounts",countchars,countaccs);
    return 1;
}

#endif  /* WITH_MYSQL */

