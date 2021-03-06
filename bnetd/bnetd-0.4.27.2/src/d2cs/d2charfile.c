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
#include <stdio.h>
#include <ctype.h>
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
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#include "compat/memset.h"
#include "compat/memcpy.h"
#include <errno.h>
#include "compat/strerror.h"
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef TIME_WITH_SYS_TIME
# include <time.h>
# include <sys/time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include "d2charfile.h"
#include "prefs.h"
#include "xstring.h"
#include "common/bn_type.h"
#include "common/eventlog.h"
#include "common/util.h"
#ifdef WITH_STORAGE_DB
# ifdef WITH_MYSQL
#  include "d2char_db_mysql.h"
# endif
#endif
#include "common/setup_after.h"

static int d2charsave_init(void * buffer,char const * charname,unsigned char class,unsigned short status);
static int d2charinfo_init(t_d2charinfo_file * chardata, char const * account, char const * charname,
				unsigned char class, unsigned short status);
static int d2charsave_checksum(unsigned char const * data, unsigned int len, unsigned int offset);


static int d2charsave_init(void * buffer,char const * charname,unsigned char class,unsigned short status)
{
	ASSERT(buffer,-1);
	ASSERT(charname,-1);
	bn_byte_set((bn_byte *)((char *)buffer+D2CHARSAVE_CLASS_OFFSET), class);
	bn_short_set((bn_short *)((char *)buffer+D2CHARSAVE_STATUS_OFFSET),status);
	strncpy((char *)buffer+D2CHARSAVE_CHARNAME_OFFSET,charname,MAX_CHARNAME_LEN);
	return 0;
}


static int d2charinfo_init(t_d2charinfo_file * chardata, char const * account, char const * charname,
			   unsigned char class, unsigned short status)
{
	int		i;
	time_t		now;

	now=time(NULL);
	bn_int_set(&chardata->header.magicword,D2CHARINFO_MAGICWORD);
	bn_int_set(&chardata->header.version,D2CHARINFO_VERSION);
	bn_int_set(&chardata->header.create_time,now);
	bn_int_set(&chardata->header.last_time,now);

	strncpy((char *)chardata->header.charname, charname, MAX_CHARNAME_LEN);
	chardata->header.charname[MAX_CHARNAME_LEN-1]='\0';
	strncpy((char *)chardata->header.account, account, MAX_ACCTNAME_LEN);
	chardata->header.account[MAX_ACCTNAME_LEN-1]='\0';
	strncpy((char *)chardata->header.realmname, prefs_get_realmname(),
		MAX_REALMNAME_LEN);
	chardata->header.realmname[MAX_REALMNAME_LEN-1]='\0';
	bn_int_set(&chardata->header.checksum,0);
	for (i=0; i<NELEMS(chardata->header.reserved); i++) {
		bn_int_set(&chardata->header.reserved[i],0);
	}
	bn_int_set(&chardata->summary.charlevel,1);
	bn_int_set(&chardata->summary.experience,0);
	bn_int_set(&chardata->summary.charclass,class);
	bn_int_set(&chardata->summary.charstatus,status);

	memset(chardata->portrait.gfx,D2CHARINFO_PORTRAIT_PADBYTE,sizeof(chardata->portrait.gfx));
	memset(chardata->portrait.color,D2CHARINFO_PORTRAIT_PADBYTE,sizeof(chardata->portrait.color));
	memset(chardata->portrait.u2,D2CHARINFO_PORTRAIT_PADBYTE,sizeof(chardata->portrait.u1));
	memset(chardata->portrait.u1,D2CHARINFO_PORTRAIT_MASK,sizeof(chardata->portrait.u2));
	memset(chardata->pad,0,sizeof(chardata->pad));

	bn_short_set(&chardata->portrait.header,D2CHARINFO_PORTRAIT_HEADER);
	bn_byte_set(&chardata->portrait.status,status|D2CHARINFO_PORTRAIT_MASK);
	bn_byte_set(&chardata->portrait.class,class+1);
	bn_byte_set(&chardata->portrait.level,1);
	bn_byte_set(&chardata->portrait.end,'\0');
	return 0;
}


