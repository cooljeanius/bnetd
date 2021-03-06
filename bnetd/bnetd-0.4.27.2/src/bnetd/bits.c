/*
 * Copyright (C) 1999,2000,2001  Marco Ziech (mmz@gmx.net)
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
#define CONNECTION_INTERNAL_ACCESS
#include "common/setup_before.h"
#ifdef WITH_BITS
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
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
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
#include "compat/strdup.h"
#include <errno.h>
#include "compat/strerror.h"
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#include "compat/socket.h"
#include "compat/send.h"
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#include "compat/netinet_in.h"
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#include "compat/inet_ntoa.h"
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#include "compat/uint.h"
#include "compat/psock.h"
#include "common/packet.h"
#include "common/init_protocol.h"
#include "connection.h"
#include "channel.h"
#include "common/addr.h"
#include "common/eventlog.h"
#include "common/queue.h"
#include "common/network.h"
#include "prefs.h"
#include "account.h"
#include "common/list.h"
#include "server.h"
#include "common/bn_type.h"
#include "account.h"
#include "message.h"
#include "bits.h"
#include "handle_bits.h"
#include "bits_query.h"
#include "bits_packet.h"
#include "bits_va.h"
#include "bits_net.h"
#include "bits_ext.h"
#include "bits_rconn.h"
#include "bits_chat.h"
#include "bits_game.h"
#include "bits_motd.h"
#include "bits_login.h"
#include "query.h"
#include "tick.h"
#include "common/bnethash.h"
#include "common/bnethashconv.h"
#include "common/version.h"
#include "common/util.h"
#include "timer.h"
#include "common/setup_after.h"

t_connection	* bits_uplink_connection = NULL;
t_connection	* bits_loopback_connection = NULL;
t_list * bits_hostlist = NULL;
int bits_debug = 1;

int bits_master = -1;

extern void bits_ping_timer(t_connection * c, time_t now, t_timer_data data)
{
    if (!c)
    {
        eventlog(eventlog_level_error,"bits_ping_timer","got NULL connection");
        return;
    }

    if (now==(time_t)0) /* zero means user logged out before expiration */
	return;

    if (conn_get_state(c)==conn_state_loggedin) {
	t_query * q;

	q = query_create(bits_query_type_ping);
	query_attach_conn(q,"client",c);
	query_attach_num(q,"addr",bits_ext_get_peeraddr(c));
	query_attach_num(q,"ticks",get_ticks());	
	send_bits_net_ping(bits_ext_get_peeraddr(c),query_get_qid(q)); /* FIXME: */
    }
    if (timerlist_add_timer(c,now+(time_t)prefs_get_bits_ping_interval(),bits_ping_timer,data)<0)
	eventlog(eventlog_level_error,"bits_ping_timer","could not add timer");
}

extern unsigned short bits_new_dyn_addr(void) {
    static unsigned short current_addr = BITS_ADDR_DYNAMIC_START;
    t_elem const * curr;
    int bad = 1;
    int counter = 0;

    if (current_addr < BITS_ADDR_DYNAMIC_START)
	current_addr = BITS_ADDR_DYNAMIC_START;
    

    while (bad && (counter < 4096)) {
	bad = 0;
	counter++;
	LIST_TRAVERSE_CONST(bits_hostlist,curr) {
	    t_bits_hostlist_entry const * e = elem_get_data(curr);

	    if (e->address==current_addr) {
		current_addr++;
		if (current_addr < BITS_ADDR_DYNAMIC_START)
		    current_addr = BITS_ADDR_DYNAMIC_START;
		bad = 1;
		break;
	    }
	}
    }

    if (bad)
	eventlog(eventlog_level_fatal,"bits_new_dyn_addr","THERE ARE NO FREE BITS ADDRESSES LEFT!!!");

    return current_addr;
}

