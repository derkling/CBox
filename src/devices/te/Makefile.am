# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

# AXIS_USABLE_LIBS = GLIBC
# -include $(AXIS_TOP_DIR)/tools/build/Rules.axis

@AXIS_CONFIG@

INCLUDES	= -I@top_srcdir@

SOURCES = DeviceTE.h DeviceTE.ih DeviceTE.cpp
SOURCES+= Sampi500.h Sampi500.ih Sampi500.cpp
SOURCES+= Sampi550.h Sampi550.ih Sampi550.cpp
SOURCES+= VegaII.h VegaII.ih VegaII.cpp
SOURCES+= Isoil.h Isoil.ih Isoil.cpp
SOURCES+= Somefi.h Somefi.ih Somefi.cpp

noinst_LTLIBRARIES	= libte.la
libte_la_SOURCES	= $(SOURCES)
libte_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @CCEXT2_CFLAGS@ @LOG4CPP_CFLAGS@
libte_la_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @CCEXT2_LIBS@ @LOG4CPP_LIBS@
