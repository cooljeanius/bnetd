/*
 * Copyright (C) 2000  Onlyer (onlyer@263.net)
 * Copyright (C) 2002  Ross Combs (rocombs@cs.nmsu.edu)
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
#define ALIAS_COMMAND_INTERNAL_ACCESS
#include "common/setup_before.h"
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
#include "compat/strcasecmp.h"
#include <ctype.h>
#include <errno.h>
#include "compat/strerror.h"
#include "common/field_sizes.h"
#include "common/util.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "message.h"
#include "connection.h"
#include "alias_command.h"
#include "common/setup_after.h"


static t_list * aliaslist_head=NULL;

#define MAX_ALIAS_LEN 32


static int list_aliases(t_connection * c);
static char * replace_args(char const * in, unsigned int * offsets, unsigned int numargs);
static int do_alias(t_connection * c, char const * cmd, char const * args);


static int list_aliases(t_connection * c)
{
    t_elem const *  elem1;
    t_elem const *  elem2;
    t_alias const * alias;
    t_output *      output;
    char            temp[MAX_MESSAGE_LEN];
    
    message_send_text(c,message_type_info,c,"Alias list:");
    LIST_TRAVERSE_CONST(aliaslist_head,elem1)
    {
	if (!(alias = elem_get_data(elem1)))
	    continue;
	
	sprintf(temp,"@%.128s",alias->alias);
	message_send_text(c,message_type_info,c,temp);
	LIST_TRAVERSE_CONST(alias->output,elem2)
	{
	    if (!(output = elem_get_data(elem2)))
		continue;
	    
	    /*
	     * FIXME: need a more user-friendly way to express this... maybe
             * add a help line to the file format?
	     */
	    sprintf(temp,"[%u-%u]%.128s",output->min,output->max,output->line);
	    message_send_text(c,message_type_info,c,temp);
	}
    }
    return 0;
}


static char * replace_args(char const * in, unsigned int * offsets, unsigned int numargs)
{
    char *         out;
    unsigned int   inpos;
    unsigned int   outpos;
    unsigned int   off1;
    unsigned int   off2;
    unsigned int   i;
    
    if (!(out = malloc(1))) /* for nul */
        return NULL;
    
    for (inpos=outpos=0; inpos<strlen(in); inpos++)
    {
	if (in[inpos]!='$')
	{
            out[outpos] = in[inpos];
            outpos++;
	    continue;
        }
	
	inpos++;
	
	if (in[inpos]=='*')
	{
	    off1 = 0;
	    off2 = strlen(in)-1;
	}
        else if (in[inpos]=='{')
	{
	    unsigned int arg1;
	    unsigned int arg2;
	    
	    if (sscanf(&in[inpos],"{%u-%u}",&arg1,&arg2)!=2)
		if (sscanf(&in[inpos],"{%u-}",&arg1)!=1)
		{
		    if (sscanf(&in[inpos],"{-%u}",&arg2)!=1)
		    {
			if (sscanf(&in[inpos],"{%u}",&arg1)==1)
			{
/* FIXME: no goto, but should be same as isdigit case... */
			}
			while (in[inpos]!='\0' && in[inpos]!='}')
			    inpos++;
			continue;
		    }
		    else
			arg1 = 0;
		}
		else
		    arg2 = numargs-1;
	    
	    if (arg2>=numargs)
		arg2 = numargs-1;
	    if (arg1>arg2)
	    {
		while (in[inpos]!='\0' && in[inpos]!='}')
		    inpos++;
		continue;
	    }
	    off1 = offsets[arg1];
	    if (arg2+1==numargs)
		off2 = strlen(in)-1;
	    else
		off2 = offsets[arg2+1]-1;
	    
	    while (in[inpos]!='\0' && in[inpos]!='}')
		inpos++;
	}
	else if (isdigit((int)in[inpos]))
	{
	    unsigned int arg;
	    
	    if (in[inpos]=='0') arg = 0;
	    else if (in[inpos]=='1') arg = 1;
	    else if (in[inpos]=='2') arg = 2;
	    else if (in[inpos]=='3') arg = 3;
	    else if (in[inpos]=='4') arg = 4;
	    else if (in[inpos]=='5') arg = 5;
	    else if (in[inpos]=='6') arg = 6;
	    else if (in[inpos]=='7') arg = 7;
	    else if (in[inpos]=='8') arg = 8;
	    else arg = 9;
	    
	    if (arg>=numargs)
		continue;
	    for (off1=off2=offsets[arg]; in[off2]!='\0' && in[off2]!=' ' && in[off2]!='\t'; off2++);
	    if (in[off2]!='\0')
		off2--;
	}
	
        {
            char * newout;
	    
            if (!(newout = realloc(out,outpos+(off2-off1)+1))) /* curr + new + nul */
            {
                free(out);
                return NULL;
            }
            out = newout;
	    
	    while (off1<off2)
		out[outpos++] = in[off1++];
        }
    }
    out[outpos] = '\0';
    
    return out;
}


