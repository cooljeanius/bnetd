dnl#                                               -*- Autoconf -*-
# Copyright (C) 1999 Rob Crittenden (rcrit@greyoak.com)
# Copyright (C) 1999, 2000 Ross Combs (rocombs@cs.nmsu.edu)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place: Suite 330, Boston, MA 02111-1307, USA.
#

# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([bnetd],[0.4.27],[owen@greenend.org.uk])
dnl# removed ".2" from version number for gnits standard
AC_CONFIG_SRCDIR([bnetd/handle_bnet.c])
AC_CONFIG_HEADERS([config.h])
AUTOCONF_CONFIG_DIR="autoconf"
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([m4])
builtin(include,autoconf/mkdir.m4)

AC_CANONICAL_TARGET dnl# also calls the "_BUILD" and "_HOST" versions
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([1.13 gnits subdir-objects])
AM_MAINTAINER_MODE
AM_DEP_TRACK
AM_SANITY_CHECK
AM_MAKE_INCLUDE
m4_ifdef([AM_SILENT_RULES],[
AM_SILENT_RULES([no])
],[])dnl

# Checks for programs.
AM_PROG_AS
AM_PROG_AR
if test "x${AWK}" = "x"; then
    test -z "${AWK}"
    AC_PROG_AWK
else
    test ! -z "${AWK}" && export AWK
    AC_SUBST([AWK])
fi
AC_PROG_FGREP
AC_PROG_INSTALL
AM_PROG_INSTALL_SH
AM_PROG_INSTALL_STRIP
AC_PROG_LN_S
AC_PROG_MKDIR_P

# Checks paths of programs.
AC_PATH_PROG([XCODEBUILD],[xcodebuild])
if test "x${XCODEBUILD}" != "x"; then
      AC_MSG_CHECKING([for Xcode version])
      XCODEBUILD_VERSION=`xcodebuild -version`
      AC_MSG_RESULT([${XCODEBUILD_VERSION}])
fi
AC_PATH_PROG([RMDIR],[rmdir])
AC_PATH_PROG([ECHO],[echo])dnl

# Checks compiler.
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AM_PROG_CC_STDC

# Checks for systems and system characteristics.
dnl# AC\_USE\_SYSTEM\_EXTENSIONS has been moved above
AC_SEARCH_LIBS([strerror],[cposix]) dnl# used to be AC\_ISC\_POSIX
AC_SYS_LARGEFILE
AC_SYS_POSIX_TERMIOS

# libtool
LT_INIT([win32-dll])
LT_LANG([C])
LT_LANG([Windows Resource])
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
for warning_flag in $(echo "-Wall -Wextra -Wshadow -Wpointer-arith -Wnull-dereference -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wunreachable-code -Winline -Wpacked -Wimplicit -Wno-attributes -Wno-sign-compare -Wno-parentheses -Wdangling-else"); do
	AX_CHECK_COMPILE_FLAG([${warning_flag}],[zwarnflags="${zwarnflags} ${warning_flag}"])
done
])
AC_ARG_WITH([warn-lite],[AS_HELP_STRING([--with-warn-lite],[enable few compiler warnings])],[
for lite_warning_flag in $(echo "-pedantic -Wall -Wno-attributes -Wno-sign-compare -Wno-parentheses"); do
	AX_CHECK_COMPILE_FLAG([${lite_warning_flag}],[zwarnflags="${zwarnflags} ${lite_warning_flag}"])
done
])dnl

zlangflags=""
AC_ARG_WITH([ansi],[AS_HELP_STRING([--with-ansi],[use ANSI C mode])], [zlangflags="-ansi -pedantic"])dnl

extra_include=""
AC_ARG_WITH([includes],[AS_HELP_STRING([--with-includes=DIR],[search include DIR for header files])],[
case "x${withval}" in
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
esac])dnl

extra_lib_dir=""
AC_ARG_WITH([libraries],[AS_HELP_STRING([--with-libraries=DIR],[search library DIR for libraries])],[
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
esac])dnl

extra_lib=""
AC_ARG_WITH([efence],[AS_HELP_STRING([--with-efence],[link with Electric Fence to find memory problems])],[extra_lib="${extra_lib} -lefence"])dnl

extra_define=""
AC_ARG_WITH([debug-all],[AS_HELP_STRING([--with-debug-all],[include debugging code])],[extra_define="${extra_define} -DDEBUG_ACCOUNT -DCLIENTDEBUG -DLIST_DEBUG -DHASHTABLE_DEBUG -DUSE_CHECK_ALLOC -DTESTUNGET -DLADDER_DEBUG"])
AC_ARG_WITH([acct-dyn-load],[AS_HELP_STRING([--with-acct-dyn-load],[do not load all account at start])],[extra_define="${extra_define} -DACCT_DYN_LOAD -UWITH_BITS"])
AC_ARG_WITH([acct-dyn-unload],[AS_HELP_STRING([--with-acct-dyn-unload],[unload unused account])],[extra_define="${extra_define} -DACCT_DYN_UNLOAD -DACCT_DYN_LOAD -UWITH_BITS"])
AC_ARG_WITH([gamei-server],[AS_HELP_STRING([--with-gamei-server],[gamei like server punctation])],[extra_define="${extra_define} -DGAMEI_SERVER"])
AC_ARG_WITH([bits],[AS_HELP_STRING([--with-bits],[include BITS patch. (EXPERIMENTAL)])],[extra_define="${extra_define} -DWITH_BITS"])dnl

