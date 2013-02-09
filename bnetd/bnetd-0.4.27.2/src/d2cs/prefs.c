/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Some DB-Storage (MySQL) modifications:
 *      Copyright (C) 2002  Joerg Ebeling (jebs@shbe.net)
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
#include "setup.h"

#ifdef HAVE_STDDEF_H
# include <stddef.h>
#else
# ifndef NULL
#  define NULL ((void *)0)
# endif
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
# ifdef HAVE_MEMORY_H
#  include <memory.h>
# endif
#endif
#include "compat/memset.h"

#include "conf.h"
#include "prefs.h"
#include "common/eventlog.h"
#ifdef WITH_STORAGE_DB
# ifdef WITH_MYSQL
	#include <mysql.h>
# endif
#endif
#include "common/setup_after.h"


typedef struct
{
        char const      * logfile;
        char const      * loglevels;
        char const      * servaddrs;
        char const      * gameservlist;
        char const      * bnetdaddr;
        char const      * charsavedir;
        char const      * charinfodir;
        char const      * ladderdir;
        char const      * newbiefile;
        char const      * motd;
        char const      * realmname;
        char const      * d2gs_password;
        unsigned int    ladder_refresh_interval;
        unsigned int    maxchar;
        unsigned int    listpurgeinterval;
        unsigned int    gqcheckinterval;
        unsigned int    s2s_retryinterval;
        unsigned int    s2s_timeout;
        unsigned int    s2s_idletime;
        unsigned int    sq_checkinterval;
        unsigned int    sq_timeout;
        unsigned int    maxgamelist;
        unsigned int    max_game_idletime;
        unsigned int    gamelist_showall;
        unsigned int    game_maxlifetime;
        unsigned int    allow_gamelimit;
        unsigned int    allow_newchar;
        unsigned int    idletime;
        unsigned int    shutdown_delay;
        unsigned int    shutdown_decr;
        unsigned int    d2gs_checksum;
        unsigned int    d2gs_version;
        unsigned int    check_multilogin;
        unsigned int    timeout_checkinterval;
        unsigned int    s2s_keepalive_interval;
        unsigned int    d2gs_restart_delay;
	#ifdef WITH_STORAGE_DB
	    char const 		* db_host;
	    unsigned int  	db_port;
	    char const 		* db_name;
	    char const 		* db_char_table;
	    char const 		* db_user;
	    char const 		* db_pw;
	#endif
} t_prefs;

static t_prefs prefs_conf;


