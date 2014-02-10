/* account_db_mysql.c
 * Copyright (C) 2002,2003  JEBs@shbe.net
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
 * JE20021025 - Merged back the QuickAndDirty update code and made some
 *              bug testing/fixing with valgrind
 * JE20021024 - Reorganized sources to share some code together with d2cs and d2dbs
 * JE20020909 - Removed compiler warning "pointer from int without cast"
 *            - Fixed init mistake of MYSQL struct
 * JE20020724 - Initial version 0.1
 */

#ifdef WITH_MYSQL
#include "common/setup_before.h"
#include <stdio.h>
#ifdef HAVE_STDDEF_H
# include <stddef.h>
#else
# ifndef NULL
#  define NULL ((void *)0)
# endif /* !NULL */
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
#include "compat/strncasecmp.h"
#include <errno.h>
#include "compat/pdir.h"
#include "common/eventlog.h"
#include "prefs.h"
#include "common/util.h"
#include "common/field_sizes.h"
#include <mysql.h>
#include "common/db_mysql.h"
#include "common/setup_after.h"

static int db_server_connect_done = 0;		/* Info logging & table update checking */


static unsigned int db_table_check(char const * db_table);
static unsigned int db_account_table_create(char const * db_table);
static unsigned int db_account_table_import(char const * db_table);
static unsigned int db_account_update_default_user(char const * db_table);
static char const * db_account_get_username(char * pathname);
static unsigned int db_account_file2table(char const * username, char const * pathname, char const * db_table);


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

	if(db_server_connect_done) return 1;

	db_server_connect_done = 1;
	eventlog(eventlog_level_info, __FUNCTION__, "connected to MySQL (server version %s)", mysql_get_server_info(mysql));

	/* Do some DB checks, updates, imports, ... */
	if(!db_table_check(prefs_get_db_acc_table())) return -1;

	return 1;
}


extern void db_account_save(char const * id, char const * key, char const * val,char const * db_table,t_db_result * myresult)
{
    /* This shall allow us to reconnect, after a disconnected MySQL link */
    if (!db_connect(&myresult->mysql)) return;

    /* At the moment each key/value pair is saved in an own record.
       Later on i'll test how much query-overhead we pay for that */
    if (!(myresult->sql = malloc(strlen("REPLACE  (id,attr,val) VALUES (\"\",\"\",\"\")")+strlen(db_table)+strlen(id)+strlen(key)+strlen(val)+1)))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to allocate memory for query");
    	free(myresult->sql);
	    return;
    }
    /* Remember that mySQL uses escaped strings to.
     * Send an "BNETD\\..." string to mySQL means in record is an "BNETD\..." !!!
     */
    sprintf(myresult->sql,"REPLACE %s (id,attr,val) VALUES (\"%s\",\"%s\",\"%s\")",db_table,id,key,val);

    if (mysql_query(&myresult->mysql,myresult->sql))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"could not replace attribute records \"%s\"=\"%s\" for account \"%s\" (%d:%s)",key,val,id,mysql_errno(&myresult->mysql),mysql_error(&myresult->mysql));
    	return;
    }
    free(myresult->sql);
    return;
}


extern int db_account_list(char const * db_table,t_db_result * myresult)
{
    if (!db_connect(&myresult->mysql)) return -1;

    if (!(myresult->sql = malloc(strlen("SELECT id from  WHERE id!='default_user' GROUP BY id")+strlen(db_table)+1)))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to allocate memory for query");
    	return -1;
    }
    sprintf(myresult->sql,"SELECT id from %s WHERE id!='default_user' GROUP BY id",db_table);
    if (mysql_query(&myresult->mysql,myresult->sql))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"could not query account list (%d:%s)",mysql_errno(&myresult->mysql),mysql_error(&myresult->mysql));
    	return -1;
    }

    if (!(myresult->res = mysql_use_result(&myresult->mysql)))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"could not get query result (%d:%s)",mysql_errno(&myresult->mysql),mysql_error(&myresult->mysql));
    	return -1;
    }
    return 1;
}


extern char * db_account_list_getnext(t_db_result * myresult)
{
    if (!(myresult->row = mysql_fetch_row(myresult->res)))
    	return '\0';     /* no more results */
    return myresult->row[0];
}


