/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Copyright (C) 2001		sousou	(liupeng.cs@263.net)
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
#include "compat/strcasecmp.h"
#include "compat/memset.h"
#include <errno.h>
#include "compat/strerror.h"
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif

#include "prefs.h"
#include "d2charfile.h"
#include "d2ladder.h"
#include "d2cs_protocol.h"
#include "common/tag.h"
#include "common/eventlog.h"
#include "common/setup_after.h"

static t_d2ladder	* ladder_data=NULL;
static unsigned int	max_ladder_type=0;

static int d2ladderlist_create(unsigned int maxtype);
static int d2ladder_create(unsigned int type, unsigned int len);
static int d2ladder_append_ladder(unsigned int type, t_d2ladderfile_ladderinfo * info);
static int d2ladder_readladder(void);

extern int d2ladder_init(void)
{
	if (d2ladder_readladder()<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "failed to initialize ladder data");
		return -1;
	} 
	eventlog(eventlog_level_info, __FUNCTION__, "ladder data initialized");
	return 0;
}

static int d2ladder_readladder(void)
{
	FILE				* fp;
	t_d2ladderfile_ladderindex	* ladderheader;
	t_d2ladderfile_ladderinfo	* ladderinfo;
	char				* ladderfile;
	t_d2ladderfile_header		header;
	unsigned int			i, n, temp, count, type, number;

	if (!(ladderfile=malloc(strlen(prefs_get_ladder_dir())+1+strlen(LADDER_FILE_PREFIX)+1+
			strlen(CLIENTTAG_DIABLO2DV)+1))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for ladderfile");
		return -1;
	}
	sprintf(ladderfile,"%s/%s.%s",prefs_get_ladder_dir(),LADDER_FILE_PREFIX,CLIENTTAG_DIABLO2DV);
	if (!(fp=fopen(ladderfile,"rb"))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error opening ladder file \"%s\" for reading (fopen: %s)",ladderfile,strerror(errno));
		free(ladderfile);
		return -1;
	}
	free(ladderfile);
	if (fread(&header,1,sizeof(header),fp)!=sizeof(header)) {
		eventlog(eventlog_level_error, __FUNCTION__, "error reading ladder file");
		fclose(fp);
		return -1;
	}
	max_ladder_type= bn_int_get(header.maxtype);
	if (d2ladderlist_create(max_ladder_type)<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "error create ladder list");
		fclose(fp);
		return -1;
	}
	temp= max_ladder_type * sizeof(*ladderheader);
	if (!(ladderheader=malloc(temp))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate ladderheader");
		fclose(fp);
		return -1;
	}
	if (fread(ladderheader,1,temp,fp)!=temp) {
		eventlog(eventlog_level_error, __FUNCTION__, "error read ladder file");
		free(ladderheader);
		fclose(fp);
		return -1;
	}
	for (i=0, count=0; i< max_ladder_type ; i++) {
		type=bn_int_get(ladderheader[i].type);
		number=bn_int_get(ladderheader[i].number);
		if (d2ladder_create(type,number)<0) {
			eventlog(eventlog_level_error, __FUNCTION__, "error create ladder %d",type);
			continue;
		}
		fseek(fp,bn_int_get(ladderheader[i].offset),SEEK_SET);
		temp=number * sizeof(*ladderinfo);
		if (!(ladderinfo=malloc(temp))) {
			eventlog(eventlog_level_error, __FUNCTION__, "error allocate ladder info");
			continue;
		}
		if (fread(ladderinfo,1,temp,fp)!=temp) {
			eventlog(eventlog_level_error, __FUNCTION__, "error read ladder file");
			free(ladderinfo);
			continue;
		}
		for (n=0; n< number; n++) {
			d2ladder_append_ladder(type,ladderinfo+n);
		}
		free(ladderinfo);
		if (number) count++;
	}
	free(ladderheader);
	fclose(fp);
	eventlog(eventlog_level_info, __FUNCTION__, "ladder file loaded successfully (%d types %d maxtype)",count,max_ladder_type);
	return 0;
}

static int d2ladderlist_create(unsigned int maxtype)
{
	if (!(ladder_data=malloc(maxtype * sizeof(*ladder_data)))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate ladder_data");
		return -1;
	}
	memset(ladder_data,0, maxtype * sizeof(*ladder_data));
	return 0;
}

extern int d2ladder_refresh(void)
{
	d2ladder_destroy();
	return d2ladder_readladder();
}

