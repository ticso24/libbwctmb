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

aclocal19 -I aclocal && \
autoheader259 && \
automake19 --foreign --add-missing --copy -f && \
autoconf259
