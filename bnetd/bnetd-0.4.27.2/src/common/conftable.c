/*
 * Copyright (C) 1998,1999,2000,2001,2002  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
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
#define CONFTABLE_INTERNAL_ACCESS
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
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include "compat/strdup.h"
#include "compat/strcasecmp.h"
#include <errno.h>
#include "compat/strerror.h"
#include "common/util.h"
#include "common/eventlog.h"
#include "conftable.h"
#include "common/setup_after.h"


extern int conftable_set_value(t_conf_entry const * conf_table, char const * name, char const * value)
{
    unsigned int row;
    
    if (!name)
    {
	eventlog(eventlog_level_error,"conftable_set_value","got NULL name");
	return -1;
    }
    if (!value)
    {
	eventlog(eventlog_level_error,"conftable_set_value","got NULL value");
	return -1;
    }
    
    for (row=0; conf_table[row].name; row++)
        if (strcasecmp(conf_table[row].name,name)==0)
	{
            switch (conf_table[row].type)
            {
	    case conf_type_bool:
		switch (str_get_bool(value))
		{
		case 1:
		    *(unsigned int *)conf_table[row].var = 1;
		    break;
		case 0:
		    *(unsigned int *)conf_table[row].var = 0;
		    break;
		default:
		    eventlog(eventlog_level_error,"conftable_set_value","invalid boolean value for name \"%s\"",name);
		}
		break;
		
	    case conf_type_int:
               	*(int *)conf_table[row].var = atoi(value); /* FIXME: check for conversion error */
		break;
		
	    case conf_type_uint:
		{
		    unsigned int temp;
		    
		    if (str_to_uint(value,&temp)<0)
			eventlog(eventlog_level_error,"conftable_set_value","invalid unsigned integer value \"%s\" for name \"%s\"",value,name);
		    else
                	*(unsigned int *)conf_table[row].var = temp;
		}
		break;
		
	    case conf_type_str:
		{
		    char const * temp;
		    
		    if (!(temp = strdup(value)))
		    {
			eventlog(eventlog_level_error,"conftable_set_value","could not allocate memory for value");
			return -1;
		    }
		    if (*(char const * *)conf_table[row].var)
			free((void *)*(char const * * )conf_table[row].var); /* avoid warning */
		    *(char const * *)conf_table[row].var = temp;
		}
		break;
		
	    case conf_type_cstr:
		{
		    char const * temp;
		    
		    if (!(temp = unescape_chars(value)))
		    {
			eventlog(eventlog_level_error,"conftable_set_value","could not allocate memory for value");
			return -1;
		    }
		    if (*(char const * *)conf_table[row].var)
			free((void *)*(char const * * )conf_table[row].var); /* avoid warning */
		    *(char const * *)conf_table[row].var = temp;
		}
		break;
		
	    case conf_type_double:
               	*(double *)conf_table[row].var = atof(value); /* FIXME: check for conversion error */
		break;
		
	    default:
		eventlog(eventlog_level_error,"conftable_set_value","invalid type %d in table entry %u",(int)conf_table[row].type,row);
	    }
	    return 0;
	}
    
    eventlog(eventlog_level_error,"conftable_set_value","unknown name \"%s\"",name);
    return -1;
}


extern unsigned int conftable_lookup_boolentry(t_conf_entry const * conf_table, char const * name)
{
    unsigned int row;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_lookup_boolentry","got NULL conf_table");
	return 0;
    }
    if (!name)
    {
	eventlog(eventlog_level_error,"conftable_lookup_boolentry","got NULL name");
	return 0;
    }
    
    for (row=0; conf_table[row].name; row++)
	if (conf_table[row].type==conf_type_bool && strcasecmp(conf_table[row].name,name)==0)
	    return *(unsigned int *)conf_table[row].var;
    
    return 0;
}


extern int conftable_lookup_int(t_conf_entry const * conf_table, char const * name)
{
    unsigned int row;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_lookup_int","got NULL conf_table");
	return -1;
    }
    if (!name)
    {
	eventlog(eventlog_level_error,"conftable_lookup_int","got NULL name");
	return -1;
    }
    
    for (row=0; conf_table[row].name; row++)
	if (conf_table[row].type==conf_type_int && strcasecmp(conf_table[row].name,name)==0)
	    return *(int *)conf_table[row].var;
    
    return -1;
}


