/*
 * Copyright (C) 1998,1999,2000,2001,2002  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 1999  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1999,2000  Marco Ziech (mmz@gmx.net)
 * Some DB-Storage (MySQL) modifications:
 *     Copyright (C) 2002  Joerg Ebeling (jebs@shbe.net)
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
#define PREFS_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "common/conftable.h"
#include "prefs.h"
#ifdef WITH_STORAGE_DB
# ifdef WITH_MYSQL
	#include <mysql.h>
# endif
#endif
#include "common/setup_after.h"


static struct
{
    char const * filedir;
    char const * userdir;
    char const * logfile;
    char const * loglevels;
    char const * defacct;
    char const * welcomefile;
    char const * motdfile;
    char const * newsfile;
    char const * channelfile;
    char const * pidfile;
    char const * adfile;
    unsigned int usersync;
    unsigned int userflush;
    char const * servername;
    unsigned int track;
    char const * location;
    char const * description;
    char const * url;
    char const * contact_name;
    char const * contact_email;
    unsigned int latency;
    unsigned int irc_latency;
    unsigned int shutdown_delay;
    unsigned int shutdown_decr;
    int          new_accounts;
    unsigned int max_accounts;
    int          kick_old_login;
    int          ask_new_channel;
    int          hide_pass_games;
    int          hide_started_games;
    int          hide_temp_channels;
    int          hide_addr;
    int          enable_conn_all;
    int          extra_commands;
    int          stats_all_ladder;
    char const * reportdir;
    int          report_all_games;
    int          report_diablo_games;
    char const * iconfile;
    char const * tosfile;
    char const * mpqfile;
    char const * trackaddrs;
    char const * servaddrs;
    char const * ircaddrs;
    int          use_keepalive;
    unsigned int udptest_port;
    int          do_uplink;
    char const * uplink_server;
    char const * uplink_username;
    int          allow_uplink;
    char const * bits_password_file;
    char const * ipbanfile;
    int          bits_debug;
    int          disc_is_loss;
    char const * helpfile;
    char const * fortunecmd;
    char const * transfile;
    int          chanlog;
    char const * chanlogdir;
    int          quota;
    unsigned int quota_lines;
    unsigned int quota_time;
    unsigned int quota_wrapline;
    unsigned int quota_maxline;
    unsigned int ladder_init_rating;
    unsigned int quota_dobae;
    char const * realmfile;
    char const * issuefile;
    char const * bits_motd_file;
    char const * effective_user;
    char const * effective_group;
    unsigned int nullmsg;
    int          mail_support;
    unsigned int mail_quota;
    char const * maildir;
    char const * log_notice;
#ifndef ACCT_DYN_LOAD
    int          savebyname;
#endif
    int          skip_versioncheck;
    int          allow_bad_version;
    int          allow_unknown_version;
    char const * versioncheck_file;
    unsigned int d2cs_version;
    int          allow_d2cs_setname;
    unsigned int hashtable_size;
    char const * telnetaddrs;
    unsigned int ipban_check_int;
    unsigned int bits_ping_interval;
    unsigned int bits_ping_timeout;
    char const * version_exeinfo_match;
    unsigned int version_exeinfo_maxdiff;
    unsigned int kick_notify;
    unsigned int kick_notify_all;
#ifdef WITH_STORAGE_DB
    char const * db_host;
    unsigned int db_port;
    char const * db_name;
    char const * db_acc_table;
    char const * db_user;
    char const * db_pw;
#endif    
} bnetd_vars;


static t_conf_entry const bnetd_preftable[] =
{
    INIT_CONF_ENTRY_STR (filedir,                BNETD_FILE_DIR,       bnetd_vars),
    INIT_CONF_ENTRY_STR (userdir,                BNETD_USER_DIR,       bnetd_vars),
    INIT_CONF_ENTRY_STR (logfile,                BNETD_LOG_FILE,       bnetd_vars),
    INIT_CONF_ENTRY_STR (loglevels,              BNETD_LOG_LEVELS,     bnetd_vars),
    INIT_CONF_ENTRY_STR (defacct,                BNETD_TEMPLATE_FILE,  bnetd_vars),
    INIT_CONF_ENTRY_STR (welcomefile,            BNETD_WELCOME_FILE,   bnetd_vars),
    INIT_CONF_ENTRY_STR (motdfile,               BNETD_MOTD_FILE,      bnetd_vars),
    INIT_CONF_ENTRY_STR (newsfile,               BNETD_NEWS_DIR,       bnetd_vars),
    INIT_CONF_ENTRY_STR (channelfile,            BNETD_CHANNEL_FILE,   bnetd_vars),
    INIT_CONF_ENTRY_STR (pidfile,                BNETD_PID_FILE,       bnetd_vars),
    INIT_CONF_ENTRY_STR (adfile,                 BNETD_AD_FILE,        bnetd_vars),
    INIT_CONF_ENTRY_UINT(usersync,               BNETD_USERSYNC,       bnetd_vars),
    INIT_CONF_ENTRY_UINT(userflush,              BNETD_USERFLUSH,      bnetd_vars),
    INIT_CONF_ENTRY_STR (servername,             "",                   bnetd_vars),
    INIT_CONF_ENTRY_UINT(track,                  BNETD_TRACK_TIME,     bnetd_vars),
    INIT_CONF_ENTRY_STR (location,               "",                   bnetd_vars),
    INIT_CONF_ENTRY_STR (description,            "",                   bnetd_vars),
    INIT_CONF_ENTRY_STR (url,                    "",                   bnetd_vars),
    INIT_CONF_ENTRY_STR (contact_name,           "",                   bnetd_vars),
    INIT_CONF_ENTRY_STR (contact_email,          "",                   bnetd_vars),
    INIT_CONF_ENTRY_UINT(latency,                BNETD_LATENCY,        bnetd_vars),
    INIT_CONF_ENTRY_UINT(irc_latency,            BNETD_IRC_LATENCY,    bnetd_vars),
    INIT_CONF_ENTRY_UINT(shutdown_delay,         BNETD_SHUTDELAY,      bnetd_vars),
    INIT_CONF_ENTRY_UINT(shutdown_decr,          BNETD_SHUTDECR,       bnetd_vars),
    INIT_CONF_ENTRY_BOOL(new_accounts,           1,                    bnetd_vars),
    INIT_CONF_ENTRY_UINT(max_accounts,           0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(kick_old_login,         1,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(ask_new_channel,        1,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(hide_pass_games,        1,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(hide_started_games,     0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(hide_temp_channels,     1,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(hide_addr,              1,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(enable_conn_all,        0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(extra_commands,         0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(stats_all_ladder,       0,                    bnetd_vars),
    INIT_CONF_ENTRY_STR (reportdir,              BNETD_REPORT_DIR,     bnetd_vars),
    INIT_CONF_ENTRY_BOOL(report_all_games,       0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(report_diablo_games,    0,                    bnetd_vars),
    INIT_CONF_ENTRY_STR (iconfile,               BNETD_ICON_FILE,      bnetd_vars),
    INIT_CONF_ENTRY_STR (tosfile,                BNETD_TOS_FILE,       bnetd_vars),
    INIT_CONF_ENTRY_STR (mpqfile,                BNETD_MPQ_FILE,       bnetd_vars),
    INIT_CONF_ENTRY_STR (trackaddrs,             BNETD_TRACK_ADDRS,    bnetd_vars),
    INIT_CONF_ENTRY_STR (servaddrs,              BNETD_SERV_ADDRS,     bnetd_vars),
    INIT_CONF_ENTRY_STR (ircaddrs,               BNETD_IRC_ADDRS,      bnetd_vars),
    INIT_CONF_ENTRY_BOOL(use_keepalive,          0,                    bnetd_vars),
    INIT_CONF_ENTRY_UINT(udptest_port,           BNETD_DEF_TEST_PORT,  bnetd_vars),
    INIT_CONF_ENTRY_BOOL(do_uplink,              BITS_DO_UPLINK,       bnetd_vars),
    INIT_CONF_ENTRY_STR (uplink_server,          BITS_UPLINK_SERVER,   bnetd_vars),
    INIT_CONF_ENTRY_STR (uplink_username,        BITS_UPLINK_USERNAME, bnetd_vars),
    INIT_CONF_ENTRY_BOOL(allow_uplink,           BITS_ALLOW_UPLINK,    bnetd_vars),
    INIT_CONF_ENTRY_STR (bits_password_file,     BITS_PASSWORD_FILE,   bnetd_vars),
    INIT_CONF_ENTRY_STR (ipbanfile,              BNETD_IPBAN_FILE,     bnetd_vars),
    INIT_CONF_ENTRY_BOOL(bits_debug,             BITS_DEBUG,           bnetd_vars),
    INIT_CONF_ENTRY_BOOL(disc_is_loss,           0,                    bnetd_vars),
    INIT_CONF_ENTRY_STR (helpfile,               BNETD_HELP_FILE,      bnetd_vars),
    INIT_CONF_ENTRY_STR (fortunecmd,             BNETD_FORTUNECMD,     bnetd_vars),
    INIT_CONF_ENTRY_STR (transfile,              BNETD_TRANS_FILE,     bnetd_vars),
    INIT_CONF_ENTRY_BOOL(chanlog,                BNETD_CHANLOG,        bnetd_vars),
    INIT_CONF_ENTRY_STR (chanlogdir,             BNETD_CHANLOG_DIR,    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(quota,                  0,                    bnetd_vars),
    INIT_CONF_ENTRY_UINT(quota_lines,            BNETD_QUOTA_LINES,    bnetd_vars),
    INIT_CONF_ENTRY_UINT(quota_time,             BNETD_QUOTA_TIME,     bnetd_vars),
    INIT_CONF_ENTRY_UINT(quota_wrapline,         BNETD_QUOTA_WLINE,    bnetd_vars),
    INIT_CONF_ENTRY_UINT(quota_maxline,          BNETD_QUOTA_MLINE,    bnetd_vars),
    INIT_CONF_ENTRY_UINT(ladder_init_rating,     BNETD_LADDER_INIT_RAT,bnetd_vars),
    INIT_CONF_ENTRY_UINT(quota_dobae,            BNETD_QUOTA_DOBAE,    bnetd_vars),
    INIT_CONF_ENTRY_STR (realmfile,              BNETD_REALM_FILE,     bnetd_vars),
    INIT_CONF_ENTRY_STR (issuefile,              BNETD_ISSUE_FILE,     bnetd_vars),
    INIT_CONF_ENTRY_STR (bits_motd_file,         BITS_MOTD_FILE,       bnetd_vars),
    INIT_CONF_ENTRY_STR (effective_user,         "",                   bnetd_vars),
    INIT_CONF_ENTRY_STR (effective_group,        "",                   bnetd_vars),
    INIT_CONF_ENTRY_UINT(nullmsg,                BNETD_DEF_NULLMSG,    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(mail_support,           BNETD_MAIL_SUPPORT,   bnetd_vars),
    INIT_CONF_ENTRY_UINT(mail_quota,             BNETD_MAIL_QUOTA,     bnetd_vars),
    INIT_CONF_ENTRY_STR (maildir,                BNETD_MAIL_DIR,       bnetd_vars),
    INIT_CONF_ENTRY_STR (log_notice,             BNETD_LOG_NOTICE,     bnetd_vars),
#ifndef ACCT_DYN_LOAD
    INIT_CONF_ENTRY_BOOL(savebyname,             1,                    bnetd_vars),
#endif
    INIT_CONF_ENTRY_BOOL(skip_versioncheck,      0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(allow_bad_version,      0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(allow_unknown_version,  0,                    bnetd_vars),
    INIT_CONF_ENTRY_STR (versioncheck_file,      BNETD_VERSIONCHECK,   bnetd_vars),
    INIT_CONF_ENTRY_UINT(d2cs_version,           0,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(allow_d2cs_setname,     1,                    bnetd_vars),
    INIT_CONF_ENTRY_UINT(hashtable_size,         BNETD_HASHTABLE_SIZE, bnetd_vars),
    INIT_CONF_ENTRY_STR (telnetaddrs,            BNETD_TELNET_ADDRS,   bnetd_vars),
    INIT_CONF_ENTRY_UINT(ipban_check_int,        30,                   bnetd_vars),
    INIT_CONF_ENTRY_UINT(bits_ping_interval,     BITS_PING_INTERVAL,   bnetd_vars),
    INIT_CONF_ENTRY_UINT(bits_ping_timeout,      BITS_PING_TIMEOUT,    bnetd_vars),
    INIT_CONF_ENTRY_STR (version_exeinfo_match,  BNETD_EXEINFO_MATCH,  bnetd_vars),
    INIT_CONF_ENTRY_UINT(version_exeinfo_maxdiff,BNETD_VERSION_TIMEDIV,bnetd_vars),
    INIT_CONF_ENTRY_BOOL(kick_notify,            1,                    bnetd_vars),
    INIT_CONF_ENTRY_BOOL(kick_notify_all,        0,                    bnetd_vars),
#ifdef WITH_STORAGE_DB
    INIT_CONF_ENTRY_STR (db_host,                LOCAL_HOST,           bnetd_vars),
    INIT_CONF_ENTRY_UINT(db_port,                MYSQL_PORT,           bnetd_vars),
    INIT_CONF_ENTRY_STR (db_name,                DB_NAME,              bnetd_vars),
    INIT_CONF_ENTRY_STR (db_acc_table,           DB_ACC_TABLE,         bnetd_vars),
    INIT_CONF_ENTRY_STR (db_user,                "",                   bnetd_vars),
    INIT_CONF_ENTRY_STR (db_pw,                  "",                   bnetd_vars),
#endif    
    END_CONF_ENTRY()
};


extern int prefs_load(char const * filename)
{
    if (conftable_load_defaults(bnetd_preftable)<0)
		return -1;
    return conftable_load_file(bnetd_preftable,filename);
}


extern void prefs_unload(void)
{
    conftable_unload(bnetd_preftable);
}


/****/


