# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

# AXIS_USABLE_LIBS = GLIBC
# -include $(AXIS_TOP_DIR)/tools/build/Rules.axis

@AXIS_CONFIG@

INCLUDES = -I@top_srcdir@

SOURCES = DeviceGPRS.h DeviceGPRS.ih DeviceGPRS.cpp
SOURCES+= EnforaAPI.h EnforaAPI.ih EnforaAPI.cpp
# SOURCES+= SonyEricsson.h SonyEricsson.ih SonyEricsson.cpp
# SOURCES+= Enfora.h Enfora.ih Enfora.cpp

noinst_LTLIBRARIES	= libgprs.la
libgprs_la_SOURCES	= $(SOURCES)
libgprs_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @CCEXT2_CFLAGS@ @LOG4CPP_CFLAGS@
libgprs_la_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @CCEXT2_LIBS@ @LOG4CPP_CFLAGS@
