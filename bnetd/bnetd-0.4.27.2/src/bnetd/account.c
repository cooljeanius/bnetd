/* account.c
 * Copyright (C) 1998,1999,2000,2001,2002  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2002,2003  JEBs@shbe.net
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
#define ACCOUNT_INTERNAL_ACCESS
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
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# else
#  ifdef HAVE_MALLOC_MALLOC_H
#   include <malloc/malloc.h>
#  else
#   warning account.c expects a malloc-related header to be included.
#  endif /* HAVE_MALLOC_MALLOC_H */
# endif /* HAVE_MALLOC_H */
#endif /* STDC_HEADERS */
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# else
#  warning account.c expects a string-related header to be included.
# endif /* HAVE_STRINGS_H */
#endif /* HAVE_STRING_H */
#include "compat/strchr.h"
#include "compat/strdup.h"
#include "compat/strcasecmp.h"
#include "compat/strncasecmp.h"
#include <ctype.h>
#ifdef HAVE_LIMITS_H
# include <limits.h>
#else
# warning account.c expects <limits.h> to be included.
#endif /* HAVE_LIMITS_H */
#include "compat/char_bit.h"
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
#   warning account.c expects a time-related header to be included.
#  endif /* HAVE_TIME_H */
# endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */
#include <errno.h>
#include "compat/strerror.h"
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# warning account.c expects <sys/types.h> to be included.
#endif /* HAVE_SYS_TYPES_H */
#include "compat/pdir.h"
#include "common/eventlog.h"
#include "prefs.h"
#include "common/util.h"
#include "common/field_sizes.h"
#include "common/bnethash.h"
#ifdef WITH_BITS
# include "connection.h"
# include "bits_va.h"
# include "bits.h"
#endif /* WITH_BITS */
#ifdef WITH_STORAGE_DB
# ifdef WITH_MYSQL
/* FIXME: SET_FLAG is defined in <mysql.h> & here (approx. 19 lines later) too! */
#  include "common/db_mysql.h"
#  include "account_db_mysql.h"
# endif /* WITH_MYSQL */
#endif /* WITH_STORAGE_DB */
#include "common/introtate.h"
#include "account.h"
#include "common/hashtable.h"
#include "common/setup_after.h"
#ifdef ACCT_DYN_UNLOAD
# include "connection.h"
# include "friends.h"
# include "watch.h"
# include "game.h"
#endif /* ACCT_DYN_UNLOAD */

static t_hashtable * accountlist_head=NULL;

static t_account * default_acct=NULL;
#ifndef ACCT_DYN_LOAD
static unsigned int maxuserid=0;
#endif /* !ACCT_DYN_LOAD */

#define CLEAR_FLAGS(A)  ( (A)->flags = 0     )
#define SET_FLAG(A,F)   ( (A)->flags |= (F)  )
#define CLEAR_FLAG(A,F) ( (A)->flags &= ~(F) )
#define TEST_FLAG(A,F)  ( (A)->flags & (F)   )

/*
 * This is to force the creation of all accounts even if it goes past the
 * maximum number when we initially load the accountlist.
 */
static int force_account_add=0;

static unsigned int account_hash(char const * username);
static int account_insert_attr(t_account * account, char const * key, char const * val);
static t_account * account_load(char const * filename);
static int account_load_attrs(t_account * account);
static void account_unload_attrs(t_account * account);
static int account_check_name(char const * name);
#ifdef ACCT_DYN_UNLOAD
static void accountlist_remove_unused();
#endif /* ACCT_DYN_UNLOAD */

static unsigned int account_hash(char const * username)
{
    unsigned int i;
    unsigned int pos;
    unsigned int hash;
    unsigned int ch;
    size_t       strl;

    if (!username)
    {
    	eventlog(eventlog_level_error,"account_hash","got NULL username");
	    return 0;
    }

    strl = strlen(username);
    hash = (strl+1)*120343021;
    for (pos=0,i=0; i<strl; i++)
    {
        if (isascii((int)username[i]) && isupper((int)username[i])) {
            ch = (unsigned int)(unsigned char)tolower((int)username[i]);
        } else {
            ch = (unsigned int)(unsigned char)username[i];
		}
        hash ^= ROTL(ch,pos,sizeof(unsigned int)*CHAR_BIT);
        hash ^= ROTL((i+1)*314159,ch&0x1f,sizeof(unsigned int)*CHAR_BIT);
        pos += CHAR_BIT-1;
    }

    return hash;
}


