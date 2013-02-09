#! /bin/sh

# Guess values for system-dependent variables and create Makefiles.
# Generated automatically using autoconf version 2.13 
# Copyright (C) 1992, 93, 94, 95, 96 Free Software Foundation, Inc.
#
# This configure script is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.

# Defaults:
ac_help=
ac_default_prefix=/usr/local
# Any additions from configure.in:
ac_help="$ac_help
  --with-warn             enable compiler warnings"
ac_help="$ac_help
  --with-ansi             use ANSI C mode"
ac_help="$ac_help
  --with-includes=DIR     search include DIR for header files"
ac_help="$ac_help
  --with-libraries=DIR    search library DIR for libraries"
ac_help="$ac_help
  --with-efence           link with Electric Fence to find memory problems"
ac_help="$ac_help
  --with-bits             include BITS patch. (EXPERIMENTAL)"
ac_help="$ac_help
  --enable-poll           Enable poll() instead of select().  Normally poll
                          is preferred over select, but configure knows poll
                          is broken on some platforms.  If you think you are
                          smarter than the configure script, you may enable
                          poll with this option.
  --disable-poll          Disable the use of poll()."

# Initialize some variables set by options.
# The variables have the same names as the options, with
# dashes changed to underlines.
build=NONE
cache_file=./config.cache
exec_prefix=NONE
host=NONE
no_create=
nonopt=NONE
no_recursion=
prefix=NONE
program_prefix=NONE
program_suffix=NONE
program_transform_name=s,x,x,
silent=
site=
srcdir=
target=NONE
verbose=
x_includes=NONE
x_libraries=NONE
bindir='${exec_prefix}/bin'
sbindir='${exec_prefix}/sbin'
libexecdir='${exec_prefix}/libexec'
datadir='${prefix}/share'
sysconfdir='${prefix}/etc'
sharedstatedir='${prefix}/com'
localstatedir='${prefix}/var'
libdir='${exec_prefix}/lib'
includedir='${prefix}/include'
oldincludedir='/usr/include'
infodir='${prefix}/info'
mandir='${prefix}/man'

# Initialize some other variables.
subdirs=
MFLAGS= MAKEFLAGS=
SHELL=${CONFIG_SHELL-/bin/sh}
# Maximum number of lines to put in a shell here document.
ac_max_here_lines=12

ac_prev=
for ac_option
do

  # If the previous option needs an argument, assign it.
  if test -n "$ac_prev"; then
    eval "$ac_prev=\$ac_option"
    ac_prev=
    continue
  fi

  case "$ac_option" in
  -*=*) ac_optarg=`echo "$ac_option" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) ac_optarg= ;;
  esac

  # Accept the important Cygnus configure options, so we can diagnose typos.

  case "$ac_option" in

  -bindir | --bindir | --bindi | --bind | --bin | --bi)
    ac_prev=bindir ;;
  -bindir=* | --bindir=* | --bindi=* | --bind=* | --bin=* | --bi=*)
    bindir="$ac_optarg" ;;

  -build | --build | --buil | --bui | --bu)
    ac_prev=build ;;
  -build=* | --build=* | --buil=* | --bui=* | --bu=*)
    build="$ac_optarg" ;;

  -cache-file | --cache-file | --cache-fil | --cache-fi \
  | --cache-f | --cache- | --cache | --cach | --cac | --ca | --c)
    ac_prev=cache_file ;;
  -cache-file=* | --cache-file=* | --cache-fil=* | --cache-fi=* \
  | --cache-f=* | --cache-=* | --cache=* | --cach=* | --cac=* | --ca=* | --c=*)
    cache_file="$ac_optarg" ;;

  -datadir | --datadir | --datadi | --datad | --data | --dat | --da)
    ac_prev=datadir ;;
  -datadir=* | --datadir=* | --datadi=* | --datad=* | --data=* | --dat=* \
  | --da=*)
    datadir="$ac_optarg" ;;

  -disable-* | --disable-*)
    ac_feature=`echo $ac_option|sed -e 's/-*disable-//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_feature| sed 's/[-a-zA-Z0-9_]//g'`"; then
      { echo "configure: error: $ac_feature: invalid feature name" 1>&2; exit 1; }
    fi
    ac_feature=`echo $ac_feature| sed 's/-/_/g'`
    eval "enable_${ac_feature}=no" ;;

  -enable-* | --enable-*)
    ac_feature=`echo $ac_option|sed -e 's/-*enable-//' -e 's/=.*//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_feature| sed 's/[-_a-zA-Z0-9]//g'`"; then
      { echo "configure: error: $ac_feature: invalid feature name" 1>&2; exit 1; }
    fi
    ac_feature=`echo $ac_feature| sed 's/-/_/g'`
    case "$ac_option" in
      *=*) ;;
      *) ac_optarg=yes ;;
    esac
    eval "enable_${ac_feature}='$ac_optarg'" ;;

  -exec-prefix | --exec_prefix | --exec-prefix | --exec-prefi \
  | --exec-pref | --exec-pre | --exec-pr | --exec-p | --exec- \
  | --exec | --exe | --ex)
    ac_prev=exec_prefix ;;
  -exec-prefix=* | --exec_prefix=* | --exec-prefix=* | --exec-prefi=* \
  | --exec-pref=* | --exec-pre=* | --exec-pr=* | --exec-p=* | --exec-=* \
  | --exec=* | --exe=* | --ex=*)
    exec_prefix="$ac_optarg" ;;

  -gas | --gas | --ga | --g)
    # Obsolete; use --with-gas.
    with_gas=yes ;;

  -help | --help | --hel | --he)
    # Omit some internal or obsolete options to make the list less imposing.
    # This message is too long to be a string in the A/UX 3.1 sh.
    cat << EOF
