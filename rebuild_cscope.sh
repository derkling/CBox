#!/bin/sh

find ./src -iname '*.c' -o -iname '*.cpp' -o -iname '*.h' -o -iname '*.ih' > cscope.files
cscope -b -q -k