extern unsigned int conftable_lookup_uint(t_conf_entry const * conf_table, char const * name)
{
    unsigned int row;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_lookup_uint","got NULL conf_table");
	return 0;
    }
    if (!name)
    {
	eventlog(eventlog_level_error,"conftable_lookup_uint","got NULL name");
	return 0;
    }
    
    for (row=0; conf_table[row].name; row++)
	if (conf_table[row].type==conf_type_uint && strcasecmp(conf_table[row].name,name)==0)
	    return *(unsigned int *)conf_table[row].var;
    
    return 0;
}


extern char const * conftable_lookup_str(t_conf_entry const * conf_table, char const * name)
{
    unsigned int row;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_lookup_str","got NULL conf_table");
	return NULL;
    }
    if (!name)
    {
	eventlog(eventlog_level_error,"conftable_lookup_str","got NULL name");
	return NULL;
    }
    
    for (row=0; conf_table[row].name; row++)
	if (conf_table[row].type==conf_type_str && strcasecmp(conf_table[row].name,name)==0)
	    return *(char const * *)conf_table[row].var;
    
    return NULL;
}


extern char const * conftable_lookup_cstr(t_conf_entry const * conf_table, char const * name)
{
    unsigned int row;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_lookup_cstr","got NULL conf_table");
	return NULL;
    }
    if (!name)
    {
	eventlog(eventlog_level_error,"conftable_lookup_cstr","got NULL name");
	return NULL;
    }
    
    for (row=0; conf_table[row].name; row++)
	if (conf_table[row].type==conf_type_cstr && strcasecmp(conf_table[row].name,name)==0)
	    return *(char const * *)conf_table[row].var;
    
    return NULL;
}


extern double conftable_lookup_double(t_conf_entry const * conf_table, char const * name)
{
    unsigned int row;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_lookup_double","got NULL conf_table");
	return 0.0;
    }
    if (!name)
    {
	eventlog(eventlog_level_error,"conftable_lookup_double","got NULL name");
	return 0.0;
    }
    
    for (row=0; conf_table[row].name; row++)
	if (conf_table[row].type==conf_type_double && strcasecmp(conf_table[row].name,name)==0)
	    return *(double *)conf_table[row].var;
    
    return 0.0;
}


extern int conftable_load_defaults(t_conf_entry const * conf_table)
{
    unsigned int row;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_load_defaults","got NULL conf_table");
	return -1;
    }
    
    for (row=0; conf_table[row].name; row++)
	if (conftable_set_value(conf_table,conf_table[row].name,conf_table[row].defval)<0)
	    return -1;
    
    return 0;
}