extern t_account * account_create(char const * username, char const * passhash1)
{
    t_account * account;

    if (username && account_check_name(username)<0)
    {
        eventlog(eventlog_level_error,"account_create","got bad account name");
        return NULL;
    }

    if (!(account = malloc(sizeof(t_account))))
    {
		eventlog(eventlog_level_error,"account_create","could not allocate memory for account");
		return NULL;
    }

    account->filename = NULL;
    account->attrs    = NULL;
    account->age      = 0;
    CLEAR_FLAGS(account);

    account->namehash = 0; /* hash it later before inserting */
#ifndef ACCT_DYN_LOAD
    account->uid      = 0;
#endif /* !ACCT_DYN_LOAD */
#ifdef ACCT_DYN_UNLOAD
    account->ref      = 0;
#endif /* ACCT_DYN_UNLOAD */


    if (username) /* actually making a new account */
    {
        char * temp;

		if (account_check_name(username)<0)
		{
	    	eventlog(eventlog_level_error,"account_create","invalid account name \"%s\"",username);
		    account_destroy(account);
		    return NULL;
		}
		if (!passhash1)
		{
            eventlog(eventlog_level_error,"account_create","got NULL passhash1");
            account_destroy(account);
            return NULL;
		}

#ifdef WITH_STORAGE_DB
	    /* In DB (MySQL) storage i always use the real login name as id */
	    if (!(temp = malloc(strlen(username)+1))) /* name + NUL */
	    {
	        eventlog(eventlog_level_error,"account_create","could not allocate memory for temp");
	        account_destroy(account);
	        return NULL;
	    }
	    sprintf(temp,"%s",username);
#else
# ifndef ACCT_DYN_LOAD
		if (prefs_get_savebyname()==0)
		{
	        if (!(temp = malloc(strlen(prefs_get_userdir())+1+8+1))) /* dir + / + uid + NUL */
	        {
				eventlog(eventlog_level_error,"account_create","could not allocate memory for temp");
				account_destroy(account);
				return NULL;
		    }
		    sprintf(temp,"%s/%06u",prefs_get_userdir(),maxuserid+1); /* FIXME: hmm, maybe up the %06 to %08... */
		}
		else
# endif /* ! ACCT_DYN_LOAD */
		{
		    char const * safename;
		    char *	 filename;

		    if (!(filename = strdup(username)))
		    {
				eventlog(eventlog_level_error,"account_create","could not allocate memory for filename");
				account_destroy(account);
				return NULL;
		    }
		    str_to_lower(filename);
		    if (!(safename = escape_fs_chars(filename,strlen(filename))))
		    {
				eventlog(eventlog_level_error,"account_create","could not escape username");
				account_destroy(account);
				free(filename);
				return NULL;
		    }
		    free(filename);
		    if (!(temp = malloc(strlen(prefs_get_userdir())+1+strlen(safename)+1))) /* dir + / + name + NUL */
		    {
				eventlog(eventlog_level_error,"account_create","could not allocate memory for temp");
				account_destroy(account);
				return NULL;
		    }
		    sprintf(temp,"%s/%s",prefs_get_userdir(),safename);
		    free((void *)safename); /* avoid warning */
		}
#endif /* WITH_DB_STORAGE */
		account->filename = temp;

	    SET_FLAG(account,account_flag_loaded);

		if (account_set_strattr(account,"BNET\\acct\\username",username)<0)
		{
	    	eventlog(eventlog_level_error,"account_create","could not set username");
		    account_destroy(account);
		    return NULL;
		}
#ifndef ACCT_DYN_LOAD
		if (account_set_numattr(account,"BNET\\acct\\userid",maxuserid+1)<0)
		{
		    eventlog(eventlog_level_error,"account_create","could not set userid");
		    account_destroy(account);
		    return NULL;
		}
#endif /* !ACCT_DYN_LOAD */
		if (account_set_strattr(account,"BNET\\acct\\passhash1",passhash1)<0)
		{
		    eventlog(eventlog_level_error,"account_create","could not set passhash1");
		    account_destroy(account);
		    return NULL;
		}

#ifdef WITH_BITS
		account_set_bits_state(account,account_state_valid);
		if (!bits_master) {
		    eventlog(eventlog_level_warn,"account_create","account_create should not be called on BITS clients");
		}
#endif /* WITH_BITS */
    }
#ifdef WITH_BITS
    else /* empty account to be filled in later */
		account_set_bits_state(account,account_state_valid);
#endif /* WITH_BITS */

    return account;
}

#ifdef WITH_BITS
extern t_account * create_vaccount(const char *username, unsigned int uid)
{
    /* this is a modified(?) version of account_create */
    t_account * account;

    account = malloc(sizeof(t_account));
    if (!account)
    {
		eventlog(eventlog_level_error,"create_vaccount","could not allocate memory for account");
		return NULL;
    }
    account->attrs    = NULL;
    account->age      = 0;
    CLEAR_FLAGS(account);

    account->namehash = 0; /* hash it later */
    account->uid      = 0;

    account->filename = NULL;	/* there is no local account file */
    account->bits_state = account_state_unknown;

    if (username)
    {
		if (username[0]!='#') {
			if (strchr(username,' '))
			{
				eventlog(eventlog_level_error,"create_vaccount","username contains spaces");
				account_destroy(account);
				return NULL;
			}
			if (strlen(username)>=MAX_USER_NAME)
			{
				eventlog(eventlog_level_error,"create_vaccount","username \"%s\" is too long (%u chars)",username,strlen(username));
				account_destroy(account);
				return NULL;
			}
			account_set_strattr(account,"BNET\\acct\\username",username);
            account->namehash = account_hash(username);
		} else {
			if (str_to_uint(&username[1],&account->uid)<0) {
				eventlog(eventlog_level_warn,"create_vaccount","invalid username \"%s\"",username);
			}
		}
    }
    account_set_numattr(account,"BNET\\acct\\userid",account->uid);
    return account;
}
#endif /* WITH_BITS */


static void account_unload_attrs(t_account * account)
{
    t_attribute const * attr;
    t_attribute const * temp;

    if (!account)
    {
		eventlog(eventlog_level_error,"account_unload_attrs","got NULL account");
		return;
    }

    for (attr=account->attrs; attr; attr=temp)
    {
		if (attr->key)
			free((void *)attr->key); /* avoid warning */
		if (attr->val)
			free((void *)attr->val); /* avoid warning */
        temp = attr->next;
		free((void *)attr); /* avoid warning */
    }
    account->attrs = NULL;
    CLEAR_FLAG(account,account_flag_loaded);
}


extern void account_destroy(t_account * account)
{
    if (!account)
    {
		eventlog(eventlog_level_error,"account_destroy","got NULL account");
		return;
    }
    account_unload_attrs(account);
    if (account->filename)
		free((void *)account->filename); /* avoid warning */
    free(account);
}


#ifndef ACCT_DYN_LOAD
extern unsigned int account_get_uid(t_account const * account)
{
    if (!account)
    {
		eventlog(eventlog_level_error,"account_get_uid","got NULL account");
		return 0;
    }

    return account->uid;
}
#endif /* !ACCT_DYN_LOAD */

extern int account_match(t_account * account, char const * username)
{
    unsigned int namehash;
    char const * tname;

    if (!account)
    {
		eventlog(eventlog_level_error,"account_match","got NULL account");
		return -1;
    }
    if (!username)
    {
		eventlog(eventlog_level_error,"account_match","got NULL username");
		return -1;
    }

    if (username[0]=='#')
		eventlog(eventlog_level_error,"account_match","got old style account name !!!"); /* FIXME: core dump? :> XXXXXX */

    namehash = account_hash(username);
    if (account->namehash==namehash && (tname = account_get_name(account)))
    {
        if (strcasecmp(tname,username)==0)
        {
			account_unget_name(tname);
			return 1;
		}
		else
			account_unget_name(tname);
    }

    return 0;
}


