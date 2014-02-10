/* mail.h
 * Copyright (C) 2001            Dizzy (dizzy@roedu.net)
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


/*****/
#ifndef INCLUDED_MAIL_TYPES
#define INCLUDED_MAIL_TYPES


#define MAX_FUNC_LEN 10
#define MAX_MAIL_QUOTA 10
#define MAIL_FUNC_SEND 1
#define MAIL_FUNC_READ 2
#define MAIL_FUNC_DELETE 3
#define MAIL_FUNC_SUMMARY 4
#define MAIL_FUNC_HELP 5
#define MAIL_FUNC_UNKNOWN 6


#ifdef MAIL_INTERNAL_ACCESS

#ifdef JUST_NEED_TYPES
# ifdef TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   ifdef HAVE_TIME_H
#    include <time.h>
#   else
#    warning mail.h expects a time-related header to be included.
#   endif /* HAVE_TIME_H */
#  endif /* HAVE_SYS_TIME_H */
# endif /* TIME_WITH_SYS_TIME */
# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# else
#  warning mail.h expects <sys/types.h> to be included.
# endif /* HAVE_SYS_TYPES_H */
# include "compat/pdir.h"
#else
# define JUST_NEED_TYPES
# ifdef TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   ifdef HAVE_TIME_H
#    include <time.h>
#   else
#    warning mail.h expects a time-related header to be included.
#   endif /* HAVE_TIME_H */
#  endif /* HAVE_SYS_TIME_H */
# endif /* TIME_WITH_SYS_TIME */
# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# else
#  warning mail.h expects <sys/types.h> to be included.
# endif /* HAVE_SYS_TYPES_H */
# include "compat/pdir.h"
# undef JUST_NEED_TYPES
#endif /* JUST_NEED_TYPES */

typedef struct mailbox_struct {
    t_pdir *     maildir;
    char *       path;
} t_mailbox;

typedef struct mail_struct {
    char * sender;
    char * message;
    time_t timestamp;
} t_mail;

typedef struct maillist_struct {
    int    idx;
    char * sender;
    time_t timestamp;
    struct maillist_struct * next;
} t_maillist;

#endif /* MAIL_INTERNAL_ACCESS */

#endif /* !INCLUDED_MAIL_TYPES */

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_MAIL_PROTOS
#define INCLUDED_MAIL_PROTOS

#define JUST_NEED_TYPES
#include "connection.h"
#undef JUST_NEED_TYPES

extern int handle_mail_command(t_connection * c, char const * text);
extern int mail_number_of_new(t_account * user);
extern int mail_save_number(t_connection * c);

#endif /* !INCLUDED_MAIL_PROTOS */
#endif /* !JUST_NEED_TYPES */

/* EOF */
