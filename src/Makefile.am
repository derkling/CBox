# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

@AXIS_CONFIG@

SUBDIRS = base devices

INCLUDES = -I@top_srcdir@

noinst_LTLIBRARIES		= libcontrolbox.la
libcontrolbox_la_SOURCES	=
libcontrolbox_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@
libcontrolbox_la_LIBADD 	= base/libbase.la devices/libdevices.la \
					@CCGNU2_LIBS@

bin_PROGRAMS 		= cboxtest cbox

cboxtest_SOURCES	= cboxtest.cpp
cboxtest_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @CCEXT2_CFLAGS@
cboxtest_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @CCEXT2_LIBS@
cboxtest_LDADD		= libcontrolbox.la

cbox_SOURCES	= cbox.cpp
cbox_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @CCEXT2_CFLAGS@
cbox_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @CCEXT2_LIBS@
cbox_LDADD	= libcontrolbox.la