extern int d2char_create(char const * account, char const * charname, unsigned char class, unsigned short status)
{
	t_d2charinfo_file	chardata;
	char				buffer[1024];
	unsigned int		size;
#ifdef WITH_STORAGE_DB
	t_d2charinfo_file	dummy_chardata;
#else
	char				* savefile, * infofile;
	FILE				* fp;
#endif

	ASSERT(account,-1);
	ASSERT(charname,-1);
	if (class>D2CHAR_MAX_CLASS) class=0;
	status &= 0xFF;
	charstatus_set_init(status,1);
	if (!prefs_allow_newchar()) {
		eventlog(eventlog_level_warn, __FUNCTION__, "creation of new character is disabled");
		return -1;
	}
	if (d2char_check_charname(charname)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"",charname);
		return -1;
	}
	if (d2char_check_acctname(account)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"",account);
		return -1;
	}
	size=sizeof(buffer);
	if (file_read(prefs_get_charsave_newbie(), buffer, &size)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "error loading newbie save file");
		return -1;
	}
	if (size>=sizeof(buffer)) {
		eventlog(eventlog_level_error, __FUNCTION__, "newbie save file \"%s\" is corrupt (length %lu, expected <%lu)",prefs_get_charsave_newbie(),(unsigned long)size,(unsigned long)sizeof(buffer));
		return -1;
	}

	/* Check if charname already exist */
#ifdef WITH_STORAGE_DB
	if(db_d2char_loadinfo(prefs_get_db_char_table(),account,charname,prefs_get_realmname(),&dummy_chardata, sizeof(dummy_chardata))) {
		eventlog(eventlog_level_warn, __FUNCTION__, "character \"%s@%s\" already exist",charname,prefs_get_realmname());
		return -1;
	}
#else
	if (!(savefile=malloc(strlen(prefs_get_charsave_dir())+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for file");
		return -1;
	}
	d2char_get_savefile_name(savefile,charname);
	if ((fp=fopen(savefile,"rb"))) {
		eventlog(eventlog_level_warn, __FUNCTION__, "character save file \"%s\" for \"%s\" already exist",savefile,charname);
		fclose(fp);
		free(savefile);
		return -1;
	}

	if (!(infofile=malloc(strlen(prefs_get_charinfo_dir())+1+strlen(account)+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charinfo file");
		free(savefile);
		return -1;
	}
	d2char_get_infofile_name(infofile,account,charname);
#endif

	d2charsave_init(buffer,charname,class,status);
	d2charinfo_init(&chardata,account,charname,class,status);

#ifdef WITH_STORAGE_DB
	if (!db_d2char_create(prefs_get_db_char_table(),account,charname,&chardata,buffer,size))
        return -1;
#else
	if (file_write(infofile,&chardata,sizeof(chardata))<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "error writing info file \"%s\"",infofile);
		unlink(infofile);
		free(infofile);
		free(savefile);
		return -1;
	}

	if (file_write(savefile,buffer,size)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "error writing save file \"%s\"",savefile);
		unlink(infofile);
		unlink(savefile);
		free(savefile);
		free(infofile);
		return -1;
	}
	free(savefile);
	free(infofile);
#endif
	eventlog(eventlog_level_info, __FUNCTION__, "character %s(*%s) class %d status 0x%X created",charname,account,class,status);
	return 0;
}


/* JEBs20021020 Haven't found the use of this function, so i marked it as comment.
	Is it needed anymore ?!
extern int d2char_find(char const * account, char const * charname)
{
	char		* file;
	FILE		* fp;

#ifdef WITH_MYSQL
eventlog(eventlog_level_error, __FUNCTION__, "!!! @JEBS:FORGOTTEN FUNCTION !!! account/charname \"%s/%s\"",account,charname);
#endif

	ASSERT(account,-1);
	ASSERT(charname,-1);
	if (!(file=malloc(strlen(prefs_get_charinfo_dir())+1+strlen(account)+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charinfo file");
		return -1;
	}
	d2char_get_infofile_name(file,account,charname);
	fp=fopen(file,"rb");
	free(file);
	if (fp) {
		fclose(fp);
		return 0;
	}
	return -1;
}
*/

extern int d2char_convert(char const * account, char const * charname)
{
#ifndef WITH_STORAGE_DB
	FILE			* fp;
	char			* file;
#endif
	unsigned char		buffer[MAX_SAVEFILE_SIZE];
	unsigned int		status_offset;
	unsigned char		status;
	unsigned int		charstatus;
	t_d2charinfo_file	charinfo;
	unsigned int		size;
	unsigned int		version;
	unsigned int		checksum;

	ASSERT(account,-1);
	ASSERT(charname,-1);
	if (d2char_check_charname(charname)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"",charname);
		return -1;
	}
	if (d2char_check_acctname(account)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"",account);
		return -1;
	}