static int do_alias(t_connection * c, char const * cmd, char const * text)
{
    t_elem const *  elem1;
    t_elem const *  elem2;
    t_alias const * alias;
    t_output *      output;
    unsigned int *  offsets;
    unsigned int    numargs;
    
    
    
    LIST_TRAVERSE_CONST(aliaslist_head,elem1)
    {
	if (!(alias = elem_get_data(elem1)))
	    continue;
	
	LIST_TRAVERSE_CONST(alias->output,elem2)
	{
	    if (!(output = elem_get_data(elem2)))
		continue;
	    
	    if (!output->line || strcasecmp(output->line,cmd)!=0)
		continue;
	    
            {
		char const * msgtmp;
		
		if ((msgtmp = replace_args(output->line,offsets,numargs)))
		{
/* FIXME: add %C to start of line */
		    message_send_formatted(c,msgtmp);
		    free((void *)msgtmp); /* avoid warning */
		}
		else
		    eventlog(eventlog_level_error,"do_alias","could not perform argument replacement");
	    }
	    return 0;
	}
    }
    
    return -1;
}


extern int aliasfile_load(char const * filename)
{
    FILE *       afp;
    char *       buff;
    char *       temp;
    unsigned int line;
    unsigned int pos;
    int          inalias;
    
    if (!filename)
    {
	eventlog(eventlog_level_error,"aliasfile_load","got NULL filename");
	return -1;
    }
    if (!(afp = fopen(filename,"r")))
    {
	eventlog(eventlog_level_error,"aliasfile_load","unable to open alias file \"%s\" for reading (fopen: %s)",filename,strerror(errno));
	return -1;
    }
    
    inalias = 0;
    for (line=1; (buff = file_get_line(afp)); line++)
    {
	for (pos=0; buff[pos]=='\t' || buff[pos]==' '; pos++);
	if (buff[pos]=='\0' || buff[pos]=='#')
	{
	    free(buff);
	    continue;
	}
	if (!(temp = strrchr(buff,'"'))) /* FIXME: assumes comments don't contain " */
	    temp = buff;
	if ((temp = strrchr(temp,'#')))
	{
	    unsigned int len;
	    unsigned int endpos;
	    
	    *temp = '\0';
	    len = strlen(buff)+1;
	    for (endpos=len-1;  buff[endpos]=='\t' || buff[endpos]==' '; endpos--);
	    buff[endpos+1] = '\0';
	}
	
	switch (inalias)
	{
	case 0:
	    if (buff[pos]!='@') /* not start of alias */
	    {
		eventlog(eventlog_level_error,"aliasfile_load","expected start of alias stanza on line %u of alias file \"%s\" but found \"%s\"",line,filename,&buff[pos]);
		break;
	    }
	    inalias = 1;
	    break;
	
	case 1:
	    {
		unsigned int j;
		char         cmd[MAX_ALIAS_LEN];
		
		for (;;)
		{
		    for (; buff[pos]==' '; pos++);
		    for (j=0; buff[pos]!=' ' && buff[pos]!='\0'; pos++) /* get command */
			if (j<sizeof(cmd)-1) cmd[j++] = buff[pos];
		    cmd[j] = '\0';
    
		    if (cmd[0]=='\0')
			break;
		    
		    
		}
		inalias = 2;
		continue;
	    }
	    break;
	
	case 2:
	    if (buff[pos]!='[')
	    {
		eventlog(eventlog_level_error,"aliasfile_load","expected output entry on line %u of alias file \"%s\" but found \"%s\"",line,filename,&buff[pos]);
		break;
	    }
	}
	free(buff);
    }
    
    fclose(afp);
    return 0;
}


extern int aliasfile_unload(void)
{
    t_elem       *  elem1;
    t_elem       *  elem2;
    t_alias const * alias;
    t_output *      output;
    
    if (aliaslist_head)
    {
	LIST_TRAVERSE(aliaslist_head,elem1)
	{
	    if (!(alias = elem_get_data(elem1))) /* should not happen */
	    {
		eventlog(eventlog_level_error,"aliasfile_unload","alias list contains NULL item");
		continue;
	    }
	    
	    if (list_remove_elem(aliaslist_head,elem1)<0)
	    {
	        eventlog(eventlog_level_error,"aliasfile_unload","could not remove alias");
		continue;
	    }
	    if (alias->output)
	    {
		LIST_TRAVERSE(alias->output,elem2)
		{
		    if (!(output = elem_get_data(elem2)))
		    {
			eventlog(eventlog_level_error,"aliasfile_unload","output list contains NULL item");
			continue;
		    }
		    
		    if (list_remove_elem(alias->output,elem2)<0)
		    {
		        eventlog(eventlog_level_error,"aliasfile_unload","could not remove output");
			continue;
		    }
		    free((void *)output->line); /* avoid warning */
		    free(output);
		}
	    }
	}
	
	if (list_destroy(aliaslist_head)<0)
	    return -1;
	aliaslist_head = NULL;
    }
    
    return 0;
}


extern int handle_alias_command(t_connection * c, char const * text)
{
    unsigned int i,j;
    char         cmd[MAX_COMMAND_LEN];
    
    for (i=j=0; text[i]!=' ' && text[i]!='\0'; i++) /* get command */
        if (j<sizeof(cmd)-1) cmd[j++] = text[i];
    cmd[j] = '\0';
    
    if (cmd[0]=='\0')
	return list_aliases(c);
    
    if (do_alias(c,cmd,text)<0)
    {
	message_send_text(c,message_type_info,c,"No such alias.  Use // to show the list.");
	return -1;
    }
    return 0;
}