extern char const * prefs_get_userdir(void)
{
    return bnetd_vars.userdir;
}


extern char const * prefs_get_filedir(void)
{
    return bnetd_vars.filedir;
}


extern char const * prefs_get_logfile(void)
{
    return bnetd_vars.logfile;
}


extern char const * prefs_get_loglevels(void)
{
    return bnetd_vars.loglevels;
}


extern char const * prefs_get_defacct(void)
{
    return bnetd_vars.defacct;
}


extern char const * prefs_get_welcomefile(void)
{
    return bnetd_vars.welcomefile;
}


extern char const * prefs_get_motdfile(void)
{
    return bnetd_vars.motdfile;
}


extern char const * prefs_get_newsfile(void)
{
    return bnetd_vars.newsfile;
}


extern char const * prefs_get_adfile(void)
{
    return bnetd_vars.adfile;
}


extern unsigned int prefs_get_user_sync_timer(void)
{
    return bnetd_vars.usersync;
}


extern unsigned int prefs_get_user_flush_timer(void)
{
    return bnetd_vars.userflush;
}

extern char const * prefs_get_servername(void)
{
    return bnetd_vars.servername;
}

extern unsigned int prefs_get_track(void)
{
    unsigned int rez;
    
    rez = bnetd_vars.track;
    if (rez>0 && rez<60) rez = 60;
    return rez;
}


