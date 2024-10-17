/*
 * Copyright (C) 1999,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2003       Ilyak Kasnacheev (ilyak@online.ru)
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
#endif
#include "compat/exitstatus.h"
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#ifdef HAVE_MEMORY_H
# include <memory.h>
#endif
#include "compat/memset.h"
#include "compat/memcpy.h"
#include <ctype.h>
#include <errno.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include "compat/strerror.h"
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include "compat/strftime.h"
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#include "compat/termios.h"
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
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include "compat/psock.h"
#include "common/packet.h"
#include "common/init_protocol.h"
#include "common/file_protocol.h"
#include "common/tag.h"
#include "common/bn_type.h"
#include "common/field_sizes.h"
#include "common/network.h"
#include "common/version.h"
#include "common/util.h"
#include "common/bnettime.h"
#ifdef CLIENTDEBUG
#include "common/eventlog.h"
#endif
#include "common/hexdump.h"
#include "client.h"
#include "common/setup_after.h"


static void usage(char const * progname);


static void usage(char const * progname)
{
    fprintf(stderr,"usage: %s [<options>] [<servername> [<TCP portnumber>]]\n",progname);
    fprintf(stderr,
            "    -b, --client=SEXP           report client as Brood Wars\n"
            "    -d, --client=DRTL           report client as Diablo Retail\n"
            "    --client=DSHR               report client as Diablo Shareware\n"
            "    -s, --client=STAR           report client as Starcraft (default)\n");
    fprintf(stderr,
	    "    --client=SSHR               report client as Starcraft Shareware\n"
	    "    -w, --client=W2BN           report client as Warcraft II BNE\n"
            "    --client=D2DV               report client as Diablo II\n"
            "    --client=D2XP               report client as Diablo II: LoD\n"
            "    --client=WAR3               report client as Warcraft III\n"
            "    --hexdump=FILE              do hex dump of packets into FILE\n");
    fprintf(stderr,
	    "    --arch=IX86                 report client arch as win32/x86 (default)\n"
	    "    --arch=PMAC                 report client arch as macos9/PPC\n"
            "    --arch=XMAC                 report client arch as macosX/PPC\n");
    fprintf(stderr,
	    "    --startoffset=OFFSET        force offset to be OFFSET\n"
	    "    --exists=ACTION             Ask/Overwrite/Backup/Resume if the file exists\n"
	    "    --file=FILENAME             use FILENAME instead of asking\n"
            "    -h, --help, --usage         show this information and exit\n"
            "    -v, --version               print version number and exit\n");
    exit(STATUS_FAILURE);
}


extern int main(int argc, char * argv[])
{
    int                a;
    int                sd;
    struct sockaddr_in saddr;
    t_packet *         packet;
    t_packet *         rpacket;
    t_packet *         fpacket;
    char const *       clienttag=NULL;
    char const *       archtag=NULL;
    char const *       servname=NULL;
    unsigned short     servport=0;
    char const *       hexfile=NULL;
    char               text[MAX_MESSAGE_LEN];
    char const *       reqfile=NULL;
    struct hostent *   host;
    unsigned int       commpos;
    struct termios     in_attr_old;
    struct termios     in_attr_new;
    int                changed_in;
    unsigned int       currsize;
    unsigned int       filelen;
    unsigned int       startoffset;
    int                startoffsetoverride=0;
#define EXIST_ACTION_UNSPEC    -1
#define EXIST_ACTION_ASK        0
#define EXIST_ACTION_OVERWRITE  1
#define EXIST_ACTION_BACKUP     2
#define EXIST_ACTION_RESUME     3
    int		       exist_action=EXIST_ACTION_UNSPEC;
    struct stat        exist_buf;
    char const *       filename;
    FILE *             fp;
    FILE *             hexstrm=NULL;
    int                fd_stdin;
    t_bnettime         bntime;
    time_t             tm;
    char               timestr[FILE_TIME_MAXLEN];
    unsigned int       screen_width,screen_height;
    int                munged;

    if (argc<1 || !argv || !argv[0])
    {
	fprintf(stderr,"bad arguments\n");
	return STATUS_FAILURE;
    }

    for (a=1; a<argc; a++)
	if (servname && isdigit((int)argv[a][0]) && a+1>=argc)
	{
            if (str_to_ushort(argv[a],&servport)<0)
            {
                fprintf(stderr,"%s: \"%s\" should be a positive integer\n",argv[0],argv[a]);
                usage(argv[0]);
            }
	}
	else if (!servname && argv[a][0]!='-' && a+2>=argc)
	    servname = argv[a];
        else if (strcmp(argv[a],"-b")==0 || strcmp(argv[a],"--client=SEXP")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_BROODWARS;
        }
        else if (strcmp(argv[a],"-d")==0 || strcmp(argv[a],"--client=DRTL")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_DIABLORTL;
        }
        else if (strcmp(argv[a],"--client=DSHR")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_DIABLOSHR;
        }
        else if (strcmp(argv[a],"-s")==0 || strcmp(argv[a],"--client=STAR")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_STARCRAFT;
        }
        else if (strcmp(argv[a],"--client=SSHR")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_SHAREWARE;
        }
	else if (strcmp(argv[a],"-w")==0 || strcmp(argv[a],"--client=W2BN")==0)
	{
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_WARCIIBNE;
	}
        else if (strcmp(argv[a],"--client=D2DV")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_DIABLO2DV;
        }
        else if (strcmp(argv[a],"--client=D2XP")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_DIABLO2XP;
        }
        else if (strcmp(argv[a],"--client=WAR3")==0)
        {
            if (clienttag)
            {
                fprintf(stderr,"%s: client type was already specified as \"%s\"\n",argv[0],clienttag);
                usage(argv[0]);
            }
            clienttag = CLIENTTAG_WARCRAFT3;
        }
        else if (strncmp(argv[a],"--client=",9)==0)
        {
            fprintf(stderr,"%s: unknown client tag \"%s\"\n",argv[0],&argv[a][9]);
            usage(argv[0]);
        }
	else if (strncmp(argv[a],"--hexdump=",10)==0)
	{
	    if (hexfile)
	    {
		fprintf(stderr,"%s: hexdump file was already specified as \"%s\"\n",argv[0],hexfile);
		usage(argv[0]);
	    }
	    hexfile = &argv[a][10];
	}
        else if (strcmp(argv[a],"--arch=IX86")==0)
        {
            if (archtag)
            {
                fprintf(stderr,"%s: client arch was already specified as \"%s\"\n",argv[0],archtag);
                usage(argv[0]);
            }
            archtag = ARCHTAG_WINX86;
        }
        else if (strcmp(argv[a],"--arch=PMAC")==0)
        {
            if (archtag)
            {
                fprintf(stderr,"%s: client arch was already specified as \"%s\"\n",argv[0],archtag);
                usage(argv[0]);
            }
            archtag = ARCHTAG_MACPPC;
        }
        else if (strcmp(argv[a],"--arch=XMAC")==0)
        {
            if (archtag)
            {
                fprintf(stderr,"%s: client arch was already specified as \"%s\"\n",argv[0],archtag);
                usage(argv[0]);
            }
            archtag = ARCHTAG_OSXPPC;
        }
        else if (strncmp(argv[a],"--arch=",7)==0)
        {
            fprintf(stderr,"%s: unknown arch tag \"%s\"\n",argv[0],&argv[a][9]);
            usage(argv[0]);
        }
	else if (strncmp(argv[a],"--startoffset=",14)==0)
	{
	    if (startoffsetoverride)
	    {
		fprintf(stderr,"%s: startoffset was already specified as %u\n",argv[0],startoffset);
		usage(argv[0]);
	    }
            if (str_to_uint(&argv[a][14],&startoffset)<0)
            {
                fprintf(stderr,"%s: startoffset \"%s\" should be a positive integer\n",argv[0],&argv[a][14]);
                usage(argv[0]);
            }
	    startoffsetoverride = 1;
	}
	else if (strncmp(argv[a],"--exists=",9)==0)
	{
	    if (exist_action!=EXIST_ACTION_UNSPEC)
	    {
		fprintf(stderr,"%s: exists was already specified\n",argv[0]);
		usage(argv[0]);
	    }
	    if (argv[a][9]=='o' || argv[a][9]=='O')
	    	exist_action = EXIST_ACTION_OVERWRITE;
	    else if (argv[a][9]=='a' || argv[a][9]=='A')
	    	exist_action = EXIST_ACTION_ASK;
	    else if (argv[a][9]=='b' || argv[a][9]=='B')
	    	exist_action = EXIST_ACTION_BACKUP;
	    else if (argv[a][9]=='r' || argv[a][9]=='R')
	    	exist_action = EXIST_ACTION_RESUME;
	    else {
		fprintf(stderr,"%s: exists must begin with a,A,o,O,b,B,r or R",argv[0]);
		usage(argv[0]);
	    }
	}
	else if (strncmp(argv[a],"--file=",7)==0)
	{
	    if (reqfile)
	    {
		fprintf(stderr,"%s: file was already specified as \"%s\"\n",argv[0],reqfile);
		usage(argv[0]);
	    }
	    reqfile = &argv[a][7];
	}
	else if (strcmp(argv[a],"-v")==0 || strcmp(argv[a],"--version")==0)
	{
            printf("version "BNETD_VERSION"\n");
            return 0;
	}
	else if (strcmp(argv[a],"-h")==0 || strcmp(argv[a],"--help")==0 || strcmp(argv[a],"--usage")==0)
            usage(argv[0]);
        else if (strcmp(argv[a],"--client")==0 || strcmp(argv[a],"--hexdump")==0)
        {
            fprintf(stderr,"%s: option \"%s\" requires an argument\n",argv[0],argv[a]);
            usage(argv[0]);
        }
	else
	{
	    fprintf(stderr,"%s: unknown option \"%s\"\n",argv[0],argv[a]);
	    usage(argv[0]);
	}

    if (servport==0)
        servport = BNETD_SERV_PORT;
    if (!clienttag)
        clienttag = CLIENTTAG_STARCRAFT;
    if (!archtag)
        archtag = ARCHTAG_WINX86;
    if (!servname)
	servname = BNETD_DEFAULT_HOST;
    if (exist_action==EXIST_ACTION_UNSPEC)
	exist_action = EXIST_ACTION_ASK;

    if (hexfile)
    {
	if (!(hexstrm = fopen(hexfile,"w")))
	    fprintf(stderr,"%s: could not open file \"%s\" for writing the hexdump (fopen: %s)",argv[0],hexfile,strerror(errno));
	else
	    fprintf(hexstrm,"# dump generated by bnftp version "BNETD_VERSION"\n");
    }

    if (psock_init()<0)
    {
        fprintf(stderr,"%s: could not inialialize socket functions\n",argv[0]);
        return STATUS_FAILURE;
    }

    if (!(host = gethostbyname(servname)))
    {
	fprintf(stderr,"%s: unknown host \"%s\"\n",argv[0],servname);
	return STATUS_FAILURE;
    }

    fd_stdin = fileno(stdin);
    if (tcgetattr(fd_stdin,&in_attr_old)>=0)
    {
        in_attr_new = in_attr_old;
        in_attr_new.c_lflag &= ~(ECHO | ICANON); /* turn off ECHO and ICANON */
	in_attr_new.c_cc[VMIN]  = 1; /* require reads to return at least one byte */
        in_attr_new.c_cc[VTIME] = 0; /* no timeout */
        tcsetattr(fd_stdin,TCSANOW,&in_attr_new);
        changed_in = 1;
    }
    else
    {
	fprintf(stderr,"%s: could not get terminal attributes for stdin\n",argv[0]);
	changed_in = 0;
    }

    if (client_get_termsize(fd_stdin,&screen_width,&screen_height)<0)
    {
        fprintf(stderr,"%s: could not determine screen size\n",argv[0]);
        if (changed_in)
            tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
        return STATUS_FAILURE;
    }

    if ((sd = psock_socket(PSOCK_PF_INET,PSOCK_SOCK_STREAM,PSOCK_IPPROTO_TCP))<0)
    {
	fprintf(stderr,"%s: could not create socket (psock_socket: %s)\n",argv[0],strerror(psock_errno()));
	if (changed_in)
	    tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
	return STATUS_FAILURE;
    }

    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = PSOCK_AF_INET;
    saddr.sin_port   = htons(servport);
    memcpy(&saddr.sin_addr.s_addr,host->h_addr_list[0],host->h_length);
    if (psock_connect(sd,(struct sockaddr *)&saddr,sizeof(saddr))<0)
    {
	fprintf(stderr,"%s: could not connect to server \"%s\" port %hu (psock_connect: %s)\n",argv[0],servname,servport,strerror(psock_errno()));
	if (changed_in)
	    tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
	return STATUS_FAILURE;
    }

    printf("Connected to %s:%hu.\n",inet_ntoa(saddr.sin_addr),servport);

