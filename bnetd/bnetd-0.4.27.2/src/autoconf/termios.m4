dnl# From Jim Meyering.

# serial 1

AC_DEFUN([AM_SYS_POSIX_TERMIOS],
[AC_REQUIRE([AC_HEADER_STDC])
 AC_CACHE_CHECK([POSIX termios],[am_cv_sys_posix_termios],
  [AC_LINK_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
#include <unistd.h>
#include <termios.h>]],
  [[/* SunOS 4.0.3 has termios.h but not the library calls.  */
   tcgetattr(0, 0);]])],
  [am_cv_sys_posix_termios=yes],
  [am_cv_sys_posix_termios=no])])
])