extern char const * prefs_get_location(void)
{
    return bnetd_vars.location;
}


extern char const * prefs_get_description(void)
{
    return bnetd_vars.description;
}


extern char const * prefs_get_url(void)
{
    return bnetd_vars.url;
}


extern char const * prefs_get_contact_name(void)
{
    return bnetd_vars.contact_name;
}


extern char const * prefs_get_contact_email(void)
{
    return bnetd_vars.contact_email;
}


extern unsigned int prefs_get_latency(void)
{
    return bnetd_vars.latency;
}


extern unsigned int prefs_get_irc_latency(void)
{
    return bnetd_vars.irc_latency;
}


extern unsigned int prefs_get_shutdown_delay(void)
{
    return bnetd_vars.shutdown_delay;
}


extern unsigned int prefs_get_shutdown_decr(void)
{
    return bnetd_vars.shutdown_decr;
}


extern unsigned int prefs_get_allow_new_accounts(void)
{
    return bnetd_vars.new_accounts;
}


extern unsigned int prefs_get_max_accounts(void)
{
    return bnetd_vars.max_accounts;
}


extern unsigned int prefs_get_kick_old_login(void)
{
    return bnetd_vars.kick_old_login;
}


extern char const * prefs_get_channelfile(void)
{
    return bnetd_vars.channelfile;
}