Usage: configure [options] [host]
Options: [defaults in brackets after descriptions]
Configuration:
  --cache-file=FILE       cache test results in FILE
  --help                  print this message
  --no-create             do not create output files
  --quiet, --silent       do not print \`checking...' messages
  --version               print the version of autoconf that created configure
Directory and file names:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [$ac_default_prefix]
  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX
                          [same as prefix]
  --bindir=DIR            user executables in DIR [EPREFIX/bin]
  --sbindir=DIR           system admin executables in DIR [EPREFIX/sbin]
  --libexecdir=DIR        program executables in DIR [EPREFIX/libexec]
  --datadir=DIR           read-only architecture-independent data in DIR
                          [PREFIX/share]
  --sysconfdir=DIR        read-only single-machine data in DIR [PREFIX/etc]
  --sharedstatedir=DIR    modifiable architecture-independent data in DIR
                          [PREFIX/com]
  --localstatedir=DIR     modifiable single-machine data in DIR [PREFIX/var]
  --libdir=DIR            object code libraries in DIR [EPREFIX/lib]
  --includedir=DIR        C header files in DIR [PREFIX/include]
  --oldincludedir=DIR     C header files for non-gcc in DIR [/usr/include]
  --infodir=DIR           info documentation in DIR [PREFIX/info]
  --mandir=DIR            man documentation in DIR [PREFIX/man]
  --srcdir=DIR            find the sources in DIR [configure dir or ..]
  --program-prefix=PREFIX prepend PREFIX to installed program names
  --program-suffix=SUFFIX append SUFFIX to installed program names
  --program-transform-name=PROGRAM
                          run sed PROGRAM on installed program names
EOF
    cat << EOF
Host type:
  --build=BUILD           configure for building on BUILD [BUILD=HOST]
  --host=HOST             configure for HOST [guessed]
  --target=TARGET         configure for TARGET [TARGET=HOST]
Features and packages:
  --disable-FEATURE       do not include FEATURE (same as --enable-FEATURE=no)
  --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]
  --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
  --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)
  --x-includes=DIR        X include files are in DIR
  --x-libraries=DIR       X library files are in DIR
EOF
    if test -n "$ac_help"; then
      echo "--enable and --with options recognized:$ac_help"
    fi
    exit 0 ;;

  -host | --host | --hos | --ho)
    ac_prev=host ;;
  -host=* | --host=* | --hos=* | --ho=*)
    host="$ac_optarg" ;;

  -includedir | --includedir | --includedi | --included | --include \
  | --includ | --inclu | --incl | --inc)
    ac_prev=includedir ;;
  -includedir=* | --includedir=* | --includedi=* | --included=* | --include=* \
  | --includ=* | --inclu=* | --incl=* | --inc=*)
    includedir="$ac_optarg" ;;

  -infodir | --infodir | --infodi | --infod | --info | --inf)
    ac_prev=infodir ;;
  -infodir=* | --infodir=* | --infodi=* | --infod=* | --info=* | --inf=*)
    infodir="$ac_optarg" ;;

  -libdir | --libdir | --libdi | --libd)
    ac_prev=libdir ;;
  -libdir=* | --libdir=* | --libdi=* | --libd=*)
    libdir="$ac_optarg" ;;

  -libexecdir | --libexecdir | --libexecdi | --libexecd | --libexec \
  | --libexe | --libex | --libe)
    ac_prev=libexecdir ;;
  -libexecdir=* | --libexecdir=* | --libexecdi=* | --libexecd=* | --libexec=* \
  | --libexe=* | --libex=* | --libe=*)
    libexecdir="$ac_optarg" ;;

  -localstatedir | --localstatedir | --localstatedi | --localstated \
  | --localstate | --localstat | --localsta | --localst \
  | --locals | --local | --loca | --loc | --lo)
    ac_prev=localstatedir ;;
  -localstatedir=* | --localstatedir=* | --localstatedi=* | --localstated=* \
  | --localstate=* | --localstat=* | --localsta=* | --localst=* \
  | --locals=* | --local=* | --loca=* | --loc=* | --lo=*)
    localstatedir="$ac_optarg" ;;

  -mandir | --mandir | --mandi | --mand | --man | --ma | --m)
    ac_prev=mandir ;;
  -mandir=* | --mandir=* | --mandi=* | --mand=* | --man=* | --ma=* | --m=*)
    mandir="$ac_optarg" ;;

  -nfp | --nfp | --nf)
    # Obsolete; use --without-fp.
    with_fp=no ;;

  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c)
    no_create=yes ;;

  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r)
    no_recursion=yes ;;

  -oldincludedir | --oldincludedir | --oldincludedi | --oldincluded \
  | --oldinclude | --oldinclud | --oldinclu | --oldincl | --oldinc \
  | --oldin | --oldi | --old | --ol | --o)
    ac_prev=oldincludedir ;;
  -oldincludedir=* | --oldincludedir=* | --oldincludedi=* | --oldincluded=* \
  | --oldinclude=* | --oldinclud=* | --oldinclu=* | --oldincl=* | --oldinc=* \
  | --oldin=* | --oldi=* | --old=* | --ol=* | --o=*)
    oldincludedir="$ac_optarg" ;;

  -prefix | --prefix | --prefi | --pref | --pre | --pr | --p)
    ac_prev=prefix ;;
  -prefix=* | --prefix=* | --prefi=* | --pref=* | --pre=* | --pr=* | --p=*)
    prefix="$ac_optarg" ;;

  -program-prefix | --program-prefix | --program-prefi | --program-pref \
  | --program-pre | --program-pr | --program-p)
    ac_prev=program_prefix ;;
  -program-prefix=* | --program-prefix=* | --program-prefi=* \
  | --program-pref=* | --program-pre=* | --program-pr=* | --program-p=*)
    program_prefix="$ac_optarg" ;;

  -program-suffix | --program-suffix | --program-suffi | --program-suff \
  | --program-suf | --program-su | --program-s)
    ac_prev=program_suffix ;;
  -program-suffix=* | --program-suffix=* | --program-suffi=* \
  | --program-suff=* | --program-suf=* | --program-su=* | --program-s=*)
    program_suffix="$ac_optarg" ;;

  -program-transform-name | --program-transform-name \
  | --program-transform-nam | --program-transform-na \
  | --program-transform-n | --program-transform- \
  | --program-transform | --program-transfor \
  | --program-transfo | --program-transf \
  | --program-trans | --program-tran \
  | --progr-tra | --program-tr | --program-t)
    ac_prev=program_transform_name ;;
  -program-transform-name=* | --program-transform-name=* \
  | --program-transform-nam=* | --program-transform-na=* \
  | --program-transform-n=* | --program-transform-=* \
  | --program-transform=* | --program-transfor=* \
  | --program-transfo=* | --program-transf=* \
  | --program-trans=* | --program-tran=* \
  | --progr-tra=* | --program-tr=* | --program-t=*)
    program_transform_name="$ac_optarg" ;;

  -q | -quiet | --quiet | --quie | --qui | --qu | --q \
  | -silent | --silent | --silen | --sile | --sil)
    silent=yes ;;

  -sbindir | --sbindir | --sbindi | --sbind | --sbin | --sbi | --sb)
    ac_prev=sbindir ;;
  -sbindir=* | --sbindir=* | --sbindi=* | --sbind=* | --sbin=* \
  | --sbi=* | --sb=*)
    sbindir="$ac_optarg" ;;

  -sharedstatedir | --sharedstatedir | --sharedstatedi \
  | --sharedstated | --sharedstate | --sharedstat | --sharedsta \
  | --sharedst | --shareds | --shared | --share | --shar \
  | --sha | --sh)
    ac_prev=sharedstatedir ;;
  -sharedstatedir=* | --sharedstatedir=* | --sharedstatedi=* \
  | --sharedstated=* | --sharedstate=* | --sharedstat=* | --sharedsta=* \
  | --sharedst=* | --shareds=* | --shared=* | --share=* | --shar=* \
  | --sha=* | --sh=*)
    sharedstatedir="$ac_optarg" ;;

  -site | --site | --sit)
    ac_prev=site ;;
  -site=* | --site=* | --sit=*)
    site="$ac_optarg" ;;

  -srcdir | --srcdir | --srcdi | --srcd | --src | --sr)
    ac_prev=srcdir ;;
  -srcdir=* | --srcdir=* | --srcdi=* | --srcd=* | --src=* | --sr=*)
    srcdir="$ac_optarg" ;;

  -sysconfdir | --sysconfdir | --sysconfdi | --sysconfd | --sysconf \
  | --syscon | --sysco | --sysc | --sys | --sy)
    ac_prev=sysconfdir ;;
  -sysconfdir=* | --sysconfdir=* | --sysconfdi=* | --sysconfd=* | --sysconf=* \
  | --syscon=* | --sysco=* | --sysc=* | --sys=* | --sy=*)
    sysconfdir="$ac_optarg" ;;

  -target | --target | --targe | --targ | --tar | --ta | --t)
    ac_prev=target ;;
  -target=* | --target=* | --targe=* | --targ=* | --tar=* | --ta=* | --t=*)
    target="$ac_optarg" ;;

  -v | -verbose | --verbose | --verbos | --verbo | --verb)
    verbose=yes ;;

  -version | --version | --versio | --versi | --vers)
    echo "configure generated by autoconf version 2.13"
    exit 0 ;;

  -with-* | --with-*)
    ac_package=`echo $ac_option|sed -e 's/-*with-//' -e 's/=.*//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_package| sed 's/[-_a-zA-Z0-9]//g'`"; then
      { echo "configure: error: $ac_package: invalid package name" 1>&2; exit 1; }
    fi
    ac_package=`echo $ac_package| sed 's/-/_/g'`
    case "$ac_option" in
      *=*) ;;
      *) ac_optarg=yes ;;
    esac
    eval "with_${ac_package}='$ac_optarg'" ;;

  -without-* | --without-*)
    ac_package=`echo $ac_option|sed -e 's/-*without-//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_package| sed 's/[-a-zA-Z0-9_]//g'`"; then
      { echo "configure: error: $ac_package: invalid package name" 1>&2; exit 1; }
    fi
    ac_package=`echo $ac_package| sed 's/-/_/g'`
    eval "with_${ac_package}=no" ;;

  --x)
    # Obsolete; use --with-x.
    with_x=yes ;;

  -x-includes | --x-includes | --x-include | --x-includ | --x-inclu \
  | --x-incl | --x-inc | --x-in | --x-i)
    ac_prev=x_includes ;;
  -x-includes=* | --x-includes=* | --x-include=* | --x-includ=* | --x-inclu=* \
  | --x-incl=* | --x-inc=* | --x-in=* | --x-i=*)
    x_includes="$ac_optarg" ;;

  -x-libraries | --x-libraries | --x-librarie | --x-librari \
  | --x-librar | --x-libra | --x-libr | --x-lib | --x-li | --x-l)
    ac_prev=x_libraries ;;
  -x-libraries=* | --x-libraries=* | --x-librarie=* | --x-librari=* \
  | --x-librar=* | --x-libra=* | --x-libr=* | --x-lib=* | --x-li=* | --x-l=*)
    x_libraries="$ac_optarg" ;;

  -*) { echo "configure: error: $ac_option: invalid option; use --help to show usage" 1>&2; exit 1; }
    ;;

  *)
    if test -n "`echo $ac_option| sed 's/[-a-z0-9.]//g'`"; then
      echo "configure: warning: $ac_option: invalid host type" 1>&2
    fi
    if test "x$nonopt" != xNONE; then
      { echo "configure: error: can only configure for one host and one target at a time" 1>&2; exit 1; }
    fi
    nonopt="$ac_option"
    ;;

  esac
done

if test -n "$ac_prev"; then
  { echo "configure: error: missing argument to --`echo $ac_prev | sed 's/_/-/g'`" 1>&2; exit 1; }
fi

trap 'rm -fr conftest* confdefs* core core.* *.core $ac_clean_files; exit 1' 1 2 15

# File descriptor usage:
# 0 standard input
# 1 file creation
# 2 errors and warnings
# 3 some systems may open it to /dev/tty
# 4 used on the Kubota Titan
# 6 checking for... messages and results
# 5 compiler messages saved in config.log
if test "$silent" = yes; then
  exec 6>/dev/null
else
  exec 6>&1
fi
exec 5>./config.log

echo "\
This file contains any messages produced by compilers while
running configure, to aid debugging if configure makes a mistake.
" 1>&5

# Strip out --no-create and --no-recursion so they do not pile up.
# Also quote any args containing shell metacharacters.
ac_configure_args=
for ac_arg
do
  case "$ac_arg" in
  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c) ;;
  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r) ;;
  *" "*|*"	"*|*[\[\]\~\#\$\^\&\*\(\)\{\}\\\|\;\<\>\?]*)
  ac_configure_args="$ac_configure_args '$ac_arg'" ;;
  *) ac_configure_args="$ac_configure_args $ac_arg" ;;
  esac
done

# NLS nuisances.
# Only set these to C if already set.  These must not be set unconditionally
# because not all systems understand e.g. LANG=C (notably SCO).
# Fixing LC_MESSAGES prevents Solaris sh from translating var values in `set'!
# Non-C LC_CTYPE values break the ctype check.
if test "${LANG+set}"   = set; then LANG=C;   export LANG;   fi
if test "${LC_ALL+set}" = set; then LC_ALL=C; export LC_ALL; fi
if test "${LC_MESSAGES+set}" = set; then LC_MESSAGES=C; export LC_MESSAGES; fi
if test "${LC_CTYPE+set}"    = set; then LC_CTYPE=C;    export LC_CTYPE;    fi

# confdefs.h avoids OS command line length limits that DEFS can exceed.
rm -rf conftest* confdefs.h
# AIX cpp loses on an empty file, so make sure it contains at least a newline.
echo > confdefs.h

# A filename unique to this package, relative to the directory that
# configure is in, which we can look for to find out if srcdir is correct.
ac_unique_file=bnetd/handle_bnet.c

# Find the source files, if location was not specified.
if test -z "$srcdir"; then
  ac_srcdir_defaulted=yes
  # Try the directory containing this script, then its parent.
  ac_prog=$0
  ac_confdir=`echo $ac_prog|sed 's%/[^/][^/]*$%%'`
  test "x$ac_confdir" = "x$ac_prog" && ac_confdir=.
  srcdir=$ac_confdir
  if test ! -r $srcdir/$ac_unique_file; then
    srcdir=..
  fi
else
  ac_srcdir_defaulted=no
fi
if test ! -r $srcdir/$ac_unique_file; then
  if test "$ac_srcdir_defaulted" = yes; then
    { echo "configure: error: can not find sources in $ac_confdir or .." 1>&2; exit 1; }
  else
    { echo "configure: error: can not find sources in $srcdir" 1>&2; exit 1; }
  fi
fi
srcdir=`echo "${srcdir}" | sed 's%\([^/]\)/*$%\1%'`

# Prefer explicitly selected file to automatically selected ones.
if test -z "$CONFIG_SITE"; then
  if test "x$prefix" != xNONE; then
    CONFIG_SITE="$prefix/share/config.site $prefix/etc/config.site"
  else
    CONFIG_SITE="$ac_default_prefix/share/config.site $ac_default_prefix/etc/config.site"
  fi
fi
for ac_site_file in $CONFIG_SITE; do
  if test -r "$ac_site_file"; then
    echo "loading site script $ac_site_file"
    . "$ac_site_file"
  fi
done

if test -r "$cache_file"; then
  echo "loading cache $cache_file"
  . $cache_file
else
  echo "creating cache $cache_file"
  > $cache_file
fi

ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$CPP $CPPFLAGS'
ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.$ac_ext 1>&5'
ac_link='${CC-cc} -o conftest${ac_exeext} $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&5'
cross_compiling=$ac_cv_prog_cc_cross

ac_exeext=
ac_objext=o
if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  # Stardent Vistra SVR4 grep lacks -e, says ghazi@caip.rutgers.edu.
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    ac_n= ac_c='
' ac_t='	'
  else
    ac_n=-n ac_c= ac_t=
  fi
else
  ac_n= ac_c='\c' ac_t=
fi




AUTOCONF_CONFIG_DIR="autoconf"
ac_aux_dir=
for ac_dir in ${AUTOCONF_CONFIG_DIR} $srcdir/${AUTOCONF_CONFIG_DIR}; do
  if test -f $ac_dir/install-sh; then
    ac_aux_dir=$ac_dir
    ac_install_sh="$ac_aux_dir/install-sh -c"
    break
  elif test -f $ac_dir/install.sh; then
    ac_aux_dir=$ac_dir
    ac_install_sh="$ac_aux_dir/install.sh -c"
    break
  fi
done
if test -z "$ac_aux_dir"; then
  { echo "configure: error: can not find install-sh or install.sh in ${AUTOCONF_CONFIG_DIR} $srcdir/${AUTOCONF_CONFIG_DIR}" 1>&2; exit 1; }