extern int account_save(t_account * account, unsigned int delta)
{
#ifdef WITH_STORAGE_DB
	t_db_result	  accsave = {0};
#else
    FILE *        accountfile;
    char *        tempname;
#endif /* WITH_STORAGE_DB */
    t_attribute * attr;
    char const *  key;
    char const *  val;

    if (!account)
    {
		eventlog(eventlog_level_error,"account_save","got NULL account");
		return -1;
    }


    /* account aging logic */
    if (TEST_FLAG(account,account_flag_accessed)) {
		account->age >>= 1;
    } else {
		account->age += delta;
	}
    if (account->age>( (3*prefs_get_user_flush_timer()) >>1)) {
        account->age = ( (3*prefs_get_user_flush_timer()) >>1);
	}
    CLEAR_FLAG(account,account_flag_accessed);
#ifdef WITH_BITS
    /* We do not have to save the account information to disk if we are a BITS client */
    if (!bits_master)
    {
        if (account->age>=prefs_get_user_flush_timer())
	    {
	        if (!connlist_find_connection_by_accountname(account_get_name(account)))
	        {
		        account_set_bits_state(account,account_state_delete);
		        /*	account_set_locked(account,3);  To be deleted */
		        /*	bits_va_unlock_account(account); */
	        }
	    }
	    return 0;
    }
#endif /* WITH_BITS */

    if (!account->filename)
    {
#ifdef WITH_BITS
		if (!bits_master) {
			return 0; /* It is OK since we do NOT have the files on the bits clients */
		}
#endif /* WITH_BITS */
#ifdef ACCT_DYN_LOAD
		eventlog(eventlog_level_error,"account_save","has NULL filename");
#else
		eventlog(eventlog_level_error,"account_save","account "UID_FORMAT" has NULL filename",account->uid);
#endif /* ACCT_DYN_LOAD */
		return -1;
    }
    if (!TEST_FLAG(account,account_flag_loaded))
		return 0;

    if (!TEST_FLAG(account,account_flag_dirty))
    {
		if (account->age>=prefs_get_user_flush_timer())
			account_unload_attrs(account);
		return 0;
    }

#ifndef WITH_STORAGE_DB
    if (!(tempname = malloc(strlen(prefs_get_userdir())+1+strlen(BNETD_ACCOUNT_TMP)+1)))
    {
		eventlog(eventlog_level_error,"account_save","unable to allocate memory for tempname");
		return -1;
    }

    sprintf(tempname,"%s/%s",prefs_get_userdir(),BNETD_ACCOUNT_TMP);

    if (!(accountfile = fopen(tempname,"w")))
    {
		eventlog(eventlog_level_error,"account_save","unable to open file \"%s\" for writing (fopen: %s)",tempname,strerror(errno));
		free(tempname);
		return -1;
    }
#endif /* !WITH_STORAGE_DB */

    for (attr=account->attrs; attr; attr=attr->next)
    {
		if (attr->key) {
			key = escape_chars(attr->key,strlen(attr->key));
		} else {
			eventlog(eventlog_level_error,"account_save","attribute with NULL key in list");
			key = NULL;
		}
		if (attr->val)
			val = escape_chars(attr->val,strlen(attr->val));
		else
		{
			eventlog(eventlog_level_error,"account_save","attribute with NULL val in list");
			val = NULL;
		}
		if (key && val)
		{
			if (strncmp("BNET\\CharacterDefault\\", key, 20) == 0)
			{
				eventlog(eventlog_level_debug,"account_save","skipping attribute key=\"%s\"",attr->key);
			}
			else
			{
				/*	        eventlog(eventlog_level_debug,"account_save","saving attribute key=\"%s\" val=\"%s\"",attr->key,attr->val); */
#ifdef WITH_STORAGE_DB
				db_account_save(account->filename,key,val,prefs_get_db_acc_table(),&accsave);
#else
				fprintf(accountfile,"\"%s\"=\"%s\"\n",key,val);
#endif /* WITH_STORAGE_DB */
			}
		} else {
			eventlog(eventlog_level_error,"account_save","could not save attribute key=\"%s\"",attr->key);
		}
		if (key) {
			free((void *)key); /* avoid warning */
		}
		if (val) {
			free((void *)val); /* avoid warning */
		}
    }

#ifdef WITH_STORAGE_DB
	db_close(&accsave.mysql);
#else
    if (fclose(accountfile)<0)
    {
		eventlog(eventlog_level_error,"account_save","could not close account file \"%s\" after writing (fclose: %s)",tempname,strerror(errno));
		free(tempname);
		return -1;
    }

# ifdef WIN32
    /* We are about to rename the temporary file
	 * to replace the existing account. In Windows,
	 * we have to remove the previous file or the
	 * rename function will fail saying the file
	 * already exists. This defeats the purpose of
	 * the rename which was to make this an atomic
	 * operation. At least the race window is small.
	 */
    if (access(account->filename, 0) == 0)
    {
		if (remove(account->filename)<0)
		{
			eventlog(eventlog_level_error,"account_save","could not delete account file \"%s\" (remove: %s)",account->filename,strerror(errno));
			free(tempname);
			return -1;
		}
    }
# endif /* WIN32 */

    if (rename(tempname,account->filename)<0)
    {
		eventlog(eventlog_level_error,"account_save","could not rename account file to \"%s\" (rename: %s)",account->filename,strerror(errno));
		free(tempname);
		return -1;
    }

    free(tempname);
#endif /* WITH_STORAGE_DB */
    CLEAR_FLAG(account,account_flag_dirty);

    return 1;
}


static int account_insert_attr(t_account * account, char const * key, char const * val)
{
    t_attribute * nattr;
    char *        nkey;
    char *        nval;

    if (!(nattr = malloc(sizeof(t_attribute))))
    {
		eventlog(eventlog_level_error,"account_insert_attr","could not allocate attribute");
		return -1;
    }
    if (!(nkey = strdup(key)))
    {
		eventlog(eventlog_level_error,"account_insert_attr","could not allocate attribute key");
		free(nattr);
		return -1;
    }
    if (!(nval = strdup(val)))
    {
		eventlog(eventlog_level_error,"account_insert_attr","could not allocate attribute value");
		free(nkey);
		free(nattr);
		return -1;
    }
    nattr->key  = nkey;
    nattr->val  = nval;
    nattr->next = account->attrs;

    account->attrs = nattr;

    return 0;
}


