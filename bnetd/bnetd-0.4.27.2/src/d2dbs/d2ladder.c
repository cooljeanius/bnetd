/* d2ladder.c
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
# endif /* !NULL */
#endif /* HAVE_STDDEF_H */
#ifdef STDC_HEADERS
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# else
#  warning d2ladder.c expects <stdlib.h> to be included.
# endif /* HAVE_STDLIB_H */
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# else
#  ifdef HAVE_MALLOC_MALLOC_H
#   include <malloc/malloc.h>
#  else
#   warning d2ladder.c expects a malloc-related header to be included.
#  endif /* HAVE_MALLOC_MALLOC_H */
# endif /* HAVE_MALLOC_H */
#endif /* STDC_HEADERS */
#include <stdio.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# else
#  warning d2ladder.c expects a string-related header to be included.
# endif /* HAVE_STRINGS_H */
# ifdef HAVE_MEMORY_H
#  include <memory.h>
# else
#  warning d2ladder.c expects <memory.h> to be included in the absence of <string.h>
# endif /* HAVE_MEMORY_H */
#endif /* HAVE_STRING_H */
#include <errno.h>
#include "compat/strerror.h"
#include <dirent.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# warning d2ladder.c expects <sys/types.h> to be included.
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#else
# warning d2ladder.c expects <sys/stat.h> to be included.
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# ifdef HAVE_SYS_FILE_H
#  include <sys/file.h>
# else
#  warning d2ladder.c expects a file-control-related header to be included.
# endif /* HAVE_SYS_FILE_H */
#endif /* HAVE_FCNTL_H */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#else
# warning d2ladder.c expects <unistd.h> to be included.
#endif /* HAVE_UNISTD_H */

#include "d2ladder.h"
#include "prefs.h"
#include "common/tag.h"
#include "common/list.h"
#include "common/eventlog.h"
#include "common/setup_after.h"


static char *		d2ladder_ladder_file = NULL;
static char *		d2ladder_backup_file = NULL;

static t_d2ladderlist *	d2ladder_list = NULL;
static unsigned long	d2ladder_maxtype;
static int 		d2ladder_change_count=0;
static int		d2ladder_need_rebuild = 0;


static int d2ladderlist_init(void);
static int d2ladderlist_destroy(void);
static int d2ladder_empty(void);
static int d2ladder_initladderfile(void);

static t_d2ladder * d2ladderlist_find_type(unsigned int type);

static int d2ladder_check(void);
static int d2ladder_readladder(void);

static int d2ladder_insert(t_d2ladder * d2ladder,t_d2ladder_info * pcharladderinfo);
static int d2ladder_find_char(t_d2ladder * d2ladder, t_d2ladder_info * info);
static int d2ladder_find_char_all(t_d2ladder * d2ladder, t_d2ladder_info * info);
static int d2ladder_find_pos(t_d2ladder * d2ladder, t_d2ladder_info * info);
static int d2ladder_update_info_and_pos(t_d2ladder * d2ladder, t_d2ladder_info * info, int oldpos, int newpos);
static unsigned long d2ladder_buffer(char const * filename, unsigned char * * buffer);
static unsigned int d2ladder_checksum(unsigned char const * data, unsigned int len);
static int d2ladder_checksum_set(void);
static int d2ladder_checksum_check(void);


