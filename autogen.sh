#! /bin/sh

set -x
mkdir ./config > /dev/null 2>&1

aclocal -I config
libtoolize --force --copy
autoheader
automake --add-missing --copy --gnu
autoconf