extern int bits_hostlist_add(t_uint16 addr, char const * name, unsigned int ip, unsigned short port)
{
    t_bits_hostlist_entry * e;

    if (!name) {
	eventlog(eventlog_level_error,"bits_hostlist_add","got NULL name");
	return -1;
    }
    if (bits_hostlist_find(addr)) {
	eventlog(eventlog_level_error,"bits_hostlist_add","host 0x%04x already exists",addr);
	return -1;
    }
    if (!(e = malloc(sizeof(t_bits_hostlist_entry)))) {
	eventlog(eventlog_level_error,"bits_hostlist_add","could not allocate memory: %s",strerror(errno));
	return -1;
    }
    e->address = addr;
    e->ip = ip;
    e->port = port;
    e->name = strdup(name);
    if (!e->name) {
	eventlog(eventlog_level_error,"bits_hostlist_add","could not allocate memory for name: %s",strerror(errno));
	free(e);
	return -1;
    }
    list_append_data(bits_hostlist,e);
    return 0;
}

extern int bits_hostlist_del(t_uint16 addr)
{
    t_bits_hostlist_entry * e;

    e = bits_hostlist_find(addr);
    if (e) {
	list_remove_data(bits_hostlist,e);
	list_purge(bits_hostlist);
	return 0;
    } else {
	eventlog(eventlog_level_error,"bits_hostlist_del","could not find entry for 0x%04x",addr);
	return -1;
    }
}

extern t_bits_hostlist_entry * bits_hostlist_find(t_uint16 addr)
{
    t_elem * curr;

    LIST_TRAVERSE(bits_hostlist,curr) {
	t_bits_hostlist_entry * e = elem_get_data(curr);

	if (e->address == addr) {
	    return e;
	}
    }
    return NULL;
}

extern t_bits_auth_password * bits_get_user_password_from_file(char const *user)
{
	FILE * passwd_file;
	int lineno = 0;
	const char * filename;
	char * obuf;
	static t_bits_auth_password pass;
	
	eventlog(eventlog_level_debug,"bits_get_user_password_from_file","getting password of user [%s]",user);
	filename = prefs_get_bits_password_file();
	if (filename == NULL) {
		eventlog(eventlog_level_error,"bits_get_user_password_from_file","password filename is NULL. Make sure you have specified \"bits_password_file\".");
		return NULL;
	}
	passwd_file = fopen( filename, "r" );
	if (!passwd_file) {
		eventlog(eventlog_level_error,"bits_get_user_password_from_file","can't open file [%s]: %s",prefs_get_bits_password_file(),strerror(errno));
		return NULL;
	}
	while ((obuf = file_get_line(passwd_file))) {
		char * tuser;
		char * ttype;
		char * tpass;
		char * tnext; /* for compatibility */
		char * buf = obuf;
		
		lineno++;
		while ((buf[0]==' ')||(buf[0]=='\t')) buf++;
		if (buf[0]=='\n' || buf[0]=='\0' || buf[0]=='#') {
		    free(obuf);
		    continue;
		}
		tuser = buf;
		ttype = strchr(tuser,':');
		if (!ttype) {
			eventlog(eventlog_level_warn,"bits_get_user_password_from_file","%s: parse error in line %d",filename,lineno);
			free(obuf); continue;
		}
		*ttype = '\0'; ttype++;
		tpass = strchr(ttype,':');
		if (!tpass) {
			eventlog(eventlog_level_warn,"bits_get_user_password_from_file","%s: parse error in line %d",filename,lineno);
			free(obuf); continue;
		}
		*tpass='\0'; tpass++;
		if ((tnext = strchr(tpass,':'))) *tnext = '\0'; /* ignore extra fields */

		if (strcmp(tuser,user)==0) {
			fclose(passwd_file);
			if (ttype[0]=='P') {
			    pass.is_plain=1;
			    strncpy(pass.plain,tpass,sizeof(pass.plain));
			    bnet_hash(&pass.hash,strlen(tpass),tpass);
			} else if (ttype[0]=='H') {
			    pass.is_plain=0;
			    pass.plain[0]='\0';
			    hash_set_str(&pass.hash,tpass);
			} else {
			    eventlog(eventlog_level_warn,"bits_get_user_password_from_file","user found but has unrecognized type \"%s\"",ttype);
			    free(obuf);
			    return NULL;
			}
			free(obuf);
			return &pass;
		}
	}
	fclose(passwd_file);
	return NULL;
}