#ifdef DEBUG_ACCOUNT
extern char const * account_get_strattr_real(t_account * account, char const * key, char const * fn, unsigned int ln)
#else
extern char const * account_get_strattr(t_account * account, char const * key)
#endif /* DEBUG_ACCOUNT */
{
    t_attribute * curr;
    t_attribute * prev;
    char const *  newkey;

    if (!account)
    {
#ifdef DEBUG_ACCOUNT
		eventlog(eventlog_level_error,"account_get_strattr","got NULL account (from %s:%u)",fn,ln);
#else
		eventlog(eventlog_level_error,"account_get_strattr","got NULL account");
#endif /* DEBUG_ACCOUNT */
		return NULL;
    }
    if (!key)
    {
#ifdef DEBUG_ACCOUNT
		eventlog(eventlog_level_error,"account_get_strattr","got NULL key (from %s:%u)",fn,ln);
#else
		eventlog(eventlog_level_error,"account_get_strattr","got NULL key");
#endif /* DEBUG_ACCOUNT */
		return NULL;
    }

    SET_FLAG(account,account_flag_accessed);

    if (!TEST_FLAG(account,account_flag_loaded))
        if (account_load_attrs(account)<0)
		{
			eventlog(eventlog_level_error,"account_get_strattr","could not load attributes");
			return NULL;
		}

    if (strncasecmp(key,"DynKey",6)==0)
    {
		char * temp;

		/* Recent Starcraft clients seems to query DynKey\*\1\rank instead of
		 * Record\*\1\rank. So replace Dynkey with Record for key lookup.
		 */
		if (!(temp = strdup(key)))
		{
			eventlog(eventlog_level_error,"account_get_strattr","could not allocate memory for temp");
			return NULL;
		}
		strncpy(temp,"Record",6);
		newkey = temp;
    }
    else
		newkey = key;

    if (account->attrs)
		for (curr=account->attrs; curr; curr=curr->next)
		{
			if (strcasecmp(curr->key,newkey)==0)
			{
				if (newkey!=key)
					free((void *)newkey); /* avoid warning */
				if (curr!=account->attrs)
				{
					prev->next = curr->next;
					curr->next = account->attrs;
					account->attrs = curr;
				}
#ifdef TESTUNGET
				return strdup(curr->val);
#else
                return curr->val;
#endif /* TESTUNGET */
			}
			prev = curr;
		}
    if (newkey!=key) {
		free((void *)newkey); /* avoid warning */
	}

    if (account==default_acct) { /* do NOT recurse infinitely */
		return NULL;
	}

    return account_get_strattr(default_acct,key); /* FIXME: this is sorta dangerous because this pointer can go away if we re-read the config files... verify that nobody caches non-username, userid strings */
}


extern int account_unget_strattr(char const * val)
{
    if (!val)
    {
		eventlog(eventlog_level_error,"account_unget_strattr","got NULL val");
		return -1;
    }
#ifdef TESTUNGET
    free((void *)val); /* avoid warning */
#endif /* TESTUNGET */
    return 0;
}

#ifdef WITH_BITS
extern int account_set_strattr(t_account * account, char const * key, char const * val)
{
	char const * oldvalue;

	if (!account) {
		eventlog(eventlog_level_error,"account_set_strattr(bits)","got NULL account");
		return -1;
	}

	oldvalue = account_get_strattr(account,key); /* To check whether the value has changed. */
	if (oldvalue) {
	    if (val && strcmp(oldvalue,val)==0) {
			account_unget_strattr(oldvalue);
			return 0; /* The value has NOT changed. Do NOT produce unnecessary traffic. */
	    }
	    /* The value must have changed. Send the update to the msster and update local account. */
	    account_unget_strattr(oldvalue);
	}
	if (send_bits_va_set_attr(account_get_uid(account),key,val,NULL)<0) return -1;
	return account_set_strattr_nobits(account,key,val);
}

extern int account_set_strattr_nobits(t_account * account, char const * key, char const * val)
#else
extern int account_set_strattr(t_account * account, char const * key, char const * val)
#endif /* WITH_BITS */
{
    t_attribute * curr;

    if (!account)
    {
		eventlog(eventlog_level_error,"account_set_strattr","got NULL account");
		return -1;
    }
    if (!key)
    {
		eventlog(eventlog_level_error,"account_set_strattr","got NULL key");
		return -1;
    }

#ifndef WITH_BITS
    if (!TEST_FLAG(account,account_flag_loaded))
        if (account_load_attrs(account)<0)
		{
			eventlog(eventlog_level_error,"account_set_strattr","could not load attributes");
			return -1;
		}
#endif /* !WITH_BITS */
    curr = account->attrs;
    if (!curr) /* if no keys in attr list then we need to insert it */
    {
		if (val)
		{
			SET_FLAG(account,account_flag_dirty); /* we are inserting an entry */
			return account_insert_attr(account,key,val);
		}
		return 0;
    }

    if (strcasecmp(curr->key,key)==0) /* if key is already the first in the attr list */
    {
		if (val)
		{
			char * temp;

			if (!(temp = strdup(val)))
			{
				eventlog(eventlog_level_error,"account_set_strattr","could not allocate attribute value");
				return -1;
			}

			if (strcmp(curr->val,temp)!=0)
				SET_FLAG(account,account_flag_dirty); /* we are changing an entry */
			free((void *)curr->val); /* avoid warning */
			curr->val = temp;
		}
		else
		{
			t_attribute * temp;

			temp = curr->next;

			SET_FLAG(account,account_flag_dirty); /* we are deleting an entry */
			free((void *)curr->key); /* avoid warning */
			free((void *)curr->val); /* avoid warning */
			free((void *)curr); /* avoid warning */

			account->attrs = temp;
		}
		return 0;
    }

    for (; curr->next; curr=curr->next)
		if (strcasecmp(curr->next->key,key)==0)
			break;

    if (curr->next) /* if key is already in the attr list */
    {
		if (val)
		{
			char * temp;

			if (!(temp = strdup(val)))
			{
				eventlog(eventlog_level_error,"account_set_strattr","could not allocate attribute value");
				return -1;
			}

			if (strcmp(curr->next->val,temp)!=0)
				SET_FLAG(account,account_flag_dirty); /* we are changing an entry */
			free((void *)curr->next->val); /* avoid warning */
			curr->next->val = temp;
		}
		else
		{
			t_attribute * temp;

			temp = curr->next->next;

			SET_FLAG(account,account_flag_dirty); /* we are deleting an entry */
			free((void *)curr->next->key); /* avoid warning */
			free((void *)curr->next->val); /* avoid warning */
			free(curr->next);

			curr->next = temp;
		}
		return 0;
    }

    if (val)
    {
		SET_FLAG(account,account_flag_dirty); /* we are inserting an entry */
		return account_insert_attr(account,key,val);
    }
    return 0;
}