#ifdef WITH_STORAGE_DB
	if(!db_d2char_loadinfo(prefs_get_db_char_table(),account,charname,prefs_get_realmname(),&charinfo,sizeof(charinfo)))
		return -1;
	if(!(size=db_d2char_loaddata(prefs_get_db_char_table(),account,charname,prefs_get_realmname(),buffer,MAX_SAVEFILE_SIZE)))
		return -1;
#else
	if (!(file=malloc(strlen(prefs_get_charinfo_dir())+1+strlen(account)+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charinfo file");
		return -1;
	}
	d2char_get_infofile_name(file,account,charname);
	if (!(fp=fopen(file,"rb+"))) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to open charinfo file \"%s\" for reading and writing (fopen: %s)",file,strerror(errno));
		free(file);
		return -1;
	}
	free(file);
	if (fread(&charinfo,1,sizeof(charinfo),fp)!=sizeof(charinfo)) {
		eventlog(eventlog_level_error, __FUNCTION__, "error reading charinfo file for character \"%s\" (fread: %s)",charname,strerror(errno));
		fclose(fp);
		return -1;
	}
#endif
	charstatus=bn_int_get(charinfo.summary.charstatus);
	charstatus_set_expansion(charstatus,1);
	bn_int_set(&charinfo.summary.charstatus,charstatus);

	status=bn_byte_get(charinfo.portrait.status);
	charstatus_set_expansion(status,1);
	bn_byte_set(&charinfo.portrait.status,status);

#ifndef WITH_STORAGE_DB
	fseek(fp,0,SEEK_SET); /* FIXME: check return */
	if (fwrite(&charinfo,1,sizeof(charinfo),fp)!=sizeof(charinfo)) {
		eventlog(eventlog_level_error, __FUNCTION__, "error writing charinfo file for character \"%s\" (fwrite: %s)",charname,strerror(errno));
		fclose(fp);
		return -1;
	}
	if (fclose(fp)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not close charinfo file for character \"%s\" after writing (fclose: %s)",charname,strerror(errno));
		return -1;
	}

	if (!(file=malloc(strlen(prefs_get_charsave_dir())+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charsave file");
		return -1;
	}
	d2char_get_savefile_name(file,charname);
	if (!(fp=fopen(file,"rb+"))) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not open charsave file \"%s\" for reading and writing (fopen: %s)",file,strerror(errno));
		free(file);
		return -1;
	}
	free(file);
	size=fread(buffer,1,sizeof(buffer),fp);
	if (!feof(fp)) {
		eventlog(eventlog_level_error, __FUNCTION__, "error reading charsave file for character \"%s\" (fread: %s)",charname,strerror(errno));
		fclose(fp);
		return -1;
	}
#endif
	version=bn_int_get(buffer+D2CHARSAVE_VERSION_OFFSET);
	if (version>=0x0000005C) {
		status_offset=D2CHARSAVE_STATUS_OFFSET_109;
	} else {
		status_offset=D2CHARSAVE_STATUS_OFFSET;
	}
	status=bn_byte_get(buffer+status_offset);
	charstatus_set_expansion(status,1);
	bn_byte_set((bn_byte *)(buffer+status_offset),status); /* FIXME: shouldn't abuse bn_*_set()... what's the best way to do this? */
	if (version>=0x0000005C) {
		checksum=d2charsave_checksum(buffer,size,D2CHARSAVE_CHECKSUM_OFFSET);
		bn_int_set((bn_int *)(buffer+D2CHARSAVE_CHECKSUM_OFFSET),checksum); /* FIXME: shouldn't abuse bn_*_set()... what's the best way to do this? */
	}
