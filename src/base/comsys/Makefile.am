# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

# AXIS_USABLE_LIBS = GLIBC
# -include $(AXIS_TOP_DIR)/tools/build/Rules.axis

@AXIS_CONFIG@

INCLUDES = -I@top_srcdir@

SOURCES = Command.h Command.ih Command.cpp
SOURCES+= Handler.h
SOURCES+= EventHandler.h EventHandler.ih EventHandler.cpp
SOURCES+= CommandHandler.h CommandHandler.ih CommandHandler.cpp
SOURCES+= Dispatcher.h
SOURCES+= MultipleDispatcher.h MultipleDispatcher.ih MultipleDispatcher.cpp
SOURCES+= EventDispatcher.h EventDispatcher.ih EventDispatcher.cpp
SOURCES+= CommandDispatcher.h CommandDispatcher.ih CommandDispatcher.cpp
SOURCES+= Generator.h
SOURCES+= EventGenerator.h EventGenerator.ih EventGenerator.cpp
SOURCES+= CommandGenerator.h CommandGenerator.ih CommandGenerator.cpp

noinst_LTLIBRARIES	= libcomsys.la
libcomsys_la_SOURCES	= $(SOURCES)
libcomsys_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @LOG4CPP_CFLAGS@
libcomsys_la_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @LOG4CPP_LIBS@
