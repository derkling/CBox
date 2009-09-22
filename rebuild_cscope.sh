#!/bin/sh

cd src
find . -iname '*.c' -o -iname '*.cpp' -o -iname '*.h' -o -iname '*.ih' > cscope.files
cscope -b -q -k
cd -