extern int conftable_load_file(t_conf_entry const * conf_table, char const * filename)
{
    FILE *       fp;
    char *       buff;
    char *       cp;
    char *       temp;
    unsigned int currline;
    char const * name;
    char const * value;
    char *       rawvalue;
    
    if (!conf_table)
    {
	eventlog(eventlog_level_error,"conftable_load_file","got NULL conf_table");
	return -1;
    }
    if (!filename)
    {
	eventlog(eventlog_level_error,"conftable_load_file","got NULL filename");
	return -1;
    }
    
    if (!(fp = fopen(filename,"r")))
    {
	eventlog(eventlog_level_error,"conftable_load_file","could not open file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
	return -1;
    }
    
    /* Read the configuration file */
    for (currline=1; (buff = file_get_line(fp)); currline++)
    {
	cp = buff;
	
        while (*cp=='\t' || *cp==' ') cp++;
	if (*cp=='\0' || *cp=='#')
	{
	    free(buff);
	    continue;
	}
	temp = cp;
	while (*cp!='\t' && *cp!=' ' && *cp!='\0') cp++;
	if (*cp!='\0')
	{
	    *cp = '\0';
	    cp++;
	}
	if (!(name = strdup(temp)))
	{
	    eventlog(eventlog_level_error,"conftable_load_file","could not allocate memory for name");
	    free(buff);
	    continue;
	}
        while (*cp=='\t' || *cp==' ') cp++;
	if (*cp!='=')
	{
	    eventlog(eventlog_level_error,"conftable_load_file","missing = on line %u of \"%s\"",currline,filename);
	    free((void *)name); /* avoid warning */
	    free(buff);
	    continue;
	}
	cp++;
	while (*cp=='\t' || *cp==' ') cp++;
	if (*cp=='\0')
	{
	    eventlog(eventlog_level_error,"conftable_load_file","missing value after = on line %u of \"%s\"",currline,filename);
	    free((void *)name); /* avoid warning */
	    free(buff);
	    continue;
	}
	if (!(rawvalue = strdup(cp)))
	{
	    eventlog(eventlog_level_error,"conftable_load_file","could not allocate memory for rawvalue");
	    free((void *)name); /* avoid warning */
	    free(buff);
	    continue;
	}
	
	if (rawvalue[0]=='"')
	{
	    unsigned int jj;
	    char         prev;
	    
	    for (jj=1,prev='\0'; rawvalue[jj]!='\0'; jj++)
	    {
		switch (rawvalue[jj])
		{
		case '"':
		    if (prev!='\\')
			break;
		    prev = '"';
		    continue;
		case '\\':
		    if (prev=='\\')
			prev = '\0';
		    else
			prev = '\\';
		    continue;
		default:
		    prev = rawvalue[jj];
		    continue;
		}
		break;
	    }
	    if (rawvalue[jj]!='"')
	    {
		eventlog(eventlog_level_error,"conftable_load_file","missing end quote for value of name \"%s\" on line %u of \"%s\"",name,currline,filename);
		free(rawvalue);
		free((void *)name); /* avoid warning */
		free(buff);
		continue;
	    }
	    rawvalue[jj] = '\0';
	    if (rawvalue[jj+1]!='\0' && rawvalue[jj+1]!='#')
	    {
		eventlog(eventlog_level_error,"conftable_load_file","extra characters after the value for name \"%s\" on line %u of \"%s\"",name,currline,filename);
		free(rawvalue);
		free((void *)name); /* avoid warning */
		free(buff);
		continue;
	    }
	    value = &rawvalue[1];
	}
	else
	{
	    unsigned int jj;
	    unsigned int kk;
	    
	    for (jj=0; rawvalue[jj]!='\0' && rawvalue[jj]!=' ' && rawvalue[jj]!='\t'; jj++);
	    kk = jj;
	    while (rawvalue[kk]==' ' || rawvalue[kk]=='\t') kk++;
	    if (rawvalue[kk]!='\0' && rawvalue[kk]!='#')
	    {
		eventlog(eventlog_level_error,"conftable_load_file","extra characters (%s) after the value for name \"%s\" on line %u of \"%s\"",&rawvalue[kk],name,currline,filename);
		free(rawvalue);
		free((void *)name); /* avoid warning */
		free(buff);
		continue;
	    }
	    rawvalue[jj] = '\0';
	    value = rawvalue;
	}
	
	if (conftable_set_value(conf_table,name,value)<0)
	    eventlog(eventlog_level_error,"conftable_load_file","unable to set entry \"%s\" on line %u of \"%s\"",name,currline,filename);
	
	free(rawvalue);
	free((void *)name); /* avoid warning */
	free(buff);
    }
    if (fclose(fp)<0)
	eventlog(eventlog_level_error,"conftable_load_file","could not close prefs file \"%s\" after reading (fclose: %s)",filename,strerror(errno));
    
    return 0;
}


extern int conftable_unload(t_conf_entry const * conf_table)
{
    unsigned int row;
    
    for (row=0; conf_table[row].name; row++)
	switch (conf_table[row].type)
	{
	case conf_type_bool:
	case conf_type_int:
	case conf_type_uint:
	case conf_type_double:
	    break;
	    
	case conf_type_str:
	case conf_type_cstr:
	    if (*(char const * *)conf_table[row].var)
	    {
		free((void *)*(char const * *)conf_table[row].var); /* avoid warning */
		*(char const * *)conf_table[row].var = NULL;
	    }
	    break;
	    
	default:
	    eventlog(eventlog_level_error,"conftable_unload","invalid type %d in table",(int)conf_table[row].type);
	    break;
	}
    
    return 0;
}