static int account_load_attrs(t_account * account)
{
#ifdef WITH_STORAGE_DB
    t_db_result accload = {0};
#else
    FILE * accountfile;
#endif /* WITH_STORAGE_DB */
    char const * key;
    unsigned int line;
    char const * buff;
    unsigned int len;
    char *       esckey;
    char *       escval;
    char const * val;

    if (!account)
    {
		eventlog(eventlog_level_error,"account_load_attrs","got NULL account");
		return -1;
    }
    if (!account->filename)
    {
#ifndef WITH_BITS
		eventlog(eventlog_level_error,"account_load_attrs","account has NULL filename");
		return -1;
#else /* WITH_BITS */
		if (!bits_uplink_connection) {
			eventlog(eventlog_level_error,"account_load_attrs","account has NULL filename on BITS master");
			return -1;
		}
		if (account->uid==0) {
			eventlog(eventlog_level_debug,"account_load_attrs","userid is unknown");
			return 0;
		} else if (!TEST_FLAG(account,account_flag_loaded)) {
			if (account_get_bits_state(account)==account_state_valid) {
				eventlog(eventlog_level_debug,"account_load_attrs","bits: virtual account "UID_FORMAT": loading attrs",account->uid);
				send_bits_va_get_allattr(account->uid);
			} else {
				eventlog(eventlog_level_debug,"account_load_attrs","waiting for account "UID_FORMAT" to be locked",account->uid);
			}
			return 0;
		}
#endif /* WITH_BITS */
    }

    if (TEST_FLAG(account,account_flag_loaded)) /* already done */
		return 0;
    if (TEST_FLAG(account,account_flag_dirty)) /* if not loaded, how can it be dirty? */
    {
		eventlog(eventlog_level_error,"account_load_attrs","can not load modified account");
		return -1;
    }

    eventlog(eventlog_level_trace,"account_load_attrs","loading \"%s\"",account->filename);
#ifdef WITH_STORAGE_DB
    if (db_account_load(account->filename,prefs_get_db_acc_table(),&accload) < 0) {
	    return -1;
	}
#else
    if (!(accountfile = fopen(account->filename,"r")))
    {
	    eventlog(eventlog_level_error,"account_load_attrs","could not open account file \"%s\" for reading (fopen: %s)",account->filename,strerror(errno));
	    return -1;
    }
#endif /* WITH_STORAGE_DB */

    SET_FLAG(account,account_flag_loaded); /* set now so set_strattr works */
#ifdef WITH_STORAGE_DB
    for (line=1; (buff=db_account_load_getnextattrib(&accload)); line++)
#else
		for (line=1; (buff=file_get_line(accountfile)); line++)
#endif /* WITH_STORAGE_DB */
		{
			if (buff[0]=='#' || buff[0]=='\0')
			{
				free((void *)buff); /* avoid warning */
				continue;
			}
			if (strlen(buff)<6) /* "?"="" */
			{
				eventlog(eventlog_level_error,"account_load_attrs","malformed line %d of account file \"%s\"",line,account->filename);
				free((void *)buff); /* avoid warning */
				continue;
			}

			len = strlen(buff)-5+1; /* - ""="" + NUL */
			if (!(esckey = malloc(len)))
			{
				eventlog(eventlog_level_error,"account_load_attrs","could not allocate memory for esckey on line %d of account file \"%s\"",line,account->filename);
				free((void *)buff); /* avoid warning */
				continue;
			}
			if (!(escval = malloc(len)))
			{
				eventlog(eventlog_level_error,"account_load_attrs","could not allocate memory for escval on line %d of account file \"%s\"",line,account->filename);
				free((void *)buff); /* avoid warning */
				free(esckey);
				continue;
			}

			if (sscanf(buff,"\"%[^\"]\" = \"%[^\"]\"",esckey,escval)!=2)
			{
				if (sscanf(buff,"\"%[^\"]\" = \"\"",esckey)!=1) /* hack for an empty value field */
				{
					eventlog(eventlog_level_error,"account_load_attrs","malformed entry on line %d of account file \"%s\"",line,account->filename);
					free(escval);
					free(esckey);
					free((void *)buff); /* avoid warning */
					continue;
				}
				escval[0] = '\0';
			}
#ifdef WITH_STORAGE_DB
			/* Result buffer is handled by DB and is freed later.
			 * The key in DB is already in unescaped form.
			 * Therefore a simple copy should be enough.
			 */
			key = esckey;
#else
			free((void *)buff); /* avoid warning */
			key = unescape_chars(esckey);
			free(esckey);
#endif /* WITH_STORAGE_DB */
			val = unescape_chars(escval);
			free(escval);

			if (key && val)
#ifdef WITH_BITS
				account_set_strattr_nobits(account,key,val);
#else
			account_set_strattr(account,key,val);
#endif /* WITH_BITS */
			if (key) {
				free((void *)key); /* avoid warning */
			}
			if (val) {
				free((void *)val); /* avoid warning */
			}
		}

#ifdef WITH_STORAGE_DB
    db_free_result(accload.res);
    db_close(&accload.mysql);
#else
    if (fclose(accountfile)<0) {
		eventlog(eventlog_level_error,"account_load_attrs","could not close account file \"%s\" after reading (fclose: %s)",account->filename,strerror(errno));
	}
#endif /* WITH_STORAGE_DB */
    CLEAR_FLAG(account,account_flag_dirty);
#ifdef WITH_BITS
    account_set_bits_state(account,account_state_valid);
#endif /* WITH_BITS */
    return 0;
}


static t_account * account_load(char const * filename)
{
    t_account * account;

    if (!filename)
    {
		eventlog(eventlog_level_error,"account_load","got NULL filename");
		return NULL;
    }

    if (!(account = account_create(NULL,NULL)))
    {
		eventlog(eventlog_level_error,"account_load","could not load account from file \"%s\"",filename);
		return NULL;
    }
    if (!(account->filename = strdup(filename)))
    {
		eventlog(eventlog_level_error,"account_load","could not allocate memory for account->filename");
		account_destroy(account);
		return NULL;
    }
#ifdef ACCT_DYN_UNLOAD
    account->ref = 0;
#endif /* ACCT_DYN_UNLOAD */

    return account;
}


extern int accountlist_load_default(void)
{
    if (default_acct)
    	account_destroy(default_acct);

#ifdef WITH_STORAGE_DB
    if (!(default_acct = account_load("default_user")))
#else
		if (!(default_acct = account_load(prefs_get_defacct())))
#endif /* WITH_STORAGE_DB */
		{
			eventlog(eventlog_level_error,"accountlist_load_default","could not load default account template from file \"%s\"",prefs_get_defacct());
			return -1;
		}
    if (account_load_attrs(default_acct)<0)
    {
    	eventlog(eventlog_level_error,"accountlist_load_default","could not load default account template attributes");
	    return -1;
    }

    eventlog(eventlog_level_debug,"accountlist_load_default","loaded default account template");
    return 0;
}


