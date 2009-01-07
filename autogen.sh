#!/bin/sh -x
#
# Copyright (c) 2001,04 Bernd Walter Computer Technology
# All rights reserved.
#
# $URL$
# $Date$
# $Author$
# $Rev$
#

aclocal-1.9 -I aclocal && \
autoheader-2.62 && \
automake-1.9 --foreign --add-missing --copy -f && \
autoconf-2.62