extern int d2ladder_update(t_d2ladder_info * pcharladderinfo)
{
	t_d2ladder * d2ladder;
	unsigned short hardcore,expansion,status;
	unsigned char class;
	unsigned int ladder_overall_type,ladder_class_type;

	if (!pcharladderinfo->charname[0]) return 0;
	class=pcharladderinfo->class;
	status=pcharladderinfo->status;

	hardcore=charstatus_get_hardcore(status);
	expansion=charstatus_get_expansion(status);
	ladder_overall_type=0;
	if (!expansion && class> D2CHAR_CLASS_MAX ) return -1;
	if (expansion && class> D2CHAR_EXP_CLASS_MAX ) return -1;
	if (hardcore && expansion) {
		ladder_overall_type=D2LADDER_EXP_HC_OVERALL;
	} else if (!hardcore && expansion) {
		ladder_overall_type=D2LADDER_EXP_STD_OVERALL;
	} else if (hardcore && !expansion) {
		ladder_overall_type=D2LADDER_HC_OVERALL;
	} else if (!hardcore && !expansion) {
		ladder_overall_type=D2LADDER_STD_OVERALL;
	}

	ladder_class_type=ladder_overall_type + class+1;

	d2ladder=d2ladderlist_find_type(ladder_overall_type);
	if (d2ladder_insert(d2ladder,pcharladderinfo)==1) {
		d2ladder_change_count++;
	}

	d2ladder=d2ladderlist_find_type(ladder_class_type);
	if (d2ladder_insert(d2ladder,pcharladderinfo)==1) {
		d2ladder_change_count++;
	}
	return 0;
}


static int d2ladder_initladderfile(void)
{
	FILE * fp;
	t_d2ladderfile_ladderindex lhead[D2LADDER_MAXTYPE];
	t_d2ladderfile_header fileheader;
	int start;
	unsigned long maxtype;
	t_d2ladderfile_ladderinfo emptydata;
	unsigned int i,j, number;

	maxtype=D2LADDER_MAXTYPE;
	start=sizeof(t_d2ladderfile_header)+sizeof(lhead);
	for (i=0;i<D2LADDER_MAXTYPE;i++) {
		bn_int_set(&lhead[i].type,i);
		bn_int_set(&lhead[i].offset,start);
		if ( i==D2LADDER_HC_OVERALL ||
			i==D2LADDER_STD_OVERALL ||
			i==D2LADDER_EXP_HC_OVERALL ||
			i==D2LADDER_EXP_STD_OVERALL) {
				number=D2LADDER_OVERALL_MAXNUM;
		} else if ((i>D2LADDER_HC_OVERALL && i<=D2LADDER_HC_OVERALL+D2CHAR_CLASS_MAX +1) ||
			(i>D2LADDER_STD_OVERALL && i<=D2LADDER_STD_OVERALL+D2CHAR_CLASS_MAX +1) ||
			(i>D2LADDER_EXP_HC_OVERALL && i<=D2LADDER_EXP_HC_OVERALL+D2CHAR_EXP_CLASS_MAX +1) ||
			(i>D2LADDER_EXP_STD_OVERALL && i<=D2LADDER_EXP_STD_OVERALL+D2CHAR_EXP_CLASS_MAX +1)) {
				number=D2LADDER_MAXNUM;
		} else {
				number=0;
		}
		bn_int_set(&lhead[i].number,number);
		start += number*sizeof(emptydata);
	}
	memset(&emptydata,0,sizeof(emptydata));

	if (!d2ladder_ladder_file) return -1;
	fp=fopen(d2ladder_ladder_file,"wb");
	if (!fp) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to open ladder file \"%s\" for writing (open: %s)",d2ladder_ladder_file,strerror(errno));
		return -1;
	}
	bn_int_set(&fileheader.maxtype,maxtype);
	fwrite(&fileheader,1,sizeof(fileheader),fp);
	fwrite(lhead,1,sizeof(lhead),fp);
	for (i=0; i<maxtype; i++) {
		for (j=0; j<bn_int_get(lhead[i].number); j++) {
			fwrite(&emptydata,1,sizeof(emptydata),fp);
		}
	}
	fclose(fp);
	d2ladder_checksum_set();
	return 0;
}