/* FIXME: arg... pointers can't always fit into ints nor are they guaranteed to be numeric */
static t_conf_table prefs_conf_table[]={
    /* I commented stuff out here because it was causing compiler errors and I didn't know how to fix them */
    /*{ "logfile",                offsetof(t_prefs,logfile),                conf_type_str,    (const int)DEFAULT_LOG_FILE         },*/
    /*{ "loglevels",              offsetof(t_prefs,loglevels),              conf_type_str,    (const int)DEFAULT_LOG_LEVELS       },*/
    /*{ "servaddrs",              offsetof(t_prefs,servaddrs),              conf_type_str,    (const int)D2CS_SERVER_ADDRS        },*/
    /*{ "gameservlist",           offsetof(t_prefs,gameservlist),           conf_type_str,    (const int)D2GS_SERVER_LIST         },*/
    /*{ "bnetdaddr",              offsetof(t_prefs,bnetdaddr),              conf_type_str,    (const int)BNETD_SERVER_LIST        },*/
    /*{ "charsavedir",            offsetof(t_prefs,charsavedir),            conf_type_str,    (const int)D2CS_CHARSAVE_DIR        },*/
    /*{ "charinfodir",            offsetof(t_prefs,charinfodir),            conf_type_str,    (const int)D2CS_CHARINFO_DIR        },*/
    /*{ "ladderdir",              offsetof(t_prefs,ladderdir),              conf_type_str,    (const int)D2CS_LADDER_DIR          },*/
    { "ladder_refresh_interval",offsetof(t_prefs,ladder_refresh_interval),conf_type_int,    3600                          },
    /*{ "newbiefile",             offsetof(t_prefs,newbiefile),             conf_type_str,    (const int)D2CS_CHARSAVE_NEWBIE     },*/
    /*{ "motd",                   offsetof(t_prefs,motd),                   conf_type_hexstr, (const int)D2CS_MOTD                },*/
    /*{ "realmname",              offsetof(t_prefs,realmname),              conf_type_str,    (const int)DEFAULT_REALM_NAME       },*/
    { "maxchar",                offsetof(t_prefs,maxchar),                conf_type_int,    MAX_CHAR_PER_ACCT             },
    { "listpurgeinterval",      offsetof(t_prefs,listpurgeinterval),      conf_type_int,    LIST_PURGE_INTERVAL           },
    { "gqcheckinterval",        offsetof(t_prefs,gqcheckinterval),        conf_type_int,    GAMEQUEUE_CHECK_INTERVAL      },
    { "maxgamelist",            offsetof(t_prefs,maxgamelist),            conf_type_int,    MAX_GAME_LIST                 },
    { "max_game_idletime",      offsetof(t_prefs,max_game_idletime),      conf_type_int,    MAX_GAME_IDLE_TIME            },
    { "gamelist_showall",       offsetof(t_prefs,gamelist_showall),       conf_type_bool,   0                             },
    { "game_maxlifetime",       offsetof(t_prefs,game_maxlifetime),       conf_type_int,    0                             },
    { "allow_gamelimit",        offsetof(t_prefs,allow_gamelimit),        conf_type_bool,   1                             },
    { "allow_newchar",          offsetof(t_prefs,allow_newchar),          conf_type_bool,   1                             },
    { "idletime",               offsetof(t_prefs,idletime),               conf_type_int,    MAX_CLIENT_IDLETIME           },
    { "shutdown_delay",         offsetof(t_prefs,shutdown_delay),         conf_type_int,    DEFAULT_SHUTDOWN_DELAY        },
    { "shutdown_decr",          offsetof(t_prefs,shutdown_decr),          conf_type_int,    DEFAULT_SHUTDOWN_DECR         },
    { "s2s_retryinterval",      offsetof(t_prefs,s2s_retryinterval),      conf_type_int,    DEFAULT_S2S_RETRYINTERVAL     },
    { "s2s_timeout",            offsetof(t_prefs,s2s_timeout),            conf_type_int,    DEFAULT_S2S_TIMEOUT           },
    { "sq_checkinterval",       offsetof(t_prefs,sq_checkinterval),       conf_type_int,    DEFAULT_SQ_CHECKINTERVAL      },
    { "sq_timeout",             offsetof(t_prefs,sq_timeout),             conf_type_int,    DEFAULT_SQ_TIMEOUT            },
    { "d2gs_checksum",          offsetof(t_prefs,d2gs_checksum),          conf_type_int,    0                             },
    { "d2gs_version",           offsetof(t_prefs,d2gs_version),           conf_type_int,    0                             },
    /*{ "d2gs_password",          offsetof(t_prefs,d2gs_password),          conf_type_str,    (const int)""                       },*/
    { "check_multilogin",       offsetof(t_prefs,check_multilogin),       conf_type_int,    1                             },
    { "s2s_idletime",           offsetof(t_prefs,s2s_idletime),           conf_type_int,    DEFAULT_S2S_IDLETIME          },
    { "s2s_keepalive_interval", offsetof(t_prefs,s2s_keepalive_interval), conf_type_int,    DEFAULT_S2S_KEEPALIVE_INTERVAL},
    { "timeout_checkinterval",  offsetof(t_prefs,timeout_checkinterval),  conf_type_int,    DEFAULT_TIMEOUT_CHECKINTERVAL },
#ifdef WITH_STORAGE_DB
    { "db_host",                offsetof(t_prefs,db_host),                conf_type_str,    (const int)LOCAL_HOST       		  },
    { "db_port",                offsetof(t_prefs,db_port),                conf_type_int,    MYSQL_PORT             		  },
    { "db_name",                offsetof(t_prefs,db_name),                conf_type_str,    (const int)DB_NAME          		  },
    { "db_char_table",          offsetof(t_prefs,db_char_table),          conf_type_str,    (const int)DB_CHAR_TABLE            },
    { "db_user",                offsetof(t_prefs,db_user),                conf_type_str,    (const int)""                       },
    { "db_pw",                  offsetof(t_prefs,db_pw),                  conf_type_str,    (const int)""                       },
#endif
    { "d2gs_restart_delay",		offsetof(t_prefs,d2gs_restart_delay),	  conf_type_int,    DEFAULT_D2GS_RESTART_DELAY	  },
    { NULL,                     0,                                        conf_type_none,   0                             }
};


extern int prefs_load(char const * filename)
{
	memset(&prefs_conf,0,sizeof(prefs_conf));
	if (conf_load_file(filename,prefs_conf_table,&prefs_conf,sizeof(prefs_conf))<0) {
		return -1;
	}
	return 0;
}

extern int prefs_reload(char const * filename)
{
	prefs_unload();
	if (prefs_load(filename)<0) return -1;
	return 0;
}

extern int prefs_unload(void)
{
	return conf_cleanup(prefs_conf_table, &prefs_conf, sizeof(prefs_conf));
}

