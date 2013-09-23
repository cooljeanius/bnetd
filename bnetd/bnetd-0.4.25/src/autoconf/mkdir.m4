dnl# AC_FUNC_MKDIR
dnl# Check mkdir arguments.  
dnl# Defines MKDIR_TAKES_ONE_ARG.
dnl#
dnl# Based on code written by Alexandre Duret-Lutz <duret_g@epita.fr>.

AC_DEFUN([AC_FUNC_MKDIR_ARGS],
[AC_CHECK_FUNCS([mkdir _mkdir])
AC_CACHE_CHECK([whether mkdir takes one argument],
                [ac_cv_mkdir_takes_one_arg],
[AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_DIR_H
# include <dir.h>
#endif /* HAVE_DIR_H */
#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif /* HAVE_DIRECT_H */
#ifndef HAVE_MKDIR
# ifdef HAVE__MKDIR
#  define mkdir _mkdir
# endif /* HAVE__MKDIR */
#endif /* !HAVE_MKDIR */
]],[[mkdir(".");]])],
[ac_cv_mkdir_takes_one_arg=yes],[ac_cv_mkdir_takes_one_arg=no])])
if test x"$ac_cv_mkdir_takes_one_arg" = xyes; then
  AC_DEFINE([MKDIR_TAKES_ONE_ARG],[1],
            [Define to 1 if mkdir takes only one argument.])
fi
])