static int d2ladder_find_pos(t_d2ladder * d2ladder, t_d2ladder_info * info)
{
	int i;

	if (!d2ladder || !info) return -1;
	/* only allow if the experience threshold is reached */
	if (info->experience <  prefs_get_ladderupdate_threshold())return -1;
	i=d2ladder->len;
	while (i--) {
		if (d2ladder->info[i].experience >= info->experience) {
			if (strcmp(d2ladder->info[i].charname,info->charname)) {
				i++;
			}
			break;
		}
		if (i<=0) break;
	}
	return i;
}


static int d2ladder_insert(t_d2ladder * d2ladder,t_d2ladder_info * info)
{
	int	oldpos, newpos;

	newpos=d2ladder_find_pos(d2ladder,info);
	/* we currectly do nothing when character is being kick out of ladder for simple */
	/*
	if (newpos<0 || newpos >= d2ladder->len) return 0;
	*/
	oldpos=d2ladder_find_char(d2ladder,info);
	return d2ladder_update_info_and_pos(d2ladder,info,oldpos,newpos);
}


static int d2ladder_find_char_all(t_d2ladder * d2ladder, t_d2ladder_info * info)
{
	int		i;
	t_d2ladder_info * ladderdata;

	if (!d2ladder || !info) return -1;
	ladderdata=d2ladder->info;
	if (!ladderdata) return -1;
	i=d2ladder->len;
	while (i--) {
		if (!strncmp(ladderdata[i].charname,info->charname,MAX_CHARNAME_LEN)) return i;
	}
	return -1;
}


static int d2ladder_find_char(t_d2ladder * d2ladder, t_d2ladder_info * info)
{
	int		i;
	t_d2ladder_info * ladderdata;

	if (!d2ladder || !info) return -1;
	ladderdata=d2ladder->info;
	if (!ladderdata) return -1;
	i=d2ladder->len;
	while (i--) {
		if (ladderdata[i].level > info->level) return -1;
		if (!strncmp(ladderdata[i].charname,info->charname,MAX_CHARNAME_LEN)) return i;
	}
	return -1;
}


static int d2ladder_update_info_and_pos(t_d2ladder * d2ladder, t_d2ladder_info * info, int oldpos, int newpos)
{
	int	i;
	int	direction;
	int	outflag;
	t_d2ladder_info * ladderdata;

	if (!d2ladder || !info) return -1;
	ladderdata=d2ladder->info;
	if (!ladderdata) return -1;

	/* character not in ladder before */
	outflag=0;
	if (oldpos < 0 || oldpos >= d2ladder->len) {
		oldpos = d2ladder->len-1;
	}
	if (newpos < 0 || newpos >= d2ladder->len) {
		newpos = d2ladder->len-1;
		outflag = 1;
	}
	if ((oldpos == d2ladder->len-1) && outflag) {
		return 0;
	}
	if (newpos > oldpos && !outflag ) newpos--;
	direction = (newpos > oldpos)? 1: -1;
	for (i=oldpos; i!=newpos; i+=direction) {
		ladderdata[i] = ladderdata[i+direction];
	}
	ladderdata[i]=*info;
	return 1;
}


extern int d2ladder_rebuild(void)
{
	d2ladder_empty();
	d2ladder_need_rebuild = 0;
	return 0;
}


static int d2ladder_check(void)
{
	if (!d2ladder_ladder_file) return -1;
	if (!d2ladder_backup_file) return -1;
	if (d2ladder_checksum_check()!=1) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder file checksum error, attempting to use backup file");
		if (rename(d2ladder_backup_file,d2ladder_ladder_file)==-1) {
			eventlog(eventlog_level_error, __FUNCTION__, "uname to rename backup ladder file \"%s\" to \"%s\" (rename: %s)", d2ladder_backup_file,d2ladder_ladder_file,strerror(errno));
		}
		if (d2ladder_checksum_check()!=1) {
			eventlog(eventlog_level_error, __FUNCTION__, "ladder backup file checksum error, rebuilding ladder");
			if (d2ladder_initladderfile()<0) return -1;
			/* we need to ensure the ladder structure is fully
			 * populated else the writes will all be 0 length
			 * and only the header goes. to do this just reload
			 * the newly created ladderfile
			 */
			d2ladder_readladder();
			d2ladder_need_rebuild=1;
			return 0;
		}
	}
	return 1;
}