extern int db_account_load(char const * id,char const * db_table,t_db_result * myresult)
{
    if (!db_connect(&myresult->mysql)) return -1;

    if (!(myresult->sql = malloc(strlen("SELECT CONCAT('\"',attr,'\"=\"',val,'\"') FROM  WHERE id=\"\"")+strlen(db_table)+strlen(id)+1)))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to allocate memory for query");
    	return -1;
    }
    sprintf(myresult->sql,"SELECT CONCAT('\"',attr,'\"=\"',val,'\"') FROM %s WHERE id=\"%s\"",db_table,id);
    if (mysql_query(&myresult->mysql, myresult->sql))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"could not query account \"%s\" (%d:%s)",id,mysql_errno(&myresult->mysql),mysql_error(&myresult->mysql));
    	free(myresult->sql);
	    return -1;
    }
    free(myresult->sql);

    if (!(myresult->res = mysql_use_result(&myresult->mysql)))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not get query result (%d:%s)",mysql_errno(&myresult->mysql),mysql_error(&myresult->mysql));
        return -1;
    }
    return 1;
}


extern char const * db_account_load_getnextattrib(t_db_result * myresult)
{
    if (!(myresult->row = mysql_fetch_row(myresult->res)))
    	return '\0';     /* no more results */
    return myresult->row[0];
}


/* ********** Table checking, creation and update code ********** */


static unsigned int db_table_check(char const * db_table)
{
	t_db_result		mycon = {0};
    MYSQL_FIELD 	*field;
    unsigned int    i,
                    cfn,
                    tfn;

    eventlog(eventlog_level_trace,__FUNCTION__,"checking account table");
    if (!db_connect(&mycon.mysql)) return 0;

    /* Check if we have the right version */
    if (!(mycon.sql = malloc(strlen("SHOW TABLE STATUS LIKE ''")+strlen(db_table)+1)))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to allocate memory for query");
    	return 0;
    }
    sprintf(mycon.sql,"SHOW TABLE STATUS LIKE '%s'",db_table);
    if (mysql_query(&mycon.mysql, mycon.sql))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not check account table (%d:%s)",mysql_errno(&mycon.mysql),mysql_error(&mycon.mysql));
        return 0;
    }
    if (!(mycon.res = mysql_use_result(&mycon.mysql)))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not get query result (%d:%s)",mysql_errno(&mycon.mysql),mysql_error(&mycon.mysql));
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
        /* FIXME: In next account table update change this to "account v0.x" */
        if(strncasecmp(mycon.row[cfn],"v0.1",4))
        {
            eventlog(eventlog_level_error,__FUNCTION__,"wrong table version (\"%s\" != \"v0.1\")",mycon.row[cfn]);
            mysql_free_result(mycon.res);
            return 0;
        }
    } else {
        /* Zero rows in "SHOW TABLE STATUS LIKE 'account'" means that there is no table with that name
         * Well, lets create a new one
         */
        if(!db_account_table_create(db_table))
        {
            mysql_free_result(mycon.res);
            return 0;
        }

        /* Now that the table exists, fill it from the flat files */
        if(!db_account_table_import(db_table))
        {
            mysql_free_result(mycon.res);
            return 0;
        }
    }

    if(!db_account_update_default_user(db_table))
        eventlog(eventlog_level_warn,__FUNCTION__,"could not update bnetd_default_user");

    mysql_free_result(mycon.res);

    return 1;
}


static unsigned int db_account_table_create(char const * db_table)
{
	t_db_result		mycon = {0};

    if (!db_connect(&mycon.mysql)) return 0;
    if (!(mycon.sql = malloc(strlen("CREATE TABLE  (id varchar(16) NOT NULL default '', attr varchar(255) NOT NULL default '', val varchar(255) default NULL, PRIMARY KEY (id,attr), KEY id(id), KEY attr(attr)) TYPE=MyISAM CHECKSUM=1 COMMENT='v0.1'")+strlen(db_table)+1)))
    {
	    eventlog(eventlog_level_error,__FUNCTION__,"unable to allocate memory for query");
    	return -1;
    }
    sprintf(mycon.sql,"CREATE TABLE %s (id varchar(16) NOT NULL default '', attr varchar(255) NOT NULL default '', val varchar(255) default NULL, PRIMARY KEY (id,attr), KEY id(id), KEY attr(attr)) TYPE=MyISAM CHECKSUM=1 COMMENT='v0.1'",db_table);
    if (mysql_query(&mycon.mysql, mycon.sql))
    {
        eventlog(eventlog_level_error,__FUNCTION__,"could not create account table (%d:%s)",mysql_errno(&mycon.mysql),mysql_error(&mycon.mysql));
        return 0;
    }
    eventlog(eventlog_level_info,__FUNCTION__,"new 'account' table (version 0.1) created");
    return 1;
}


