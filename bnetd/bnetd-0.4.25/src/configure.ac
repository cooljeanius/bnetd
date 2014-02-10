#                                               -*- Autoconf -*-
# Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
# Copyright (C) 1999, 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
# Foundation, Inc., 59 Temple Place: Suite 330, Boston, MA 02111-1307, USA.
#

# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([bnetd],[0.4.25],[owen@greenend.org.uk])
AC_CONFIG_SRCDIR([bnetd/handle_bnet.c])
AC_CONFIG_HEADERS([config.h])
AUTOCONF_CONFIG_DIR="autoconf"
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([m4])
builtin([include],[autoconf/mkdir.m4])

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([gnits -Wall])
AM_MAINTAINER_MODE
AM_SANITY_CHECK

# Checks for programs.
if test "x${AWK}" = "x"; then
    test -z "${AWK}"
    AC_PROG_AWK
else
    test ! -z "${AWK}" && export AWK
    AC_SUBST([AWK])
fi
AC_PROG_INSTALL
AM_PROG_INSTALL_STRIP
AC_PROG_LN_S
AC_PROG_MKDIR_P

AC_PATH_PROG([XCODEBUILD],[xcodebuild])
if test "x${XCODEBUILD}" != "x"; then
      AC_MSG_CHECKING([for Xcode version])
      XCODEBUILD_VERSION=`xcodebuild -version`
      AC_MSG_RESULT([${XCODEBUILD_VERSION}])
fi

# Checks compiler.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_CXX

# Checks for systems.
AC_SEARCH_LIBS([strerror],[cposix])
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_SYS_LONG_FILE_NAMES
LT_INIT([win32-dll])
LT_LANG([C])
if test "x${RANLIB}" = "x"; then
	test -z "${RANLIB}"
	AC_PROG_RANLIB
else
	test ! -z "${RANLIB}" && export RANLIB
	AC_SUBST([RANLIB])
fi

# Custom checks.
zwarnflags=""
AC_ARG_WITH([warn],[AS_HELP_STRING([--with-warn],[enable a certain set of specific compiler warnings])],[
for warning_flag in $(echo "-Wall -W -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wunreachable-code -Winline -Wpacked -Wno-sign-compare"); do
	AX_CHECK_COMPILE_FLAG([${warning_flag}],[zwarnflags="${zwarnflags} ${warning_flag}"])
done
])
AX_CHECK_COMPILE_FLAG([-Wno-attributes],[CFLAGS="${CFLAGS} -Wno-attributes"])

zlangflags=""
AC_ARG_WITH([ansi],[AS_HELP_STRING([--with-ansi],[force ANSI C mode (deprecated)])],[zlangflags="-ansi -pedantic"])

