#
# Copyright (c) 2003 Bernd Walter Computer Technology
# All rights reserved.
#
# $URL$
# $Date$
# $Author$
# $Rev$
#
# Configure paths for LBWCTMB take from GTK--
# Erik Andersen	30 May 1998
# Modified by Tero Pulkkinen (added the compiler checks... I hope they work..)
# Modified by Thomas Langen 16 Jan 2000 (corrected CXXFLAGS)

dnl Test for LBWCTMB, and define LBWCTMB_CFLAGS and LBWCTMB_LIBS
dnl   to be used as follows:
dnl AM_PATH_LBWCTMB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_LBWCTMB,
[dnl 
dnl Get the cflags and libraries from the libbwctmb-config script
dnl
AC_ARG_WITH(libbwctmb-prefix,[  --with-libbwctmb-prefix=PREFIX
                          Prefix where libbwctmb is installed (optional)],
            libbwctmb_config_prefix="$withval", libbwctmb_config_prefix="")
AC_ARG_WITH(libbwctmb-exec-prefix,[  --with-libbwctmb-exec-prefix=PREFIX
                          Exec prefix where libbwctmb is installed (optional)],
            libbwctmb_config_exec_prefix="$withval", libbwctmb_config_exec_prefix="")
AC_ARG_ENABLE(libbwctmbtest, [  --disable-libbwctmbtest     Do not try to compile and run a test libbwctmb program],
		    , enable_libbwctmbtest=yes)

  if test x$libbwctmb_config_exec_prefix != x ; then
     libbwctmb_config_args="$libbwctmb_config_args --exec-prefix=$libbwctmb_config_exec_prefix"
     if test x${LBWCTMB_CONFIG+set} != xset ; then
        LBWCTMB_CONFIG=$libbwctmb_config_exec_prefix/bin/libbwctmb-config
     fi
  fi
  if test x$libbwctmb_config_prefix != x ; then
     libbwctmb_config_args="$libbwctmb_config_args --prefix=$libbwctmb_config_prefix"
     if test x${LBWCTMB_CONFIG+set} != xset ; then
        LBWCTMB_CONFIG=$libbwctmb_config_prefix/bin/libbwctmb-config
     fi
  fi

  AC_PATH_PROG(LBWCTMB_CONFIG, libbwctmb-config, no)
  min_libbwctmb_version=ifelse([$1], ,0.1,$1)

  AC_MSG_CHECKING(for libbwctmb - version >= $min_libbwctmb_version)
  AC_LANG_SAVE
  no_libbwctmb=""
  if test "$LBWCTMB_CONFIG" = "no" ; then
    no_libbwctmb=yes
  else
    AC_LANG_CPLUSPLUS

    LBWCTMB_CFLAGS=`$LBWCTMB_CONFIG $libbwctmb_config_args --cflags`
    LBWCTMB_LIBS=`$LBWCTMB_CONFIG $libbwctmb_config_args --libs`
    libbwctmb_config_major_version=`$LBWCTMB_CONFIG $libbwctmb_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libbwctmb_config_minor_version=`$LBWCTMB_CONFIG $libbwctmb_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  fi
  if test "x$no_libbwctmb" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$LBWCTMB_CONFIG" = "no" ; then
       echo "*** The libbwctmb-config script installed by libbwctmb could not be found"
       echo "*** If libbwctmb was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LBWCTMB_CONFIG environment variable to the"
       echo "*** full path to libbwctmb-config."
     fi
     LBWCTMB_CFLAGS=""
     LBWCTMB_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_LANG_RESTORE
  AC_SUBST(LBWCTMB_CFLAGS)
  AC_SUBST(LBWCTMB_LIBS)
])

