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
#ifndef INCLUDED_PREFS_H
#define INCLUDED_PREFS_H

extern int prefs_load(char const * filename);
extern int prefs_reload(char const * filename);
extern int prefs_unload(void);

extern char const * prefs_get_logfile(void);
extern char const * prefs_get_servaddrs(void);
extern char const * prefs_get_charsave_dir(void);
extern char const * prefs_get_charinfo_dir(void);
extern char const * prefs_get_charsave_newbie(void);
extern char const * prefs_get_motd(void);
extern char const * prefs_get_realmname(void);
extern char const * prefs_get_d2gs_list(void);
extern unsigned int prefs_get_maxchar(void);
extern unsigned int prefs_get_list_purgeinterval(void);
extern unsigned int prefs_get_maxgamelist(void);
extern unsigned int prefs_allow_newchar(void);
extern unsigned int prefs_get_gamequeue_checkinterval(void);
extern unsigned int prefs_get_idletime(void);
extern unsigned int prefs_get_shutdown_delay(void);
extern unsigned int prefs_get_shutdown_decr(void);
extern char const * prefs_get_bnetdaddr(void);
extern unsigned int prefs_get_s2s_retryinterval(void);
extern unsigned int prefs_get_s2s_timeout(void);
extern unsigned int prefs_get_sq_timeout(void);
extern unsigned int prefs_get_sq_checkinterval(void);
extern unsigned int prefs_get_d2gs_checksum(void);
extern unsigned int prefs_get_d2gs_version(void);
extern unsigned int prefs_get_ladderlist_count(void);
extern unsigned int prefs_get_d2ladder_refresh_interval(void);
extern unsigned int prefs_get_game_maxlifetime(void);
extern char const * prefs_get_ladder_dir(void);
extern char const * prefs_get_loglevels(void);
extern unsigned int prefs_allow_gamelist_showall(void);
extern unsigned int prefs_allow_gamelimit(void);
extern unsigned int prefs_check_multilogin(void);
extern char const * prefs_get_d2gs_password(void);
extern unsigned int prefs_get_s2s_idletime(void);
extern unsigned int prefs_get_s2s_keepalive_interval(void);
extern unsigned int prefs_get_timeout_checkinterval(void);
extern unsigned int prefs_get_max_game_idletime(void);
extern unsigned int prefs_get_d2gs_restart_delay(void);
#ifdef WITH_STORAGE_DB
    extern char const * prefs_get_db_host(void);
    extern unsigned int prefs_get_db_port(void);
    extern char const * prefs_get_db_name(void);
    extern char const * prefs_get_db_char_table(void);
    extern char const * prefs_get_db_user(void);
    extern char const * prefs_get_db_pw(void);
#endif

#endif
