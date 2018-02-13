/*
 * Copyright (C) 2000 Onlyer (onlyer@263.net)
 * Copyright (C) 2001 Ross Combs (ross@bnetd.org)
 * Copyright (C) 2002 Gianluigi Tiesi (sherpya@netfarm.it)
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
#define VERSIONCHECK_INTERNAL_ACCESS
#include "common/setup_before.h"
#include <stdio.h>
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
#include "compat/strtoul.h"
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#ifdef HAVE_MKTIME
# ifdef TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif
#endif
#include "compat/strchr.h"
#include "compat/strrchr.h"
#include "compat/strdup.h"
#include <ctype.h>
#include <errno.h>
#include "compat/strerror.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/util.h"
#include "common/proginfo.h"
#include "common/token.h"
#include "common/field_sizes.h"
#include "prefs.h"
#include "versioncheck.h"
#include "common/setup_after.h"


static t_list * versioninfo_head=NULL;
static t_versioncheck dummyvc={ "A=42 B=42 C=42 4 A=A^S B=B^B C=C^C A=A^S", "IX86ver1.mpq" };

static int normalize_year(int mangled);
static int wildcard_casecmp(char const * pattern, char const * str);
static void wildcard_replace(char * pattern, char const * str);
static int versioncheck_compare_exeinfo(char const * pattern, char const * match);


extern t_versioncheck * versioncheck_create(char const * archtag, char const * clienttag)
{
    t_elem const *   curr;
    t_versioninfo *  vi;
    t_versioncheck * vc;

    LIST_TRAVERSE_CONST(versioninfo_head,curr)
    {
        if (!(vi = elem_get_data(curr))) /* should not happen */
        {
            eventlog(eventlog_level_error,"versioncheck_create","version list contains NULL item");
            continue;
        }

	if (strcmp(vi->archtag,archtag)!=0)
	    continue;
	if (strcmp(vi->clienttag,clienttag)!=0)
	    continue;

	/* FIXME: randomize the selection if more than one match */
	if (!(vc = malloc(sizeof(t_versioncheck))))
	{
	    eventlog(eventlog_level_error,"versioncheck_create","unable to allocate memory for vc");
	    return &dummyvc;
	}
	if (!(vc->eqn = strdup(vi->eqn)))
	{
	    eventlog(eventlog_level_error,"versioncheck_create","unable to allocate memory for eqn");
	    free(vc);
	    return &dummyvc;
	}
	if (!(vc->mpqfile = strdup(vi->mpqfile)))
	{
	    eventlog(eventlog_level_error,"versioncheck_create","unable to allocate memory for mpqfile");
	    free((void *)vc->eqn); /* avoid warning */
	    free(vc);
	    return &dummyvc;
	}

	return vc;
    }

    /*
     * No entries in the file that match, return the dummy because we have to send
     * some equation and auth mpq to the client.  The client is not going to pass the
     * validation later unless skip_versioncheck or allow_unknown_version is enabled.
     */
    return &dummyvc;
}


extern int versioncheck_destroy(t_versioncheck * vc)
{
    if (!vc)
    {
	eventlog(eventlog_level_error,"versioncheck_destroy","got NULL vc");
	return -1;
    }

    if (vc==&dummyvc)
	return 0;

    free((void *)vc->mpqfile);
    free((void *)vc->eqn);
    free(vc);

    return 0;
}


extern char const * versioncheck_get_mpqfile(t_versioncheck const * vc)
{
    if (!vc)
    {
	eventlog(eventlog_level_error,"versioncheck_get_mpqfile","got NULL vc");
	return NULL;
    }

    return vc->mpqfile;
}


extern char const * versioncheck_get_eqn(t_versioncheck const * vc)
{
    if (!vc)
    {
	eventlog(eventlog_level_error,"versioncheck_get_mpqfile","got NULL vc");
	return NULL;
    }

    return vc->eqn;
}


/* Now we have a Y2K problem :)  Thanks to Blizzard for randomly using
 * either 2 digit decimal years (in either 19xx or 20xx), a full year,
 * or an offset from 1900.
 */
static int normalize_year(int mangled)
{
    /* negative -> before 1900
     * 00-79    -> 2000-2079
     * 00-79    -> 2000-2079
     * 80-999   -> 1980-2899
     * 1000+    -> unchanged
     */

    if (mangled>=0 && mangled<80) /* assume last 2 digits of year in 20xx */
	return 2000+mangled;

    if (mangled<1000) /* assume C-style offset from 1900 */
	return 1900+mangled;

    /* else assume complete 4 (or more) digit year */
    return mangled;
}