extern unsigned int prefs_get_ask_new_channel(void)
{
    return bnetd_vars.ask_new_channel;
}


extern unsigned int prefs_get_hide_pass_games(void)
{
    return bnetd_vars.hide_pass_games;
}


extern unsigned int prefs_get_hide_started_games(void)
{
    return bnetd_vars.hide_started_games;
}


extern unsigned int prefs_get_hide_temp_channels(void)
{
    return bnetd_vars.hide_temp_channels;
}


extern unsigned int prefs_get_hide_addr(void)
{
    return bnetd_vars.hide_addr;
}


extern unsigned int prefs_get_enable_conn_all(void)
{
    return bnetd_vars.enable_conn_all;
}


extern unsigned int prefs_get_extra_commands(void)
{
    return bnetd_vars.extra_commands;
}


extern unsigned int prefs_get_stats_all_ladder(void)
{
#ifdef WITH_GAMEI
    return 1;
#else
    return bnetd_vars.stats_all_ladder;
#endif
}


extern char const * prefs_get_reportdir(void)
{
    return bnetd_vars.reportdir;
}


extern unsigned int prefs_get_report_all_games(void)
{
    return bnetd_vars.report_all_games;
}

extern unsigned int prefs_get_report_diablo_games(void)
{
    return bnetd_vars.report_diablo_games;
}

