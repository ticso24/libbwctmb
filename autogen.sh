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

aclocal -I aclocal && \
autoheader && \
automake --foreign --add-missing --copy -f && \
autoconf
