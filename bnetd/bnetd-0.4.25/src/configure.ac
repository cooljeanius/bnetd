#
# Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
# Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([bnetd], [0.4.25], [owen@greenend.org.uk])
AC_CONFIG_SRCDIR([bnetd/handle_bnet.c])
AC_CONFIG_HEADERS([config.h])
AUTOCONF_CONFIG_DIR="autoconf"
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([m4])
builtin(include,autoconf/mkdir.m4)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AM_SANITY_CHECK

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks compiler.
AC_PROG_CPP
AC_PROG_CXX

# Checks for systems.
AC_AIX
AC_ISC_POSIX
AC_MINIX
LT_INIT

# Custom checks.
zwarnflags=""
AC_ARG_WITH(warn, [  --with-warn             enable compiler warnings], [zwarnflags="-pedantic -Wall -W -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wunreachable-code -Winline -Wpacked -Wno-sign-compare"])

zlangflags=""
AC_ARG_WITH(ansi, [  --with-ansi             use ANSI C mode], [zlangflags="-ansi -pedantic"])

extra_include=""
AC_ARG_WITH(includes, [  --with-includes=DIR     search include DIR for header files], [
case "x$withval" in
x/*|x.*)
  AC_MSG_RESULT([adding ${withval} to include search path])
  if test ! -d ${withval}; then
    AC_MSG_RESULT([Warning: directory ${withval} does not exist])
  fi
  extra_include="${extra_include} -I${withval}"
  ;;
*)
  AC_MSG_RESULT([not adding any includes])
  ;;
esac])

extra_lib_dir=""
AC_ARG_WITH(libraries, [  --with-libraries=DIR    search library DIR for libraries], [
case "x${withval}" in
x/*|x.*)
  AC_MSG_RESULT([adding ${withval} to library search path])
  if test ! -d ${withval}; then
    AC_MSG_RESULT([Warning: directory ${withval} does not exist])
  fi
  extra_lib_dir="${extra_lib_dir} -L${withval}"
  ;;
*)
  AC_MSG_RESULT([not adding any libs])
  ;;
esac])

extra_lib=""
AC_ARG_WITH(efence, [  --with-efence           link with Electric Fence to find memory problems], [extra_lib="${extra_lib} -lefence"])

extra_define=""
AC_ARG_WITH(bits, [  --with-bits             include BITS patch. (EXPERIMENTAL)], [extra_define="${extra_define} -DWITH_BITS"])

dnl Enable poll()
AC_ARG_ENABLE(poll,
[  --enable-poll           Enable poll() instead of select().  Normally poll
                          is preferred over select, but configure knows poll
                          is broken on some platforms.  If you think you are
                          smarter than the configure script, you may enable
                          poll with this option.
  --disable-poll          Disable the use of poll().],
[
  case "$enableval" in
  yes)
    echo "Forcing poll() to be enabled"
    ac_cv_func_poll='yes'
    ;;
  no)
    echo "Forcing poll() to be disabled"
    ac_cv_func_poll='no'
    ;;
  esac
])

dnl Shamesly stolen from squid 2.3 configure.in
dnl This has to be before AC_CHECK_FUNCS
# Disable poll() on certain platforms. Override by setting ac_cv_func_poll
# when running configure.
if test -z "$ac_cv_func_poll"; then
        case "$host" in
		[alpha-dec-osf3.*])
			# John Kay (jkay@nlanr.net) 19970818
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
		[*-hp-hpux*.*])
			# Duane Wessels
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
		[*-linux-*])
			# Henrik Nordstrom (hno@hem.passagen.se) 19980817
			# poll is problematic on Linux.  We disable it
			# by default until Linux gets it right.
			rev=`uname -r | awk -F. '{printf "%03d%03d",$1,$2}'`
			if test $rev -lt 002002; then
			    echo "disabling poll for $host < 2.2..."
			    ac_cv_func_poll='no'
			fi
			;;
		[powerpc-ibm-aix4.1.*])
			# Mike Laster (mlaster@metavillage.com) 19981021
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
		[*-pc-sco3.2*])
			# Robert Side <rside@aiinc.bc.ca>
			# Mon, 18 Jan 1999 17:48:00 GMT
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
		[darwin*])
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
	esac
fi

LDFLAGS="${LDFLAGS} ${extra_lib_dir}"
LIBS="${LIBS} ${extra_lib}"
CFLAGS="${CFLAGS} ${zlangflags} ${zwarnflags}"
DEFINES="${DEFINES} ${extra_define}"
CPPFLAGS="${CPPFLAGS} ${extra_include}"

# This is our invention so we have to substitute it ourselves
# autoconf uses DEFS
AC_SUBST(DEFINES)

# Checks for compiler quirks.
AC_PROG_GCC_TRADITIONAL
AC_C_CONST
AM_SET_LEADING_DOT
AC_EXEEXT
AX_CXX_MEMBER_CONSTANTS
AC_TYPE_LONG_LONG_INT

# Checks for libraries.
AC_CHECK_LIB(m, pow, [LIBS="$LIBS -lm"])
AC_CHECK_LIB(nsl, gethostbyname, [LIBS="$LIBS -lnsl"])
AC_CHECK_LIB(socket, socket, [LIBS="$LIBS -lsocket"])
AC_CHECK_LIB(resolv, inet_aton, [LIBS="$LIBS -lresolv"])
# FIXME: Replace `main' with a function in `-lpcap':
AC_CHECK_LIB([pcap], [main])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_STAT
AC_CHECK_HEADERS([arpa/inet.h dir.h direct.h fcntl.h grp.h libintl.h limits.h locale.h malloc.h memory.h netdb.h netinet/in.h poll.h pwd.h stdarg.h stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/poll.h sys/select.h sys/socket.h sys/stat.h sys/stropts.h sys/time.h sys/timeb.h sys/types.h sys/utsname.h sys/wait.h termios.h stropts.h unistd.h varargs.h])

# Checks for typedefs and structures
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_C_VOLATILE
AC_STRUCT_TM
AC_TYPE_SIGNAL
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_SIZEOF([unsigned char], [1])
AC_CHECK_SIZEOF([unsigned short], [2])
AC_CHECK_SIZEOF([unsigned int], [4])
AC_CHECK_SIZEOF([unsigned long], [4])
AC_CHECK_SIZEOF([unsigned long long], [0])

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKDIR_ARGS
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_SETPGRP
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 gethostname gettimeofday inet_aton inet_ntoa isascii memchr recv select socket strdup strpbrk strstr strtoul strerror uname send recvfrom sendto uname fork getpid sigaction sigprocmask sigaddset setpgid setpgrp ftime strcasecmp strncasecmp stricmp strnicmp chdir difftime strchr strrchr index rindex memcpy memset memmove bcopy wait waitpid pipe getenv ioctl setsid mktime poll gethostbyname getservbyname getlogin pow getpwnam getgrnam getuid getgid setuid setgid mkdir _mkdir])

if test X"${am_cv_prog_cc_stdc}" = X"no"; then
    COMPILEANSI1='${PREPROCESS} $< | sed '"'"'s/^# *\([0-9][0-9]*\)  *\([^ ]*[a-z][^ ]*\) *.*$$/#line \1 \2/'"'"' | ${SRC_ACDIR}/ansi2knr > ${SRC_SRCDIR}/.ansi2knr_tmp.c'
    COMPILEANSI2='${COMPILE} ${SRC_SRCDIR}/.ansi2knr_tmp.c -o $@'
    COMPILEANSI3='@${RMF} ${SRC_SRCDIR}/.ansi2knr_tmp.c'
    ANSI2KNR_TMP='${SRC_SRCDIR}/.ansi2knr_tmp.c'
    ANSI2KNR_DEP='${SRC_ACDIR}/ansi2knr'
else
    COMPILEANSI1='${COMPILE} $< -o $@'
    COMPILEANSI2='@:'
    COMPILEANSI3='@:'
    ANSI2KNR_TMP=''
    ANSI2KNR_DEP=''
fi
AC_SUBST(COMPILEANSI1)
AC_SUBST(COMPILEANSI2)
AC_SUBST(COMPILEANSI3)
AC_SUBST(ANSI2KNR_TMP)
AC_SUBST(ANSI2KNR_DEP)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