#define safe_toupper(X) (islower((int)X)?toupper((int)X):(X))

static int wildcard_casecmp(char const * pattern, char const * str)
{
    unsigned int i;

    for (i=0; i<strlen(pattern); i++)
	if (pattern[i]!='?' && /* our "don't care" sign */
	    safe_toupper(pattern[i])!=safe_toupper(str[i]))
		return 1; /* neq */

    return 0; /* ok */
}


static void wildcard_replace(char * pattern, char const * str)
{
    unsigned int ii;

    for (ii=0;; ii++)
    {
	if (pattern[ii]=='*')
	    pattern[ii] = str[ii];
	if (pattern[ii]=='\0' || str[ii]=='\0')
	    break;
	if (pattern[ii]=='?')
	    pattern[ii] = str[ii];
    }
}


/* This implements some dumb kind of pattern matching. Any '?'
 * signs in the pattern are treated as "don't care" signs. This
 * means that it doesn't matter what's on this place in the match.
 */
static int versioncheck_compare_exeinfo(char const * pattern, char const * match)
{
    if (!pattern) {
	eventlog(eventlog_level_error,"versioncheck_compare_exeinfo","got NULL pattern");
	return -1; /* neq/fail */
    }
    if (!match) {
	eventlog(eventlog_level_error,"versioncheck_compare_exeinfo","got NULL match");
	return -1; /* neq/fail */
    }

    eventlog(eventlog_level_trace,"versioncheck_compare_exeinfo","pattern=\"%s\" match=\"%s\"",pattern,match);

    if (strcmp(prefs_get_version_exeinfo_match(),"exact")==0) {
	return wildcard_casecmp(pattern,match);
    } else if (strcmp(prefs_get_version_exeinfo_match(),"parse")==0) {
#ifdef HAVE_MKTIME
	struct tm    t1;
	struct tm    t2;
	char         exe1[MAX_EXEINFO_STR];
	char         exe2[MAX_EXEINFO_STR];
	char         dt1_mon[8];
	char         dt1_day[8];
	char         dt1_year[8];
	char         dt1_hour[8];
	char         dt1_min[8];
	char         dt1_sec[8];
	char         dt2_mon[8];
	char         dt2_day[8];
	char         dt2_year[8];
	char         dt2_hour[8];
	char         dt2_min[8];
	char         dt2_sec[8];
	char         format[64];
	unsigned int size1,size2;

	snprintf(format, sizeof(format),
		 "%%%zu[^/]/%%%zu[^/]/%%%zus %%%zu[^:]:%%%zu[^:]:%%%zus %%zu",
		 (sizeof(exe1) - 1UL), (sizeof(dt1_day) - 1UL),
		 (sizeof(dt1_year) - 1UL), (sizeof(dt1_hour) - 1UL),
		 (sizeof(dt1_min) - 1UL), (sizeof(dt1_sec) - 1UL));

	if (sscanf(pattern,format,exe1,dt1_day,dt1_year,dt1_hour,dt1_min,dt1_sec,&size1)!=7) {
	    eventlog(eventlog_level_warn,"versioncheck_compare_exeinfo","parser error while parsing pattern \"%s\"",pattern);
	    return 1; /* neq */
	}
	if (sscanf(match,format,exe2,dt2_day,dt2_year,dt2_hour,dt2_min,dt2_sec,&size2)!=7) {
	    eventlog(eventlog_level_warn,"versioncheck_compare_exeinfo","parser error while parsing match \"%s\"",match);
	    return 1; /* neq */
	}

	/* Split the first chunk into filename + month.  This can't be done above because
	 * the filename can contain just about any characters including spaces.
	 */
	{
	    char * tmp;

	    if (!(tmp = strrchr(exe1,' ')))
	    {
		eventlog(eventlog_level_warn,"versioncheck_compare_exeinfo","parser error while parsing pattern \"%s\"",pattern);
		return 1; /* neq */
	    }

	    strncpy(dt1_mon,&tmp[1],sizeof(dt1_mon));
	    dt1_mon[sizeof(dt1_mon)-1] = '\0';

	    *tmp = '\0';
	    while ((tmp = strrchr(exe1,' ')))
		*tmp = '\0';


	    if (!(tmp = strrchr(exe2,' ')))
	    {
		eventlog(eventlog_level_warn,"versioncheck_compare_exeinfo","parser error while parsing pattern \"%s\"",pattern);
		return 1; /* neq */
	    }

	    strncpy(dt2_mon,&tmp[1],sizeof(dt2_mon));
	    dt2_mon[sizeof(dt2_mon)-1] = '\0';

	    *tmp = '\0';
	    while ((tmp = strrchr(exe2,' ')))
		*tmp = '\0';
	}

	wildcard_replace(dt1_mon,dt2_mon);
	wildcard_replace(dt1_day,dt2_day);
	wildcard_replace(dt1_year,dt2_year);
	wildcard_replace(dt1_hour,dt2_hour);
	wildcard_replace(dt1_min,dt2_min);
	wildcard_replace(dt1_sec,dt2_sec);

	/* zero them out in case of any sytem specific extensions */
	memset(&t1,0,sizeof(t1));
	memset(&t2,0,sizeof(t2));

	t1.tm_mon = atoi(dt1_mon);
	t1.tm_mday = atoi(dt1_day);
	t1.tm_year = normalize_year(atoi(dt1_year))-1900;
	t1.tm_hour = atoi(dt1_hour);
	t1.tm_min = atoi(dt1_min);
	t1.tm_sec = atoi(dt1_sec);

	t2.tm_mon = atoi(dt2_mon);
	t2.tm_mday = atoi(dt2_day);
	t2.tm_year = normalize_year(atoi(dt2_year))-1900;
	t2.tm_hour = atoi(dt2_hour);
	t2.tm_min = atoi(dt2_min);
	t2.tm_sec = strtoul(dt2_sec,NULL,10);

	if (wildcard_casecmp(exe1,exe2)!=0)
	    return 1; /* neq */
	if (size1!=size2)
	    return 1; /* neq */
	if (abs((int)(mktime(&t1)-mktime(&t2)))>prefs_get_version_exeinfo_maxdiff())
	    return 1;
	return 0; /* ok */
#else
	eventlog(eventlog_level_error,"versioncheck_compare_exeinfo","Your system does not support mktime(). Please select another exeinfo matching method.");
	return -1; /* neq/fail */
#endif
    } else {
	eventlog(eventlog_level_error,"versioncheck_compare_exeinfo","unknown version exeinfo match method \"%s\"",prefs_get_version_exeinfo_match());
	return -1; /* neq/fail */
    }
}


