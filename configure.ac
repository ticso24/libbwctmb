#
# Copyright (c) 2001,02.04 Bernd Walter Computer Technology
# All rights reserved.
#
# $URL$
# $Date$
# $Author$
# $Rev$
#

dnl Process this file with autoconf to produce a configure script.

AC_INIT([libbwctmb],[0.6],[bernd@bwct.de])
AC_CONFIG_SRCDIR(bwctmb/bwctmb.h)
AM_INIT_AUTOMAKE(libbwctmb, 0.6)
AM_CONFIG_HEADER(bwctmb/config.h)

AC_AIX

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

AC_LANG([C++])

AM_PATH_LBWCT(0.9,,[
	echo "no"
])
CXXFLAGS="$CXXFLAGS $LBWCT_CFLAGS"
XCXXFLAGS="$CXXFLAGS $LBWCT_CFLAGS"
LIBS="$LIBS $LBWCT_LIBS"

AC_SUBST(XLDFLAGS)
AC_SUBST(XCXXFLAGS)
AC_SUBST(LIBS)

AC_OUTPUT([Makefile \
	libbwctmb-config
])

chmod +x libbwctmb-config

