/* mail.c
 * Copyright (C) 2001  Dizzy (dizzy@roedu.net)
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
#define MAIL_INTERNAL_ACCESS
#include "common/setup_before.h"
#include <stdio.h>
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
#  warning mail.c expects <stdlib.h> to be included.
# endif /* HAVE_STDLIB_H */
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# else
#  ifdef HAVE_MALLOC_MALLOC_H
#   include <malloc/malloc.h>
#  else
#   warning mail.c expects a malloc-related header to be included.
#  endif /* HAVE_MALLOC_MALLOC_H */
# endif /* HAVE_MALLOC_H */
#endif /* STDC_HEADERS */
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# else
#  warning mail.c expects a string-related header to be included.
# endif /* HAVE_STRINGS_H */
#endif /* HAVE_STRING_H */
#include "compat/strcasecmp.h"
#include <ctype.h>
#include <errno.h>
#include "compat/strerror.h"
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  ifdef HAVE_TIME_H
#   include <time.h>
#  else
#   warning mail.c expects a time-related header to be included.
#  endif /* HAVE_TIME_H */
# endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# warning mail.c expects <sys/types.h> to be included.
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#else
# warning mail.c expects <sys/stat.h> to be included.
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#else
# warning mail.c expects <unistd.h> to be included.
#endif /* HAVE_UNISTD_H */
#include "compat/statmacros.h"
#include "compat/mkdir.h"
#include "compat/pdir.h"
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# ifdef HAVE_SYS_FILE_H
#  include <sys/file.h>
# else
#  warning mail.c expects a file-related header to be included.
# endif /* HAVE_SYS_FILE_H */
#endif /* HAVE_FCNTL_H */
#include "message.h"
#include "connection.h"
#include "common/util.h"
#include "common/eventlog.h"
#include "account.h"
#include "prefs.h"
#include "mail.h"
#include "common/setup_after.h"


static int identify_mail_function(const char *);
static void mail_usage(t_connection*);
static void mail_func_send(t_connection*,const char *);
static void mail_func_read(t_connection*,const char *);
static void mail_func_delete(t_connection*,const char *);
static int get_mail_quota(t_account *);

/* Mail API */
/* for now this functions are only for internal use */
static t_mailbox * mailbox_open(t_account *);
static int mailbox_count(t_mailbox *);
static int mailbox_deliver(t_mailbox *, const char *, const char *);
static t_mail * mailbox_read(t_mailbox *, unsigned int);
static void mailbox_unread(t_mail *);
static struct maillist_struct * mailbox_get_list(t_mailbox *);
static void mailbox_unget_list(struct maillist_struct *);
static int mailbox_delete(t_mailbox *, unsigned int);
static int mailbox_delete_all(t_mailbox *);
static void mailbox_close(t_mailbox *);
static void mail_func_summary(t_connection * c);
static int mail_number_of_mails(t_account * user);

static char * clean_str(char *);