static unsigned int db_account_table_import(char const * db_table)
{
    /* Quick and Dirty import of all flat file user accounts */
    char const 		* dentry;
    char 			* pathname;
    unsigned long	count = 0;
    t_pdir 			* accountdir;
    char const 		* username;

    eventlog(eventlog_level_debug,"db_mysql_account_table_import","import account table from flat file accounts");

    if (!(accountdir = p_opendir(prefs_get_userdir())))
    {
        eventlog(eventlog_level_error,"db_mysql_account_table_import","unable to open user directory \"%s\" for reading (p_opendir: %s)",prefs_get_userdir(),strerror(errno));
        return 0;
    }

    while ((dentry = p_readdir(accountdir)))
    {
        if (dentry[0]=='.') continue;
        if (!(pathname = malloc(strlen(prefs_get_userdir())+1+strlen(dentry)+1))) /* dir + / + file + NUL */
        {
            eventlog(eventlog_level_error,"db_mysql_account_table_import","could not allocate memory for pathname");
            continue;
        }
        sprintf(pathname,"%s/%s",prefs_get_userdir(),dentry);

        if(!strlen(pathname))
        {
            eventlog(eventlog_level_error,"db_mysql_account_table_import","empty account filename");
            free(pathname);
            continue;
        }

        username = db_account_get_username(pathname);
        if(strlen(username))
        {
            eventlog(eventlog_level_debug,"db_mysql_account_table_import","importing username \"%s\" from file \"%s\"",username, pathname);
            if(!db_account_file2table(username, pathname, db_table))
                eventlog(eventlog_level_warn,"db_mysql_account_table_import","import failed for user \"%s\", file \"%s\" !",username, pathname);
        }
        free((void *)username); /* avoid warning */
        free(pathname);
        count++;
    }

    if (p_closedir(accountdir)<0)
	    eventlog(eventlog_level_error,"db_mysql_account_table_import","unable to close user directory \"%s\" (p_closedir: %s)",prefs_get_userdir(),strerror(errno));

    eventlog(eventlog_level_info,"db_mysql_account_table_import","imported %lu user accounts",count);
    return 1;
}


static unsigned int db_account_update_default_user(char const * db_table)
{
    char const * pathname;

    pathname = prefs_get_defacct();
    if(!strlen(pathname))
    {
        eventlog(eventlog_level_debug,"db_mysql_account_update_default_user","got empty bnetd_default_user path");
        return 0;
    }

    if(!db_account_file2table("default_user", pathname, db_table))
        eventlog(eventlog_level_warn,"db_mysql_account_update_default_user","import failed for user \"default_user\", file \"%s\" !", pathname);

    return 1;
}


static char const * db_account_get_username(char * pathname)
{
    /* Quick and Dirty username analysation (cause this runs only once) */
    FILE *       accountfile;
    unsigned int line;
    char const * buff;
    char const * key;
    char const * val;
    char *       esckey;
    char *       escval;
    unsigned int len;

    if(strlen(pathname))
    {
        if (!(accountfile = fopen(pathname,"r")))
        {
            eventlog(eventlog_level_error,"db_mysql_account_get_username","could not open account file \"%s\" for reading (fopen: %s)",pathname,strerror(errno));
        } else {
            for (line=1; (buff=file_get_line(accountfile)); line++)
            {
                if (buff[0]=='#' || buff[0]=='\0')
                {
                    free((void *)buff); /* avoid warning */
                    continue;
                }
                if (strlen(buff)<6) /* "?"="" */
                {
                    eventlog(eventlog_level_error,"db_mysql_account_get_username","malformed line %d of account file \"%s\"",line,pathname);
                    free((void *)buff); /* avoid warning */
                    continue;
                }

                len = strlen(buff)-5+1; /* - ""="" + NUL */
                if (!(esckey = malloc(len)))
                {
                    eventlog(eventlog_level_error,"db_mysql_account_get_username","could not allocate memory for esckey on line %d of account file \"%s\"",line,pathname);
                    free((void *)buff); /* avoid warning */
                    continue;
                }
                if (!(escval = malloc(len)))
                {
                    eventlog(eventlog_level_error,"db_mysql_account_get_username","could not allocate memory for escval on line %d of account file \"%s\"",line,pathname);
                    free((void *)buff); /* avoid warning */
                    free(esckey);
                    continue;
                }

                if (sscanf(buff,"\"%[^\"]\" = \"%[^\"]\"",esckey,escval)!=2)
                {
                    if (sscanf(buff,"\"%[^\"]\" = \"\"",esckey)!=1) /* hack for an empty value field */
                    {
                        eventlog(eventlog_level_error,"db_mysql_account_get_username","malformed entry on line %d of account file \"%s\"",line,pathname);
                        free(escval);
                        free(esckey);
                        free((void *)buff); /* avoid warning */
                        continue;
                    }
                    escval[0] = '\0';
                }
                free((void *)buff); /* avoid warning */
                key = unescape_chars(esckey);

                if(!strncasecmp(key,"BNET\\acct\\username",strlen("BNET\\acct\\username")))
                {
                    val = unescape_chars(escval);
                    free(esckey);
                    free(escval);
                    if (key) free((void *)key); /* avoid warning */

                    if (fclose(accountfile)<0)
                        eventlog(eventlog_level_error,"db_mysql_account_get_username","could not close account file \"%s\" after reading (fclose: %s)",pathname,strerror(errno));

                    return val;
                }
                free(esckey);
                free(escval);

                if (key) free((void *)key); /* avoid warning */
            }
            if (fclose(accountfile)<0)
                eventlog(eventlog_level_error,"db_mysql_account_get_username","could not close account file \"%s\" after reading (fclose: %s)",pathname,strerror(errno));
        }
    } /* empty pathname */
    return NULL;
}