static int d2ladder_create(unsigned int type, unsigned int len)
{
	if (type>max_ladder_type) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder type %d exceed max ladder type %d",type,max_ladder_type);
		return -1;
	}
	if (!(ladder_data[type].info=malloc(sizeof(t_d2cs_client_ladderinfo) * len))) {
		eventlog(eventlog_level_error, __FUNCTION__, "error allocate memory for ladder info");
		return -1;
	}
	ladder_data[type].len=len;
	ladder_data[type].type=type;
	ladder_data[type].curr_len=0;
	return 0;
}

static int d2ladder_append_ladder(unsigned int type, t_d2ladderfile_ladderinfo * info)
{
	t_d2cs_client_ladderinfo	* ladderinfo;
	unsigned short			ladderstatus;
	unsigned short			status;
	unsigned char			class;

	if (!info) {
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL info");
		return -1;
	}
	if (type > max_ladder_type) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder type %d exceed max ladder type %d",type,max_ladder_type);
		return -1;
	}
	if (!ladder_data[type].info) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder data info not initialized");
		return -1;
	}
	if (ladder_data[type].curr_len >= ladder_data[type].len) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder data overflow %d > %d", ladder_data[type].curr_len, ladder_data[type].len);
		return -1;
	}
	status = bn_short_get(info->status);
	class = bn_byte_get(info->class);
	ladderstatus = (status & LADDERSTATUS_FLAG_DIFFICULTY);
	if (charstatus_get_hardcore(status)) {
		ladderstatus |= LADDERSTATUS_FLAG_HARDCORE;
		if (charstatus_get_dead(status)) {
			ladderstatus |= LADDERSTATUS_FLAG_DEAD;
		}
	}
	if (charstatus_get_expansion(status)) {
		ladderstatus |= LADDERSTATUS_FLAG_EXPANSION;
		ladderstatus |= min(class,D2CHAR_EXP_CLASS_MAX);
	} else {
		ladderstatus |= min(class,D2CHAR_CLASS_MAX);
	}
	ladderinfo=ladder_data[type].info+ladder_data[type].curr_len;
	bn_int_set(&ladderinfo->explow, bn_int_get(info->experience));
	bn_int_set(&ladderinfo->exphigh,0);
	bn_short_set(&ladderinfo->status,ladderstatus);
	bn_byte_set(&ladderinfo->level, bn_int_get(info->level));
	bn_byte_set(&ladderinfo->u1, 0);
	strncpy(ladderinfo->charname, info->charname, MAX_CHARNAME_LEN);
	ladder_data[type].curr_len++;
	return 0;
}

extern int d2ladder_destroy(void)
{
	unsigned int i;

	if (ladder_data) {
		for (i=0; i< max_ladder_type; i++) {
			if (ladder_data[i].info) {
				free(ladder_data[i].info);
				ladder_data[i].info=NULL;
			}
		}
	}
	free(ladder_data);
	ladder_data=NULL;
	max_ladder_type=0;
	return 0;
}

extern int d2ladder_get_ladder(unsigned int * from, unsigned int * count, unsigned int type,
					t_d2cs_client_ladderinfo const * * info)
{
	t_d2ladder	* ladder;

	if (!ladder_data) return -1;
	ladder=ladder_data+type;
	if (!ladder->curr_len || !ladder->info) {
		eventlog(eventlog_level_warn, __FUNCTION__, "ladder type %d not found",type);
		return -1;
	}
	if (ladder->type != type) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad ladder data");
		return -1;
	}
	if (ladder->curr_len < *count) {
		*from = 0;
		*count=ladder->curr_len;
	} else if (*from + *count> ladder->curr_len) {
		*from= ladder->curr_len - *count;
	}
	*info = ladder->info+ *from;
	return 0;
}

extern int d2ladder_find_character_pos(unsigned int type, char const * charname)
{
	unsigned int	i;
	t_d2ladder	* ladder;

	if (!ladder_data) return -1;
	ladder=ladder_data+type;
	if (!ladder->curr_len || !ladder->info) {
		eventlog(eventlog_level_warn, __FUNCTION__, "ladder type %d not found",type);
		return -1;
	}
	if (ladder->type != type) {
		eventlog(eventlog_level_error, __FUNCTION__, "got bad ladder data");
		return -1;
	}
	for (i=0; i< ladder->curr_len; i++) {
		if (!strcasecmp(ladder->info[i].charname,charname)) return i;
	}
	return -1;
}