static t_mailbox * mailbox_open(t_account * user)
{
    t_mailbox *  rez = NULL;
    char * 	 path = NULL;
    char const * maildir;
    char const * username;
    char const * safeusername;
    char *	 filename;

    if (!user)
    {
	eventlog(eventlog_level_error,"mailbox_open","got NULL user");
	return NULL;
    }

    if (!(username = account_get_name(user)))
    {
	eventlog(eventlog_level_error,"mailbox_open","could not get username");
	return NULL;
    }
    filename = strdup(username); /* FIXME: add checking */
    str_to_lower(filename);
    if (!(safeusername = escape_fs_chars(filename,strlen(filename))))
    {
	eventlog(eventlog_level_error,"mailbox_open","could not escape username");
	account_unget_name(username);
	free(filename);
	return NULL;
    }
    free(filename);

    if ((rez = malloc(sizeof(t_mailbox)))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_open","colud not allocate memory for rez");
	account_unget_name(username);
	free((void *)safeusername); /* avoid warning */
	return NULL;
    }
    maildir = prefs_get_maildir();
	/* p_mkdir() is a wrapper around mkdir() from "compat/mkdir.h" */
    p_mkdir(maildir,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if (!(path = malloc(strlen(maildir)+1+strlen(safeusername)+1)))
    {
	eventlog(eventlog_level_error,"mailbox_open","could not allocate memory for path");
	account_unget_name(username);
	free((void *)safeusername);
	free(rez);
	return NULL;
    }
    sprintf(path,"%s/%s",maildir,safeusername);
    p_mkdir(path,S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH);
    if ((rez->maildir = p_opendir(path))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_open","error opening maildir");
	account_unget_name(username);
	free((void *)safeusername);
	free(path);
	free(rez);
	return NULL;
    }
    rez->path = path;

    account_unget_name(username);
    free((void *)safeusername);
    return rez;
}


static int mailbox_count(t_mailbox *mailbox)
{
    char const * dentry;
    int 	 count = 0;

    if (mailbox==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_count","got NULL mailbox");
	return -1;
    }
    if (mailbox->maildir==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_count","got NULL maildir");
	return -1;
    }
    p_rewinddir(mailbox->maildir);
    while ((dentry = p_readdir(mailbox->maildir))!=NULL)
	if (dentry[0]!='.') count++;

    return count;
}


static int mailbox_deliver(t_mailbox * mailbox, const char * sender, const char * message)
{
    FILE * fd;
    char * filename;

    if (mailbox==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_deliver","got NULL mailbox");
	return -1;
    }
    if (mailbox->maildir==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_deliver","got NULL maildir");
	return -1;
    }
    if (mailbox->path==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_deliver","got NULL path");
	return -1;
    }
    if ((filename = malloc(strlen(mailbox->path)+1+15+1))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_deliver","could notallocate memory for filename");
	return -1;
    }
    sprintf(filename,"%s/%015lu",mailbox->path,(unsigned long)time(NULL));
    if ((fd = fopen(filename,"wb"))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_deliver","got NULL file descriptor. check permissions");
	free(filename);
	return -1;
    }
    fprintf(fd,"%s\n",sender); /* write the sender on the first line of message */
    fprintf(fd,"%s\n",message); /* then write the actual message */
    fclose(fd);
    free(filename);

   return 0;
}


static t_mail * mailbox_read(t_mailbox * mailbox, unsigned int idx)
{
    char const * dentry;
    unsigned int i;
    t_mail *     rez;
    FILE *       fd;
    char *       filename;

    if (mailbox==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","got NULL mailbox");
	return NULL;
    }
    if (mailbox->maildir==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","got NULL maildir");
	return NULL;
    }
    if ((rez = malloc(sizeof(t_mail)))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","could not enough allocate memory for rez");
	return NULL;
    }
    p_rewinddir(mailbox->maildir);
    dentry = NULL; /* if idx < 1 we should not crash :-) */
    for(i = 0; i<idx && (dentry = p_readdir(mailbox->maildir))!=NULL; i++)
	if (dentry[0]=='.') i--;
    if (dentry==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","index out of range");
	free(rez);
	return NULL;
    }
    rez->timestamp = atoi(dentry);
    if ((filename = malloc(strlen(dentry)+1+strlen(mailbox->path)+1))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","could not allocate memory for filename");
	free(rez);
	return NULL;
    }
    sprintf(filename,"%s/%s",mailbox->path,dentry);
    if ((fd = fopen(filename,"rb"))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","error while opening message");
	free(rez);
	free(filename);
	return NULL;
    }
    free(filename);
    if ((rez->sender = malloc(MAX_NICK_LEN))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","not enough memory for storing sender");
	fclose(fd);
	free(rez);
	return NULL;
    }
    fgets(rez->sender,MAX_NICK_LEN,fd);
    clean_str(rez->sender);
    if ((rez->message = malloc(MAX_MESSAGE_LEN))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_read","not enough memory for storing message");
	fclose(fd);
	free(rez->sender);
	free(rez);
	return NULL;
    }
    fgets(rez->message,MAX_MESSAGE_LEN,fd);
    clean_str(rez->message);
    fclose(fd);
    rez->timestamp = atoi(dentry);

    return rez;
}


static void mailbox_unread(t_mail * mail)
{
    if (mail==NULL)
	eventlog(eventlog_level_error,"mailbox_unread","got NULL mail");
    else
    {
	if (mail->sender==NULL)
	    eventlog(eventlog_level_error,"mailbox_unread","got NULL sender");
	else free(mail->sender);
	if (mail->message==NULL)
	    eventlog(eventlog_level_error,"mailbox_unread","got NULL message");
	else free(mail->message);
	free(mail);
    }
}


static struct maillist_struct * mailbox_get_list(t_mailbox *mailbox)
{
    char const * 		dentry;
    FILE * 			fd;
    struct maillist_struct * 	rez = NULL;
    struct maillist_struct * 	p = NULL;
    struct maillist_struct * 	q;
    char *			sender;
    char *			filename;
    int 			i;

    if (mailbox==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_get_list","got NULL mailbox");
	return NULL;
    }
    if (mailbox->maildir==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_get_list","got NULL maildir");
	return NULL;
    }
    if ((filename = malloc(strlen(mailbox->path)+1+15+1))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_get_list","not allocate memory for filename");
	return NULL;
    }
    p_rewinddir(mailbox->maildir);
    for(i = 0; (dentry = p_readdir(mailbox->maildir))!=NULL;)
	if (dentry[0]!='.')
	{
	    if (!(q = malloc(sizeof(struct maillist_struct))))
	    {
		eventlog(eventlog_level_error,"mailbox_get_list","could not allocate memory for q");
		free(filename);
		return rez;
	    }
	    sprintf(filename,"%s/%s",mailbox->path,dentry);
	    if ((fd = fopen(filename,"rb"))==NULL)
	    {
		eventlog(eventlog_level_error,"mailbox_get_list","error while opening message file");
		free(filename);
		free(q);
		return rez;
	    }
	    if ((sender = malloc(MAX_NICK_LEN))==NULL)
	    {
		eventlog(eventlog_level_error,"mailbox_get_list","could not allocate memory for sender");
		fclose(fd);
		free(filename);
		free(q);
		continue;
	    }
	    fgets(sender,MAX_NICK_LEN,fd);
	    clean_str(sender);
	    fclose(fd);
	    q->timestamp = atoi(dentry);
	    q->next = NULL;
	    if (p==NULL)
		rez = q;
	    else
		p->next = q;
	    p = q;
	    i++;
	}
    free(filename);

    return rez;
}


static void mailbox_unget_list(struct maillist_struct * maill)
{
    struct maillist_struct * p;
    struct maillist_struct * q;

    for(p = maill; p!=NULL; p = q)
    {
	if (p->sender!=NULL)
	    free(p->sender);
	q = p->next;
	free(p);
    }
}


static int mailbox_delete(t_mailbox * mailbox, unsigned int idx)
{
    char *	 filename;
    char const * dentry;
    unsigned int i;
    int		 rez;

    if (mailbox==NULL) {
	eventlog(eventlog_level_error,"mailbox_delete","got NULL mailbox");
	return -1;
    }
    if (mailbox->maildir==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_delete","got NULL maildir");
	return -1;
    }
    p_rewinddir(mailbox->maildir);
    dentry = NULL; /* if idx < 1 we should not crash :-) */
    for(i = 0; i<idx && (dentry = p_readdir(mailbox->maildir))!=NULL; i++)
	if (dentry[0]=='.') i--;
    if (dentry==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_delete","index out of range");
	return -1;
    }
    if ((filename = malloc(strlen(dentry)+1+strlen(mailbox->path)+1))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_delete","could not allocate memory for filename");
	return -1;
    }
    sprintf(filename,"%s/%s",mailbox->path,dentry);
    rez = remove(filename);
    if (rez<0)
	eventlog(eventlog_level_info,"mailbox_delete","could not remove file \"%s\" (remove: %s)",filename,strerror(errno));
    free(filename);

    return rez;
}


static int mailbox_delete_all(t_mailbox * mailbox)
{
   char *	filename;
   char const * dentry;
   int		count;

    if (mailbox==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_delete_all","got NULL mailbox");
	return -1;
    }
    if (mailbox->maildir==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_delete_all","got NULL maildir");
	return -1;
    }
    if ((filename = malloc(strlen(mailbox->path)+1+15+1))==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_delete_all","could not allocate memory for filename");
	return -1;
    }
    p_rewinddir(mailbox->maildir);
    count = 0;
    while ((dentry = p_readdir(mailbox->maildir))!=NULL)
    if (dentry[0]!='.')
    {
	sprintf(filename,"%s/%s",mailbox->path,dentry);
	if (!remove(filename))
	    count++;
    }
    free(filename);

    return count;
}


static void mailbox_close(t_mailbox * mailbox)
{
    if (mailbox==NULL)
    {
	eventlog(eventlog_level_error,"mailbox_close","got NULL mailbox");
	return;
    }
    if (mailbox->maildir!=NULL)
	p_closedir(mailbox->maildir);
    else
	eventlog(eventlog_level_error,"mailbox_close","got NULL maildir");
    if (mailbox->path)
	free(mailbox->path);
    free(mailbox);
}


static char * clean_str(char * str) {
   char *p;

   for(p=str;*p!='\0';p++)
     if (*p=='\n' || *p=='\r') {
	*p='\0'; break;
     }
   return str;
}

extern int mail_number_of_new(t_account * user)
{
    int		tnbr;

    if (!user) /* should not happen */
    {
	eventlog(eventlog_level_error,"mail_number_of_new","got NULL account");
	return -1;
    }
    tnbr = mail_number_of_mails(user) - account_get_numattr(user,"BNET\\mail\\number");
    if (tnbr<0)
    {
	eventlog(eventlog_level_error,"mail_numbers_of_new","tnbr < 0");
	tnbr = 0;
    }

    return tnbr;
}


static int mail_number_of_mails(t_account * user)
{
    t_mailbox * mailbox;
    int		tnbr;

    if (!user) /* should not happen */
    {
	eventlog(eventlog_level_error,"mail_number_of_mails","got NULL account");
	return -1;
    }
    if (!(mailbox = mailbox_open(user)))
    {
	eventlog(eventlog_level_error,"mail_number_of_mails","got NULL mailbox");
	return -1;
    }
    tnbr = mailbox_count(mailbox);
    mailbox_close(mailbox);

    return tnbr;
}


extern int mail_save_number(t_connection * c)
{
    t_account * account;

    if (!c)
    {
	eventlog(eventlog_level_error,"mail_save_number","got NULL connection");
	return -1;
    }
    if (!(account = conn_get_account(c)))
    {
	eventlog(eventlog_level_error,"mail_save_number","got NULL account");
	return -1;
    }

    if (account_set_numattr(account,"BNET\\mail\\number",mail_number_of_mails(account))<0)
    {
	eventlog(eventlog_level_error,"mail_save_number","could not set numattr");
	return -1;
    }

    return 0;
}


extern int handle_mail_command(t_connection * c, char const * text)
{
    unsigned int i,j;
    char         comm[MAX_FUNC_LEN];

    if (!prefs_get_mail_support())
    {
	message_send_text(c,message_type_error,c,"This server has NO mail support.");
	return -1;
    }

    for (i=0; text[i]!=' ' && text[i]!='\0'; i++); /* skip /mail command */
    for (; text[i]==' '; i++); /* skip any spaces after it */

    for (j=0; text[i]!=' ' && text[i]!='\0' && j<sizeof(comm)-1; i++) /* get function */
	if (j<sizeof(comm)-1) comm[j++] = text[i];
    comm[j] = '\0';

    switch (identify_mail_function(comm))
    {
    case MAIL_FUNC_SEND:
	mail_func_send(c,text+i);
	break;
    case MAIL_FUNC_READ:
	mail_func_read(c,text+i);
	break;
    case MAIL_FUNC_DELETE:
	mail_func_delete(c,text+i);
	break;
    case MAIL_FUNC_SUMMARY:
	mail_func_summary(c);
	break;
    case MAIL_FUNC_HELP:
	message_send_text(c,message_type_info,c,"The mail command supports the following patterns.");
	mail_usage(c);
	break;
    default:
	message_send_text(c,message_type_error,c,"The command is incorrect. Use one of the following patterns.");
	mail_usage(c);
    }

    return 0;
}


static int identify_mail_function(const char *funcstr)
{
    if (strcasecmp(funcstr,"send")==0 ||
	strcasecmp(funcstr,"s")==0)
	return MAIL_FUNC_SEND;
    if (strcasecmp(funcstr,"read")==0 ||
        strcasecmp(funcstr,"r")==0)
	return MAIL_FUNC_READ;
    if (strcasecmp(funcstr,"")==0)
	return MAIL_FUNC_SUMMARY;
    if (strcasecmp(funcstr,"delete")==0 ||
        strcasecmp(funcstr,"del")==0)
	return MAIL_FUNC_DELETE;
    if (strcasecmp(funcstr,"help")==0 ||
        strcasecmp(funcstr,"h")==0)
	return MAIL_FUNC_HELP;

    return MAIL_FUNC_UNKNOWN;
}


static int get_mail_quota(t_account * user)
{
    int quota;

    quota = account_get_numattr(user,"BNET\\mail\\quota");
    if (quota==0)
	quota = prefs_get_mail_quota();
    if (quota<1)
	quota = 1;
    if (quota>MAX_MAIL_QUOTA)
	quota = MAX_MAIL_QUOTA;

    return quota;
}


static void mail_func_send(t_connection * c, const char * str)
{
    int              i;
    char *           dest;
    char const * p,* myname;
    t_account *      recv;
    t_mailbox *      mailbox;
    t_connection *   tconn;

    if (c==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_send","got NULL connection");
	return;
    }
    if (str==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_send","got NULL command string");
	return;
    }
    for(i=0;str[i]==' ';i++); /* skip any spaces */
    if (str[i]=='\0') /* the %mail send command has no receiver */
    {
	message_send_text(c,message_type_error,c,"You must specify the receiver");
	message_send_text(c,message_type_error,c,"Syntax: /mail send <receiver> <message>");
	return;
    }
    p = str+i; /* set ip at the start of receiver string */
    for(i = 1; p[i]!=' ' && p[i]!='\0'; i++); /* skip the receiver string */
    if (p[i]=='\0') /* it seems user forgot to write any message */
    {
	message_send_text(c,message_type_error,c,"Your message is empty!");
	message_send_text(c,message_type_error,c,"Syntax: /mail send <receiver> <message>");
	return;
    }
    if ((dest = malloc(i+1))==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_send","could not allocate memory for dest");
	message_send_text(c,message_type_error,c,"Not enough resources to complete request!");
	return;
    }
    memmove(dest,p,i);
    dest[i] = '\0'; /* copy receiver in his separate string */
    if ((recv=accountlist_find_account(dest))==NULL) /* is dest a valid account on this server ? */
    {
	message_send_text(c,message_type_error,c,"Receiver UNKNOWN!");
	free(dest);
	return;
    }
    if ((mailbox = mailbox_open(recv))==NULL)
    {
	message_send_text(c,message_type_error,c,"There was an error completing your request!");
	free(dest);
	return;
    }
    if (get_mail_quota(recv)<=mailbox_count(mailbox)) /* check quota */
    {
	message_send_text(c,message_type_error,c,"Receiver has reached his mail quota. Your message will NOT be sent.");
	mailbox_close(mailbox);
	free(dest);
	return;
    }
    myname = conn_get_username(c); /* who am i ? */
    if (account_check_ignoring(recv,myname))
    {
	message_send_text(c,message_type_info,c,"Reciver ignores you.");
	mailbox_close(mailbox);
	conn_unget_username(c,myname);
	free(dest);
	return;
    }
    if (mailbox_deliver(mailbox,myname,p+i+1)<0)
	message_send_text(c,message_type_error,c,"There was an error completing your request!");
    else
	message_send_text(c,message_type_info,c,"Your mail has been sent successfully.");
    if ((tconn = connlist_find_connection_by_accountname(dest)))
	if (!(conn_get_dndstr(tconn)))
	    message_send_text(tconn,message_type_info,tconn,"You have new mail.");

    free(dest);
    conn_unget_username(c,myname);
    mailbox_close(mailbox);
}


static void mail_func_read(t_connection * c, const char * str)
{
    t_account *  user;
    t_mailbox *  mailbox;
    const char * p;
    char 	 tmp[MAX_MESSAGE_LEN];
    int 	 i;
    int 	 idx;
    t_mail * 	 mail;

    if (c==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_read","got NULL connection");
	return;
    }
    if (str==NULL) {
	eventlog(eventlog_level_error,"mail_func_read","got NULL command string");
	return;
    }

    for(i = 0; str[i]==' '; i++);
    p = str+i;
    if ((user = conn_get_account(c))==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_read","got NULL account");
	return;
    }
    if ((mailbox = mailbox_open(user))==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_read","got NULL mailbox");
	return;
    }

    for (i = 0; p[i]>='0' && p[i]<='9' && p[i]!='\0'; i++);
    if (p[i]!='\0' && p[i]!=' ')
    {
	message_send_text(c,message_type_error,c,"Invalid index. Please use /mail read <index> where <index> is a number.");
	mailbox_close(mailbox);
	return;
    }
    if (mailbox_count(mailbox) < 1)
    {
	message_send_text(c,message_type_info,c,"You have empty mailbox");
	mailbox_close(mailbox);
	return;
    }

    idx=atoi(p);
    if (idx<0 || idx>mailbox_count(mailbox))
    {
	message_send_text(c,message_type_error,c,"That index is out of range.");
	mailbox_close(mailbox);
	return;
    }
    if (idx == 0)
	idx = 1; /* if no index given show firs mail */
    if ((mail = mailbox_read(mailbox,idx))==NULL)
    {
	message_send_text(c,message_type_error,c,"There was an error completing your request.");
	mailbox_close(mailbox);
	return;
    }
    sprintf(tmp,"Message #%d from %s on %s:",idx,mail->sender,clean_str(ctime(&mail->timestamp)));
    message_send_text(c,message_type_info,c,tmp);
    message_send_text(c,message_type_info,c,mail->message);
    mailbox_delete(mailbox,idx);
    mailbox_unread(mail);
    mailbox_close(mailbox);
}


static void mail_func_delete(t_connection * c, const char * str)
{
    t_account *  user;
    t_mailbox *  mailbox;
    const char * p;
    char 	 tmp[MAX_MESSAGE_LEN];
    int 	 i;

    if (c==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_delete","got NULL connection");
	return;
    }
    if (str==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_delete","got NULL command string");
	return;
    }

    for(i = 0; str[i]==' '; i++);
    p = str+i;
    if (*p=='\0')
    {
	message_send_text(c,message_type_error,c,"Please specify which message to delete. Use the following syntax: /mail delete {<index>|all} .");
	return;
    }
    if ((user = conn_get_account(c))==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_read","got NULL account");
	return;
    }
    if ((mailbox = mailbox_open(user))==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_read","got NULL mailbox");
	return;
    }
    if (strcmp(p,"all")==0)
    {
	int rez;

	if ((rez=mailbox_delete_all(mailbox))<0)
	{
	    message_send_text(c,message_type_error,c,"There was an error completing your request.");
	    mailbox_close(mailbox);
	    return;
	}
	sprintf(tmp,"Successfuly deleted %d messages.",rez);
	message_send_text(c,message_type_info,c,tmp);
    }
    else
    {
	int idx;

	for(i = 0; p[i]>='0' && p[i]<='9' && p[i]!='\0'; i++);
	if (p[i]!='\0' && p[i]!=' ')
	{
	    message_send_text(c,message_type_error,c,"Invalid index. Please use /mail delete {<index>|all} where <index> is a number.");
	    mailbox_close(mailbox);
	    return;
	}
	idx = atoi(p);
	if (idx<1 || idx>mailbox_count(mailbox))
	{
	    message_send_text(c,message_type_error,c,"That index is out of range.");
	    mailbox_close(mailbox);
	    return;
	}
	if (mailbox_delete(mailbox,idx)<0)
	{
	    message_send_text(c,message_type_error,c,"There was an error completing your request.");
	    mailbox_close(mailbox);
	    return;
	}
	sprintf(tmp,"Succesfully deleted message #%02d.",idx);
	message_send_text(c,message_type_info,c,tmp);
    }
    mailbox_close(mailbox);
}


static void mail_usage(t_connection * c)
{
    message_send_text(c,message_type_info,c,"to print this information:");
    message_send_text(c,message_type_info,c,"    /mail help");
    message_send_text(c,message_type_info,c,"to print an index of you messages:");
    message_send_text(c,message_type_info,c,"    /mail [read]");
    message_send_text(c,message_type_info,c,"to send a message:");
    message_send_text(c,message_type_info,c,"    /mail send <receiver> <message>");
    message_send_text(c,message_type_info,c,"to read a message:");
    message_send_text(c,message_type_info,c,"    /mail read <index num>");
    message_send_text(c,message_type_info,c,"to delete a message:");
    message_send_text(c,message_type_info,c,"    /mail delete {<index>|all}");
    message_send_text(c,message_type_info,c,"Commands may be abbreviated as follows:");
    message_send_text(c,message_type_info,c,"    help: h");
    message_send_text(c,message_type_info,c,"    read: r");
    message_send_text(c,message_type_info,c,"    send: s");
    message_send_text(c,message_type_info,c,"    delete: del");
}

static void mail_func_summary(t_connection * c)
{
    struct maillist_struct 	* maill, * mp;
    t_account * 		user;
    unsigned int		idx;
    t_mailbox *			mailbox;
    char			tstr[MAX_MESSAGE_LEN];

    if (!c)
    {
	eventlog(eventlog_level_error,"mail_func_summary","got NULL connection");
	return;
    }
    if (!(user = conn_get_account(c)))
    {
	eventlog(eventlog_level_error,"mail_func_summary","got NULL account");
	return;
    }
    if (!(mailbox = mailbox_open(user)))
    {
	eventlog(eventlog_level_error,"mail_func_summary","got NULL mailbox");
	return;
    }

    if (mailbox_count(mailbox)==0)
    {
	message_send_text(c,message_type_info,c,"You have no mail.");
	mailbox_close(mailbox);
	return;
    }
    if ((maill = mailbox_get_list(mailbox))==NULL)
    {
	eventlog(eventlog_level_error,"mail_func_read","got NULL maillist");
	mailbox_close(mailbox);
	return;
    }
    sprintf(tstr,"You have %d messages. Your mail qouta is set to %d.",mailbox_count(mailbox),get_mail_quota(user));
    message_send_text(c,message_type_info,c,tstr);
    message_send_text(c,message_type_info,c,"ID    Sender          Date");
    message_send_text(c,message_type_info,c,"-------------------------------------");
    for (mp = maill,idx = 1; mp!=NULL; mp = mp->next,idx++)
    {
	sprintf(tstr,"%02u    %-14s %s",idx,mp->sender,ctime(&mp->timestamp));
	clean_str(tstr); /* ctime() appends an newline that we get cleaned */
	message_send_text(c,message_type_info,c,tstr);
    }
    message_send_text(c,message_type_info,c,"Use /mail read <ID> to read the content of any message");
    mailbox_unget_list(maill);
    mailbox_close(mailbox);
}

/* EOF */