#ifdef WITH_STORAGE_DB
    if (!db_d2char_saveinfo(prefs_get_db_char_table(),account,charname,charinfo.header.realmname,&charinfo,sizeof(charinfo))) {
        eventlog(eventlog_level_error, __FUNCTION__, "error writing character info record \"%s(*%s)@%s\"",charname,account,charinfo.header.realmname);
        return -1;
    }
    if (!db_d2char_savedata(prefs_get_db_char_table(),account,charname,charinfo.header.realmname,buffer,size)) {
        eventlog(eventlog_level_error, __FUNCTION__, "error writing character data record \"%s(*%s)@%s\"",charname,account,charinfo.header.realmname);
        return -1;
    }
#else
	fseek(fp,0,SEEK_SET); /* FIXME: check return */
	if (fwrite(buffer,1,size,fp)!=size) {
		eventlog(eventlog_level_error, __FUNCTION__, "error writing charsave file for character %s (fwrite: %s)",charname,strerror(errno));
		fclose(fp);
		return -1;
	}
	if (fclose(fp)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not close charsave file for character \"%s\" after writing (fclose: %s)",charname,strerror(errno));
		return -1;
	}
#endif
	eventlog(eventlog_level_info, __FUNCTION__, "character %s(*%s) converted to expansion",charname,account);
	return 0;
}


extern int d2char_delete(char const * account, char const * charname)
{
#ifndef WITH_MYSQL
	char		* file;
#endif

	ASSERT(account,-1);
	ASSERT(charname,-1);
	if (d2char_check_charname(charname)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"",charname);
		return -1;
	}
	if (d2char_check_acctname(account)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"",account);
		return -1;
	}
#ifdef WITH_MYSQL
	if (!db_d2char_delete(prefs_get_db_char_table(),account,charname,prefs_get_realmname()))
		return -1;
#else
	if (!(file=malloc(strlen(prefs_get_charinfo_dir())+1+strlen(account)+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charinfo file");
		return -1;
	}
	d2char_get_infofile_name(file,account,charname);
	if (unlink(file)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "failed to unlink charinfo file \"%s\" (unlink: %s)",file,strerror(errno));
		free(file);
		return -1;
	}
	free(file);

	if (!(file=malloc(strlen(prefs_get_charsave_dir())+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for charsave file");
		return -1;
	}
	d2char_get_savefile_name(file,charname);
	if (unlink(file)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "failed to unlink charsave file \"%s\" (unlink: %s)",file,strerror(errno));
	}
	free(file);
#endif
	eventlog(eventlog_level_info, __FUNCTION__, "character %s(*%s) deleted",charname,account);
	return 0;
}


extern int d2char_get_summary(char const * account, char const * charname,t_d2charinfo_summary * charinfo)
{
	t_d2charinfo_file	data;

	ASSERT(account,-1);
	ASSERT(charname,-1);
	ASSERT(charinfo,-1);
	if (d2charinfo_load(account, charname, &data)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "error loading character %s(*%s)",charname,account);
		return -1;
	}
	memcpy(charinfo,&data.summary,sizeof(data.summary));
	eventlog(eventlog_level_info, __FUNCTION__, "character %s difficulty %d expansion %d hardcore %d dead %d loaded",charname,
		d2charinfo_get_difficulty(charinfo), d2charinfo_get_expansion(charinfo),
		d2charinfo_get_hardcore(charinfo),d2charinfo_get_dead(charinfo));
	return 0;
}


extern int d2charinfo_load(char const * account, char const * charname, t_d2charinfo_file * data)
{
#ifndef WITH_STORAGE_DB
	char			* file;
#endif
	unsigned int		size;

	if (d2char_check_charname(charname)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"",charname);
		return -1;
	}
	if (d2char_check_acctname(account)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"",account);
		return -1;
	}
	size=sizeof(t_d2charinfo_file);
#ifdef WITH_STORAGE_DB
	if(!db_d2char_loadinfo(prefs_get_db_char_table(),account,charname,prefs_get_realmname(),data,sizeof(*data)))
		return -1;
