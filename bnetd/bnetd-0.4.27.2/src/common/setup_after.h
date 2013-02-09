/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_SETUP_BEFORE_H
# error "This file must be included after all other header files"
#endif
#ifndef INCLUDED_SETUP_AFTER_H
#define INCLUDED_SETUP_AFTER_H

/*
 * select() hackery... works most places, need to add autoconf checks
 * because some systems may redefine FD_SETSIZE, have it as a variable,
 * or not have the concept of such a value.
 */
/* Win32 defaults to 64, BSD and Linux default to 1024 */
/* FIXME: how big can this be before things break? */
#ifndef FD_SETSIZE
# define FD_SETSIZE 4096
#endif

#include "common/check_alloc.h"

#endif