extern int accountlist_create(void)
{
#ifdef WITH_STORAGE_DB
    t_db_result	 acclist = {0};
#else
    t_pdir *     accountdir;
#endif /* WITH_STORAGE_DB */
    char const * dentry;
    char *       pathname;
    t_account *  account;
    unsigned int count;

    if (!(accountlist_head = hashtable_create(prefs_get_hashtable_size())))
    {
    	eventlog(eventlog_level_error,"accountlist_create","could not create accountlist_head");
	    return -1;
    }

#ifdef WITH_BITS
    if (!bits_master)
    {
    	eventlog(eventlog_level_info,"accountlist_create","running as BITS client -> no accounts loaded");
	    return 0;
    }
#endif /* WITH_BITS */

#ifdef ACCT_DYN_LOAD
    return 0;
#endif /* ACCT_DYN_LOAD */

#ifndef WITH_STORAGE_DB
    if (!(accountdir = p_opendir(prefs_get_userdir())))
    {
        eventlog(eventlog_level_error,"accountlist_create","unable to open user directory \"%s\" for reading (p_opendir: %s)",prefs_get_userdir(),strerror(errno));
        return -1;
    }
#endif /* !WITH_STORAGE_DB */

    force_account_add = 1; /* disable the protection */
    count = 0;

#ifdef WITH_STORAGE_DB
    if (db_account_list(prefs_get_db_acc_table(),&acclist)<0) {
		return -1;
	}
    while ((dentry = db_account_list_getnext(&acclist)))
    {
        if (!(pathname = malloc(strlen(dentry)+1)))
		{
    	    eventlog(eventlog_level_error,"accountlist_create","could not allocate memory for name");
            continue;
		}
        sprintf(pathname,"%s",dentry);
#else
		while ((dentry = p_readdir(accountdir)))
		{
			if (dentry[0]=='.')
				continue;
			if (!(pathname = malloc(strlen(prefs_get_userdir())+1+strlen(dentry)+1))) /* dir + / + file + NUL */
			{
				eventlog(eventlog_level_error,"accountlist_create","could not allocate memory for pathname");
				continue;
			}
			sprintf(pathname,"%s/%s",prefs_get_userdir(),dentry);
#endif /* WITH_STORAGE_DB */

			if (!(account = account_load(pathname)))
			{
				eventlog(eventlog_level_error,"accountlist_create","could not load account from file \"%s\"",pathname);
				free(pathname);
				continue;
			}

			if (!accountlist_add_account(account))
			{
				eventlog(eventlog_level_error,"accountlist_create","could not add account from file \"%s\" to list",pathname);
				free(pathname);
				account_destroy(account);
				continue;
			}

			free(pathname);

			/* might as well free up the memory since we probably will NOT need it */
			CLEAR_FLAG(account,account_flag_accessed); /* lie */
			account_save(account,1000); /* big delta to force unload */

			count++;
		}

		force_account_add = 0; /* enable the protection */

#ifdef WITH_STORAGE_DB
		db_free_result(acclist.res);
		db_close(&acclist.mysql);
#else
		if (p_closedir(accountdir)<0) {
			eventlog(eventlog_level_error,"accountlist_create","unable to close user directory \"%s\" (p_closedir: %s)",prefs_get_userdir(),strerror(errno));
		}

		eventlog(eventlog_level_info,"accountlist_create","loaded %u user accounts",count);
#endif /* WITH_STORAGE_DB */
		return 0;
	}


	extern int accountlist_destroy(void)
	{
		t_entry *   curr;
		t_account * account;

		HASHTABLE_TRAVERSE(accountlist_head,curr)
		{
			if (!(account = entry_get_data(curr)))
				eventlog(eventlog_level_error,"accountlist_destroy","found NULL account in list");
			else
			{
				if (account_save(account,0)<0)
					eventlog(eventlog_level_error,"accountlist_destroy","could not save account");

				account_destroy(account);
			}
			hashtable_remove_entry(accountlist_head,curr);
		}

		if (hashtable_destroy(accountlist_head)<0)
			return -1;
		accountlist_head = NULL;
		return 0;
	}


	extern t_hashtable * accountlist(void)
	{
		return accountlist_head;
	}


	extern void accountlist_unload_default(void)
	{
		account_destroy(default_acct);
	}


	extern unsigned int accountlist_get_length(void)
	{
		return hashtable_get_length(accountlist_head);
	}


	extern int accountlist_save(unsigned int delta)
	{
		t_entry *    curr;
		t_account *  account;
		unsigned int scount;
		unsigned int tcount;

		scount=tcount = 0;
		HASHTABLE_TRAVERSE(accountlist_head,curr)
		{
			account = entry_get_data(curr);
			switch (account_save(account,delta))
			{
				case -1:
					eventlog(eventlog_level_error,"accountlist_save","could not save account");
					break;
				case 1:
					scount++;
					break;
				case 0:
				default:
					break;
			}
			tcount++;
		}

#ifdef WITH_BITS
		bits_va_lock_check();
#endif /* WITH_BITS */
#ifdef ACCT_DYN_UNLOAD
		accountlist_remove_unused(); /* FIXME make configurable XXXXXXXXX */
#endif /* ACCT_DYN_UNLOAD */
		if (scount>0) {
			eventlog(eventlog_level_debug,"accountlist_save","saved %u of %u user accounts",scount,tcount);
		}
		return 0;
	}


	extern t_account * accountlist_find_account(char const * username)  /* username | #uid */
	{
		t_entry   * curr;
		t_account * account;

		if (!username)
		{
			eventlog(eventlog_level_error,"accountlist_find_account","got NULL username");
			return NULL;
		}

		if (username[0]=='#') {     /* username = '#'+uid */
			HASHTABLE_TRAVERSE(accountlist_head,curr)
			{
				account = entry_get_data(curr);
#ifdef ACCT_DYN_LOAD
				/* not sure if this is the right approach... might want to just
				 * make "uid" available in the relevant struct in account.h
				 * without the ifdef instead...
				 */
				if (atoi(&username[1])>0) {
#else
				if (atoi(&username[1])>0 && atoi(&username[1]) == account->uid) {
#endif /* ACCT_DYN_LOAD */
					hashtable_entry_release(curr);
					return account;
				}
			}
		}
		else    /* username = username */
		{
			unsigned int namehash;
			char const * tname;

			namehash = account_hash(username);
			HASHTABLE_TRAVERSE_MATCHING(accountlist_head,curr,namehash)
			{
				account = entry_get_data(curr);
				if ((tname = account_get_name(account)))
				{
					if (strcasecmp(tname,username)==0)
					{
						account_unget_name(tname);
						hashtable_entry_release(curr);
						return account;
					}
					else
						account_unget_name(tname);
				}
			}
		}
#ifdef ACCT_DYN_LOAD
		{
			char * pathname;
			char * filename;
			char * safename;

			if (!(filename = strdup(username)))
			{
				eventlog(eventlog_level_error,"accountlist_find_account","could not allocate memory for filename");
				free(pathname);
				return NULL;
			}
			str_to_lower(filename);
			if (!(safename = escape_fs_chars(filename,strlen(filename))))
			{
				eventlog(eventlog_level_error,"accountlist_find_account","could not escape filename");
				free(filename);
				return NULL;
			}
			free(filename);
			if (!(pathname = malloc(strlen(prefs_get_userdir())+1+strlen(safename)+1)))
			{
				eventlog(eventlog_level_error,"accountlist_find_account","could not allocate memory for pathname");
				free(filename);
				free(safename);
				return NULL;
			}

			sprintf(pathname,"%s/%s",prefs_get_userdir(),safename);
			free(safename);
# ifndef WITH_BITS /* FIXME: and what if BITS _are_ enabled? */
			{
				FILE * accountfile;

				if (!(accountfile = fopen(pathname,"r")))
				{
					free(pathname);
					return NULL;
				} else {
					fclose(accountfile);
				}
			}
# endif /* WITH_BITS */
			if (!(account = account_load(pathname)))
			{
				eventlog(eventlog_level_error,"accountlist_find_account","could not load account");
				free(pathname);
				return NULL;
			}
			if (!accountlist_add_account(account))
			{
				eventlog(eventlog_level_error,"accountlist_find_account","could not add account to list");
				free(pathname);
				account_destroy(account);
				return NULL;
			}
			free(pathname);
			return accountlist_find_account(username);
		}
#endif /* ACCT_DYN_LOAD */

		return NULL;
	}


	extern int accountlist_allow_add(void)
	{
#ifdef WITH_BITS
		/* Client may tend to fill the accountlist with junk... let them proceed */
		if (!bits_master) {
			return 1;
		}
#endif /* WITH_BITS */

		if (force_account_add)
			return 1; /* the permission was forced */

		if (prefs_get_max_accounts()==0)
			return 1; /* allow infinite accounts */

		if (prefs_get_max_accounts()<=hashtable_get_length(accountlist_head))
			return 0; /* maximum account limit reached */

		return 1; /* otherwise let them proceed */
	}

	extern t_account * accountlist_add_account(t_account * account)
	{
		char const * username;
#ifndef ACCT_DYN_LOAD
		unsigned int uid;
#endif /* !ACCT_DYN_LOAD */

		if (!account)
		{
			eventlog(eventlog_level_error,"accountlist_add_account","got NULL account");
			return NULL;
		}

		username = account_get_strattr(account,"BNET\\acct\\username");
#ifndef ACCT_DYN_LOAD
		uid = account_get_numattr(account,"BNET\\acct\\userid");
#endif /* !ACCT_DYN_LOAD */

		if (!username || strlen(username)<1)
		{
			eventlog(eventlog_level_error,"accountlist_add_account","got bad account (empty username)");
			return NULL;
		}
#ifndef ACCT_DYN_LOAD
		if (uid<1)
		{
# ifndef WITH_BITS
			eventlog(eventlog_level_error,"accountlist_add_account","got bad account (bad uid)");
			account_unget_name(username);
			return NULL;
# else
			uid = 0;
# endif /* !WITH_BITS */
		}
#endif /* !ACCT_DYN_LOAD */

		/* check whether the account limit was reached */
		if (!accountlist_allow_add()) {
			eventlog(eventlog_level_warn,"accountlist_add_account","account limit reached (current is %u, storing %u)",prefs_get_max_accounts(),hashtable_get_length(accountlist_head));
			return NULL;
		}

		/* delayed hash, do it before inserting account into the list */
		account->namehash = account_hash(username);
#ifndef ACCT_DYN_LOAD
		account->uid = uid;
#endif /* !ACCT_DYN_LOAD */
		/* mini version of accountlist_find_account(username) || accountlist_find_account(uid)  */
		{
			t_entry *    curr;
			t_account *  curraccount;
			char const * tname;

			HASHTABLE_TRAVERSE(accountlist_head,curr)
			{
				curraccount = entry_get_data(curr);
#ifndef ACCT_DYN_LOAD
				if (curraccount->uid==uid)
				{
					eventlog(eventlog_level_error,"accountlist_add_account","user \"%s\":"UID_FORMAT" already has an account (\"%s\":"UID_FORMAT")",username,uid,(tname = account_get_name(curraccount)),curraccount->uid);
					account_unget_name(tname);
					hashtable_entry_release(curr);
					account_unget_strattr(username);
					return NULL;
				}
#endif /* !ACCT_DYN_LOAD */
				if (curraccount->namehash==account->namehash && (tname = account_get_name(curraccount)))
				{
					if (strcasecmp(tname,username)==0)
					{
#ifdef ACCT_DYN_LOAD
						eventlog(eventlog_level_error,"accountlist_add_account","user %s already has an account",tname);
#else
						eventlog(eventlog_level_error,"accountlist_add_account","user \"%s\":"UID_FORMAT" already has an account (\"%s\":"UID_FORMAT")",username,uid,tname,curraccount->uid);
#endif /* ACCT_DYN_LOAD */

						account_unget_name(tname);
						hashtable_entry_release(curr);
						account_unget_strattr(username);
						return NULL;
					}
					else
						account_unget_name(tname);
				}
			}
		}
		account_unget_strattr(username);

		if (hashtable_insert_data(accountlist_head,account,account->namehash)<0)
		{
			eventlog(eventlog_level_error,"accountlist_add_account","could not add account to list");
			return NULL;
		}

#ifndef ACCT_DYN_LOAD
		if (uid>maxuserid) {
			maxuserid = uid;
		}
#endif /* !ACCT_DYN_LOAD */

		return account;
	}

#ifdef WITH_BITS

	extern int accountlist_remove_account(t_account const * account)
	{
		return hashtable_remove_data(accountlist_head,account,account->namehash);
	}

	/* This function checks whether the server knows if an account exists or not.
	 * It returns 1 if the server knows the account and 0 if the server does NOT
	 * know whether it exists. */
	extern int account_name_is_unknown(char const * name)
	{
		t_account * account;

		if (!name) {
			eventlog(eventlog_level_error,"account_name_is_unknown","got NULL name");
			return -1;
		}
		if (bits_master)
			return 0; /* The master server knows about all accounts */
		account = accountlist_find_account(name);
		if (!account)
			return 1; /* not in the accountlist */
		else if (account_get_bits_state(account)==account_state_unknown)
			return 1; /* in the accountlist, but still unknown */
		return 0; /* account is known */
	}

	extern int account_state_is_pending(t_account const * account)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_state_is_pending","got NULL account");
			return -1;
		}
		if (account_get_bits_state(account)==account_state_pending)
			return 1;
		else
			return 0;
	}

	extern int account_is_ready_for_use(t_account const * account)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_is_ready_for_use","got NULL account");
			return -1;
		}
		if ((account_get_bits_state(account)==account_state_valid)&&(account_is_loaded(account)))
			return 1;
		return 0;
	}

	extern int account_is_invalid(t_account const * account)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_is_invalid","got NULL account");
			return -1;
		}
		if ((account_get_bits_state(account)==account_state_invalid)||(account_get_bits_state(account)==account_state_delete))
			return 1;
		return 0;
	}

	extern t_bits_account_state account_get_bits_state(t_account const * account)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_get_bits_state","got NULL account");
			return -1;
		}
		return account->bits_state;
	}

	extern int account_set_bits_state(t_account * account, t_bits_account_state state)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_get_bits_state","got NULL account");
			return -1;
		}
		account->bits_state = state;
		return 0;
	}


	extern int account_set_accessed(t_account * account, int accessed) {
		if (!account) {
			eventlog(eventlog_level_error,"account_set_accessed","got NULL account");
			return -1;
		}
		if (accessed)
			SET_FLAG(account,account_flag_accessed);
		else
			CLEAR_FLAG(account,account_flag_accessed);
		return 0;
	}

	extern int account_is_loaded(t_account const * account)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_is_loaded","got NULL account");
			return -1;
		}
		return TEST_FLAG(account,account_flag_loaded);
	}

	extern int account_set_loaded(t_account * account, int loaded)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_set_loaded","got NULL account");
			return -1;
		}
		if (loaded)
			SET_FLAG(account,account_flag_loaded);
		else
			CLEAR_FLAG(account,account_flag_loaded);
		return 0;
	}

	extern int account_set_uid(t_account * account, int uid)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_set_uid","got NULL account");
			return -1;
		}
		account->uid = uid;
		return account_set_numattr(account,"BNET\\acct\\userid",uid);
	}