#ifdef CLIENTDEBUG
    eventlog_set(stderr);
#endif

    if (!(packet = packet_create(packet_class_init)))
    {
	fprintf(stderr,"%s: could not create packet\n",argv[0]);
	if (changed_in)
	    tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
	return STATUS_FAILURE;
    }
    bn_byte_set(&packet->u.client_initconn.class,CLIENT_INITCONN_CLASS_FILE);
    if (hexstrm)
    {
	fprintf(hexstrm,"%d: send class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
		sd,
		packet_get_class_str(packet),(unsigned int)packet_get_class(packet),
		packet_get_type_str(packet,packet_dir_from_client),packet_get_type(packet),
		packet_get_size(packet));
	hexdump(hexstrm,packet_get_raw_data(packet,0),packet_get_size(packet));
    }
    client_blocksend_packet(sd,packet);
    packet_del_ref(packet);

    if (!(rpacket = packet_create(packet_class_file)))
    {
	fprintf(stderr,"%s: could not create packet\n",argv[0]);
	if (changed_in)
	    tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
	return STATUS_FAILURE;
    }

    if (!(fpacket = packet_create(packet_class_raw)))
    {
	fprintf(stderr,"%s: could not create packet\n",argv[0]);
	if (changed_in)
	    tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
	packet_del_ref(rpacket);
	return STATUS_FAILURE;
    }

    if (!reqfile) /* if not specified on the command line then prompt for it */
    {
    	munged = 1;
    	commpos = 0;
    	text[0] = '\0';

    	for (;;)
    	{
	    switch (client_get_comm("filename: ",text,sizeof(text),&commpos,1,munged,screen_width))
	    {
	    case -1: /* cancel or error */
	    	printf("\n");
	    	if (changed_in)
		    tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
	    	packet_del_ref(fpacket);
	    	packet_del_ref(rpacket);
	    	return STATUS_FAILURE;

	    case 0: /* timeout */
	    	munged = 0;
	    	continue;

	    case 1:
	    	munged = 0;
	    	if (text[0]=='\0')
		    continue;
	    	printf("\n");
	    }
	    break;
    	}
	reqfile = text;
    }

    if (stat(reqfile,&exist_buf)==0) /* check if the file exists */
    {
	char text2[MAX_MESSAGE_LEN];

    	munged = 1;
	commpos = 0;
	text2[0] = '\0';

  	while (exist_action==EXIST_ACTION_ASK)
	{
	    switch (client_get_comm("File exists [O]verwrite, [B]ackup or [R]esume?: ",text2,sizeof(text2),&commpos,1,munged,screen_width))
	    {
	    case -1: /* cancel or error */
	    	printf("\n");
	    	if (changed_in)
		    tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
	    	packet_del_ref(fpacket);
	    	packet_del_ref(rpacket);
	    	return STATUS_FAILURE;

	    case 0: /* timeout */
	    	munged = 0;
	    	continue;

	    case 1:
	    	munged = 0;
	    	if (text2[0]=='\0')
		    continue;
	    	printf("\n");
		break;
	    }

	    switch (text2[0])
	    {
	    case 'o':
	    case 'O':
		exist_action = EXIST_ACTION_OVERWRITE;
		break;
	    case 'b':
	    case 'B':
		exist_action = EXIST_ACTION_BACKUP;
		break;
	    case 'r':
	    case 'R':
		exist_action = EXIST_ACTION_RESUME;
		break;
	    default:
		printf("Please answer with o,O,b,B,r or R.\n");
		munged = 1;
		continue;
	    }
	    break;
	}

	switch (exist_action)
	{
	case EXIST_ACTION_OVERWRITE:
	    if (!startoffsetoverride)
	    	startoffset = 0;
	    break;
	case EXIST_ACTION_BACKUP:
	    {
		char *       bakfile;
		unsigned int bnr;
		int          renamed=0;

		if (!(bakfile = malloc(strlen(reqfile)+1+2+1))) /* assuming we go up to bnr 99 we need reqfile+'.'+'99'+'\0' */
		{
		    fprintf(stderr,"%s: unable to allocate memory for backup filename.\n",argv[0]);
		    if (changed_in)
			tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
		    packet_del_ref(fpacket);
		    packet_del_ref(rpacket);
		    return STATUS_FAILURE;
		}

		for (bnr=0; bnr<100; bnr++)
		{
		    sprintf(bakfile,"%s.%d",reqfile,bnr);
		    int bak_fd = open(bakfile, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);
		    if (bak_fd < 0) {
		        if (errno == EEXIST)
		            continue; /* backup exists */
		        fprintf(stderr, "%s: could not create backup file \"%s\" (open: %s)\n", argv[0], bakfile, strerror(errno));
		    } else {
		        close(bak_fd);
		        if (link(reqfile, bakfile) < 0) {
		            fprintf(stderr, "%s: could not create backup file \"%s\" (link: %s)\n", argv[0], bakfile, strerror(errno));
		        } else {
		            if (unlink(reqfile) < 0) {
		                fprintf(stderr, "%s: could not remove original file \"%s\" (unlink: %s)\n", argv[0], reqfile, strerror(errno));
		            } else {
		                renamed = 1;
		                printf("Renaming \"%s\" to \"%s\".\n", reqfile, bakfile);
		            }
		        }
		    }
		    break;
		}
		free(bakfile);
		if (!renamed)
		{
		    fprintf(stderr,"%s: could not create backup for \"%s\".\n",argv[0],reqfile);
		    if (changed_in)
			tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);
		    packet_del_ref(fpacket);
		    packet_del_ref(rpacket);
		    return STATUS_FAILURE;
		}
	    	if (!startoffsetoverride)
	    	    startoffset = 0;
	    }
	    break;
	case EXIST_ACTION_RESUME:
	    if (!startoffsetoverride)
	    	startoffset = exist_buf.st_size;
	    break;
	}
    }
    else
	if (!startoffsetoverride)
	    startoffset = 0;

    if (changed_in)
	tcsetattr(fd_stdin,TCSAFLUSH,&in_attr_old);

    if (!(packet = packet_create(packet_class_file)))
    {
	fprintf(stderr,"%s: could not create packet\n",argv[0]);
	packet_del_ref(fpacket);
	packet_del_ref(rpacket);
	return STATUS_FAILURE;
    }
    packet_set_size(packet,sizeof(t_client_file_req));
    packet_set_type(packet,CLIENT_FILE_REQ);
    bn_int_tag_set(&packet->u.client_file_req.archtag,archtag);
    bn_int_tag_set(&packet->u.client_file_req.clienttag,clienttag);
    bn_int_set(&packet->u.client_file_req.adid,0);
    bn_int_set(&packet->u.client_file_req.extensiontag,0);
    bn_int_set(&packet->u.client_file_req.startoffset,startoffset);
    bn_long_set_a_b(&packet->u.client_file_req.timestamp,0x00000000,0x00000000);
    packet_append_string(packet,reqfile);
    if (hexstrm)
    {
	fprintf(hexstrm,"%d: send class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
		sd,
		packet_get_class_str(packet),(unsigned int)packet_get_class(packet),
		packet_get_type_str(packet,packet_dir_from_client),packet_get_type(packet),
		packet_get_size(packet));
	hexdump(hexstrm,packet_get_raw_data(packet,0),packet_get_size(packet));
    }
    printf("\nRequesting info...");
    fflush(stdout);
    client_blocksend_packet(sd,packet);
    packet_del_ref(packet);

    do
    {
	if (client_blockrecv_packet(sd,rpacket)<0)
	{
	    fprintf(stderr,"%s: server closed connection\n",argv[0]);
	    packet_del_ref(fpacket);
	    packet_del_ref(rpacket);
	    return STATUS_FAILURE;
	}
	if (hexstrm)
	{
	    fprintf(hexstrm,"%d: recv class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
		     sd,
		     packet_get_class_str(rpacket),(unsigned int)packet_get_class(rpacket),
		     packet_get_type_str(rpacket,packet_dir_from_server),packet_get_type(rpacket),
		     packet_get_size(rpacket));
	    hexdump(hexstrm,packet_get_raw_data(rpacket,0),packet_get_size(rpacket));
	}
    }
    while (packet_get_type(rpacket)!=SERVER_FILE_REPLY);

    filelen = bn_int_get(rpacket->u.server_file_reply.filelen);
    bn_long_to_bnettime(rpacket->u.server_file_reply.timestamp,&bntime);
    tm = bnettime_to_time(bntime);
    strftime(timestr,FILE_TIME_MAXLEN,FILE_TIME_FORMAT,localtime(&tm));
    filename = packet_get_str_const(rpacket,sizeof(t_server_file_reply),MAX_FILENAME_STR);

    if (exist_action==EXIST_ACTION_RESUME)
    {
	if (!(fp = fopen(reqfile,"ab")))
	{
	    fprintf(stderr,"%s: could not open file \"%s\" for appending (fopen: %s)\n",argv[0],reqfile,strerror(errno));
	    packet_del_ref(fpacket);
	    packet_del_ref(rpacket);
	    return STATUS_FAILURE;
	}
    }
    else
    {
	if (!(fp = fopen(reqfile,"wb")))
	{
	    fprintf(stderr,"%s: could not open file \"%s\" for writing (fopen: %s)\n",argv[0],reqfile,strerror(errno));
	    packet_del_ref(fpacket);
	    packet_del_ref(rpacket);
	    return STATUS_FAILURE;
	}
    }

    printf("\n name: \"");
    str_print_term(stdout,filename,0,0);
    printf("\"\n changed: %s\n length: %u bytes\n",timestr,filelen);
    fflush(stdout);

    if (startoffset>0) {
	filelen -= startoffset; /* for resuming files */
	printf("Resuming at position %u (%u bytes remaining).\n",startoffset,filelen);
    }

    printf("\nSaving to \"%s\"...",reqfile);

    for (currsize=0; currsize+MAX_PACKET_SIZE<=filelen; currsize+=MAX_PACKET_SIZE)
    {
	printf(".");
	fflush(stdout);

	if (client_blockrecv_raw_packet(sd,fpacket,MAX_PACKET_SIZE)<0)
	{
	    printf("error\n");
	    fprintf(stderr,"%s: server closed connection\n",argv[0]);
	    if (fclose(fp)<0)
		fprintf(stderr,"%s: could not close file \"%s\" after writing (fclose: %s)\n",argv[0],reqfile,strerror(errno));
	    packet_del_ref(fpacket);
	    packet_del_ref(rpacket);
	    return STATUS_FAILURE;
	}
	if (hexstrm)
	{
	    fprintf(hexstrm,"%d: recv class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
		     sd,
		     packet_get_class_str(fpacket),(unsigned int)packet_get_class(fpacket),
		     packet_get_type_str(fpacket,packet_dir_from_server),packet_get_type(fpacket),
		     packet_get_size(fpacket));
	    hexdump(hexstrm,packet_get_raw_data(fpacket,0),packet_get_size(fpacket));
	}
	if (fwrite(packet_get_raw_data_const(fpacket,0),1,MAX_PACKET_SIZE,fp)<MAX_PACKET_SIZE)
	{
	    printf("error\n");
	    fprintf(stderr,"%s: could not write to file \"%s\" (fwrite: %s)\n",argv[0],reqfile,strerror(errno));
	    if (fclose(fp)<0)
		fprintf(stderr,"%s: could not close file \"%s\" after writing (fclose: %s)\n",argv[0],reqfile,strerror(errno));
	    packet_del_ref(fpacket);
	    packet_del_ref(rpacket);
	    return STATUS_FAILURE;
	}
    }
    filelen -= currsize;
    if (filelen)
    {
	printf(".");
	fflush(stdout);

	if (client_blockrecv_raw_packet(sd,fpacket,filelen)<0)
	{
	    printf("error\n");
	    fprintf(stderr,"%s: server closed connection\n",argv[0]);
	    if (fclose(fp)<0)
		fprintf(stderr,"%s: could not close file \"%s\" after writing (fclose: %s)\n",argv[0],reqfile,strerror(errno));
	    packet_del_ref(fpacket);
	    packet_del_ref(rpacket);
	    return STATUS_FAILURE;
	}
	if (hexstrm)
	{
	    fprintf(hexstrm,"%d: recv class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
		     sd,
		     packet_get_class_str(fpacket),(unsigned int)packet_get_class(fpacket),
		     packet_get_type_str(fpacket,packet_dir_from_server),packet_get_type(fpacket),
		     packet_get_size(fpacket));
	    hexdump(hexstrm,packet_get_raw_data(fpacket,0),packet_get_size(fpacket));
	}
	if (fwrite(packet_get_raw_data_const(fpacket,0),1,filelen,fp)<filelen)
	{
	    printf("error\n");
	    fprintf(stderr,"%s: could not write to file \"%s\"\n",argv[0],reqfile);
	    if (fclose(fp)<0)
		fprintf(stderr,"%s: could not close file \"%s\" after writing (fclose: %s)\n",argv[0],reqfile,strerror(errno));
	    packet_del_ref(fpacket);
	    packet_del_ref(rpacket);
	    return STATUS_FAILURE;
	}
    }

    packet_del_ref(fpacket);
    packet_del_ref(rpacket);

    if (hexstrm)
    {
	fprintf(hexstrm,"# end of dump\n");
	if (fclose(hexstrm)<0)
	    fprintf(stderr,"%s: could not close hexdump file \"%s\" after writing (fclose: %s)",argv[0],hexfile,strerror(errno));
    }

    if (fclose(fp)<0)
    {
	fprintf(stderr,"%s: could not close file \"%s\" after writing (fclose: %s)\n",argv[0],reqfile,strerror(errno));
	return STATUS_FAILURE;
    }

    printf("done\n");
    return STATUS_FAILURE;
}