static t_d2ladder * d2ladderlist_find_type(unsigned int type)
{
	t_d2ladder *   d2ladder;
	t_elem const * elem;

	if (!d2ladder_list) {
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL d2ladder_list");
		return NULL;
	}

	LIST_TRAVERSE_CONST(d2ladder_list,elem)
	{
		if (!(d2ladder=elem_get_data(elem))) continue;
		if (d2ladder->type==type) return d2ladder;
	}
	eventlog(eventlog_level_error, __FUNCTION__, "could not find type %d in d2ladder_list",type);
	return NULL;
}


extern int d2ladder_init(void)
{
	d2ladder_change_count=0;
	d2ladder_maxtype=0;
	d2ladder_ladder_file=malloc(strlen(prefs_get_ladder_dir())+1+
			  strlen(LADDER_FILE_PREFIX)+1+strlen(CLIENTTAG_DIABLO2DV)+1+10);
	d2ladder_backup_file=malloc(strlen(prefs_get_ladder_dir())+1+
			  strlen(LADDER_BACKUP_PREFIX)+1+strlen(CLIENTTAG_DIABLO2DV)+1+10);
	if (!d2ladder_ladder_file) return -1;
	if (!d2ladder_backup_file) return -1;

	sprintf(d2ladder_ladder_file,"%s/%s.%s",prefs_get_ladder_dir(),
		LADDER_FILE_PREFIX,CLIENTTAG_DIABLO2DV);

	sprintf(d2ladder_backup_file,"%s/%s.%s",prefs_get_ladder_dir(),
		LADDER_BACKUP_PREFIX,CLIENTTAG_DIABLO2DV);

	if (d2ladderlist_init()<0) {
		return -1;
	}
	if (d2ladder_check()<0) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder file checking error");
		return -1;
	}
	if (!d2ladder_need_rebuild && d2ladder_readladder()<0) {
		return -1;
	}
	if (d2ladder_need_rebuild) d2ladder_rebuild();
	d2ladder_saveladder();
	return 0;
}


static int d2ladderlist_init(void)
{
   	t_d2ladder 	* d2ladder;
	unsigned int 	i;


	if (!d2ladder_ladder_file) return -1;
	if (!(d2ladder_list=list_create())) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not create d2ladder_list");
		return -1;
	}
	d2ladder_maxtype=D2LADDER_MAXTYPE;
	for (i=0;i<d2ladder_maxtype;i++) {
		if (!(d2ladder=malloc(sizeof(t_d2ladder)))) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not allocate d2ladder");
			return -1;
		}
		d2ladder->type=i;
		d2ladder->info=NULL;
		d2ladder->len=0;
		list_append_data(d2ladder_list,d2ladder);
	}
	return 0;
}


