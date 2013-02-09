/*
 * Copyright (C) 2001,2002  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_CONFTABLE_TYPES
#define INCLUDED_CONFTABLE_TYPES

#ifndef JUST_NEED_TYPES
# define JUST_NEED_TYPES
# ifdef HAVE_STDDEF_H
#  include <stddef.h>
# else
#  ifndef NULL
#   define NULL ((void *)0)
#  endif
# endif
# include "common/util.h"
# undef JUST_NEED_TYPES
#else
# ifdef HAVE_STDDEF_H
#  include <stddef.h>
# else
#  ifndef NULL
#   define NULL ((void *)0)
#  endif
# endif
# include "common/util.h"
#endif

typedef enum
{
    conf_type_none,
    conf_type_bool,
    conf_type_int,
    conf_type_uint,
    conf_type_str,
    conf_type_cstr,
    conf_type_double
} t_conf_type;

typedef struct
{
    char const * const name;
    t_conf_type const  type;
    char const * const defval;
    void * const       var;
}
t_conf_entry;


#define INIT_CONF_ENTRY_BOOL(NAM,DEF,ST) { STRVAL(NAM),conf_type_bool,  STRVAL(DEF),&((ST).NAM) }
#define INIT_CONF_ENTRY_INT(NAM,DEF,ST)  { STRVAL(NAM),conf_type_int,   STRVAL(DEF),&((ST).NAM) }
#define INIT_CONF_ENTRY_UINT(NAM,DEF,ST) { STRVAL(NAM),conf_type_uint,  STRVAL(DEF),&((ST).NAM) }
#define INIT_CONF_ENTRY_STR(NAM,DEF,ST)  { STRVAL(NAM),conf_type_str,   (DEF),      &((ST).NAM) }
#define INIT_CONF_ENTRY_CSTR(NAM,DEF,ST) { STRVAL(NAM),conf_type_cstr,  (DEF),      &((ST).NAM) }
#define INIT_CONF_ENTRY_DBL(NAM,DEF,ST)  { STRVAL(NAM),conf_type_double,STRVAL(DEF),&((ST).NAM) }
#define END_CONF_ENTRY()                 { NULL,       conf_type_none,  NULL,       NULL        }

#endif


#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_CONFTABLE_PROTOS
#define INCLUDED_CONFTABLE_PROTOS

extern int conftable_load_defaults(t_conf_entry const * conf_table);
extern int conftable_load_file(t_conf_entry const * conf_table, char const * filename);
extern int conftable_unload(t_conf_entry const * conf_table);
extern int conftable_set_value(t_conf_entry const * conf_table, char const * name, char const * value);
extern unsigned int conftable_lookup_boolentry(t_conf_entry const * conf_table, char const * name);
extern int conftable_lookup_int(t_conf_entry const * conf_table, char const * name);
extern unsigned int conftable_lookup_uint(t_conf_entry const * conf_table, char const * name);
extern char const * conftable_lookup_str(t_conf_entry const * conf_table, char const * name);
extern char const * conftable_lookup_cstr(t_conf_entry const * conf_table, char const * name);
extern double conftable_lookup_double(t_conf_entry const * conf_table, char const * name);

#endif
#endif