fi
ac_config_guess=$ac_aux_dir/config.guess
ac_config_sub=$ac_aux_dir/config.sub
ac_configure=$ac_aux_dir/configure # This should be Cygnus configure.

## ----------------------------------------- ##
## ANSIfy the C compiler whenever possible.  ##
## From Franc,ois Pinard                     ##
## ----------------------------------------- ##

# serial 1

# @defmac AC_PROG_CC_STDC
# @maindex PROG_CC_STDC
# @ovindex CC
# If the C compiler in not in ANSI C mode by default, try to add an option
# to output variable @code{CC} to make it so.  This macro tries various
# options that select ANSI C on some system or another.  It considers the
# compiler to be in ANSI C mode if it handles function prototypes correctly.
#
# If you use this macro, you should check after calling it whether the C
# compiler has been set to accept ANSI C; if not, the shell variable
# @code{am_cv_prog_cc_stdc} is set to @samp{no}.  If you wrote your source
# code in ANSI C, you can make an un-ANSIfied copy of it by using the
# program @code{ansi2knr}, which comes with Ghostscript.
# @end defmac








# Do some error checking and defaulting for the host and target type.
# The inputs are:
#    configure --host=HOST --target=TARGET --build=BUILD NONOPT
#
# The rules are:
# 1. You are not allowed to specify --host, --target, and nonopt at the
#    same time.
# 2. Host defaults to nonopt.
# 3. If nonopt is not specified, then host defaults to the current host,
#    as determined by config.guess.
# 4. Target and build default to nonopt.
# 5. If nonopt is not specified, then target and build default to host.

# The aliases save the names the user supplied, while $host etc.
# will get canonicalized.
case $host---$target---$nonopt in
NONE---*---* | *---NONE---* | *---*---NONE) ;;
*) { echo "configure: error: can only configure for one host and one target at a time" 1>&2; exit 1; } ;;
esac


# Make sure we can run config.sub.
if ${CONFIG_SHELL-/bin/sh} $ac_config_sub sun4 >/dev/null 2>&1; then :
else { echo "configure: error: can not run $ac_config_sub" 1>&2; exit 1; }
fi

echo $ac_n "checking host system type""... $ac_c" 1>&6
echo "configure:622: checking host system type" >&5

host_alias=$host
case "$host_alias" in
NONE)
  case $nonopt in
  NONE)
    if host_alias=`${CONFIG_SHELL-/bin/sh} $ac_config_guess`; then :
    else { echo "configure: error: can not guess host type; you must specify one" 1>&2; exit 1; }
    fi ;;
  *) host_alias=$nonopt ;;
  esac ;;
esac

host=`${CONFIG_SHELL-/bin/sh} $ac_config_sub $host_alias`
host_cpu=`echo $host | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\1/'`
host_vendor=`echo $host | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\2/'`
host_os=`echo $host | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\3/'`
echo "$ac_t""$host" 1>&6

echo $ac_n "checking target system type""... $ac_c" 1>&6
echo "configure:643: checking target system type" >&5

target_alias=$target
case "$target_alias" in
NONE)
  case $nonopt in
  NONE) target_alias=$host_alias ;;
  *) target_alias=$nonopt ;;
  esac ;;
esac

target=`${CONFIG_SHELL-/bin/sh} $ac_config_sub $target_alias`
target_cpu=`echo $target | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\1/'`
target_vendor=`echo $target | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\2/'`
target_os=`echo $target | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\3/'`
echo "$ac_t""$target" 1>&6

echo $ac_n "checking build system type""... $ac_c" 1>&6
echo "configure:661: checking build system type" >&5

build_alias=$build
case "$build_alias" in
NONE)
  case $nonopt in
  NONE) build_alias=$host_alias ;;
  *) build_alias=$nonopt ;;
  esac ;;
esac

build=`${CONFIG_SHELL-/bin/sh} $ac_config_sub $build_alias`
build_cpu=`echo $build | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\1/'`
build_vendor=`echo $build | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\2/'`
build_os=`echo $build | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\3/'`
echo "$ac_t""$build" 1>&6

test "$host_alias" != "$target_alias" &&
  test "$program_prefix$program_suffix$program_transform_name" = \
    NONENONEs,x,x, &&
  program_prefix=${target_alias}-


# Checks for programs.
# Extract the first word of "gcc", so it can be a program name with args.
set dummy gcc; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:688: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_CC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_CC="gcc"
      break
    fi
  done
  IFS="$ac_save_ifs"
fi
fi
CC="$ac_cv_prog_CC"
if test -n "$CC"; then
  echo "$ac_t""$CC" 1>&6
else
  echo "$ac_t""no" 1>&6
fi

if test -z "$CC"; then
  # Extract the first word of "cc", so it can be a program name with args.
set dummy cc; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:718: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_CC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_prog_rejected=no
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if test "$ac_dir/$ac_word" = "/usr/ucb/cc"; then
        ac_prog_rejected=yes
	continue
      fi
      ac_cv_prog_CC="cc"
      break
    fi
  done
  IFS="$ac_save_ifs"
if test $ac_prog_rejected = yes; then
  # We found a bogon in the path, so make sure we never use it.
  set dummy $ac_cv_prog_CC
  shift
  if test $# -gt 0; then
    # We chose a different compiler from the bogus one.
    # However, it has the same basename, so the bogon will be chosen
    # first if we set CC to just the basename; use the full file name.
    shift
    set dummy "$ac_dir/$ac_word" "$@"
    shift
    ac_cv_prog_CC="$@"
  fi
fi
fi
fi
CC="$ac_cv_prog_CC"
if test -n "$CC"; then
  echo "$ac_t""$CC" 1>&6
else
  echo "$ac_t""no" 1>&6
fi

  if test -z "$CC"; then
    case "`uname -s`" in
    *win32* | *WIN32*)
      # Extract the first word of "cl", so it can be a program name with args.
set dummy cl; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:769: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_CC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_CC="cl"
      break
    fi
  done
  IFS="$ac_save_ifs"
fi
fi
CC="$ac_cv_prog_CC"
if test -n "$CC"; then
  echo "$ac_t""$CC" 1>&6
else
  echo "$ac_t""no" 1>&6
fi
 ;;
    esac
  fi
  test -z "$CC" && { echo "configure: error: no acceptable cc found in \$PATH" 1>&2; exit 1; }
fi

echo $ac_n "checking whether the C compiler ($CC $CFLAGS $LDFLAGS) works""... $ac_c" 1>&6
echo "configure:801: checking whether the C compiler ($CC $CFLAGS $LDFLAGS) works" >&5

ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$CPP $CPPFLAGS'
ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.$ac_ext 1>&5'
ac_link='${CC-cc} -o conftest${ac_exeext} $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&5'
cross_compiling=$ac_cv_prog_cc_cross

cat > conftest.$ac_ext << EOF

#line 812 "configure"
#include "confdefs.h"