extern char const * prefs_get_pidfile(void)
{
    return bnetd_vars.pidfile;
}


extern char const * prefs_get_iconfile(void)
{
    return bnetd_vars.iconfile;
}


extern char const * prefs_get_tosfile(void)
{
    return bnetd_vars.tosfile;
}


extern char const * prefs_get_mpqfile(void)
{
    return bnetd_vars.mpqfile;
}


extern char const * prefs_get_trackserv_addrs(void)
{
    return bnetd_vars.trackaddrs;
}


extern char const * prefs_get_bnetdserv_addrs(void)
{
    return bnetd_vars.servaddrs;
}


extern char const * prefs_get_irc_addrs(void)
{
    return bnetd_vars.ircaddrs;
}


extern unsigned int prefs_get_use_keepalive(void)
{
    return bnetd_vars.use_keepalive;
}


extern unsigned int prefs_get_udptest_port(void)
{
    return bnetd_vars.udptest_port;
}


extern unsigned int prefs_get_do_uplink(void)
{
    return bnetd_vars.do_uplink;
}


extern char const * prefs_get_uplink_server(void)
{
    return bnetd_vars.uplink_server;
}


extern unsigned int prefs_get_allow_uplink(void)
{
    return bnetd_vars.allow_uplink;
}


extern char const * prefs_get_bits_password_file(void)
{
    return bnetd_vars.bits_password_file;
}


extern char const * prefs_get_uplink_username(void)
{
    return bnetd_vars.uplink_username;
}


extern char const * prefs_get_ipbanfile(void)
{
    return bnetd_vars.ipbanfile;
}


extern unsigned int prefs_get_bits_debug(void)
{
    return bnetd_vars.bits_debug;
}


extern unsigned int prefs_get_discisloss(void)
{
    return bnetd_vars.disc_is_loss;
}


extern char const * prefs_get_helpfile(void)
{
    return bnetd_vars.helpfile;
}


extern char const * prefs_get_fortunecmd(void)
{
    return bnetd_vars.fortunecmd;
}


extern char const * prefs_get_transfile(void)
{
    return bnetd_vars.transfile;
}


extern unsigned int prefs_get_chanlog(void)
{
    return bnetd_vars.chanlog;
}


extern char const * prefs_get_chanlogdir(void)
{
    return bnetd_vars.chanlogdir;
}


extern unsigned int prefs_get_quota(void)
{
    return bnetd_vars.quota;
}


extern unsigned int prefs_get_quota_lines(void)
{
    unsigned int rez;
    
    rez=bnetd_vars.quota_lines;
    if (rez<1) rez = 1;
    if (rez>100) rez = 100;
    return rez;
}


extern unsigned int prefs_get_quota_time(void)
{
    unsigned int rez;
    
    rez=bnetd_vars.quota_time;
    if (rez<1) rez = 1;
    if (rez>10) rez = 60;
    return rez;
}


extern unsigned int prefs_get_quota_wrapline(void)
{
    unsigned int rez;
    
    rez=bnetd_vars.quota_wrapline;
    if (rez<1) rez = 1;
    if (rez>256) rez = 256;
    return rez;
}


