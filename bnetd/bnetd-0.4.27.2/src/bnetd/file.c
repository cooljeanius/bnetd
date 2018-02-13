/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "compat/strchr.h"
#include <errno.h>
#include "compat/strerror.h"
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include "common/bn_type.h"
#include "common/queue.h"
#include "connection.h"
#include "common/packet.h"
#include "common/file_protocol.h"
#include "common/eventlog.h"
#include "prefs.h"
#include "common/bnettime.h"
#include "file.h"
#include "common/setup_after.h"


static char const * file_get_info(char const * rawname, unsigned int * len, bn_long * modtime);


static char const * file_get_info(char const * rawname, unsigned int * len, bn_long * modtime)
{
    char *       filename;
    struct stat  sfile;
    t_bnettime   bt;

    if (!rawname)
    {
	eventlog(eventlog_level_error,"file_get_info","got NULL rawname");
	return NULL;
    }
    if (!len)
    {
	eventlog(eventlog_level_error,"file_get_info","got NULL len");
	return NULL;
    }
    if (!modtime)
    {
	eventlog(eventlog_level_error,"file_get_info","got NULL modtime");
	return NULL;
    }

    if (strchr(rawname,'/'))
    {
	eventlog(eventlog_level_warn,"file_get_info","got rawname containing '/' \"%s\"",rawname);
	return NULL;
    }
    if (!(filename = malloc(strlen(prefs_get_filedir())+1+strlen(rawname)+1)))
    {
	eventlog(eventlog_level_error,"file_get_info","could not allocate memory for filename");
	return NULL;
    }
    sprintf(filename,"%s/%s",prefs_get_filedir(),rawname);

    if (stat(filename,&sfile)<0) /* doesn't exist */
    {
	/* FIXME: check for lower-case version of filename */
	free(filename);
       	/* old times are better to make client less likely to request missing files */
	bt = time_to_bnettime(0,0);
	bnettime_to_bn_long(bt,modtime);
	return NULL;
    }

    *len = (unsigned int)sfile.st_size;
    bt = time_to_bnettime(sfile.st_mtime,0);
    bnettime_to_bn_long(bt,modtime);

    return filename;
}


extern int file_to_mod_time(char const * rawname, bn_long * modtime)
{
    char const * filename;
    unsigned int len;

    if (!rawname)
    {
	eventlog(eventlog_level_error,"file_to_mod_time","got NULL rawname");
	return -1;
    }
    if (!modtime)
    {
	eventlog(eventlog_level_error,"file_to_mod_time","got NULL modtime");
	return -1;
    }

    if (!(filename = file_get_info(rawname, &len, modtime)))
	return -1;

    free((void *)filename); /* avoid warning */

    return 0;
}


/* Send a file.  If the file doesn't exist we still need to respond
 * to the file request.  This will set filelen to 0 and send the server
 * reply message and the client will be happy and not hang.
 */
extern int file_send(t_connection * c, char const * rawname, unsigned int adid, unsigned int etag, unsigned int startoffset, int need_header)
{
    char const * filename;
    t_packet *   rpacket;
    FILE *       fp;
    unsigned int filelen;
    int          nbytes;

    if (!c)
    {
	eventlog(eventlog_level_error,"file_send","got NULL connection");
	return -1;
    }
    if (!rawname)
    {
	eventlog(eventlog_level_error,"file_send","got NULL rawname");
	return -1;
    }

    if (!(rpacket = packet_create(packet_class_file)))
    {
	eventlog(eventlog_level_error,"file_send","could not create file packet");
	return -1;
    }
    packet_set_size(rpacket,sizeof(t_server_file_reply));
    packet_set_type(rpacket,SERVER_FILE_REPLY);

    if ((filename = file_get_info(rawname,&filelen,&rpacket->u.server_file_reply.timestamp)))
    {
	if (!(fp = fopen(filename,"rb")))
	{
	    /* FIXME: check for lower-case version of filename */
	    eventlog(eventlog_level_error,"file_send","stat() succeeded yet could not open file \"%s\" for reading (fclose: %s)",filename,strerror(errno));
	    filelen = 0;
	}
	free((void *)filename); /* avoid warning */
    }
    else
    {
	fp = NULL;
	filelen = 0;
	bn_long_set_a_b(&rpacket->u.server_file_reply.timestamp,0,0);
    }

    if (fp)
    {
	if (startoffset<filelen) {
	    fseek(fp,startoffset,SEEK_SET);
	} else {
	    eventlog(eventlog_level_warn,"file_send","[%d] startoffset is beyond end of file (%u>%u)",conn_get_socket(c),startoffset,filelen);
	    /* Keep the real filesize. Battle.net does it the same way ... */
	    fclose(fp);
	    fp = NULL;
	}
    }

    if (need_header)
    {
	/* send the header from the server with the rawname and length. */
	bn_int_set(&rpacket->u.server_file_reply.filelen,filelen);
	bn_int_set(&rpacket->u.server_file_reply.adid,adid);
	bn_int_set(&rpacket->u.server_file_reply.extensiontag,etag);
	/* rpacket->u.server_file_reply.timestamp is set above */
	packet_append_string(rpacket,rawname);
	queue_push_packet(conn_get_out_queue(c),rpacket);
    }
    packet_del_ref(rpacket);

    /* Now send the data. Since it may be longer than a packet; we use
     * the raw packet class.
     */
    if (!fp)
    {
	eventlog(eventlog_level_warn,"file_send","[%d] sending no data for file \"%s\"",conn_get_socket(c),rawname);
	return -1;
    }

    eventlog(eventlog_level_info,"file_send","[%d] sending file \"%s\" of length %d",conn_get_socket(c),rawname,filelen);
    for (;;)
    {
	if (!(rpacket = packet_create(packet_class_raw)))
	{
	    eventlog(eventlog_level_error,"file_send","could not create raw packet");
	    if (fclose(fp)<0)
		eventlog(eventlog_level_error,"file_send","could not close file \"%s\" after reading (fclose: %s)",rawname,strerror(errno));
	    return -1;
	}
	if ((nbytes = fread(packet_get_raw_data_build(rpacket,0),1,MAX_PACKET_SIZE,fp))<(int)MAX_PACKET_SIZE)
	{
	    if (nbytes>0) /* send out last portion */
	    {
		packet_set_size(rpacket,nbytes);
		queue_push_packet(conn_get_out_queue(c),rpacket);
	    }
	    packet_del_ref(rpacket);
	    if (ferror(fp))
		eventlog(eventlog_level_error,"file_send","read failed before EOF on file \"%s\" (fread: %s)",rawname,strerror(errno));
	    break;
	}
	packet_set_size(rpacket,nbytes);
	queue_push_packet(conn_get_out_queue(c),rpacket);
	packet_del_ref(rpacket);
    }

    if (fclose(fp)<0)
	eventlog(eventlog_level_error,"file_send","could not close file \"%s\" after reading (fclose: %s)",rawname,strerror(errno));
    return 0;
}
