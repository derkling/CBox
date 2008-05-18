# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

# AXIS_USABLE_LIBS = GLIBC
# -include $(AXIS_TOP_DIR)/tools/build/Rules.axis

@AXIS_CONFIG@

INCLUDES = -I@top_srcdir@

SOURCES = DeviceArdu.h DeviceArdu.ih DeviceArdu.cpp

noinst_LTLIBRARIES	= libardu.la
libardu_la_SOURCES	= $(SOURCES)
libardu_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @CCEXT2_CFLAGS@
libardu_la_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @CCEXT2_LIBS@