extern int versioncheck_validate(t_versioncheck const * vc, char const * archtag, char const * clienttag, char const * exeinfo, unsigned long versionid, unsigned long gameversion, unsigned long checksum, char const ** versiontag)
{
    t_elem const *  curr;
    t_versioninfo * vi;
    int             badexe,badcs;

    if (!vc)
    {
	eventlog(eventlog_level_error,"versioncheck_validate","got NULL vc");
	return -1;
    }

    badexe=badcs = 0;
    LIST_TRAVERSE_CONST(versioninfo_head,curr)
    {
        if (!(vi = elem_get_data(curr))) /* should not happen */
        {
	    eventlog(eventlog_level_error,"versioncheck_validate","version list contains NULL item");
	    continue;
        }

	if (strcmp(vi->eqn,vc->eqn)!=0)
	    continue;
	if (strcmp(vi->mpqfile,vc->mpqfile)!=0)
	    continue;
	if (strcmp(vi->archtag,archtag)!=0)
	    continue;
	if (strcmp(vi->clienttag,clienttag)!=0)
	    continue;

	if (vi->versionid && vi->versionid != versionid)
	    continue;

	if (vi->gameversion && vi->gameversion != gameversion)
	    continue;

	if (vi->exeinfo && (versioncheck_compare_exeinfo(vi->exeinfo,exeinfo) != 0))
	{
	    /*
	     * Found an entry matching but the exeinfo doesn't match.
	     * We need to rember this because if no other matching versions are found
	     * we will return badversion.
	     */
	    badexe = 1;
	}
	else
	    badexe = 0;

	if (vi->checksum && vi->checksum != checksum)
	{
	    /*
	     * Found an entry matching but the checksum doesn't match.
	     * We need to rember this because if no other matching versions are found
	     * we will return badversion.
	     */
	    badcs = 1;
	}
	else
	    badcs = 0;

	*versiontag = vi->versiontag;

	if (badexe || badcs)
	    continue;

	/* Ok, version and checksum matches or exeinfo/checksum are disabled
	 * anyway we have found a complete match */
	if (*versiontag)
	    eventlog(eventlog_level_info,"versioncheck_validate","got a matching entry: %s",*versiontag);
	else
	    eventlog(eventlog_level_info,"versioncheck_validate","got a matching entry");
	return 1;
    }

    if (badcs) /* A match was found but the checksum was different */
    {
	if (*versiontag)
	    eventlog(eventlog_level_info,"versioncheck_validate","bad checksum, closest match is: %s",*versiontag);
	else
	    eventlog(eventlog_level_info,"versioncheck_validate","bad checksum, closest match is: (no versiontag)");
	return -1;
    }
    if (badexe) /* A match was found but the exeinfo string was different */
    {
	if (*versiontag)
	    eventlog(eventlog_level_info,"versioncheck_validate","bad exeinfo, closest match is: %s",*versiontag);
	else
	    eventlog(eventlog_level_info,"versioncheck_validate","bad exeinfo, closest match is: (no versiontag)");
	return -1;
    }

    /* No match in list */
    eventlog(eventlog_level_info,"versioncheck_validate","no match in list");
    *versiontag = NULL;
    return 0;
}


