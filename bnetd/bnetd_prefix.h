/*
 *  bnetd_prefix.h
 *  bnetd
 *
 *  Created by Eric Gallager on 2/13/18.
 *
 */

#if defined(HAVE_STDDEF_H) || (defined(__STDC__) && defined(__STDC_VERSION__))
# include <stddef.h>
#else
# ifndef NULL
#  define NULL ((void *)0)
# endif
#endif
#if defined(STDC_HEADERS)  || (defined(__STDC__) && defined(__STDC_VERSION__))
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# else
#  ifdef HAVE_MALLOC_MALLOC_H
#   include <malloc/malloc.h>
#  endif
# endif
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include <errno.h>
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
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <ctype.h>