extern t_uint16 bits_get_myaddr(void)
{
	if (bits_uplink_connection) 
		if (bits_uplink_connection->bits)
			return bits_uplink_connection->bits->myaddr;
	return BITS_ADDR_MASTER;
}

extern int bits_auth_bnethash_second(t_hash * outhash, t_hash const inhash, t_uint32 sessionkey, t_uint32 ticks)
{
    t_bits_auth_bnethash temp;

    if (!outhash) {
	eventlog(eventlog_level_error,"bits_auth_bnethash_second","got NULL outhash");
	return -1;
    }
    memset(&temp,0,sizeof(temp));
    memset(outhash,0,sizeof(t_hash));
    bn_int_set(&temp.ticks,ticks);
    bn_int_set(&temp.sessionkey,sessionkey);
    hash_to_bnhash((t_hash const *)inhash,temp.passhash);
    if (bnet_hash(outhash,sizeof(temp),&temp)<0) {
	eventlog(eventlog_level_error,"bits_auth_bnethash_second","bnet_hash failed");
	return -1;
    }
    return 0;
}

extern int bits_auth_bnethash_eq(unsigned int sessionkey, t_hash myhash, t_hash tryhash, t_uint32 ticks)
{
    t_hash myhash2;

    bits_auth_bnethash_second(&myhash2,myhash,sessionkey,ticks);
    /*eventlog(eventlog_level_debug,"bits_auth_bnethash_eq","ticks=0x%08x sessionkey=0x%08x hash=\"%s\"",ticks,sessionkey,hash_get_str(myhash2));*/
    /*eventlog(eventlog_level_debug,"bits_auth_bnethash_eq","tryhash=\"%s\"",hash_get_str(tryhash));*/
    return hash_eq(myhash2,tryhash);
}

extern int send_bits_auth_request(t_connection * c, const char *name,const char *password)
{
    t_hash h1,h2;
    t_uint32 ticks;
    t_packet	*p;

    if (!c) {
	eventlog(eventlog_level_error,"bits_send_authreq","got NULL connection");
	return -1;
    }
    if (!name) {
	eventlog(eventlog_level_error,"bits_send_authreq","got NULL name");
	return -1;
    }
    if (!password) {
	eventlog(eventlog_level_error,"bits_send_authreq","got NULL password");
	return -1;
    }
    p = packet_create(packet_class_bits);
    if (!p) {
	eventlog(eventlog_level_error,"bits_send_authreq","could not create packet");
	return -1;
    }
    packet_set_size(p, sizeof(t_bits_auth_request));
    packet_set_type(p, BITS_AUTH_REQUEST);
    bits_packet_generic(p,BITS_ADDR_PEER);
    packet_append_string(p,name);

    /* first hash */
    bnet_hash(&h1,strlen(password),password);
    /* prepare second hash */
    ticks = time(NULL); /* hope this works on any system */
    /* second hash */
    bits_auth_bnethash_second(&h2,h1,conn_get_sessionkey(bits_uplink_connection),ticks);

    bn_int_set(&p->u.bits_auth_request.ticks,ticks);
    hash_to_bnhash((t_hash const *)&h2,p->u.bits_auth_request.passhash);
    /*eventlog(eventlog_level_debug,"send_bits_auth_request","ticks=0x%08x hash=\"%s\"",ticks,hash_get_str(h2));*/

    queue_push_packet(conn_get_out_queue(c),p);
    packet_del_ref(p);
    return 0;
}