extern int versioncheck_load(char const * filename)
{
    FILE *	    fp;
    unsigned int    line;
    unsigned int    pos;
    char *	    buff;
    char *	    temp;
    char const *    eqn;
    char const *    mpqfile;
    char const *    archtag;
    char const *    clienttag;
    char const *    exeinfo;
    char const *    versionid;
    char const *    gameversion;
    char const *    checksum;
    char const *    versiontag;
    t_versioninfo * vi;

    if (!filename)
    {
	eventlog(eventlog_level_error,"versioncheck_load","got NULL filename");
	return -1;
    }

    if (!(versioninfo_head = list_create()))
    {
	eventlog(eventlog_level_error,"versioncheck_load","could create list");
	return -1;
    }
    if (!(fp = fopen(filename,"r")))
    {
	eventlog(eventlog_level_error,"versioncheck_load","could not open file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
	list_destroy(versioninfo_head);
	versioninfo_head = NULL;
	return -1;
    }

    for (line=1; (buff = file_get_line(fp)); line++)
    {
	for (pos=0; buff[pos]=='\t' || buff[pos]==' '; pos++);
	if (buff[pos]=='\0' || buff[pos]=='#')
	{
	    free(buff);
	    continue;
	}
	if ((temp = strrchr(buff,'#')))
	{
	    unsigned int len;
	    unsigned int endpos;

	    *temp = '\0';
	    len = strlen(buff)+1;
	    for (endpos=len-1;  buff[endpos]=='\t' || buff[endpos]==' '; endpos--);
	    buff[endpos+1] = '\0';
	}

	if (!(eqn = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing eqn on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(mpqfile = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing mpqfile on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(archtag = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing archtag on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(clienttag = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing clienttag on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(exeinfo = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing exeinfo on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(versionid = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing versionid on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(gameversion = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing gameversion on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(checksum = next_token(buff,&pos)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","missing checksum on line %u of file \"%s\"",line,filename);
	    free(buff);
	    continue;
	}
	if (!(versiontag = next_token(buff,&pos)))
	{
	    versiontag = NULL;
	}

	if (!(vi = malloc(sizeof(t_versioninfo))))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","could not allocate memory for vi");
	    free(buff);
	    continue;
	}
	if (!(vi->eqn = strdup(eqn)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","could not allocate memory for eqn");
	    free(vi);
	    free(buff);
	    continue;
	}
	if (!(vi->mpqfile = strdup(mpqfile)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","could not allocate memory for mpqfile");
	    free((void *)vi->eqn); /* avoid warning */
	    free(vi);
	    free(buff);
	    continue;
	}
	if (strlen(archtag)!=4)
	{
	    eventlog(eventlog_level_error,"versioncheck_load","invalid arch tag on line %u of file \"%s\"",line,filename);
	    free((void *)vi->mpqfile); /* avoid warning */
	    free((void *)vi->eqn); /* avoid warning */
	    free(vi);
	    free(buff);
	    continue;
	}
	if (!(vi->archtag = strdup(archtag)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","could not allocate memory for archtag");
	    free((void *)vi->mpqfile); /* avoid warning */
	    free((void *)vi->eqn); /* avoid warning */
	    free(vi);
	    free(buff);
	    continue;
	}
	if (strlen(clienttag)!=4)
	{
	    eventlog(eventlog_level_error,"versioncheck_load","invalid client tag on line %u of file \"%s\"",line,filename);
	    free((void *)vi->archtag); /* avoid warning */
	    free((void *)vi->mpqfile); /* avoid warning */
	    free((void *)vi->eqn); /* avoid warning */
	    free(vi);
	    free(buff);
	    continue;
	}
	if (!(vi->clienttag = strdup(clienttag)))
	{
	    eventlog(eventlog_level_error,"versioncheck_load","could not allocate memory for clienttag");
	    free((void *)vi->archtag); /* avoid warning */
	    free((void *)vi->mpqfile); /* avoid warning */
	    free((void *)vi->eqn); /* avoid warning */
	    free(vi);
	    free(buff);
	    continue;
	}
	if (strcmp(exeinfo, "NULL") == 0)
	    vi->exeinfo = NULL;
	else
	{
	    if (!(vi->exeinfo = strdup(exeinfo)))
	    {
		eventlog(eventlog_level_error,"versioncheck_load","could not allocate memory for exeinfo");
		free((void *)vi->clienttag); /* avoid warning */
		free((void *)vi->archtag); /* avoid warning */
		free((void *)vi->mpqfile); /* avoid warning */
		free((void *)vi->eqn); /* avoid warning */
		free(vi);
		free(buff);
		continue;
	    }
	}
	vi->versionid = strtoul(versionid,NULL,0);
	if (verstr_to_vernum(gameversion,&vi->gameversion)<0)
	{
	    eventlog(eventlog_level_error,"versioncheck_load","malformed version on line %u of file \"%s\"",line,filename);
	    free((void *)vi->exeinfo); /* avoid warning */
	    free((void *)vi->clienttag); /* avoid warning */
	    free((void *)vi->archtag); /* avoid warning */
	    free((void *)vi->mpqfile); /* avoid warning */
	    free((void *)vi->eqn); /* avoid warning */
	    free(vi);
	    free(buff);
	    continue;
        }

	vi->checksum = strtoul(checksum,NULL,0);
	if (versiontag)
	{
	    if (!(vi->versiontag = strdup(versiontag)))
	    {
		eventlog(eventlog_level_error,"versioncheck_load","could not allocate memory for versiontag");
		free((void *)vi->exeinfo); /* avoid warning */
		free((void *)vi->clienttag); /* avoid warning */
		free((void *)vi->archtag); /* avoid warning */
		free((void *)vi->mpqfile); /* avoid warning */
		free((void *)vi->eqn); /* avoid warning */
		free(vi);
		free(buff);
		continue;
	    }
	}
	else
	    vi->versiontag = NULL;

	free(buff);

	if (list_append_data(versioninfo_head,vi)<0)
	{
	    eventlog(eventlog_level_error,"versioncheck_load","could not append item");
	    if (vi->versiontag)
	      free((void *)vi->versiontag); /* avoid warning */
	    free((void *)vi->exeinfo); /* avoid warning */
	    free((void *)vi->clienttag); /* avoid warning */
	    free((void *)vi->archtag); /* avoid warning */
	    free((void *)vi->mpqfile); /* avoid warning */
	    free((void *)vi->eqn); /* avoid warning */
	    free(vi);
	    continue;
	}
    }

    if (fclose(fp)<0)
	eventlog(eventlog_level_error,"versioncheck_load","could not close versioncheck file \"%s\" after reading (fclose: %s)",filename,strerror(errno));

    return 0;
}


extern int versioncheck_unload(void)
{
    t_elem *	    curr;
    t_versioninfo * vi;

    if (versioninfo_head)
    {
	LIST_TRAVERSE(versioninfo_head,curr)
	{
	    if (!(vi = elem_get_data(curr))) /* should not happen */
	    {
		eventlog(eventlog_level_error,"versioncheck_unload","version list contains NULL item");
		continue;
	    }

	    if (list_remove_elem(versioninfo_head,curr)<0)
		eventlog(eventlog_level_error,"versioncheck_unload","could not remove item from list");

	    if (vi->exeinfo)
		free((void *)vi->exeinfo); /* avoid warning */
	    free((void *)vi->clienttag); /* avoid warning */
	    free((void *)vi->archtag); /* avoid warning */
	    free((void *)vi->mpqfile); /* avoid warning */
	    free((void *)vi->eqn); /* avoid warning */
	    if (vi->versiontag)
		free((void *)vi->versiontag); /* avoid warning */
	    free(vi);
	}

	if (list_destroy(versioninfo_head)<0)
	    return -1;
	versioninfo_head = NULL;
    }

    return 0;
}

