#! /bin/sh

set -x
mkdir ./config > /dev/null 2>&1

if [ ! -f controlbox ]; then
	ln -s src controlbox
fi

aclocal -I config
libtoolize --force --copy
autoheader
automake --add-missing --copy --gnu
autoconf
