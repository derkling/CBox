#!/bin/sh
arm-angstrom-linux-gnueabi-strip src/.libs/cbox*
scp src/.libs/cboxtest root@192.168.1.202:/tmp