static int d2ladder_readladder(void)
{
	t_d2ladder			* d2ladder;
	t_d2ladderfile_header		fileheader;
	FILE				* fp;
	t_d2ladderfile_ladderindex	* lhead;
	t_d2ladderfile_ladderinfo	* ldata;
	t_d2ladder_info			* info;
	t_d2ladder_info			temp;
	long				blocksize;
	long				leftsize;
	unsigned int			laddertype;
	unsigned int			tempmaxtype;
	size_t				readlen;
	unsigned int			i, number;

	if (!d2ladder_ladder_file) return -1;
	fp=fopen(d2ladder_ladder_file,"rb");
	if (!fp) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to open ladder file \"%s\" for reading (fopen: %s)",d2ladder_ladder_file,strerror(errno));
		return -1;
	}

	fseek(fp,0,SEEK_END);
	leftsize = ftell(fp);
	rewind(fp);

	blocksize=sizeof(fileheader);
	if (leftsize<blocksize) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder file \"%s\" too short to contain header",d2ladder_ladder_file);
		fclose(fp);
		return -1;
	}

	readlen=fread(&fileheader,1,sizeof(fileheader),fp);
	if (readlen!=sizeof(fileheader)) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to read ladder file \"%s\" (fread: %s)",d2ladder_ladder_file,strerror(errno));
		fclose(fp);
		return -1;
	}
	tempmaxtype=bn_int_get(fileheader.maxtype);
	leftsize-=blocksize; /* FIXME: should this be -=readlen? */

	if (tempmaxtype>D2LADDER_MAXTYPE) {
		eventlog(eventlog_level_error, __FUNCTION__, "in header of \"%s\" ladder type > D2LADDER_MAXTYPE error",d2ladder_ladder_file);
		fclose(fp);
		return -1;
	}
	d2ladder_maxtype=tempmaxtype;

	blocksize=d2ladder_maxtype*sizeof(*lhead);
	if (leftsize < blocksize) {
		eventlog(eventlog_level_error, __FUNCTION__, "truncated ladder file \"%s\"",d2ladder_ladder_file);
		fclose(fp);
		return -1;
	}

	if (!(lhead=malloc(blocksize))) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for lhead");
		fclose(fp);
		return -1;
    	}
	readlen=fread(lhead,1,d2ladder_maxtype*sizeof(*lhead),fp);
	if (readlen!=d2ladder_maxtype*sizeof(*lhead)) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to read ladder file \"%s\" (fread: %s)",d2ladder_ladder_file,strerror(errno));
		free(lhead);
		fclose(fp);
		return -1;
	}
	leftsize-=blocksize;

	blocksize=0;
	for (i=0;i<d2ladder_maxtype;i++) {
		blocksize+=bn_int_get(lhead[i].number)*sizeof(*ldata);
	}
	if (leftsize < blocksize ) {
		eventlog(eventlog_level_error, __FUNCTION__, "truncated ladder file \"%s\"",d2ladder_ladder_file);
		free(lhead);
		fclose(fp);
		return -1;
	}

	for (laddertype=0;laddertype<d2ladder_maxtype;laddertype++) {
		number= bn_int_get(lhead[laddertype].number);
		if (number<=0) continue;
		d2ladder=d2ladderlist_find_type(laddertype);
		if (!d2ladder) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not find ladder type %d",laddertype);
			continue;
		}
		if (!(ldata=malloc(number*sizeof(*ldata)))) {
			eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for ldata");
			continue;
		}
		if (!(info=malloc(number * sizeof(*info)))) {
			eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for info");
			free(ldata);
			continue;
		}
		memset(info,0,number * sizeof(*info));
		fseek(fp,bn_int_get(lhead[laddertype].offset),SEEK_SET);
		readlen=fread(ldata,1,number*sizeof(*ldata),fp);
		if (readlen!=number*sizeof(*ldata)) {
			eventlog(eventlog_level_error, __FUNCTION__, "unable to read ladder file \"%s\" (fread: %s)",d2ladder_ladder_file,strerror(errno));
			free(ldata);
			free(info);
			continue;
		}
		d2ladder->info=info;
		d2ladder->len=number;
		for (i=0; i< number; i++) {
			if (!ldata[i].charname[0]) continue;
			temp.experience=bn_int_get(ldata[i].experience);
			temp.status=bn_short_get(ldata[i].status);
			temp.level=bn_byte_get(ldata[i].level);
			temp.class=bn_byte_get(ldata[i].class);
			strncpy(temp.charname,ldata[i].charname,sizeof(info[i].charname));
			temp.charname[sizeof(info[i].charname)-1] = '\0'; /* must terminate string */
			if (d2ladder_update_info_and_pos(d2ladder,&temp,
				d2ladder_find_char_all(d2ladder,&temp),
				d2ladder_find_pos(d2ladder,&temp))==1) {
				d2ladder_change_count++;
			}
		}
		free(ldata);
	}
	/* leftsize-=blocksize; */ /* avoid warning 8) - for brainy compilers */

	free(lhead);
	fclose(fp);
	return 0;
}