#endif /* WITH_BITS */


	extern char const * account_get_first_key(t_account * account)
	{
		if (!account) {
			eventlog(eventlog_level_error,"account_get_first_key","got NULL account");
			return NULL;
		}
		if (!account->attrs) {
			return NULL;
		}
		return account->attrs->key;
	}


	extern char const * account_get_next_key(t_account * account, char const * key)
	{
		t_attribute * attr;

		if (!account) {
			eventlog(eventlog_level_error,"account_get_first_key","got NULL account");
			return NULL;
		}
		attr = account->attrs;
		while (attr) {
			if (strcmp(attr->key,key)==0) {
				if (attr->next) {
					return attr->next->key;
				} else {
					return NULL;
				}
			}
			attr = attr->next;
		}
		return NULL;
	}


	static int account_check_name(char const * name)
	{
		unsigned int i;

		if (!name) {
			eventlog(eventlog_level_error,"account_check_name","got NULL name");
			return -1;
		}

#ifndef GAMEI_SERVER
		if (!isalnum((int)name[0])) {
			return -1;
		}
#endif /* !GAMEI_SERVER */

		for (i=0; i<strlen(name); i++)
		{
#if 0
			/* These are the Battle.net rules but they are too strict.
			 * We want to allow any characters that would NOT cause
			 * problems so this should test for what is _not_ allowed
			 * instead of what is.
			 */
			ch = name[i];
			if (isalnum(ch)) continue;
			if (ch=='-') continue;
			if (ch=='_') continue;
			if (ch=='.') continue;
			return -1;
#else
			if (name[i]==' ') {
				return -1;
			}
			if (name[i]==',') {
				return -1;
			}
			if (i==0 && name[i]=='#') {
				return -1;
			}
			/* what about * and @? */
#endif /* 0 */
		}
		if (i<MIN_USER_NAME || i>=MAX_USER_NAME)
			return -1;
		return 0;
	}