extra_include=""
AC_ARG_WITH([includes],[AS_HELP_STRING([--with-includes=DIR],[search include DIR for header files])],[
case "x${withval}" in
x/*|x.*)
  AC_MSG_NOTICE([adding ${withval} to include search path])
  if test ! -d ${withval}; then
    AC_MSG_WARN([include directory ${withval} does not exist])
  fi
  extra_include="${extra_include} -I${withval}"
  ;;
*)
  AC_MSG_NOTICE([not adding any extra include directories])
  ;;
esac])

extra_lib_dir=""
AC_ARG_WITH([libraries],[AS_HELP_STRING([--with-libraries=DIR],[search library DIR for libraries])],[
case "x${withval}" in
x/*|x.*)
  AC_MSG_NOTICE([adding ${withval} to library search path])
  if test ! -d ${withval}; then
    AC_MSG_WARN([library directory ${withval} does not exist])
  fi
  extra_lib_dir="${extra_lib_dir} -L${withval}"
  ;;
*)
  AC_MSG_NOTICE([not adding any extra library directories])
  ;;
esac])

extra_lib=""
AC_ARG_WITH([efence],[AS_HELP_STRING([--with-efence],[link with Electric Fence to find memory problems])],[extra_lib="${extra_lib} -lefence"])

extra_define=""
AC_ARG_WITH([bits],[AS_HELP_STRING([--with-bits],[include BITS patch. (EXPERIMENTAL)])],[extra_define="${extra_define} -DWITH_BITS"])

dnl# Enable poll()
AC_ARG_ENABLE([poll],
[AS_HELP_STRING([--enable-poll],[Enable poll() instead of select(). Normally bnetd prefers poll over select, but configure knows that poll is broken on some platforms. If you think that you are smarter than the configure script, you may enable poll with this option.])],
[
  case "${enableval}" in
  yes)
    AC_MSG_NOTICE([Forcing usage of poll() to be enabled])
    ac_cv_func_poll='yes'
    ;;
  no)
    AC_MSG_NOTICE([Forcing usage of poll() to be disabled])
    ac_cv_func_poll='no'
    ;;
  esac
  AC_SUBST([ac_cv_func_poll])
])

dnl# Shamesly stolen from squid 2.3 configure.in
dnl# This has to be before AC_CHECK_FUNCS
# Disable poll() on certain platforms. Override by setting ac_cv_func_poll
# when running configure.
if test -z "${ac_cv_func_poll}"; then
    case "${host}" in
        [alpha-dec-osf3.*])
            # John Kay (jkay@nlanr.net) 19970818
            AC_MSG_NOTICE([disabling usage of poll for ${host}...])
            ac_cv_func_poll='no'
            ;;
        [*-hp-hpux*.*])
            # Duane Wessels
            AC_MSG_NOTICE([disabling usage of poll for ${host}...])
            ac_cv_func_poll='no'
            ;;
        [*-linux-*])
            # Henrik Nordstrom (hno@hem.passagen.se) 19980817
            # poll is problematic on Linux. We disable it
            # by default until Linux gets it right.
            rev=`uname -r | awk -F. '{printf "%03d%03d",$1,$2}'`
            if test ${rev} -lt 002002; then
                AC_MSG_NOTICE([disabling poll for ${host} < 2.2...])
                ac_cv_func_poll='no'
            fi
            ;;
        [powerpc-ibm-aix4.1.*])
            # Mike Laster (mlaster@metavillage.com) 19981021
            AC_MSG_NOTICE([disabling usage of poll for ${host}...])
            ac_cv_func_poll='no'
            ;;
        [*-pc-sco3.2*])
            # Robert Side <rside@aiinc.bc.ca>
            # Mon, 18 Jan 1999 17:48:00 GMT
            AC_MSG_NOTICE([disabling usage of poll for ${host}...])
            ac_cv_func_poll='no'
            ;;
        [darwin*])
            echo "disabling poll for ${host}..."
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
# (autoconf uses "DEFS" instead)
AC_SUBST([DEFINES])

# Checks for compiler quirks.
AC_PROG_GCC_TRADITIONAL
AM_PROG_CC_STDC
AM_SET_LEADING_DOT
AC_EXEEXT
AC_OBJEXT
AC_TYPE_LONG_LONG_INT
dnl# AX_* macros require the autoconf-archive package
AX_CXX_MEMBER_CONSTANTS
AX_C___ATTRIBUTE__
AX_C_VAR_FUNC
AX_COMPILER_VENDOR

# Checks for libraries.
LT_LIB_M
AC_CHECK_LIB([m],[pow],[LIBS="${LIBS} -lm"])
AC_CHECK_LIB([nsl],[gethostbyname],[LIBS="${LIBS} -lnsl"])
AC_CHECK_LIB([socket],[socket],[LIBS="${LIBS} -lsocket"])
AC_CHECK_LIB([resolv],[inet_aton],[LIBS="${LIBS} -lresolv"])
# FIXME: Replace `main' with a function in `-lpcap':
AC_CHECK_LIB([pcap],[main])

# Checks for header files.
AM_HEADER_TIOCGWINSZ_NEEDS_SYS_IOCTL
AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_HEADER_STAT
AC_CHECK_HEADERS([Availability.h arpa/inet.h ctype.h dir.h direct.h \
                  errno.h fcntl.h grp.h libintl.h limits.h locale.h \
                  malloc.h malloc/malloc.h netdb.h netinet/in.h poll.h \
                  pwd.h signal.h stdarg.h stddef.h stdio.h stropts.h \
                  sys/_structs.h sys/_types.h sys/cdefs.h sys/file.h \
                  sys/ioctl.h sys/param.h sys/poll.h sys/select.h \
                  sys/socket.h sys/stropts.h sys/time.h sys/timeb.h \
                  sys/utsname.h sys/wait.h termios.h time.h varargs.h])

# Checks for typedefs and structures
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_PROTOTYPES
AC_C_RESTRICT
AC_C_VOLATILE
AC_STRUCT_TM
dnl# begin what used to be AC\_TYPE\_SIGNAL
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <signal.h>
]],[[return *(signal (0, 0)) (0) == 1;]])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[${ac_cv_type_signal}],
         [Define as the return type of signal handlers (`int' or `void').])
dnl# end what used to be AC\_TYPE\_SIGNAL
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_SIZEOF([unsigned char],[1])
AC_CHECK_ALIGNOF([unsigned char])
AC_CHECK_SIZEOF([unsigned short],[2])
AC_CHECK_ALIGNOF([unsigned short])
AC_CHECK_SIZEOF([unsigned int],[4])
AC_CHECK_ALIGNOF([unsigned int])
AC_CHECK_SIZEOF([unsigned long int])
AC_CHECK_ALIGNOF([unsigned long int])
AC_CHECK_SIZEOF([unsigned long],[4])
AC_CHECK_ALIGNOF([unsigned long])
AC_CHECK_SIZEOF([unsigned long long],[0])
AC_CHECK_ALIGNOF([unsigned long long])

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
AC_CHECK_FUNCS([bcopy chdir difftime dup2 fprintf fputs free ftime \
                getenv getgid getgrnam gethostbyname gethostname getlogin \
                getpid getpwnam getservbyname gettimeofday getuid index \
                inet_aton inet_ntoa ioctl isascii memchr memcpy memmove \
                memset mktime pipe poll pow recv recvfrom rindex select \
                send sendto setgid setpgid setpgrp setsid setuid \
                sigaction sigaddset sigprocmask socket sprintf strcasecmp \
                strchr strdup strerror stricmp strlen strncasecmp strncmp \
                strnicmp strpbrk strrchr strstr strtoul uname \
                wait waitpid])
AC_CHECK_DECLS([NULL])
AC_CHECK_DECLS([mkdir])

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
AC_SUBST([COMPILEANSI1])
AC_SUBST([COMPILEANSI2])
AC_SUBST([COMPILEANSI3])
AC_SUBST([ANSI2KNR_TMP])
AC_SUBST([ANSI2KNR_DEP])

AC_CONFIG_FILES([Makefile \
                 bnetd/Makefile \
                 bniutils/Makefile \
                 bnpass/Makefile \
                 bnpcap/Makefile \
                 bnproxy/Makefile \
                 bntrackd/Makefile \
                 client/Makefile \
                 d2cs/Makefile \
                 d2dbs/Makefile \
                 debug/Makefile])
AC_OUTPUT