static unsigned int db_account_file2table(char const * username, char const * pathname, char const * db_table)
{
	t_db_result  mycon = {0};
    FILE *       accountfile;
    unsigned int line;
    char const   * buff;
    unsigned int len;
    char *       esckey;
    char *       escval;

    if(!strlen(pathname))
    {
        eventlog(eventlog_level_warn,"db_mysql_account_file2table","got empty pathname");
        return 0;
    }

    if(!strlen(username))
    {
        eventlog(eventlog_level_warn,"db_mysql_account_file2table","got empty username");
        return 0;
    }

    if (!(accountfile = fopen(pathname,"r")))
    {
        eventlog(eventlog_level_error,"db_mysql_account_file2table","could not open account file \"%s\" for reading (fopen: %s)",pathname,strerror(errno));
        return 0;
    }

    for (line=1; (buff=file_get_line(accountfile)); line++)
    {
        if (buff[0]=='#' || buff[0]=='\0')
        {
            free((void *)buff); /* avoid warning */
            continue;
        }
        if (strlen(buff)<6) /* "?"="" */
        {
            eventlog(eventlog_level_error,"db_mysql_account_file2table","malformed line %d of account file \"%s\"",line,pathname);
            free((void *)buff); /* avoid warning */
            continue;
        }

        len = strlen(buff)-5+1; /* - ""="" + NUL */
        if (!(esckey = malloc(len)))
        {
            eventlog(eventlog_level_error,"db_mysql_account_file2table","could not allocate memory for esckey on line %d of account file \"%s\"",line,pathname);
            free((void *)buff); /* avoid warning */
            continue;
        }
        if (!(escval = malloc(len)))
        {
            eventlog(eventlog_level_error,"db_mysql_account_file2table","could not allocate memory for escval on line %d of account file \"%s\"",line,pathname);
            free((void *)buff); /* avoid warning */
            free(esckey);
            continue;
        }

        if (sscanf(buff,"\"%[^\"]\" = \"%[^\"]\"",esckey,escval)!=2)
        {
            if (sscanf(buff,"\"%[^\"]\" = \"\"",esckey)!=1) /* hack for an empty value field */
            {
                eventlog(eventlog_level_error,"db_mysql_account_file2table","malformed entry on line %d of account file \"%s\"",line,pathname);
                free(escval);
                free(esckey);
                free((void *)buff); /* avoid warning */
                continue;
            }
            escval[0] = '\0';
        }
        free((void *)buff); /* avoid warning */

        if (esckey && escval)
        {
            /* eventlog(eventlog_level_debug,"db_mysql_account_table_import","imported username = \"%s\", key = \"%s\", val = \"%s\"",username,esckey,escval); */
            db_account_save(username, esckey, escval, db_table, &mycon);
        }
        free(esckey);
        free(escval);
    } /* endfor */
    if (fclose(accountfile)<0)
        eventlog(eventlog_level_error,"db_mysql_account_file2table","could not close account file \"%s\" after reading (fclose: %s)",pathname,strerror(errno));

	db_close(&mycon.mysql);
    return 1;
}

#else
typedef int account_db_mysql_c_filenotempty; /* make ISO standard happy */
#endif  /* WITH_MYSQL */

/* EOF */
