# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

# AXIS_USABLE_LIBS = GLIBC
# -include $(AXIS_TOP_DIR)/tools/build/Rules.axis

@AXIS_CONFIG@

SUBDIRS = comsys

INCLUDES = -I@top_srcdir@

SOURCES = Configurator.h Configurator.ih Configurator.cpp
SOURCES+= Device.h Device.ih Device.cpp
SOURCES+= DeviceDB.h DeviceDB.ih DeviceDB.cpp
SOURCES+= ThreadDB.h ThreadDB.ih ThreadDB.cpp
SOURCES+= Object.h Object.ih Object.cpp
SOURCES+= Querible.h Querible.ih Querible.cpp
SOURCES+= QueryRegistry.h QueryRegistry.ih QueryRegistry.cpp
SOURCES+= Utility.h Utility.ih Utility.cpp
SOURCES+= Exception.h Exception.ih Exception.cpp
SOURCES+= base64.h base64.c

noinst_LTLIBRARIES	= libbase.la
libbase_la_SOURCES	= $(SOURCES)
libbase_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @LOG4CPP_CFLAGS@ @LOG4CPP_CFLAGS@
libbase_la_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @LOG4CPP_LIBS@ @LOG4CPP_CFLAGS@
libbase_la_LIBADD	= comsys/libcomsys.la
