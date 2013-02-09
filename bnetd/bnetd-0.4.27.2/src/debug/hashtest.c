/*
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
#include "common/setup_before.h"
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
#endif
#include "compat/exitstatus.h"
#include <stdio.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include <ctype.h>
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif
#include "compat/char_bit.h"
#include "common/eventlog.h"
#include "common/introtate.h"
#include "common/setup_after.h"


static unsigned int account_hash(char const * username)
{
    unsigned int i;
    unsigned int pos;
    unsigned int hash;
    unsigned int ch;
    
    if (!username)
    {
        eventlog(eventlog_level_error,"account_hash","got NULL username");
        return 0;
    }
    
#if 1
    hash = (strlen(username)+1)*120343021;
    for (pos=0,i=0; i<strlen(username); i++)
    {
        if (isascii((int)username[i]) && isupper((int)username[i]))
            ch = (unsigned int)(unsigned char)tolower((int)username[i]);
        else
            ch = (unsigned int)(unsigned char)username[i];
        hash ^= ROTL(ch,pos,sizeof(unsigned int)*CHAR_BIT);
        hash ^= ROTL((i+1)*314159,ch,sizeof(unsigned int)*CHAR_BIT);
        pos += CHAR_BIT-1;
    }
#else
    hash = (strlen(username)+1)*120343021;
    for (pos=0,i=0; i<strlen(username); i++)
    {
        if (isascii((int)username[i]) && isupper((int)username[i]))
            ch = (unsigned int)(unsigned char)tolower((int)username[i]);
        else
            ch = (unsigned int)(unsigned char)username[i];
        hash ^= ROTL(ch,pos,sizeof(unsigned int)*CHAR_BIT);
        hash = ROTL(hash,ch,sizeof(unsigned int)*CHAR_BIT);
        pos += CHAR_BIT-1;
    }
#endif
    
    return hash;
}


extern int main(int argc, char * argv[])
{
    char         buff[256];
    unsigned int val;

    if (argc<1 || !argv || !argv[0])
    {
	fprintf(stderr,"bad arguments\n");
	return STATUS_FAILURE;
    }
    
    eventlog_set(stderr);
#ifdef USE_CHECK_ALLOC
    check_set_file(stderr);
#endif
    
    while (fgets(buff,256,stdin))
    {
	if (buff[0]!='\0' && buff[strlen(buff)-1]=='\n')
	    buff[strlen(buff)-1] = '\0';
	val = account_hash(buff);
	printf("%011o %010u 0x%08x  ",val,val,val);
	printf("%03u %03u %03u %03u\n",val%512,val%128,val%64,val%16);
    }
    /*
     * run the output through something like:
     * awk '{ print $4 }' z3 | sort -n | uniq -c | sort -n
     */
    
#ifdef USE_CHECK_ALLOC
    check_cleanup();
#endif
    return STATUS_SUCCESS;
}