extern int d2ladder_destroy(void)
{
    	unsigned int i;
    	t_d2ladder * d2ladder;

	d2ladder_saveladder();
    	for (i=0;i<d2ladder_maxtype;i++) {
		d2ladder=d2ladderlist_find_type(i);
		if (d2ladder)
		{
			if (d2ladder->info)
				free(d2ladder->info);
			d2ladder->info=NULL;
			d2ladder->len=0;
		}
    	}
	d2ladderlist_destroy();
	if (d2ladder_ladder_file) {
		free(d2ladder_ladder_file);
		d2ladder_ladder_file=NULL;
	}
	if (d2ladder_backup_file) {
		free(d2ladder_backup_file);
		d2ladder_backup_file=NULL;
	}
	return 0;
}


static int d2ladderlist_destroy(void)
{
    	t_d2ladder * d2ladder;
    	t_elem *	elem;

	if (!d2ladder_list) return -1;
    	LIST_TRAVERSE(d2ladder_list,elem)
    	{
		if (!(d2ladder=elem_get_data(elem))) continue;
		free(d2ladder);
		list_remove_elem(d2ladder_list,elem);
    	}
    	list_destroy(d2ladder_list);
	return 0;
}


static int d2ladder_empty(void)
{
	unsigned int i;
	t_d2ladder * d2ladder;

	for (i=0;i<d2ladder_maxtype;i++) {
		d2ladder=d2ladderlist_find_type(i);
		if (d2ladder) {
			memset(d2ladder->info,0,d2ladder->len * sizeof(*d2ladder->info));
		}
	}
	return 0;
}


extern int d2ladder_saveladder(void)
{
	t_d2ladderfile_ladderindex	lhead[D2LADDER_MAXTYPE];
	t_d2ladderfile_header		fileheader;
	FILE				* fp;
	int				start;
	unsigned int			i,j, number;
	t_d2ladder			* d2ladder;
	t_d2ladderfile_ladderinfo	* ldata;

/*
	if (!d2ladder_change_count) {
		eventlog(eventlog_level_debug, __FUNCTION__, "ladder data unchanged, skip saving");
		return 0;
	}
*/
	start=sizeof(fileheader)+sizeof(lhead);

	for (i=0;i<D2LADDER_MAXTYPE;i++) {
		d2ladder=d2ladderlist_find_type(i);
		bn_int_set(&lhead[i].type,d2ladder->type);
		bn_int_set(&lhead[i].offset,start);
		bn_int_set(&lhead[i].number,d2ladder->len);
		start+=d2ladder->len*sizeof(*ldata);
	}

	if (!d2ladder_ladder_file) return -1;
	if (!d2ladder_backup_file) return -1;

	if (d2ladder_checksum_check()==1) {
		eventlog(eventlog_level_info,  __FUNCTION__, "backup ladder file");
		rename(d2ladder_ladder_file,d2ladder_backup_file);
	}

	fp=fopen(d2ladder_ladder_file,"wb");
	if (!fp) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to open ladder file \"%s\" for writing (fopen: %s)",d2ladder_ladder_file,strerror(errno));
		return -1;
	}
	bn_int_set(&fileheader.maxtype,d2ladder_maxtype);
	fwrite(&fileheader,1,sizeof(fileheader),fp);
	fwrite(lhead,1,sizeof(lhead),fp);
	for (i=0; i<d2ladder_maxtype; i++) {
		number=bn_int_get(lhead[i].number);
		if (number<=0) continue;
		d2ladder=d2ladderlist_find_type(i);
		if (!(ldata=malloc(number * sizeof(*ldata)))) {
			eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for ldata");
			continue;
		}
		memset(ldata,0,number * sizeof(*ldata));
		for (j=0; j< number; j++) {
			bn_int_set(&ldata[j].experience,d2ladder->info[j].experience);
			bn_short_set(&ldata[j].status, d2ladder->info[j].status);
			bn_byte_set(&ldata[j].level, d2ladder->info[j].level);
			bn_byte_set(&ldata[j].class, d2ladder->info[j].class);
			strncpy(ldata[j].charname,d2ladder->info[j].charname,sizeof(ldata[j].charname));
		}
		fwrite(ldata,1,number*sizeof(*ldata),fp); /* FIXME: check return value */
		free(ldata);
	}
	fclose(fp);
	d2ladder_checksum_set();
	eventlog(eventlog_level_info,  __FUNCTION__, "ladder file saved (%d changes)",d2ladder_change_count);
	d2ladder_change_count=0;
	return 0;
}