extern int send_bits_auth_reply(t_connection * c, unsigned char status, t_uint16 address)
{
    t_packet	*p;

    if (!(p = packet_create(packet_class_bits))) {
	eventlog(eventlog_level_error,"bits_send_authreply","could not create packet");
	return -1;
    }

    packet_set_size(p,sizeof(t_bits_auth_reply));
    packet_set_type(p,BITS_AUTH_REPLY);
    bits_packet_generic(p,BITS_ADDR_PEER);
	
    bn_byte_set(&p->u.bits_auth_reply.status,status);
    bn_short_set(&p->u.bits_auth_reply.address,address);	
    queue_push_packet(conn_get_out_queue(c),p);
    packet_del_ref(p);
    return 0;
}

extern int bits_check_bits_master_auth_request(t_packet const * packet)
{
    char const *username;
    t_bits_auth_password *pass;
    int passwordok = 0;

    username = packet_get_str_const(packet,sizeof(t_bits_master_auth_request),1024); /* FIXME: 1024 should be a constant */
    pass = bits_get_user_password_from_file(username);
    if ((!username)) {
	eventlog(eventlog_level_error,"bits_check_bits_master_auth_request","got malformed BITS_AUTH_REQUEST packet (username missing)");
    } else if (!pass) {
	eventlog(eventlog_level_error,"bits_check_bits_master_auth_request","failed to find password for user \"%s\"",username);
    } else {
	t_hash h;
	unsigned int sk;
	unsigned int ticks;

	sk = bn_int_get(packet->u.bits_master_auth_request.sessionkey);
	ticks = bn_int_get(packet->u.bits_master_auth_request.ticks);
	bnhash_to_hash(packet->u.bits_master_auth_request.passhash,&h);
	if (bits_auth_bnethash_eq(sk,pass->hash,h,ticks))
	    passwordok=1;
    }
    return passwordok;
}

extern void bits_setup_client(t_connection * conn)
{
    t_timer_data data; /* dummy */

    if (!conn) {
	eventlog(eventlog_level_error,"bits_setup_client","got NULL connection");
	return;
    }
    /* send all lists */
    send_bits_net_motd(conn);
    bits_chat_channellist_send_all(conn);
    bits_va_loginlist_sendall(conn);
    bits_game_sendlist(conn);
    /* ready */
    eventlog(eventlog_level_debug,"bits_setup_client","all lists uploaded to new client");
    /* setup timers */
    if (timerlist_add_timer(conn,time(NULL)+(time_t)prefs_get_bits_ping_interval(),bits_ping_timer,data)<0)
	eventlog(eventlog_level_error,"bits_setup_client","could not add ping timer");
}

extern int bits_status_ok(const bn_byte status_bn)
{
	unsigned char status = bn_byte_get(status_bn);
	if (status == BITS_STATUS_OK) return 1;
	if (status == BITS_STATUS_NOAUTH) return 1;
	
	return 0;
}

static int bits_create_loopback_connection(void)
{
	bits_loopback_connection = malloc(sizeof(t_connection));
	if (!bits_loopback_connection) {
		eventlog(eventlog_level_error,"bits_create_loopback_connection","malloc failed: %s",strerror(errno));
		return -1;
	}
	memset(bits_loopback_connection,0,sizeof(t_connection)); /* clear allocated space */
 	conn_set_class(bits_loopback_connection,conn_class_bits);
 	conn_set_state(bits_loopback_connection,conn_state_loggedin);
	create_bits_ext(bits_loopback_connection,bits_to_slave);
	return 0;
}