#ifdef ACCT_DYN_UNLOAD
# ifdef CHECK_ACCT_REFS
	static void accountlist_check_refs(void)
	{
		unsigned int real;
		t_entry *    curr;
		t_account *  account;
		char * 		 tname;


		HASHTABLE_TRAVERSE(accountlist_head,curr)
		{
			if (!(account = entry_get_data(curr)))
				eventlog(eventlog_level_error,"accountlist_destroy","found NULL account in list");
			else
			{
				real = conn_count_acc_ref(account) + friends_count_acc_ref(account) +
				game_count_acc_ref(account) + watch_count_acc_ref(account);
				if ((real != account->ref) && (real + account->ref > 1))
				{
					eventlog(eventlog_level_error,"accountlist_check_refs","accname: %s  account->ref: %u  real: %u",(tname = account_get_name(account)),account->ref,real);
					account_unget_name(tname);
				}
			}
		}
	}
# endif /* CHECK_ACCT_REFS */

	extern unsigned int account_inc_ref(t_account * account)
	{
		account->ref++;

# ifdef CHECK_ACCT_REFS
		accountlist_check_refs();
# endif /* CHECK_ACCT_REFS */

		return account->ref;
	}

	extern unsigned int account_dec_ref(t_account * account)
	{
		if (account->ref > 0)
			account->ref--;
		else
			eventlog(eventlog_level_error,"account_dec_ref","try do decrement when ref=0");
# ifdef CHECK_ACCT_REFS
		accountlist_check_refs();
# endif /* CHECK_ACCT_REFS */

		return account->ref;
	}

	extern unsigned int account_get_ref(t_account * account)
	{
		return account->ref;
	}

	extern void accountlist_log_ref()
	{
		t_entry *    curr;
		t_account *  account;
		char const * tname;
		unsigned int tref;

		HASHTABLE_TRAVERSE(accountlist_head,curr)
		{
			account = entry_get_data(curr);
			tname = account_get_name(account);
			tref = account_get_ref(account);
			eventlog(eventlog_level_debug,"accountlist_log_ref","%s: %u",tname,tref);
			account_unget_name(tname);
		}
	}

	static void accountlist_remove_unused()
	{
		t_entry *    curr;
		t_account *  account;
		unsigned int ftimer;

		ftimer = prefs_get_user_flush_timer();
		HASHTABLE_TRAVERSE(accountlist_head,curr)
		{
			if (!(account = entry_get_data(curr))) {
				eventlog(eventlog_level_error,"accountlist_remove_unused","found NULL account in list");
			} else {
				if (account->ref == 0)
				{
					account_destroy(account);
					hashtable_remove_entry(accountlist_head,curr);
				}
		}
		hashtable_purge(accountlist_head);
	}

#endif /* ACCT_DYN_UNLOAD */

/* EOF */