extern unsigned int prefs_get_quota_maxline(void)
{
    unsigned int rez;
    
    rez=bnetd_vars.quota_maxline;
    if (rez<1) rez = 1;
    if (rez>256) rez = 256;
    return rez;
}


extern unsigned int prefs_get_ladder_init_rating(void)
{
    return bnetd_vars.ladder_init_rating;
}


extern unsigned int prefs_get_quota_dobae(void)
{
    unsigned int rez;

    rez=bnetd_vars.quota_dobae;
    if (rez<1) rez = 1;
    if (rez>100) rez = 100;
    return rez;
}


extern char const * prefs_get_realmfile(void)
{
    return bnetd_vars.realmfile;
}


extern char const * prefs_get_issuefile(void)
{
    return bnetd_vars.issuefile;
}


extern char const * prefs_get_bits_motd_file(void)
{
    return bnetd_vars.bits_motd_file;
}


extern char const * prefs_get_effective_user(void)
{
    return bnetd_vars.effective_user;
}


extern char const * prefs_get_effective_group(void)
{
    return bnetd_vars.effective_group;
}


extern unsigned int prefs_get_nullmsg(void)
{
    return bnetd_vars.nullmsg;
}


extern unsigned int prefs_get_mail_support(void)
{
    return bnetd_vars.mail_support;
}


extern unsigned int prefs_get_mail_quota(void)
{
    unsigned int rez;
    
    rez=bnetd_vars.mail_quota;
    if (rez<1) rez = 1;
    if (rez>30) rez = 30;
    return rez;
}


extern char const * prefs_get_maildir(void)
{
    return bnetd_vars.maildir;
}


extern char const * prefs_get_log_notice(void)
{
    return bnetd_vars.log_notice;
}


#ifndef ACCT_DYN_LOAD
extern unsigned int prefs_get_savebyname(void)
{
    return bnetd_vars.savebyname;
}
#endif


extern unsigned int prefs_get_skip_versioncheck(void)
{
    return bnetd_vars.skip_versioncheck;
}


extern unsigned int prefs_get_allow_bad_version(void)
{
    return bnetd_vars.allow_bad_version;
}


extern unsigned int prefs_get_allow_unknown_version(void)
{
    return bnetd_vars.allow_unknown_version;
}


extern char const * prefs_get_versioncheck_file(void)
{
    return bnetd_vars.versioncheck_file;
}


extern unsigned int prefs_allow_d2cs_setname(void)
{
    return bnetd_vars.allow_d2cs_setname;
}


extern unsigned int prefs_get_d2cs_version(void)
{
    return bnetd_vars.d2cs_version;
}


extern unsigned int prefs_get_hashtable_size(void)
{
    return bnetd_vars.hashtable_size;
}


extern unsigned int prefs_get_bits_ping_interval(void)
{
    return bnetd_vars.bits_ping_interval;
}

extern unsigned int prefs_get_bits_ping_timeout(void)
{
    return bnetd_vars.bits_ping_timeout;
}


extern char const * prefs_get_telnet_addrs(void)
{
    return bnetd_vars.telnetaddrs;
}


extern unsigned int prefs_get_ipban_check_int(void)
{
    return bnetd_vars.ipban_check_int;
}


extern char const * prefs_get_version_exeinfo_match(void)
{
    return bnetd_vars.version_exeinfo_match;
}


extern unsigned int prefs_get_version_exeinfo_maxdiff(void)
{
    return bnetd_vars.version_exeinfo_maxdiff;
}


extern unsigned int prefs_get_kick_notify(void)
{
    return bnetd_vars.kick_notify;
}


extern unsigned int prefs_get_kick_notify_all(void)
{
    return bnetd_vars.kick_notify_all;
}

#ifdef WITH_STORAGE_DB
extern char const * prefs_get_db_host(void)
{
    return bnetd_vars.db_host;
}

extern unsigned int prefs_get_db_port(void)
{
    return bnetd_vars.db_port;
}

extern char const * prefs_get_db_name(void)
{
    return bnetd_vars.db_name;
}

extern char const * prefs_get_db_acc_table(void)
{
    return bnetd_vars.db_acc_table;
}

extern char const * prefs_get_db_user(void)
{
    return bnetd_vars.db_user;
}

extern char const * prefs_get_db_pw(void)
{
    return bnetd_vars.db_pw;
}
#endif