extern int bits_init(void)
{
    t_timer_data data; /* dummy */
    
	bits_routing_table = list_create();
	bits_create_loopback_connection();
	bits_debug = prefs_get_bits_debug();	
	if (prefs_get_do_uplink()) {
		/* FIXME: use bnetd/connection functions instead of psock ? */
		struct sockaddr_in	saddr;
		t_addr * addr;
		int sock = psock_socket(PSOCK_PF_INET,PSOCK_SOCK_STREAM,PSOCK_IPPROTO_TCP);

		bits_master = 0;
		eventlog(eventlog_level_info,"bits_init","Running as uplink client.");		
		if (sock<0) {
			eventlog(eventlog_level_error,"bits_init","socket failed: %s",strerror(errno));
			return -1;
		}
		/* Hostname lookup */
		saddr.sin_family = PSOCK_AF_INET;
		eventlog(eventlog_level_info,"bits_init","looking up [%s]",prefs_get_uplink_server());
		addr = addr_create_str(prefs_get_uplink_server(),0,6112);
		saddr.sin_port = htons(addr_get_port(addr));
		saddr.sin_addr.s_addr = htonl(addr_get_ip(addr));
		/* Connect */
		eventlog(eventlog_level_info,"bits_init","connecting to [%s:%u]",inet_ntoa(saddr.sin_addr),addr_get_port(addr));
		if (psock_connect(sock,(struct sockaddr *) &saddr,sizeof(struct sockaddr_in))<0) {
			eventlog(eventlog_level_error,"bits_init","connect failed: %s",strerror(errno));
			return -1;
		}
		/* send magic */
		{
			bn_short magic;
			bn_short_set(&magic,CLIENT_INITCONN_CLASS_BITS);
			eventlog(eventlog_level_info,"bits_init","sending magic");
			if (psock_send(sock,&magic,1,0) != 1) {
				eventlog(eventlog_level_error,"bits_init","send  failed: %s",strerror(errno));
				return -1;
			} 
		}
		/* init uplink struct */
		bits_uplink_connection = conn_create(sock,0,0,0,0,0,0,addr_get_port(addr));	
   	 	conn_set_class(bits_uplink_connection,conn_class_bits);
   	 	conn_set_state(bits_uplink_connection,conn_state_connected);
   	 	if (create_bits_ext(bits_uplink_connection,bits_to_master)<0) {
			eventlog(eventlog_level_error,"bits_init","cannot create bits connection extension for master uplink");
			return -1;
   	 	}
		/* Session request */
		{
			t_packet *p;
			int rc;
			unsigned int currsize = 0;

			p = packet_create(packet_class_bits);
			if (!p) {
				eventlog(eventlog_level_error,"init_uplink","could not create packet");
				return -1;
			}
			packet_set_size(p,sizeof(t_bits_session_request));
			packet_set_type(p,BITS_SESSION_REQUEST);
			bits_uplink_connection->clienttag = strdup(BITS_CLIENTTAG);
			memcpy(&p->u.bits_session_request.protocoltag,BITS_PROTOCOLTAG,4);
			memcpy(&p->u.bits_session_request.clienttag,BITS_CLIENTTAG,4);
			/* FIXME: This stuff is probably deprecated */
			bn_short_set(&p->u.bits_session_request.h.src_addr,0);
			bn_short_set(&p->u.bits_session_request.h.dst_addr,0);
			bn_byte_set(&p->u.bits_session_request.h.ttl,BITS_DEFAULT_TTL);			
			packet_append_string(p,BNETD_VERSION);
			queue_push_packet(conn_get_out_queue(bits_uplink_connection),p);
			packet_del_ref(p);
			while ((rc = net_send_packet(sock,queue_pull_packet(conn_get_out_queue(bits_uplink_connection)),&currsize)) == 0)
			if (rc < 0)
			{
				eventlog(eventlog_level_error,"bits_init","net_send_packet failed");
			}
			packet_del_ref(p); /* really delete it ...*/
		}
		/* Wait for session reply */
		{
			t_packet * reply = packet_create(packet_class_bits);
			unsigned int currsize = 0;
			int rc;
			
			eventlog(eventlog_level_debug,"bits_init","Session request sent. Waiting for reply ...");
			while ((rc = net_recv_packet(sock,reply,&currsize))==0);
			if (rc<0) 
			{
				eventlog(eventlog_level_error,"bits_init","net_recv_packet failed");
			}
			handle_bits_packet(bits_uplink_connection,reply);
			if (conn_get_state(bits_uplink_connection)!=conn_state_bits_auth) {
				return -1;
			}	
			packet_del_ref(reply); /* delete reply ... */
		}
		/* Send login */
		{
			char const *username;
			t_bits_auth_password *pass;
			unsigned int currsize = 0;
			int rc;
			t_packet * p = NULL;
			
			eventlog(eventlog_level_info,"bits_init","sending login information (user=[%s])...",prefs_get_uplink_username());
			username = prefs_get_uplink_username();
			pass = bits_get_user_password_from_file(username);
			if (pass == NULL) {
				eventlog(eventlog_level_error,"bits_init","Password lookup failed. Either the user is not in the password file or the password file cannot be opened.");
				psock_close(sock);
				return -1;
			}
			if (!pass->is_plain) {
			    eventlog(eventlog_level_error,"bits_init","password must be saved in plain text on bits clients");
			    psock_close(sock);
			    return -1;
			}
			if (send_bits_auth_request(bits_uplink_connection,username,pass->plain)<0) {
				eventlog(eventlog_level_error,"bits_init","failed to send authreq");
				psock_close(sock);
				return -1;
			}

			currsize = 0;
			while ((rc = net_send_packet(sock,(p = queue_pull_packet(conn_get_out_queue(bits_uplink_connection))),&currsize)) == 0)
				packet_del_ref(p);
			if (rc < 0)
			{
				eventlog(eventlog_level_error,"bits_init","net_send_packet failed");
			}
			if (p) packet_del_ref(p);
			/* FIXME: This code should probably be rewritten ... */
		}
		/* Wait for reply */
		{
			t_packet * reply = packet_create(packet_class_bits);
			unsigned int currsize = 0;
			int rc;
			
			while ((rc = net_recv_packet(sock,reply,&currsize))==0);
			if (rc<0) 
			{
				eventlog(eventlog_level_error,"bits_init","net_recv_packet failed");
			}
			handle_bits_packet(bits_uplink_connection,reply);
			if (conn_get_state(bits_uplink_connection)!=conn_state_loggedin) {
				return -1;
			}
			packet_del_ref(reply); /* delete reply ... */	
		}
		addr_destroy(addr);
		/* routing stuff */
		bits_net_send_discover();
		/* --- */
		bits_gamelist_create();
		/* --- */
		bits_hostlist = NULL;
		/* --- */
		if (timerlist_add_timer(bits_uplink_connection,time(NULL)+(time_t)prefs_get_bits_ping_interval(),bits_ping_timer,data)<0)
		    eventlog(eventlog_level_error,"bits_init","could not add ping timer");
		/* --- */
		eventlog(eventlog_level_info,"bits_init","BITS uplink initialized");
		return 0; /* right value? */
	} else {
		bits_gamelist_create();
		bits_motd_load_from_file();
		bits_uplink_connection = NULL;
		bits_master = 1;
		bits_hostlist = list_create();
		eventlog(eventlog_level_info,"bits_init","Running as uplink master server");
		return 0;
	}
}

extern int bits_destroy(void)
{
    t_elem * curr;

    eventlog(eventlog_level_debug,"bits_destroy","shutting down BITS");
    /* FIXME: Destroy loginlists, connections, etc.*/
    bits_motd_destroy();
    if (bits_uplink_connection)
	destroy_bits_ext(bits_uplink_connection);
    free(bits_loopback_connection);
    bits_gamelist_destroy();

    LIST_TRAVERSE(bits_routing_table,curr)
    	list_remove_elem(bits_routing_table,curr);
    list_purge(bits_routing_table);
    list_destroy(bits_routing_table);

    if (bits_hostlist) {
	LIST_TRAVERSE(bits_hostlist,curr)
	    list_remove_elem(bits_hostlist,curr);
	list_purge(bits_hostlist);
	list_destroy(bits_hostlist);
    }
    eventlog(eventlog_level_info,"bits_destroy","BITS shut down");
    return 0;
}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif /* WITH_BITS */