#else
	if (!(file=malloc(strlen(prefs_get_charinfo_dir())+1+strlen(account)+1+strlen(charname)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for file");
		return -1;
	}
	d2char_get_infofile_name(file,account,charname);
	if (file_read(file,data,&size)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "error loading character file %s",file);
		free(file);
		return -1;
	}
	free(file);
#endif
	if (size!=sizeof(t_d2charinfo_file)) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad charinfo file %s (length %d)",charname,size);
		return -1;
	}
	d2char_portrait_init(&data->portrait);
	return d2charinfo_check(data);
}


extern int d2charinfo_check(t_d2charinfo_file * data)
{
	ASSERT(data,-1);
	if (bn_int_get(data->header.magicword) != D2CHARINFO_MAGICWORD) {
		eventlog(eventlog_level_error, __FUNCTION__, "info data check failed (header 0x%08X)",bn_int_get(data->header.magicword));
		return -1;
	}
	if (bn_int_get(data->header.version) != D2CHARINFO_VERSION) {
		eventlog(eventlog_level_error, __FUNCTION__, "info data check failed (version 0x%08X)",bn_int_get(data->header.version));
		return -1;
	}
	return 0;
}


extern int d2char_portrait_init(t_d2charinfo_portrait * portrait)
{
	int		i;
	unsigned char	* p;

	p=(unsigned char *)portrait;
	for (i=0; i<sizeof(t_d2charinfo_portrait); i++) {
		if (!p[i]) p[i]=D2CHARINFO_PORTRAIT_PADBYTE;
	}
	p[i-1]='\0';
	return 0;
}


extern int d2char_get_portrait(char const * account,char const * charname, t_d2charinfo_portrait * portrait)
{
	t_d2charinfo_file	data;

	ASSERT(charname,-1);
	ASSERT(account,-1);
	ASSERT(portrait,-1);
	if (d2charinfo_load(account, charname, &data)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "error loading character %s(*%s)",charname,account);
		return -1;
	}
	strcpy((char *)portrait,(char *)&data.portrait);
	return 0;
}


extern int d2char_check_charname(char const * name)
{
	unsigned int	i;
	unsigned char	ch;

	if (!name) return -1;
	if (!isalpha(name[0])) return -1;

	for (i=1; i<=MAX_CHARNAME_LEN; i++) {
		ch=name[i];
		if (ch=='\0') break;
		if (isalpha(ch)) continue;
		if (ch=='-') continue;
		if (ch=='_') continue;
		if (ch=='.') continue;
		return -1;
	}
	if (i >= MIN_NAME_LEN || i<= MAX_CHARNAME_LEN) return 0;
	return -1;
}


extern int d2char_check_acctname(char const * name)
{
	unsigned int	i;
	unsigned char	ch;

	if (!name) return -1;
	if (!isalnum(name[0])) return -1;

	for (i=1; i<=MAX_CHARNAME_LEN; i++) {
		ch=name[i];
		if (ch=='\0') break;
		if (isalnum(ch)) continue;
		if (ch=='-') continue;
		if (ch=='_') continue;
		if (ch=='.') continue;
		return -1;
	}
	if (i >= MIN_NAME_LEN || i<= MAX_ACCTNAME_LEN) return 0;
	return -1;
}


#ifndef WITH_STORAGE_DB
extern int d2char_get_savefile_name(char * filename, char const * charname)
{
	char	tmpchar[MAX_CHARNAME_LEN];

	ASSERT(filename,-1);
	ASSERT(charname,-1);
	strncpy(tmpchar,charname,sizeof(tmpchar));
	tmpchar[sizeof(tmpchar)-1]='\0';
	str_to_lower(tmpchar);
	sprintf(filename,"%s/%s",prefs_get_charsave_dir(),tmpchar);
	return 0;
}


extern int d2char_get_infodir_name(char * filename, char const * account)
{
	char	tmpacct[MAX_ACCTNAME_LEN];

	ASSERT(filename,-1);
	ASSERT(account,-1);

	strncpy(tmpacct,account,sizeof(tmpacct));
	tmpacct[sizeof(tmpacct)-1]='\0';
	str_to_lower(tmpacct);
	sprintf(filename,"%s/%s",prefs_get_charinfo_dir(),tmpacct);
	return 0;
}