extern int d2ladder_print(FILE * ladderstrm)
{
	t_d2ladder * d2ladder;
	t_d2ladder_info * ldata;
	unsigned int i,type,overalltype,classtype;
	char const laddermode[][20]={"Hardcore", "Standard", "Expansion HC", "Expansion", "UNKNOWN" };
	char const charclass[][12]={"OverAll", "Amazon", "Sorceress", "Necromancer", "Paladin",
	       			"Barbarian", "Druid", "Assassin", "", "", "", "UNKNOWN" };

	for (type=0; type < d2ladder_maxtype; type++) {
		d2ladder=d2ladderlist_find_type(type);
		if (!d2ladder)
			continue;
		if (d2ladder->len<=0)
			continue;
		ldata=d2ladder->info;

		overalltype=0;
		classtype=0;

		if (type >= D2LADDER_HC_OVERALL && type<= D2LADDER_HC_OVERALL+D2CHAR_CLASS_MAX +1)
		{
			overalltype=0;
			classtype=type-D2LADDER_HC_OVERALL;
		}
		else if	(type >= D2LADDER_STD_OVERALL && type<= D2LADDER_STD_OVERALL+D2CHAR_CLASS_MAX +1)
		{
			overalltype=1;
			classtype=type-D2LADDER_STD_OVERALL;
		}
		else if	(type >= D2LADDER_EXP_HC_OVERALL && type<= D2LADDER_EXP_HC_OVERALL+D2CHAR_EXP_CLASS_MAX +1)
		{
			overalltype=2;
			classtype=type-D2LADDER_EXP_HC_OVERALL;
		}
		else if	(type >= D2LADDER_EXP_STD_OVERALL && type<= D2LADDER_EXP_STD_OVERALL+D2CHAR_EXP_CLASS_MAX +1)
		{
			overalltype=3;
			classtype=type-	D2LADDER_EXP_STD_OVERALL;
		}
		else
		{
			/* bad info, report unknown */
			overalltype=4;
			classtype=11;
		}

		fprintf(ladderstrm,"ladder type %u  %s %s\n",type,laddermode[overalltype],charclass[classtype]);
		fprintf(ladderstrm,"************************************************************************\n");
		fprintf(ladderstrm,"No    character name    level      exp       status   title   class     \n");
		for (i=0; i<d2ladder->len; i++)
		{
			fprintf(ladderstrm,"NO.%2u  %-16s    %2d   %10d       %2X       %1X    %s\n",
				i+1,
				ldata[i].charname,
				ldata[i].level,
				ldata[i].experience,
				ldata[i].status,
				1,
				charclass[ldata[i].class+1]);
		}
		fprintf(ladderstrm,"************************************************************************\n");
		fflush(ladderstrm);
	}
	return 0;
}