extern char const * prefs_get_servaddrs(void)
{
	return prefs_conf.servaddrs;
}

extern char const * prefs_get_charsave_dir(void)
{
	return prefs_conf.charsavedir;
}

extern char const * prefs_get_charinfo_dir(void)
{
	return prefs_conf.charinfodir;
}

extern char const * prefs_get_charsave_newbie(void)
{
	return prefs_conf.newbiefile;
}

extern char const * prefs_get_motd(void)
{
	return prefs_conf.motd;
}

extern char const * prefs_get_d2gs_list(void)
{
	return prefs_conf.gameservlist;
}

extern unsigned int prefs_get_maxchar(void)
{
	return prefs_conf.maxchar;
}

extern unsigned int prefs_get_list_purgeinterval(void)
{
	return prefs_conf.listpurgeinterval;
}

extern unsigned int prefs_get_gamequeue_checkinterval(void)
{
	return prefs_conf.gqcheckinterval;
}

extern unsigned int prefs_get_maxgamelist(void)
{
	return prefs_conf.maxgamelist;
}

extern unsigned int prefs_allow_newchar(void)
{
	return prefs_conf.allow_newchar;
}

extern unsigned int prefs_get_idletime(void)
{
	return prefs_conf.idletime;
}

extern char const * prefs_get_logfile(void)
{
	return prefs_conf.logfile;
}

extern unsigned int prefs_get_shutdown_delay(void)
{
	return prefs_conf.shutdown_delay;
}

extern unsigned int prefs_get_shutdown_decr(void)
{
	return prefs_conf.shutdown_decr;
}

extern char const * prefs_get_bnetdaddr(void)
{
	return prefs_conf.bnetdaddr;
}

extern char const * prefs_get_realmname(void)
{
	return prefs_conf.realmname;
}

extern unsigned int prefs_get_s2s_retryinterval(void)
{
	return prefs_conf.s2s_retryinterval;
}

extern unsigned int prefs_get_s2s_timeout(void)
{
	return prefs_conf.s2s_timeout;
}

extern unsigned int prefs_get_sq_timeout(void)
{
	return prefs_conf.sq_timeout;
}

extern unsigned int prefs_get_sq_checkinterval(void)
{
	return prefs_conf.sq_checkinterval;
}

extern unsigned int prefs_get_d2gs_checksum(void)
{
	return prefs_conf.d2gs_checksum;
}

extern unsigned int prefs_get_d2gs_version(void)
{
	return prefs_conf.d2gs_version;
}

extern unsigned int prefs_get_ladderlist_count(void)
{
	return 0x10;
}

extern unsigned int prefs_get_d2ladder_refresh_interval(void)
{
	return prefs_conf.ladder_refresh_interval;
}

extern unsigned int prefs_get_game_maxlifetime(void)
{
	return prefs_conf.game_maxlifetime;
}

extern char const * prefs_get_ladder_dir(void)
{
	return prefs_conf.ladderdir;
}

extern char const * prefs_get_loglevels(void)
{
	return prefs_conf.loglevels;
}

extern unsigned int prefs_allow_gamelist_showall(void)
{
	return prefs_conf.gamelist_showall;
}

extern unsigned int prefs_allow_gamelimit(void)
{
	return prefs_conf.allow_gamelimit;
}

extern unsigned int prefs_check_multilogin(void)
{
	return prefs_conf.check_multilogin;
}

extern char const * prefs_get_d2gs_password(void)
{
	return prefs_conf.d2gs_password;
}

extern unsigned int prefs_get_s2s_idletime(void)
{
	return prefs_conf.s2s_idletime;
}

extern unsigned int prefs_get_s2s_keepalive_interval(void)
{
	return prefs_conf.s2s_keepalive_interval;
}

extern unsigned int prefs_get_timeout_checkinterval(void)
{
	return prefs_conf.timeout_checkinterval;
}

extern unsigned int prefs_get_max_game_idletime(void)
{
	return prefs_conf.max_game_idletime;
}

extern unsigned int prefs_get_d2gs_restart_delay(void)
{
	return prefs_conf.d2gs_restart_delay;
}

#ifdef WITH_STORAGE_DB
extern char const * prefs_get_db_host(void)
{
	return prefs_conf.db_host;
}

extern unsigned int prefs_get_db_port(void)
{
	return prefs_conf.db_port;
}

extern char const * prefs_get_db_name(void)
{
	return prefs_conf.db_name;
}

extern char const * prefs_get_db_char_table(void)
{
	return prefs_conf.db_char_table;
}

extern char const * prefs_get_db_user(void)
{
	return prefs_conf.db_user;
}

extern char const * prefs_get_db_pw(void)
{
	return prefs_conf.db_pw;
}
#endif