main(){return(0);}
EOF
if { (eval echo configure:817: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  ac_cv_prog_cc_works=yes
  # If we can't run a trivial program, we are probably using a cross compiler.
  if (./conftest; exit) 2>/dev/null; then
    ac_cv_prog_cc_cross=no
  else
    ac_cv_prog_cc_cross=yes
  fi
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  ac_cv_prog_cc_works=no
fi
rm -fr conftest*
ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$CPP $CPPFLAGS'
ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.$ac_ext 1>&5'
ac_link='${CC-cc} -o conftest${ac_exeext} $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&5'
cross_compiling=$ac_cv_prog_cc_cross

echo "$ac_t""$ac_cv_prog_cc_works" 1>&6
if test $ac_cv_prog_cc_works = no; then
  { echo "configure: error: installation or configuration problem: C compiler cannot create executables." 1>&2; exit 1; }
fi
echo $ac_n "checking whether the C compiler ($CC $CFLAGS $LDFLAGS) is a cross-compiler""... $ac_c" 1>&6
echo "configure:843: checking whether the C compiler ($CC $CFLAGS $LDFLAGS) is a cross-compiler" >&5
echo "$ac_t""$ac_cv_prog_cc_cross" 1>&6
cross_compiling=$ac_cv_prog_cc_cross

echo $ac_n "checking whether we are using GNU C""... $ac_c" 1>&6
echo "configure:848: checking whether we are using GNU C" >&5
if eval "test \"`echo '$''{'ac_cv_prog_gcc'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if { ac_try='${CC-cc} -E conftest.c'; { (eval echo configure:857: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }; } | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
else
  ac_cv_prog_gcc=no
fi
fi

echo "$ac_t""$ac_cv_prog_gcc" 1>&6

if test $ac_cv_prog_gcc = yes; then
  GCC=yes
else
  GCC=
fi

ac_test_CFLAGS="${CFLAGS+set}"
ac_save_CFLAGS="$CFLAGS"
CFLAGS=
echo $ac_n "checking whether ${CC-cc} accepts -g""... $ac_c" 1>&6
echo "configure:876: checking whether ${CC-cc} accepts -g" >&5
if eval "test \"`echo '$''{'ac_cv_prog_cc_g'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  echo 'void f(){}' > conftest.c
if test -z "`${CC-cc} -g -c conftest.c 2>&1`"; then
  ac_cv_prog_cc_g=yes
else
  ac_cv_prog_cc_g=no
fi
rm -f conftest*

fi

echo "$ac_t""$ac_cv_prog_cc_g" 1>&6
if test "$ac_test_CFLAGS" = set; then
  CFLAGS="$ac_save_CFLAGS"
elif test $ac_cv_prog_cc_g = yes; then
  if test "$GCC" = yes; then
    CFLAGS="-g -O2"
  else
    CFLAGS="-g"
  fi
else
  if test "$GCC" = yes; then
    CFLAGS="-O2"
  else
    CFLAGS=
  fi
fi

# Find a good install program.  We prefer a C program (faster),
# so one script is as good as another.  But avoid the broken or
# incompatible versions:
# SysV /etc/install, /usr/sbin/install
# SunOS /usr/etc/install
# IRIX /sbin/install
# AIX /bin/install
# AIX 4 /usr/bin/installbsd, which doesn't work without a -g flag
# AFS /usr/afsws/bin/install, which mishandles nonexistent args
# SVR4 /usr/ucb/install, which tries to use the nonexistent group "staff"
# ./install, which can be erroneously created by make from ./install.sh.
echo $ac_n "checking for a BSD compatible install""... $ac_c" 1>&6
echo "configure:919: checking for a BSD compatible install" >&5
if test -z "$INSTALL"; then
if eval "test \"`echo '$''{'ac_cv_path_install'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
    IFS="${IFS= 	}"; ac_save_IFS="$IFS"; IFS=":"
  for ac_dir in $PATH; do
    # Account for people who put trailing slashes in PATH elements.
    case "$ac_dir/" in
    /|./|.//|/etc/*|/usr/sbin/*|/usr/etc/*|/sbin/*|/usr/afsws/bin/*|/usr/ucb/*) ;;
    *)
      # OSF1 and SCO ODT 3.0 have their own names for install.
      # Don't use installbsd from OSF since it installs stuff as root
      # by default.
      for ac_prog in ginstall scoinst install; do
        if test -f $ac_dir/$ac_prog; then
	  if test $ac_prog = install &&
            grep dspmsg $ac_dir/$ac_prog >/dev/null 2>&1; then
	    # AIX install.  It has an incompatible calling convention.
	    :
	  else
	    ac_cv_path_install="$ac_dir/$ac_prog -c"
	    break 2
	  fi
	fi
      done
      ;;
    esac
  done
  IFS="$ac_save_IFS"

fi
  if test "${ac_cv_path_install+set}" = set; then
    INSTALL="$ac_cv_path_install"
  else
    # As a last resort, use the slow shell script.  We don't cache a
    # path for INSTALL within a source directory, because that will
    # break other packages using the cache if that directory is
    # removed, or if the path is relative.
    INSTALL="$ac_install_sh"
  fi
fi
echo "$ac_t""$INSTALL" 1>&6

# Use test -z because SunOS4 sh mishandles braces in ${var-val}.
# It thinks the first close brace ends the variable substitution.
test -z "$INSTALL_PROGRAM" && INSTALL_PROGRAM='${INSTALL}'

test -z "$INSTALL_SCRIPT" && INSTALL_SCRIPT='${INSTALL_PROGRAM}'

test -z "$INSTALL_DATA" && INSTALL_DATA='${INSTALL} -m 644'


# Checks compiler.
echo $ac_n "checking how to run the C preprocessor""... $ac_c" 1>&6
echo "configure:974: checking how to run the C preprocessor" >&5
# On Suns, sometimes $CPP names a directory.
if test -n "$CPP" && test -d "$CPP"; then
  CPP=
fi
if test -z "$CPP"; then
if eval "test \"`echo '$''{'ac_cv_prog_CPP'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
    # This must be in double quotes, not single quotes, because CPP may get
  # substituted into the Makefile and "${CC-cc}" will confuse make.
  CPP="${CC-cc} -E"
  # On the NeXT, cc -E runs the code through the compiler's parser,
  # not just through cpp.
  cat > conftest.$ac_ext <<EOF
#line 989 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:995: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  CPP="${CC-cc} -E -traditional-cpp"
  cat > conftest.$ac_ext <<EOF
#line 1006 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1012: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  CPP="${CC-cc} -nologo -E"
  cat > conftest.$ac_ext <<EOF
#line 1023 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1029: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  CPP=/lib/cpp
fi
rm -f conftest*
fi
rm -f conftest*
fi
rm -f conftest*
  ac_cv_prog_CPP="$CPP"
fi
  CPP="$ac_cv_prog_CPP"
else
  ac_cv_prog_CPP="$CPP"
fi
echo "$ac_t""$CPP" 1>&6


# Checks for systems.
echo $ac_n "checking for AIX""... $ac_c" 1>&6
echo "configure:1056: checking for AIX" >&5
cat > conftest.$ac_ext <<EOF
#line 1058 "configure"
#include "confdefs.h"
#ifdef _AIX
  yes
#endif

EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "yes" >/dev/null 2>&1; then
  rm -rf conftest*
  echo "$ac_t""yes" 1>&6; cat >> confdefs.h <<\EOF
#define _ALL_SOURCE 1
EOF

else
  rm -rf conftest*
  echo "$ac_t""no" 1>&6
fi
rm -f conftest*


echo $ac_n "checking for POSIXized ISC""... $ac_c" 1>&6
echo "configure:1080: checking for POSIXized ISC" >&5
if test -d /etc/conf/kconfig.d &&
  grep _POSIX_VERSION /usr/include/sys/unistd.h >/dev/null 2>&1
then
  echo "$ac_t""yes" 1>&6
  ISC=yes # If later tests want to check for ISC.
  cat >> confdefs.h <<\EOF
#define _POSIX_SOURCE 1
EOF

  if test "$GCC" = yes; then
    CC="$CC -posix"
  else
    CC="$CC -Xp"
  fi
else
  echo "$ac_t""no" 1>&6
  ISC=
fi

ac_safe=`echo "minix/config.h" | sed 'y%./+-%__p_%'`
echo $ac_n "checking for minix/config.h""... $ac_c" 1>&6
echo "configure:1102: checking for minix/config.h" >&5
if eval "test \"`echo '$''{'ac_cv_header_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1107 "configure"
#include "confdefs.h"
#include <minix/config.h>
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1112: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=yes"
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  MINIX=yes
else
  echo "$ac_t""no" 1>&6
MINIX=
fi

if test "$MINIX" = yes; then
  cat >> confdefs.h <<\EOF
#define _POSIX_SOURCE 1
EOF

  cat >> confdefs.h <<\EOF
#define _POSIX_1_SOURCE 2
EOF

  cat >> confdefs.h <<\EOF
#define _MINIX 1
EOF

fi


# Custom checks.
zwarnflags=""
# Check whether --with-warn or --without-warn was given.
if test "${with_warn+set}" = set; then
  withval="$with_warn"
  zwarnflags="-pedantic -Wall -W -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wunreachable-code -Winline -Wpacked -Wno-sign-compare"
fi


zlangflags=""
# Check whether --with-ansi or --without-ansi was given.
if test "${with_ansi+set}" = set; then
  withval="$with_ansi"
  zlangflags="-ansi -pedantic"
fi


extra_include=""
# Check whether --with-includes or --without-includes was given.
if test "${with_includes+set}" = set; then
  withval="$with_includes"
  
case "x$withval" in
x/*|x.*)
  echo "$ac_t""adding ${withval} to include search path" 1>&6
  if test ! -d ${withval}; then
    echo "$ac_t""Warning: directory ${withval} does not exist" 1>&6
  fi
  extra_include="${extra_include} -I${withval}"
  ;;
*)
  echo "$ac_t""not adding any includes" 1>&6
  ;;
esac
fi


extra_lib_dir=""
# Check whether --with-libraries or --without-libraries was given.
if test "${with_libraries+set}" = set; then
  withval="$with_libraries"
  
case "x${withval}" in
x/*|x.*)
  echo "$ac_t""adding ${withval} to library search path" 1>&6
  if test ! -d ${withval}; then
    echo "$ac_t""Warning: directory ${withval} does not exist" 1>&6
  fi
  extra_lib_dir="${extra_lib_dir} -L${withval}"
  ;;
*)
  echo "$ac_t""not adding any libs" 1>&6
  ;;
esac
fi


extra_lib=""
# Check whether --with-efence or --without-efence was given.
if test "${with_efence+set}" = set; then
  withval="$with_efence"
  extra_lib="${extra_lib} -lefence"
fi


extra_define=""
# Check whether --with-bits or --without-bits was given.
if test "${with_bits+set}" = set; then
  withval="$with_bits"
  extra_define="${extra_define} -DWITH_BITS"
fi


# Check whether --enable-poll or --disable-poll was given.
if test "${enable_poll+set}" = set; then
  enableval="$enable_poll"
  
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

fi


# Disable poll() on certain platforms. Override by setting ac_cv_func_poll
# when running configure.
if test -z "$ac_cv_func_poll"; then
        case "$host" in
		alpha-dec-osf3.*)
			# John Kay (jkay@nlanr.net) 19970818
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
		*-hp-hpux*.*)
			# Duane Wessels
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
		*-linux-*)
			# Henrik Nordstrom (hno@hem.passagen.se) 19980817
			# poll is problematic on Linux.  We disable it
			# by default until Linux gets it right.
			rev=`uname -r | awk -F. '{printf "%03d%03d",$1,$2}'`
			if test $rev -lt 002002; then
			    echo "disabling poll for $host < 2.2..."
			    ac_cv_func_poll='no'
			fi
			;;
		powerpc-ibm-aix4.1.*)
			# Mike Laster (mlaster@metavillage.com) 19981021
			echo "disabling poll for $host..."
			ac_cv_func_poll='no'
			;;
		*-pc-sco3.2*)
			# Robert Side <rside@aiinc.bc.ca>
			# Mon, 18 Jan 1999 17:48:00 GMT
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


# Checks for compiler quirks.
if test $ac_cv_prog_gcc = yes; then
    echo $ac_n "checking whether ${CC-cc} needs -traditional""... $ac_c" 1>&6
echo "configure:1292: checking whether ${CC-cc} needs -traditional" >&5
if eval "test \"`echo '$''{'ac_cv_prog_gcc_traditional'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
    ac_pattern="Autoconf.*'x'"
  cat > conftest.$ac_ext <<EOF
#line 1298 "configure"
#include "confdefs.h"
#include <sgtty.h>
Autoconf TIOCGETP
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "$ac_pattern" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_prog_gcc_traditional=yes
else
  rm -rf conftest*
  ac_cv_prog_gcc_traditional=no
fi
rm -f conftest*


  if test $ac_cv_prog_gcc_traditional = no; then
    cat > conftest.$ac_ext <<EOF
#line 1316 "configure"
#include "confdefs.h"
#include <termio.h>
Autoconf TCGETA
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "$ac_pattern" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_prog_gcc_traditional=yes
fi
rm -f conftest*

  fi
fi

echo "$ac_t""$ac_cv_prog_gcc_traditional" 1>&6
  if test $ac_cv_prog_gcc_traditional = yes; then
    CC="$CC -traditional"
  fi
fi




echo $ac_n "checking for ${CC-cc} option to accept ANSI C""... $ac_c" 1>&6
echo "configure:1341: checking for ${CC-cc} option to accept ANSI C" >&5
if eval "test \"`echo '$''{'am_cv_prog_cc_stdc'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  am_cv_prog_cc_stdc=no
ac_save_CC="$CC"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc -D__EXTENSIONS__
for ac_arg in "" -qlanglvl=ansi -std1 "-Aa -D_HPUX_SOURCE" "-Xc -D__EXTENSIONS__"
do
  CC="$ac_save_CC $ac_arg"
  cat > conftest.$ac_ext <<EOF
#line 1357 "configure"
#include "confdefs.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Most of the following tests are stolen from RCS 5.7's src/conf.sh.  */
struct buf { int x; };
FILE * (*rcsopen) (struct buf *, struct stat *, int);
static char *e (p, i)
     char **p;
     int i;
{
  return p[i];
}
static char *f (char * (*g) (char **, int), char **p, ...)
{
  char *s;
  va_list v;
  va_start (v,p);
  s = g (p, va_arg (v,int));
  va_end (v);
  return s;
}
int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};
int pairnames (int, char **, FILE *(*)(struct buf *, struct stat *, int), int, int);
int argc;
char **argv;

int main() {

return f (e, argv, 0) != argv[0]  ||  f (e, argv, 1) != argv[1];

; return 0; }
EOF
if { (eval echo configure:1394: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  am_cv_prog_cc_stdc="$ac_arg"; break
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
fi
rm -f conftest*
done
CC="$ac_save_CC"

fi

if test -z "$am_cv_prog_cc_stdc"; then
  echo "$ac_t""none needed" 1>&6
else
  echo "$ac_t""$am_cv_prog_cc_stdc" 1>&6
fi
case "x$am_cv_prog_cc_stdc" in
  x|xno) ;;
  *) CC="$CC $am_cv_prog_cc_stdc" ;;
esac

echo $ac_n "checking for working const""... $ac_c" 1>&6
echo "configure:1418: checking for working const" >&5
if eval "test \"`echo '$''{'ac_cv_c_const'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1423 "configure"
#include "confdefs.h"