AC_ARG_WITH([storage-mysql],[AS_HELP_STRING([--with-storage-mysql],[include MySQL DataBase storage. (EXPERIMENTAL)])],[extra_define="${extra_define} -DWITH_MYSQL -DWITH_STORAGE_DB"
  extra_lib="${extra_lib} -lmysqlclient"])dnl


dnl# Enable poll()
AC_ARG_ENABLE([poll],
[AS_HELP_STRING([--enable-poll],[Enable poll() instead of select(). 
                                 Normally bnetd prefers poll over select,
                                 but this configure script knows that poll
                                 is broken on some platforms. If you think
                                 that you are smarter than the configure
                                 script, then you may try to enable
                                 the usage of poll with this option.])],
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
])dnl

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

dnl# This is our invention so we have to substitute it ourselves:
dnl# (autoconf already uses DEFS)
AC_SUBST([DEFINES])dnl

# Checks for compiler quirks.
AC_PROG_GCC_TRADITIONAL
dnl# AC\_C\_CONST has been moved below
AM_SET_LEADING_DOT
AC_EXEEXT
AC_TYPE_LONG_LONG_INT
dnl# AX_* macros require the autoconf-archive package
AX_CXX_MEMBER_CONSTANTS
AX_C___ATTRIBUTE__
AX_C_VAR_FUNC
AX_COMPILER_VENDOR
AX_CHECK_GNU_MAKE

# Checks for libraries.
LT_LIB_M
AC_CHECK_LIB([m],[pow],[LIBS="${LIBS} -lm"])
AC_CHECK_LIB([nsl],[gethostbyname],[LIBS="${LIBS} -lnsl"])
AC_CHECK_LIB([socket],[socket],[LIBS="${LIBS} -lsocket"])
AC_CHECK_LIB([resolv],[inet_aton],[LIBS="${LIBS} -lresolv"])
AC_CHECK_LIB([pcap],[pcap_lookupdev],[LIBS="${LIBS} -lpcap"])dnl

# Checks for header files.
AM_HEADER_TIOCGWINSZ_NEEDS_SYS_IOCTL
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_HEADER_STDBOOL
AC_CHECK_HEADERS_ONCE([dir.h direct.h stdarg.h stdio.h])
AC_CHECK_HEADERS([arpa/inet.h ctype.h errno.h fcntl.h grp.h i386/_types.h \
                  libintl.h limits.h locale.h malloc.h malloc/malloc.h \
                  netdb.h netinet/in.h poll.h pwd.h stddef.h \
                  strcasecmp.h strchr.h strdup.h strerror.h stropts.h  \
                  sys/file.h sys/ioctl.h sys/param.h sys/poll.h \
                  sys/select.h sys/socket.h sys/stropts.h sys/time.h \
                  sys/timeb.h sys/utsname.h sys/wait.h \
                  termio.h termios.h time.h varargs.h])dnl

# Checks for typedefs and structures
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_PROTOTYPES
AC_C_RESTRICT
AC_C_VOLATILE
AC_STRUCT_TM
AC_CHECK_HEADERS_ONCE([signal.h])
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
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_SIZEOF([unsigned char],[1])
AC_CHECK_ALIGNOF([unsigned char])
AC_CHECK_SIZEOF([unsigned short],[2])
AC_CHECK_ALIGNOF([unsigned short])
AC_CHECK_SIZEOF([unsigned int],[4])
AC_CHECK_ALIGNOF([unsigned int])
AC_CHECK_SIZEOF([unsigned long],[4])
AC_CHECK_ALIGNOF([unsigned long])
AC_CHECK_SIZEOF([unsigned long long],[0])
AC_CHECK_ALIGNOF([unsigned long long])dnl

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKDIR_ARGS
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_SETPGRP
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS_ONCE([fork _mkdir])
AC_CHECK_FUNCS([atexit atoi bcopy chdir cfree difftime dup2 ftime \
                getenv getgid getgrnam gethostbyname gethostname getlogin \
                getpid getpwnam getservbyname gettimeofday getuid \
                index inet_aton inet_ntoa ioctl isascii memchr memcpy \
                memset memmove mkdir mktime pipe poll pow recv recvfrom \
                rindex select send sendto setgid setpgid setpgrp setsid \
                setuid sigaction sigprocmask sigaddset socket strcasecmp \
                strchr strdup strerror stricmp strncasecmp strnicmp \
                strpbrk strrchr strsep strstr strtol strtoul strtoull \
                uname wait waitpid])
AC_CHECK_DECLS([NULL])
AC_CHECK_DECLS([mkdir])dnl

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
AC_SUBST([INSTALLdir])
AC_SUBST([install_sh])
AC_SUBST([RMDIR])
AC_SUBST([ECHO])dnl

# Output.
AC_CONFIG_FILES([Makefile Makefile.bak])
AC_OUTPUT