static unsigned long d2ladder_buffer(char const * filename, unsigned char * * buffer)
{
	FILE	* fp;
	long	filesize;
	size_t	readlen;

	if (!filename) return 0;
	if (!buffer) return 0;

	fp=fopen(filename,"rb");
	if (!fp) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to open ladder file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
		return 0;
	}

	fseek(fp,0,SEEK_END);
	filesize = ftell(fp);
	rewind(fp);

	if (filesize==-1) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not determine size of ladder file \"%s\" (fseek: %s)",filename,strerror(errno));
		fclose(fp);
		return 0;
	}
	if (filesize<(long)sizeof(t_d2ladderfile_header)) {
		eventlog(eventlog_level_error, __FUNCTION__, "ladder file \"%s\" too short (%lu<%lu)",filename,(unsigned long)filesize,(unsigned long)sizeof(t_d2ladderfile_header));
		fclose(fp);
		return 0;
	}

	if (!(*buffer=malloc(filesize))) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for buffer");
		fclose(fp);
		return 0;
	}

	readlen=fread(*buffer,1,filesize,fp);
	if (readlen<(size_t)filesize) {
		eventlog(eventlog_level_error, __FUNCTION__, "got short ladder file or read error (fread:%s)",strerror(errno));
		free(*buffer);
		fclose(fp);
		return 0;
	}

	fclose(fp);
	return (unsigned long)filesize;
}


static unsigned int d2ladder_checksum(unsigned char const * data, unsigned int len)
{
	int	checksum;
	unsigned int	i;
	unsigned int	ch;

	if (!data) return 0;
	/* FIXME: this should be done with unsigned arithmetic since the calculation
	 * wraps around.  However we can't change this without breaking old ladder
	 * files since the checksum wouldn't match.
	 */
	checksum=0;
	for (i=0; i<len; i++) {
		ch=data[i];
		if (i>=LADDERFILE_CHECKSUM_OFFSET && i<LADDERFILE_CHECKSUM_OFFSET+4) ch=0;
		ch+=(checksum<0);
		checksum=2*checksum+ch;
	}
	return *(unsigned int *)&checksum; /* ugh see FIXME above */
}


static int d2ladder_checksum_set(void)
{
	FILE *		fp;
	unsigned long	filesize;
	unsigned char * buffer;
	bn_int		checksum;

	filesize=d2ladder_buffer(d2ladder_ladder_file,&buffer);
	if (filesize<1) return -1;

	bn_int_set(&checksum,d2ladder_checksum(buffer,filesize));

	fp=fopen(d2ladder_ladder_file,"r+b");
	if (!fp) {
		eventlog(eventlog_level_error, __FUNCTION__, "unable to open ladder file \"%s\" for writing (fopen: %s)",d2ladder_ladder_file,strerror(errno));
		free(buffer);
		return -1;
	}
	fseek(fp,LADDERFILE_CHECKSUM_OFFSET,SEEK_SET); /* FIXME: check retval */
	fwrite(&checksum,1,sizeof(checksum),fp); /* FIXME: check retval */
	fclose(fp);

	free(buffer);
	return 0;
}


static int d2ladder_checksum_check(void)
{
	unsigned long		filesize;
	unsigned char		* buffer;
	unsigned int		checksum_calc,checksum_stor;
	t_d2ladderfile_header	* header;

	filesize=d2ladder_buffer(d2ladder_ladder_file,&buffer);
	if (filesize<1) return -1;

	header=(t_d2ladderfile_header *)buffer;
	checksum_stor=bn_int_get(header->checksum);
	checksum_calc=d2ladder_checksum(buffer,filesize);
	free(buffer);

	if (checksum_stor!=checksum_calc) {
		eventlog(eventlog_level_debug, __FUNCTION__, "ladder file \"%s\" checksum mismatch (stored=0x%08x,calc=0x%08x)",d2ladder_ladder_file,checksum_stor,checksum_calc);
		return 0;
	}
	eventlog(eventlog_level_info,  __FUNCTION__, "ladder file check pass (checksum=0x%X)",checksum_calc);
	return 1;
}

/* EOF */
