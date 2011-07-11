#!/bin/sh

aclocal
libtoolize
automake --add-missing --foreign --copy
autoconf