extern int d2char_get_infofile_name(char * filename, char const * account, char const * charname)
{
	char	tmpchar[MAX_CHARNAME_LEN];
	char	tmpacct[MAX_ACCTNAME_LEN];

	ASSERT(filename,-1);
	ASSERT(account,-1);
	ASSERT(charname,-1);
	strncpy(tmpchar,charname,sizeof(tmpchar));
	tmpchar[sizeof(tmpchar)-1]='\0';
	str_to_lower(tmpchar);

	strncpy(tmpacct,account,sizeof(tmpacct));
	tmpchar[sizeof(tmpacct)-1]='\0';
	str_to_lower(tmpacct);
	sprintf(filename,"%s/%s/%s",prefs_get_charinfo_dir(),tmpacct,tmpchar);
	return 0;
}
#endif

extern unsigned int d2charinfo_get_expansion(t_d2charinfo_summary const * charinfo)
{
	ASSERT(charinfo,0);
	return charstatus_get_expansion(bn_int_get(charinfo->charstatus));
}


extern unsigned int d2charinfo_get_level(t_d2charinfo_summary const * charinfo)
{
	ASSERT(charinfo,0);
	return bn_int_get(charinfo->charlevel);
}


extern unsigned int d2charinfo_get_class(t_d2charinfo_summary const * charinfo)
{
	ASSERT(charinfo,0);
	return bn_int_get(charinfo->charclass);
}


extern unsigned int d2charinfo_get_hardcore(t_d2charinfo_summary const * charinfo)
{
	ASSERT(charinfo,0);
	return charstatus_get_hardcore(bn_int_get(charinfo->charstatus));
}


extern unsigned int d2charinfo_get_dead(t_d2charinfo_summary const * charinfo)
{
	ASSERT(charinfo,0);
	return charstatus_get_dead(bn_int_get(charinfo->charstatus));
}


extern unsigned int d2charinfo_get_difficulty(t_d2charinfo_summary const * charinfo)
{
	unsigned int	difficulty;

	ASSERT(charinfo,0);
	if (d2charinfo_get_expansion(charinfo)) {
		difficulty=charstatus_get_difficulty_expansion(bn_int_get(charinfo->charstatus));
	} else {
		difficulty=charstatus_get_difficulty(bn_int_get(charinfo->charstatus));
	}
	if (difficulty>2) difficulty=2;
	return difficulty;
}


static int d2charsave_checksum(unsigned char const * data, unsigned int len,unsigned int offset)
{
	int		checksum;
	unsigned int	i;
	unsigned int	ch;

	if (!data) return 0;
	checksum=0;
	for (i=0; i<len; i++) {
		ch=data[i];
		if (i>=offset && i<offset+sizeof(int)) ch=0;
		ch+=(checksum<0);
		checksum=2*checksum+ch;
	}
	return checksum;
}


/* those functions should move to util.c */
extern int file_read(char const * filename, void * data, unsigned int * size)
{
	FILE		* fp;
	int		n;

	ASSERT(filename,-1);
	ASSERT(data,-1);
	ASSERT(size,-1);
	if (!(fp=fopen(filename,"rb"))) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
		return -1;
	}

	fseek(fp,0,SEEK_END); /* FIXME: check return value */
	n=ftell(fp);
	n=min(*size,n);
	rewind(fp); /* FIXME: check return value */

	if (fread(data,1,n,fp)!=n) {
		eventlog(eventlog_level_error, __FUNCTION__, "error reading file \"%s\" (fread: %s)",filename,strerror(errno));
		fclose(fp);
		return -1;
	}
	if (fclose(fp)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not close file \"%s\" after reading (fclose: %s)",filename,strerror(errno));
		return -1;
	}
	*size=n;
	return 0;
}


extern int file_write(char const * filename, void * data, unsigned int size)
{
	FILE		* fp;

	ASSERT(filename,-1);
	ASSERT(data,-1);
	ASSERT(size,-1);
	if (!(fp=fopen(filename,"wb"))) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for writing (fopen: %s)",filename,strerror(errno));
		return -1;
	}
	if (fwrite(data,1,size,fp)!=size) {
		eventlog(eventlog_level_error, __FUNCTION__, "error writing file \"%s\" (fwrite: %s)",filename,strerror(errno));
		fclose(fp);
		return -1;
	}
	if (fclose(fp)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not close file \"%s\" after writing (fclose: %s)",filename,strerror(errno));
		return -1;
	}
	return 0;
}