int main() {

/* Ultrix mips cc rejects this.  */
typedef int charset[2]; const charset x;
/* SunOS 4.1.1 cc rejects this.  */
char const *const *ccp;
char **p;
/* NEC SVR4.0.2 mips cc rejects this.  */
struct point {int x, y;};
static struct point const zero = {0,0};
/* AIX XL C 1.02.0.0 rejects this.
   It does not let you subtract one const X* pointer from another in an arm
   of an if-expression whose if-part is not a constant expression */
const char *g = "string";
ccp = &g + (g ? g-g : 0);
/* HPUX 7.0 cc rejects these. */
++ccp;
p = (char**) ccp;
ccp = (char const *const *) p;
{ /* SCO 3.2v4 cc rejects this.  */
  char *t;
  char const *s = 0 ? (char *) 0 : (char const *) 0;

  *t++ = 0;
}
{ /* Someone thinks the Sun supposedly-ANSI compiler will reject this.  */
  int x[] = {25, 17};
  const int *foo = &x[0];
  ++foo;
}
{ /* Sun SC1.0 ANSI compiler rejects this -- but not the above. */
  typedef const int *iptr;
  iptr p = 0;
  ++p;
}
{ /* AIX XL C 1.02.0.0 rejects this saying
     "k.c", line 2.27: 1506-025 (S) Operand must be a modifiable lvalue. */
  struct s { int j; const int *ap[3]; };
  struct s *b; b->j = 5;
}
{ /* ULTRIX-32 V3.1 (Rev 9) vcc rejects this */
  const int foo = 10;
}

; return 0; }
EOF
if { (eval echo configure:1472: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  ac_cv_c_const=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_c_const=no
fi
rm -f conftest*
fi

echo "$ac_t""$ac_cv_c_const" 1>&6
if test $ac_cv_c_const = no; then
  cat >> confdefs.h <<\EOF
#define const 
EOF

fi

#AC_EXEEXT

# Checks for libraries.
echo $ac_n "checking for pow in -lm""... $ac_c" 1>&6
echo "configure:1496: checking for pow in -lm" >&5
ac_lib_var=`echo m'_'pow | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_lib_$ac_lib_var'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  ac_save_LIBS="$LIBS"
LIBS="-lm  $LIBS"
cat > conftest.$ac_ext <<EOF
#line 1504 "configure"
#include "confdefs.h"
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char pow();

int main() {
pow()
; return 0; }
EOF
if { (eval echo configure:1515: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=no"
fi
rm -f conftest*
LIBS="$ac_save_LIBS"

fi
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  LIBS="$LIBS -lm"
else
  echo "$ac_t""no" 1>&6
fi

echo $ac_n "checking for gethostbyname in -lnsl""... $ac_c" 1>&6
echo "configure:1536: checking for gethostbyname in -lnsl" >&5
ac_lib_var=`echo nsl'_'gethostbyname | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_lib_$ac_lib_var'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  ac_save_LIBS="$LIBS"
LIBS="-lnsl  $LIBS"
cat > conftest.$ac_ext <<EOF
#line 1544 "configure"
#include "confdefs.h"
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char gethostbyname();

int main() {
gethostbyname()
; return 0; }
EOF
if { (eval echo configure:1555: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=no"
fi
rm -f conftest*
LIBS="$ac_save_LIBS"

fi
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  LIBS="$LIBS -lnsl"
else
  echo "$ac_t""no" 1>&6
fi

echo $ac_n "checking for socket in -lsocket""... $ac_c" 1>&6
echo "configure:1576: checking for socket in -lsocket" >&5
ac_lib_var=`echo socket'_'socket | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_lib_$ac_lib_var'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  ac_save_LIBS="$LIBS"
LIBS="-lsocket  $LIBS"
cat > conftest.$ac_ext <<EOF
#line 1584 "configure"
#include "confdefs.h"
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char socket();

int main() {
socket()
; return 0; }
EOF
if { (eval echo configure:1595: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=no"
fi
rm -f conftest*
LIBS="$ac_save_LIBS"

fi
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  LIBS="$LIBS -lsocket"
else
  echo "$ac_t""no" 1>&6
fi

echo $ac_n "checking for inet_aton in -lresolv""... $ac_c" 1>&6
echo "configure:1616: checking for inet_aton in -lresolv" >&5
ac_lib_var=`echo resolv'_'inet_aton | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_lib_$ac_lib_var'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  ac_save_LIBS="$LIBS"
LIBS="-lresolv  $LIBS"
cat > conftest.$ac_ext <<EOF
#line 1624 "configure"
#include "confdefs.h"
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char inet_aton();

int main() {
inet_aton()
; return 0; }
EOF
if { (eval echo configure:1635: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=no"
fi
rm -f conftest*
LIBS="$ac_save_LIBS"

fi
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  LIBS="$LIBS -lresolv"
else
  echo "$ac_t""no" 1>&6
fi


# Checks for header files.
ac_header_dirent=no
for ac_hdr in dirent.h sys/ndir.h sys/dir.h ndir.h
do
ac_safe=`echo "$ac_hdr" | sed 'y%./+-%__p_%'`
echo $ac_n "checking for $ac_hdr that defines DIR""... $ac_c" 1>&6
echo "configure:1662: checking for $ac_hdr that defines DIR" >&5
if eval "test \"`echo '$''{'ac_cv_header_dirent_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1667 "configure"
#include "confdefs.h"
#include <sys/types.h>
#include <$ac_hdr>
int main() {
DIR *dirp = 0;
; return 0; }
EOF
if { (eval echo configure:1675: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  eval "ac_cv_header_dirent_$ac_safe=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_header_dirent_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_dirent_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&6
    ac_tr_hdr=HAVE_`echo $ac_hdr | sed 'y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%'`
  cat >> confdefs.h <<EOF
#define $ac_tr_hdr 1
EOF
 ac_header_dirent=$ac_hdr; break
else
  echo "$ac_t""no" 1>&6
fi
done
# Two versions of opendir et al. are in -ldir and -lx on SCO Xenix.
if test $ac_header_dirent = dirent.h; then
echo $ac_n "checking for opendir in -ldir""... $ac_c" 1>&6
echo "configure:1700: checking for opendir in -ldir" >&5
ac_lib_var=`echo dir'_'opendir | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_lib_$ac_lib_var'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  ac_save_LIBS="$LIBS"
LIBS="-ldir  $LIBS"
cat > conftest.$ac_ext <<EOF
#line 1708 "configure"
#include "confdefs.h"
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char opendir();

int main() {
opendir()
; return 0; }
EOF
if { (eval echo configure:1719: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=no"
fi
rm -f conftest*
LIBS="$ac_save_LIBS"

fi
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  LIBS="$LIBS -ldir"
else
  echo "$ac_t""no" 1>&6
fi

else
echo $ac_n "checking for opendir in -lx""... $ac_c" 1>&6
echo "configure:1741: checking for opendir in -lx" >&5
ac_lib_var=`echo x'_'opendir | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_lib_$ac_lib_var'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  ac_save_LIBS="$LIBS"
LIBS="-lx  $LIBS"
cat > conftest.$ac_ext <<EOF
#line 1749 "configure"
#include "confdefs.h"
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char opendir();

int main() {
opendir()
; return 0; }
EOF
if { (eval echo configure:1760: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=no"
fi
rm -f conftest*
LIBS="$ac_save_LIBS"

fi
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  LIBS="$LIBS -lx"
else
  echo "$ac_t""no" 1>&6
fi

fi

echo $ac_n "checking for ANSI C header files""... $ac_c" 1>&6
echo "configure:1783: checking for ANSI C header files" >&5
if eval "test \"`echo '$''{'ac_cv_header_stdc'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1788 "configure"
#include "confdefs.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1796: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  rm -rf conftest*
  ac_cv_header_stdc=yes
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_header_stdc=no
fi
rm -f conftest*

if test $ac_cv_header_stdc = yes; then
  # SunOS 4.x string.h does not declare mem*, contrary to ANSI.
cat > conftest.$ac_ext <<EOF
#line 1813 "configure"
#include "confdefs.h"
#include <string.h>
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "memchr" >/dev/null 2>&1; then
  :
else
  rm -rf conftest*
  ac_cv_header_stdc=no
fi
rm -f conftest*

fi

if test $ac_cv_header_stdc = yes; then
  # ISC 2.0.2 stdlib.h does not declare free, contrary to ANSI.
cat > conftest.$ac_ext <<EOF
#line 1831 "configure"
#include "confdefs.h"
#include <stdlib.h>
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "free" >/dev/null 2>&1; then
  :
else
  rm -rf conftest*
  ac_cv_header_stdc=no
fi
rm -f conftest*

fi

if test $ac_cv_header_stdc = yes; then
  # /bin/cc in Irix-4.0.5 gets non-ANSI ctype macros unless using -ansi.
if test "$cross_compiling" = yes; then
  :
else
  cat > conftest.$ac_ext <<EOF
#line 1852 "configure"
#include "confdefs.h"
#include <ctype.h>
#define ISLOWER(c) ('a' <= (c) && (c) <= 'z')
#define TOUPPER(c) (ISLOWER(c) ? 'A' + ((c) - 'a') : (c))
#define XOR(e, f) (((e) && !(f)) || (!(e) && (f)))
int main () { int i; for (i = 0; i < 256; i++)
if (XOR (islower (i), ISLOWER (i)) || toupper (i) != TOUPPER (i)) exit(2);
exit (0); }

EOF
if { (eval echo configure:1863: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  :
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_header_stdc=no
fi
rm -fr conftest*
fi

fi
fi

echo "$ac_t""$ac_cv_header_stdc" 1>&6
if test $ac_cv_header_stdc = yes; then
  cat >> confdefs.h <<\EOF
#define STDC_HEADERS 1
EOF

fi

echo $ac_n "checking whether time.h and sys/time.h may both be included""... $ac_c" 1>&6
echo "configure:1887: checking whether time.h and sys/time.h may both be included" >&5
if eval "test \"`echo '$''{'ac_cv_header_time'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1892 "configure"
#include "confdefs.h"
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
int main() {
struct tm *tp;
; return 0; }
EOF
if { (eval echo configure:1901: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  ac_cv_header_time=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_header_time=no
fi
rm -f conftest*
fi

echo "$ac_t""$ac_cv_header_time" 1>&6
if test $ac_cv_header_time = yes; then
  cat >> confdefs.h <<\EOF
#define TIME_WITH_SYS_TIME 1
EOF

fi

echo $ac_n "checking whether stat file-mode macros are broken""... $ac_c" 1>&6
echo "configure:1922: checking whether stat file-mode macros are broken" >&5
if eval "test \"`echo '$''{'ac_cv_header_stat_broken'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1927 "configure"
#include "confdefs.h"
#include <sys/types.h>
#include <sys/stat.h>

#if defined(S_ISBLK) && defined(S_IFDIR)
# if S_ISBLK (S_IFDIR)
You lose.
# endif
#endif

#if defined(S_ISBLK) && defined(S_IFCHR)
# if S_ISBLK (S_IFCHR)
You lose.
# endif
#endif

#if defined(S_ISLNK) && defined(S_IFREG)
# if S_ISLNK (S_IFREG)
You lose.
# endif
#endif

#if defined(S_ISSOCK) && defined(S_IFREG)
# if S_ISSOCK (S_IFREG)
You lose.
# endif
#endif

EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "You lose" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_header_stat_broken=yes
else
  rm -rf conftest*
  ac_cv_header_stat_broken=no
fi
rm -f conftest*

fi

echo "$ac_t""$ac_cv_header_stat_broken" 1>&6
if test $ac_cv_header_stat_broken = yes; then
  cat >> confdefs.h <<\EOF
#define STAT_MACROS_BROKEN 1
EOF

fi

for ac_hdr in fcntl.h sys/time.h sys/select.h string.h strings.h unistd.h stdarg.h varargs.h malloc.h sys/utsname.h sys/timeb.h sys/socket.h sys/param.h netinet/in.h arpa/inet.h netdb.h termios.h stddef.h memory.h sys/types.h sys/wait.h sys/ioctl.h stdint.h sys/file.h limits.h poll.h sys/poll.h stropts.h sys/stropts.h sys/stat.h pwd.h grp.h dir.h direct.h
do
ac_safe=`echo "$ac_hdr" | sed 'y%./+-%__p_%'`
echo $ac_n "checking for $ac_hdr""... $ac_c" 1>&6
echo "configure:1981: checking for $ac_hdr" >&5
if eval "test \"`echo '$''{'ac_cv_header_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1986 "configure"
#include "confdefs.h"
#include <$ac_hdr>
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1991: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=yes"
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&6
    ac_tr_hdr=HAVE_`echo $ac_hdr | sed 'y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%'`
  cat >> confdefs.h <<EOF
#define $ac_tr_hdr 1
EOF
 
else
  echo "$ac_t""no" 1>&6
fi
done


# Checks for typedefs and structures
echo $ac_n "checking whether struct tm is in sys/time.h or time.h""... $ac_c" 1>&6
echo "configure:2020: checking whether struct tm is in sys/time.h or time.h" >&5
if eval "test \"`echo '$''{'ac_cv_struct_tm'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2025 "configure"
#include "confdefs.h"
#include <sys/types.h>
#include <time.h>
int main() {
struct tm *tp; tp->tm_sec;
; return 0; }
EOF
if { (eval echo configure:2033: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  ac_cv_struct_tm=time.h
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_struct_tm=sys/time.h
fi
rm -f conftest*
fi

echo "$ac_t""$ac_cv_struct_tm" 1>&6
if test $ac_cv_struct_tm = sys/time.h; then
  cat >> confdefs.h <<\EOF
#define TM_IN_SYS_TIME 1
EOF

fi

echo $ac_n "checking for pid_t""... $ac_c" 1>&6
echo "configure:2054: checking for pid_t" >&5
if eval "test \"`echo '$''{'ac_cv_type_pid_t'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2059 "configure"
#include "confdefs.h"
#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "(^|[^a-zA-Z_0-9])pid_t[^a-zA-Z_0-9]" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_type_pid_t=yes
else
  rm -rf conftest*
  ac_cv_type_pid_t=no
fi
rm -f conftest*

fi
echo "$ac_t""$ac_cv_type_pid_t" 1>&6
if test $ac_cv_type_pid_t = no; then
  cat >> confdefs.h <<\EOF
#define pid_t int
EOF

fi

echo $ac_n "checking return type of signal handlers""... $ac_c" 1>&6
echo "configure:2087: checking return type of signal handlers" >&5
if eval "test \"`echo '$''{'ac_cv_type_signal'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2092 "configure"
#include "confdefs.h"
#include <sys/types.h>
#include <signal.h>
#ifdef signal
#undef signal
#endif
#ifdef __cplusplus
extern "C" void (*signal (int, void (*)(int)))(int);
#else
void (*signal ()) ();
#endif

int main() {
int i;
; return 0; }
EOF
if { (eval echo configure:2109: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  ac_cv_type_signal=void
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_type_signal=int
fi
rm -f conftest*
fi

echo "$ac_t""$ac_cv_type_signal" 1>&6
cat >> confdefs.h <<EOF
#define RETSIGTYPE $ac_cv_type_signal
EOF


echo $ac_n "checking for off_t""... $ac_c" 1>&6
echo "configure:2128: checking for off_t" >&5
if eval "test \"`echo '$''{'ac_cv_type_off_t'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2133 "configure"
#include "confdefs.h"
#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "(^|[^a-zA-Z_0-9])off_t[^a-zA-Z_0-9]" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_type_off_t=yes
else
  rm -rf conftest*
  ac_cv_type_off_t=no
fi
rm -f conftest*

fi
echo "$ac_t""$ac_cv_type_off_t" 1>&6
if test $ac_cv_type_off_t = no; then
  cat >> confdefs.h <<\EOF
#define off_t long
EOF

fi

echo $ac_n "checking for size_t""... $ac_c" 1>&6
echo "configure:2161: checking for size_t" >&5
if eval "test \"`echo '$''{'ac_cv_type_size_t'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2166 "configure"
#include "confdefs.h"
#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "(^|[^a-zA-Z_0-9])size_t[^a-zA-Z_0-9]" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_type_size_t=yes
else
  rm -rf conftest*
  ac_cv_type_size_t=no
fi
rm -f conftest*

fi
echo "$ac_t""$ac_cv_type_size_t" 1>&6
if test $ac_cv_type_size_t = no; then
  cat >> confdefs.h <<\EOF
#define size_t unsigned
EOF

fi

echo $ac_n "checking size of unsigned char""... $ac_c" 1>&6
echo "configure:2194: checking size of unsigned char" >&5
if eval "test \"`echo '$''{'ac_cv_sizeof_unsigned_char'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  ac_cv_sizeof_unsigned_char=1
else
  cat > conftest.$ac_ext <<EOF
#line 2202 "configure"
#include "confdefs.h"
#include <stdio.h>
int main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) return(1);
  fprintf(f, "%d\n", sizeof(unsigned char));
  return(0);
}
EOF
if { (eval echo configure:2213: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  ac_cv_sizeof_unsigned_char=`cat conftestval`
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_sizeof_unsigned_char=0
fi
rm -fr conftest*
fi

fi
echo "$ac_t""$ac_cv_sizeof_unsigned_char" 1>&6
cat >> confdefs.h <<EOF
#define SIZEOF_UNSIGNED_CHAR $ac_cv_sizeof_unsigned_char
EOF


echo $ac_n "checking size of unsigned short""... $ac_c" 1>&6
echo "configure:2233: checking size of unsigned short" >&5
if eval "test \"`echo '$''{'ac_cv_sizeof_unsigned_short'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  ac_cv_sizeof_unsigned_short=2
else
  cat > conftest.$ac_ext <<EOF
#line 2241 "configure"
#include "confdefs.h"
#include <stdio.h>
int main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) return(1);
  fprintf(f, "%d\n", sizeof(unsigned short));
  return(0);
}
EOF
if { (eval echo configure:2252: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  ac_cv_sizeof_unsigned_short=`cat conftestval`
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_sizeof_unsigned_short=0
fi
rm -fr conftest*
fi

fi
echo "$ac_t""$ac_cv_sizeof_unsigned_short" 1>&6
cat >> confdefs.h <<EOF
#define SIZEOF_UNSIGNED_SHORT $ac_cv_sizeof_unsigned_short
EOF


echo $ac_n "checking size of unsigned int""... $ac_c" 1>&6
echo "configure:2272: checking size of unsigned int" >&5
if eval "test \"`echo '$''{'ac_cv_sizeof_unsigned_int'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  ac_cv_sizeof_unsigned_int=4
else
  cat > conftest.$ac_ext <<EOF
#line 2280 "configure"
#include "confdefs.h"
#include <stdio.h>
int main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) return(1);
  fprintf(f, "%d\n", sizeof(unsigned int));
  return(0);
}
EOF
if { (eval echo configure:2291: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  ac_cv_sizeof_unsigned_int=`cat conftestval`
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_sizeof_unsigned_int=0
fi
rm -fr conftest*
fi

fi
echo "$ac_t""$ac_cv_sizeof_unsigned_int" 1>&6
cat >> confdefs.h <<EOF
#define SIZEOF_UNSIGNED_INT $ac_cv_sizeof_unsigned_int
EOF


echo $ac_n "checking size of unsigned long""... $ac_c" 1>&6
echo "configure:2311: checking size of unsigned long" >&5
if eval "test \"`echo '$''{'ac_cv_sizeof_unsigned_long'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  ac_cv_sizeof_unsigned_long=4
else
  cat > conftest.$ac_ext <<EOF
#line 2319 "configure"
#include "confdefs.h"
#include <stdio.h>
int main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) return(1);
  fprintf(f, "%d\n", sizeof(unsigned long));
  return(0);
}
EOF
if { (eval echo configure:2330: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  ac_cv_sizeof_unsigned_long=`cat conftestval`
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_sizeof_unsigned_long=0
fi
rm -fr conftest*
fi

fi
echo "$ac_t""$ac_cv_sizeof_unsigned_long" 1>&6
cat >> confdefs.h <<EOF
#define SIZEOF_UNSIGNED_LONG $ac_cv_sizeof_unsigned_long
EOF


echo $ac_n "checking size of unsigned long long""... $ac_c" 1>&6
echo "configure:2350: checking size of unsigned long long" >&5
if eval "test \"`echo '$''{'ac_cv_sizeof_unsigned_long_long'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  ac_cv_sizeof_unsigned_long_long=0
else
  cat > conftest.$ac_ext <<EOF
#line 2358 "configure"
#include "confdefs.h"
#include <stdio.h>
int main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) return(1);
  fprintf(f, "%d\n", sizeof(unsigned long long));
  return(0);
}
EOF
if { (eval echo configure:2369: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  ac_cv_sizeof_unsigned_long_long=`cat conftestval`
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_sizeof_unsigned_long_long=0
fi
rm -fr conftest*
fi

fi
echo "$ac_t""$ac_cv_sizeof_unsigned_long_long" 1>&6
cat >> confdefs.h <<EOF
#define SIZEOF_UNSIGNED_LONG_LONG $ac_cv_sizeof_unsigned_long_long
EOF



# Checks for library functions.
echo $ac_n "checking for strftime""... $ac_c" 1>&6
echo "configure:2391: checking for strftime" >&5
if eval "test \"`echo '$''{'ac_cv_func_strftime'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2396 "configure"
#include "confdefs.h"
/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char strftime(); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char strftime();

int main() {

/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_strftime) || defined (__stub___strftime)
choke me
#else
strftime();
#endif

; return 0; }
EOF
if { (eval echo configure:2419: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_func_strftime=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_func_strftime=no"
fi
rm -f conftest*
fi

if eval "test \"`echo '$ac_cv_func_'strftime`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  cat >> confdefs.h <<\EOF
#define HAVE_STRFTIME 1
EOF

else
  echo "$ac_t""no" 1>&6
# strftime is in -lintl on SCO UNIX.
echo $ac_n "checking for strftime in -lintl""... $ac_c" 1>&6
echo "configure:2441: checking for strftime in -lintl" >&5
ac_lib_var=`echo intl'_'strftime | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_lib_$ac_lib_var'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  ac_save_LIBS="$LIBS"
LIBS="-lintl  $LIBS"
cat > conftest.$ac_ext <<EOF
#line 2449 "configure"
#include "confdefs.h"
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char strftime();

int main() {
strftime()
; return 0; }
EOF
if { (eval echo configure:2460: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_lib_$ac_lib_var=no"
fi
rm -f conftest*
LIBS="$ac_save_LIBS"

fi
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  cat >> confdefs.h <<\EOF
#define HAVE_STRFTIME 1
EOF

LIBS="-lintl $LIBS"
else
  echo "$ac_t""no" 1>&6
fi

fi

echo $ac_n "checking for vprintf""... $ac_c" 1>&6
echo "configure:2487: checking for vprintf" >&5
if eval "test \"`echo '$''{'ac_cv_func_vprintf'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2492 "configure"
#include "confdefs.h"
/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char vprintf(); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char vprintf();

int main() {

/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_vprintf) || defined (__stub___vprintf)
choke me
#else
vprintf();
#endif

; return 0; }
EOF
if { (eval echo configure:2515: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_func_vprintf=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_func_vprintf=no"
fi
rm -f conftest*
fi

if eval "test \"`echo '$ac_cv_func_'vprintf`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  cat >> confdefs.h <<\EOF
#define HAVE_VPRINTF 1
EOF

else
  echo "$ac_t""no" 1>&6
fi

if test "$ac_cv_func_vprintf" != yes; then
echo $ac_n "checking for _doprnt""... $ac_c" 1>&6
echo "configure:2539: checking for _doprnt" >&5
if eval "test \"`echo '$''{'ac_cv_func__doprnt'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2544 "configure"
#include "confdefs.h"
/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char _doprnt(); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char _doprnt();

int main() {

/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub__doprnt) || defined (__stub____doprnt)
choke me
#else
_doprnt();
#endif

; return 0; }
EOF
if { (eval echo configure:2567: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_func__doprnt=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_func__doprnt=no"
fi
rm -f conftest*
fi

if eval "test \"`echo '$ac_cv_func_'_doprnt`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  cat >> confdefs.h <<\EOF
#define HAVE_DOPRNT 1
EOF

else
  echo "$ac_t""no" 1>&6
fi

fi

echo $ac_n "checking whether setpgrp takes no argument""... $ac_c" 1>&6
echo "configure:2592: checking whether setpgrp takes no argument" >&5
if eval "test \"`echo '$''{'ac_cv_func_setpgrp_void'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  { echo "configure: error: cannot check setpgrp if cross compiling" 1>&2; exit 1; }
else
  cat > conftest.$ac_ext <<EOF
#line 2600 "configure"
#include "confdefs.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*
 * If this system has a BSD-style setpgrp, which takes arguments, exit
 * successfully.
 */
main()
{
    if (setpgrp(1,1) == -1)
	exit(0);
    else
	exit(1);
}

EOF
if { (eval echo configure:2620: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  ac_cv_func_setpgrp_void=no
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_func_setpgrp_void=yes
fi
rm -fr conftest*
fi


fi

echo "$ac_t""$ac_cv_func_setpgrp_void" 1>&6
if test $ac_cv_func_setpgrp_void = yes; then
  cat >> confdefs.h <<\EOF
#define SETPGRP_VOID 1
EOF

fi

echo $ac_n "checking whether closedir returns void""... $ac_c" 1>&6
echo "configure:2644: checking whether closedir returns void" >&5
if eval "test \"`echo '$''{'ac_cv_func_closedir_void'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  ac_cv_func_closedir_void=yes
else
  cat > conftest.$ac_ext <<EOF
#line 2652 "configure"
#include "confdefs.h"
#include <sys/types.h>
#include <$ac_header_dirent>
int closedir(); main() { exit(closedir(opendir(".")) != 0); }
EOF
if { (eval echo configure:2658: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  ac_cv_func_closedir_void=no
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  ac_cv_func_closedir_void=yes
fi
rm -fr conftest*
fi

fi

echo "$ac_t""$ac_cv_func_closedir_void" 1>&6
if test $ac_cv_func_closedir_void = yes; then
  cat >> confdefs.h <<\EOF
#define CLOSEDIR_VOID 1
EOF

fi

for ac_func in mkdir _mkdir
do
echo $ac_n "checking for $ac_func""... $ac_c" 1>&6
echo "configure:2683: checking for $ac_func" >&5
if eval "test \"`echo '$''{'ac_cv_func_$ac_func'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2688 "configure"
#include "confdefs.h"
/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char $ac_func(); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $ac_func();

int main() {

/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$ac_func) || defined (__stub___$ac_func)
choke me
#else
$ac_func();
#endif

; return 0; }
EOF
if { (eval echo configure:2711: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_func_$ac_func=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_func_$ac_func=no"
fi
rm -f conftest*
fi

if eval "test \"`echo '$ac_cv_func_'$ac_func`\" = yes"; then
  echo "$ac_t""yes" 1>&6
    ac_tr_func=HAVE_`echo $ac_func | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
  cat >> confdefs.h <<EOF
#define $ac_tr_func 1
EOF
 
else
  echo "$ac_t""no" 1>&6
fi
done

echo $ac_n "checking whether mkdir takes one argument""... $ac_c" 1>&6
echo "configure:2736: checking whether mkdir takes one argument" >&5
if eval "test \"`echo '$''{'ac_cv_mkdir_takes_one_arg'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2741 "configure"
#include "confdefs.h"

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_DIR_H
# include <dir.h>
#endif
#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif
#ifndef HAVE_MKDIR
# ifdef HAVE__MKDIR
#  define mkdir _mkdir
# endif
#endif

int main() {
mkdir(".");
; return 0; }
EOF
if { (eval echo configure:2766: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  ac_cv_mkdir_takes_one_arg=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_mkdir_takes_one_arg=no
fi
rm -f conftest*
fi

echo "$ac_t""$ac_cv_mkdir_takes_one_arg" 1>&6
if test x"$ac_cv_mkdir_takes_one_arg" = xyes; then
  cat >> confdefs.h <<\EOF
#define MKDIR_TAKES_ONE_ARG 1
EOF

fi

for ac_func in gethostname gettimeofday select socket strdup strtoul strerror inet_aton inet_ntoa uname recv send recvfrom sendto uname fork getpid sigaction sigprocmask sigaddset setpgid setpgrp ftime strcasecmp strncasecmp stricmp strnicmp chdir difftime strchr strrchr index rindex memcpy memset memmove bcopy wait waitpid pipe getenv ioctl setsid mktime poll gethostbyname getservbyname getlogin pow getpwnam getgrnam getuid getgid setuid setgid mkdir _mkdir
do
echo $ac_n "checking for $ac_func""... $ac_c" 1>&6
echo "configure:2789: checking for $ac_func" >&5
if eval "test \"`echo '$''{'ac_cv_func_$ac_func'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 2794 "configure"
#include "confdefs.h"
/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char $ac_func(); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $ac_func();

int main() {

/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$ac_func) || defined (__stub___$ac_func)
choke me
#else
$ac_func();
#endif

; return 0; }
EOF
if { (eval echo configure:2817: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  rm -rf conftest*
  eval "ac_cv_func_$ac_func=yes"
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_func_$ac_func=no"
fi
rm -f conftest*
fi

if eval "test \"`echo '$ac_cv_func_'$ac_func`\" = yes"; then
  echo "$ac_t""yes" 1>&6
    ac_tr_func=HAVE_`echo $ac_func | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
  cat >> confdefs.h <<EOF
#define $ac_tr_func 1
EOF
 
else
  echo "$ac_t""no" 1>&6
fi
done


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






trap '' 1 2 15
cat > confcache <<\EOF
# This file is a shell script that caches the results of configure
# tests run on this system so they can be shared between configure
# scripts and configure runs.  It is not useful on other systems.
# If it contains results you don't want to keep, you may remove or edit it.
#
# By default, configure uses ./config.cache as the cache file,
# creating it if it does not exist already.  You can give configure
# the --cache-file=FILE option to use a different cache file; that is
# what configure does when it calls configure scripts in
# subdirectories, so they share the cache.
# Giving --cache-file=/dev/null disables caching, for debugging configure.
# config.status only pays attention to the cache file if you give it the
# --recheck option to rerun configure.
#
EOF
# The following way of writing the cache mishandles newlines in values,
# but we know of no workaround that is simple, portable, and efficient.
# So, don't put newlines in cache variables' values.
# Ultrix sh set writes to stderr and can't be redirected directly,
# and sets the high bit in the cache file unless we assign to the vars.
(set) 2>&1 |
  case `(ac_space=' '; set | grep ac_space) 2>&1` in
  *ac_space=\ *)
    # `set' does not quote correctly, so add quotes (double-quote substitution
    # turns \\\\ into \\, and sed turns \\ into \).
    sed -n \
      -e "s/'/'\\\\''/g" \
      -e "s/^\\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\\)=\\(.*\\)/\\1=\${\\1='\\2'}/p"
    ;;
  *)
    # `set' quotes correctly as required by POSIX, so do not add quotes.
    sed -n -e 's/^\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\)=\(.*\)/\1=${\1=\2}/p'
    ;;
  esac >> confcache
if cmp -s $cache_file confcache; then
  :
else
  if test -w $cache_file; then
    echo "updating cache $cache_file"
    cat confcache > $cache_file
  else
    echo "not updating unwritable cache $cache_file"
  fi
fi
rm -f confcache

trap 'rm -fr conftest* confdefs* core core.* *.core $ac_clean_files; exit 1' 1 2 15

test "x$prefix" = xNONE && prefix=$ac_default_prefix
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

# Any assignment to VPATH causes Sun make to only execute
# the first set of double-colon rules, so remove it if not needed.
# If there is a colon in the path, we need to keep it.
if test "x$srcdir" = x.; then
  ac_vpsub='/^[ 	]*VPATH[ 	]*=[^:]*$/d'
fi

trap 'rm -f $CONFIG_STATUS conftest*; exit 1' 1 2 15

DEFS=-DHAVE_CONFIG_H

# Without the "./", some shells look in PATH for config.status.
: ${CONFIG_STATUS=./config.status}

echo creating $CONFIG_STATUS
rm -f $CONFIG_STATUS
cat > $CONFIG_STATUS <<EOF
#! /bin/sh
# Generated automatically by configure.
# Run this file to recreate the current configuration.
# This directory was configured as follows,
# on host `(hostname || uname -n) 2>/dev/null | sed 1q`:
#
# $0 $ac_configure_args
#
# Compiler output produced by configure, useful for debugging
# configure, is in ./config.log if it exists.

ac_cs_usage="Usage: $CONFIG_STATUS [--recheck] [--version] [--help]"
for ac_option
do
  case "\$ac_option" in
  -recheck | --recheck | --rechec | --reche | --rech | --rec | --re | --r)
    echo "running \${CONFIG_SHELL-/bin/sh} $0 $ac_configure_args --no-create --no-recursion"
    exec \${CONFIG_SHELL-/bin/sh} $0 $ac_configure_args --no-create --no-recursion ;;
  -version | --version | --versio | --versi | --vers | --ver | --ve | --v)
    echo "$CONFIG_STATUS generated by autoconf version 2.13"
    exit 0 ;;
  -help | --help | --hel | --he | --h)
    echo "\$ac_cs_usage"; exit 0 ;;
  *) echo "\$ac_cs_usage"; exit 1 ;;
  esac
done

ac_given_srcdir=$srcdir
ac_given_INSTALL="$INSTALL"

trap 'rm -fr `echo "Makefile config.h" | sed "s/:[^ ]*//g"` conftest*; exit 1' 1 2 15
EOF
cat >> $CONFIG_STATUS <<EOF

# Protect against being on the right side of a sed subst in config.status.
sed 's/%@/@@/; s/@%/@@/; s/%g\$/@g/; /@g\$/s/[\\\\&%]/\\\\&/g;
 s/@@/%@/; s/@@/@%/; s/@g\$/%g/' > conftest.subs <<\\CEOF
$ac_vpsub
$extrasub
s%@SHELL@%$SHELL%g
s%@CFLAGS@%$CFLAGS%g
s%@CPPFLAGS@%$CPPFLAGS%g
s%@CXXFLAGS@%$CXXFLAGS%g
s%@FFLAGS@%$FFLAGS%g
s%@DEFS@%$DEFS%g
s%@LDFLAGS@%$LDFLAGS%g
s%@LIBS@%$LIBS%g
s%@exec_prefix@%$exec_prefix%g
s%@prefix@%$prefix%g
s%@program_transform_name@%$program_transform_name%g
s%@bindir@%$bindir%g
s%@sbindir@%$sbindir%g
s%@libexecdir@%$libexecdir%g
s%@datadir@%$datadir%g
s%@sysconfdir@%$sysconfdir%g
s%@sharedstatedir@%$sharedstatedir%g
s%@localstatedir@%$localstatedir%g
s%@libdir@%$libdir%g
s%@includedir@%$includedir%g
s%@oldincludedir@%$oldincludedir%g
s%@infodir@%$infodir%g
s%@mandir@%$mandir%g
s%@host@%$host%g
s%@host_alias@%$host_alias%g
s%@host_cpu@%$host_cpu%g
s%@host_vendor@%$host_vendor%g
s%@host_os@%$host_os%g
s%@target@%$target%g
s%@target_alias@%$target_alias%g
s%@target_cpu@%$target_cpu%g
s%@target_vendor@%$target_vendor%g
s%@target_os@%$target_os%g
s%@build@%$build%g
s%@build_alias@%$build_alias%g
s%@build_cpu@%$build_cpu%g
s%@build_vendor@%$build_vendor%g
s%@build_os@%$build_os%g
s%@CC@%$CC%g
s%@INSTALL_PROGRAM@%$INSTALL_PROGRAM%g
s%@INSTALL_SCRIPT@%$INSTALL_SCRIPT%g
s%@INSTALL_DATA@%$INSTALL_DATA%g
s%@CPP@%$CPP%g
s%@DEFINES@%$DEFINES%g
s%@COMPILEANSI1@%$COMPILEANSI1%g
s%@COMPILEANSI2@%$COMPILEANSI2%g
s%@COMPILEANSI3@%$COMPILEANSI3%g
s%@ANSI2KNR_TMP@%$ANSI2KNR_TMP%g
s%@ANSI2KNR_DEP@%$ANSI2KNR_DEP%g

CEOF
EOF

cat >> $CONFIG_STATUS <<\EOF

# Split the substitutions into bite-sized pieces for seds with
# small command number limits, like on Digital OSF/1 and HP-UX.
ac_max_sed_cmds=90 # Maximum number of lines to put in a sed script.
ac_file=1 # Number of current file.
ac_beg=1 # First line for current file.
ac_end=$ac_max_sed_cmds # Line after last line for current file.
ac_more_lines=:
ac_sed_cmds=""
while $ac_more_lines; do
  if test $ac_beg -gt 1; then
    sed "1,${ac_beg}d; ${ac_end}q" conftest.subs > conftest.s$ac_file
  else
    sed "${ac_end}q" conftest.subs > conftest.s$ac_file
  fi
  if test ! -s conftest.s$ac_file; then
    ac_more_lines=false
    rm -f conftest.s$ac_file
  else
    if test -z "$ac_sed_cmds"; then
      ac_sed_cmds="sed -f conftest.s$ac_file"
    else
      ac_sed_cmds="$ac_sed_cmds | sed -f conftest.s$ac_file"
    fi
    ac_file=`expr $ac_file + 1`
    ac_beg=$ac_end
    ac_end=`expr $ac_end + $ac_max_sed_cmds`
  fi
done
if test -z "$ac_sed_cmds"; then
  ac_sed_cmds=cat
fi
EOF

cat >> $CONFIG_STATUS <<EOF

CONFIG_FILES=\${CONFIG_FILES-"Makefile"}
EOF
cat >> $CONFIG_STATUS <<\EOF
for ac_file in .. $CONFIG_FILES; do if test "x$ac_file" != x..; then
  # Support "outfile[:infile[:infile...]]", defaulting infile="outfile.in".
  case "$ac_file" in
  *:*) ac_file_in=`echo "$ac_file"|sed 's%[^:]*:%%'`
       ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;
  *) ac_file_in="${ac_file}.in" ;;
  esac

  # Adjust a relative srcdir, top_srcdir, and INSTALL for subdirectories.

  # Remove last slash and all that follows it.  Not all systems have dirname.
  ac_dir=`echo $ac_file|sed 's%/[^/][^/]*$%%'`
  if test "$ac_dir" != "$ac_file" && test "$ac_dir" != .; then
    # The file is in a subdirectory.
    test ! -d "$ac_dir" && mkdir "$ac_dir"
    ac_dir_suffix="/`echo $ac_dir|sed 's%^\./%%'`"
    # A "../" for each directory in $ac_dir_suffix.
    ac_dots=`echo $ac_dir_suffix|sed 's%/[^/]*%../%g'`
  else
    ac_dir_suffix= ac_dots=
  fi

  case "$ac_given_srcdir" in
  .)  srcdir=.
      if test -z "$ac_dots"; then top_srcdir=.
      else top_srcdir=`echo $ac_dots|sed 's%/$%%'`; fi ;;
  /*) srcdir="$ac_given_srcdir$ac_dir_suffix"; top_srcdir="$ac_given_srcdir" ;;
  *) # Relative path.
    srcdir="$ac_dots$ac_given_srcdir$ac_dir_suffix"
    top_srcdir="$ac_dots$ac_given_srcdir" ;;
  esac

  case "$ac_given_INSTALL" in
  [/$]*) INSTALL="$ac_given_INSTALL" ;;
  *) INSTALL="$ac_dots$ac_given_INSTALL" ;;
  esac

  echo creating "$ac_file"
  rm -f "$ac_file"
  configure_input="Generated automatically from `echo $ac_file_in|sed 's%.*/%%'` by configure."
  case "$ac_file" in
  *Makefile*) ac_comsub="1i\\
# $configure_input" ;;
  *) ac_comsub= ;;
  esac

  ac_file_inputs=`echo $ac_file_in|sed -e "s%^%$ac_given_srcdir/%" -e "s%:% $ac_given_srcdir/%g"`
  sed -e "$ac_comsub
s%@configure_input@%$configure_input%g
s%@srcdir@%$srcdir%g
s%@top_srcdir@%$top_srcdir%g
s%@INSTALL@%$INSTALL%g
" $ac_file_inputs | (eval "$ac_sed_cmds") > $ac_file
fi; done
rm -f conftest.s*

# These sed commands are passed to sed as "A NAME B NAME C VALUE D", where
# NAME is the cpp macro being defined and VALUE is the value it is being given.
#
# ac_d sets the value in "#define NAME VALUE" lines.
ac_dA='s%^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)'
ac_dB='\([ 	][ 	]*\)[^ 	]*%\1#\2'
ac_dC='\3'
ac_dD='%g'
# ac_u turns "#undef NAME" with trailing blanks into "#define NAME VALUE".
ac_uA='s%^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)'
ac_uB='\([ 	]\)%\1#\2define\3'
ac_uC=' '
ac_uD='\4%g'
# ac_e turns "#undef NAME" without trailing blanks into "#define NAME VALUE".
ac_eA='s%^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)'
ac_eB='$%\1#\2define\3'
ac_eC=' '
ac_eD='%g'

if test "${CONFIG_HEADERS+set}" != set; then
EOF
cat >> $CONFIG_STATUS <<EOF
  CONFIG_HEADERS="config.h"
EOF
cat >> $CONFIG_STATUS <<\EOF
fi
for ac_file in .. $CONFIG_HEADERS; do if test "x$ac_file" != x..; then
  # Support "outfile[:infile[:infile...]]", defaulting infile="outfile.in".
  case "$ac_file" in
  *:*) ac_file_in=`echo "$ac_file"|sed 's%[^:]*:%%'`
       ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;
  *) ac_file_in="${ac_file}.in" ;;
  esac

  echo creating $ac_file

  rm -f conftest.frag conftest.in conftest.out
  ac_file_inputs=`echo $ac_file_in|sed -e "s%^%$ac_given_srcdir/%" -e "s%:% $ac_given_srcdir/%g"`
  cat $ac_file_inputs > conftest.in

EOF

# Transform confdefs.h into a sed script conftest.vals that substitutes
# the proper values into config.h.in to produce config.h.  And first:
# Protect against being on the right side of a sed subst in config.status.
# Protect against being in an unquoted here document in config.status.
rm -f conftest.vals
cat > conftest.hdr <<\EOF
s/[\\&%]/\\&/g
s%[\\$`]%\\&%g
s%#define \([A-Za-z_][A-Za-z0-9_]*\) *\(.*\)%${ac_dA}\1${ac_dB}\1${ac_dC}\2${ac_dD}%gp
s%ac_d%ac_u%gp
s%ac_u%ac_e%gp
EOF
sed -n -f conftest.hdr confdefs.h > conftest.vals
rm -f conftest.hdr

# This sed command replaces #undef with comments.  This is necessary, for
# example, in the case of _POSIX_SOURCE, which is predefined and required
# on some systems where configure will not decide to define it.
cat >> conftest.vals <<\EOF
s%^[ 	]*#[ 	]*undef[ 	][ 	]*[a-zA-Z_][a-zA-Z_0-9]*%/* & */%
EOF

# Break up conftest.vals because some shells have a limit on
# the size of here documents, and old seds have small limits too.

rm -f conftest.tail
while :
do
  ac_lines=`grep -c . conftest.vals`
  # grep -c gives empty output for an empty file on some AIX systems.
  if test -z "$ac_lines" || test "$ac_lines" -eq 0; then break; fi
  # Write a limited-size here document to conftest.frag.
  echo '  cat > conftest.frag <<CEOF' >> $CONFIG_STATUS
  sed ${ac_max_here_lines}q conftest.vals >> $CONFIG_STATUS
  echo 'CEOF
  sed -f conftest.frag conftest.in > conftest.out
  rm -f conftest.in
  mv conftest.out conftest.in
' >> $CONFIG_STATUS
  sed 1,${ac_max_here_lines}d conftest.vals > conftest.tail
  rm -f conftest.vals
  mv conftest.tail conftest.vals
done
rm -f conftest.vals

cat >> $CONFIG_STATUS <<\EOF
  rm -f conftest.frag conftest.h
  echo "/* $ac_file.  Generated automatically by configure.  */" > conftest.h
  cat conftest.in >> conftest.h
  rm -f conftest.in
  if cmp -s $ac_file conftest.h 2>/dev/null; then
    echo "$ac_file is unchanged"
    rm -f conftest.h
  else
    # Remove last slash and all that follows it.  Not all systems have dirname.
      ac_dir=`echo $ac_file|sed 's%/[^/][^/]*$%%'`
      if test "$ac_dir" != "$ac_file" && test "$ac_dir" != .; then
      # The file is in a subdirectory.
      test ! -d "$ac_dir" && mkdir "$ac_dir"
    fi
    rm -f $ac_file
    mv conftest.h $ac_file
  fi
fi; done

EOF
cat >> $CONFIG_STATUS <<EOF

EOF
cat >> $CONFIG_STATUS <<\EOF

exit 0
EOF
chmod +x $CONFIG_STATUS
rm -fr confdefs* $ac_clean_files
test "$no_create" = yes || ${CONFIG_SHELL-/bin/sh} $CONFIG_STATUS || exit 1

