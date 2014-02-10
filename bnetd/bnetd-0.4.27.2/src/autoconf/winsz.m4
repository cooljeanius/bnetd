dnl# From Jim Meyering.

# serial 1

AC_DEFUN([AM_HEADER_TIOCGWINSZ_NEEDS_SYS_IOCTL],
[AC_REQUIRE([AM_SYS_POSIX_TERMIOS])
 AC_REQUIRE([AC_HEADER_STDC])
 AC_REQUIRE([AC_PROG_EGREP])
 AC_REQUIRE([AC_PROG_CPP])
 AC_CACHE_CHECK([whether use of TIOCGWINSZ requires sys/ioctl.h],
	        [am_cv_sys_tiocgwinsz_needs_sys_ioctl_h],
  [am_cv_sys_tiocgwinsz_needs_sys_ioctl_h=no

  gwinsz_in_termios_h=no
  if test "x${am_cv_sys_posix_termios}" = "xyes"; then
    AC_EGREP_CPP([yes_this_is_there],[
#include <sys/types.h>
#include <termios.h>
#ifdef TIOCGWINSZ
yes_this_is_there
#endif /* TIOCGWINSZ */
    ],[gwinsz_in_termios_h=yes])
  fi

  if test "x${gwinsz_in_termios_h}" = "xno"; then
    AC_EGREP_CPP([yes_we_have_it],[
#include <sys/types.h>
#include <sys/ioctl.h>
#ifdef TIOCGWINSZ
yes_we_have_it
#endif
    ],[am_cv_sys_tiocgwinsz_needs_sys_ioctl_h=yes])
  fi
  ])
  if test "x${am_cv_sys_tiocgwinsz_needs_sys_ioctl_h}" = "xyes"; then
    AC_DEFINE([GWINSZ_IN_SYS_IOCTL],[1],
              [Define if TIOCGWINSZ requires sys/ioctl.h])
  fi
])
