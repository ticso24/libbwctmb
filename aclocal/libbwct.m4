#
# Copyright (c) 2003 Bernd Walter Computer Technology
# All rights reserved.
#
# $URL$
# $Date$
# $Author$
# $Rev$
#
# Configure paths for LBWCT take from GTK--
# Erik Andersen	30 May 1998
# Modified by Tero Pulkkinen (added the compiler checks... I hope they work..)
# Modified by Thomas Langen 16 Jan 2000 (corrected CXXFLAGS)

dnl Test for LBWCT, and define LBWCT_CFLAGS and LBWCT_LIBS
dnl   to be used as follows:
dnl AM_PATH_LBWCT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_LBWCT,
[dnl 
dnl Get the cflags and libraries from the libbwct-config script
dnl
AC_ARG_WITH(libbwct-prefix,[  --with-libbwct-prefix=PREFIX
                          Prefix where libbwct is installed (optional)],
            libbwct_config_prefix="$withval", libbwct_config_prefix="")
AC_ARG_WITH(libbwct-exec-prefix,[  --with-libbwct-exec-prefix=PREFIX
                          Exec prefix where libbwct is installed (optional)],
            libbwct_config_exec_prefix="$withval", libbwct_config_exec_prefix="")
AC_ARG_ENABLE(libbwcttest, [  --disable-libbwcttest     Do not try to compile and run a test libbwct program],
		    , enable_libbwcttest=yes)

  if test x$libbwct_config_exec_prefix != x ; then
     libbwct_config_args="$libbwct_config_args --exec-prefix=$libbwct_config_exec_prefix"
     if test x${LBWCT_CONFIG+set} != xset ; then
        LBWCT_CONFIG=$libbwct_config_exec_prefix/bin/libbwct-config
     fi
  fi
  if test x$libbwct_config_prefix != x ; then
     libbwct_config_args="$libbwct_config_args --prefix=$libbwct_config_prefix"
     if test x${LBWCT_CONFIG+set} != xset ; then
        LBWCT_CONFIG=$libbwct_config_prefix/bin/libbwct-config
     fi
  fi

  AC_PATH_PROG(LBWCT_CONFIG, libbwct-config, no)
  min_libbwct_version=ifelse([$1], ,0.7,$1)

  AC_MSG_CHECKING(for libbwct - version >= $min_libbwct_version)
  AC_LANG_SAVE
  no_libbwct=""
  if test "$LBWCT_CONFIG" = "no" ; then
    no_libbwct=yes
  else
    AC_LANG_CPLUSPLUS

    LBWCT_CFLAGS=`$LBWCT_CONFIG $libbwct_config_args --cflags`
    LBWCT_LIBS=`$LBWCT_CONFIG $libbwct_config_args --libs`
    libbwct_config_major_version=`$LBWCT_CONFIG $libbwct_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libbwct_config_minor_version=`$LBWCT_CONFIG $libbwct_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  fi
  if test "x$no_libbwct" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$LBWCT_CONFIG" = "no" ; then
       echo "*** The libbwct-config script installed by libbwct could not be found"
       echo "*** If libbwct was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LBWCT_CONFIG environment variable to the"
       echo "*** full path to libbwct-config."
     fi
     LBWCT_CFLAGS=""
     LBWCT_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_LANG_RESTORE
  AC_SUBST(LBWCT_CFLAGS)
  AC_SUBST(LBWCT_LIBS)
])

