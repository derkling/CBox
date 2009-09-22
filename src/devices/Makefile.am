# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

# AXIS_USABLE_LIBS = GLIBC
# -include $(AXIS_TOP_DIR)/tools/build/Rules.axis

@AXIS_CONFIG@

#SUBDIRS = arduino te gprs wsproxy
SUBDIRS = atgps te gprs wsproxy

INCLUDES = -I@top_srcdir@

#prefix = /usr/local/axis/devboard-R2_01/os/linux-2.6

SOURCES = DeviceFactory.h DeviceFactory.ih DeviceFactory.cpp
SOURCES+= DeviceGPIO.h DeviceGPIO.ih DeviceGPIO.cpp
SOURCES+= DeviceSignals.h DeviceSignals.ih DeviceSignals.cpp
if CRIS_COMPILE
SOURCES+= DeviceGPIOAxis.h DeviceGPIOAxis.ih DeviceGPIOAxis.cpp
SOURCES+= DeviceSignalsAxis.h DeviceSignalsAxis.ih DeviceSignaslAxis.cpp
endif
if ARM_COMPILE
SOURCES+= DeviceGPIOLib.h DeviceGPIOLib.ih DeviceGPIOLib.cpp
SOURCES+= DeviceSignalsLinuxEvt.h DeviceSignalsLinuxEvt.ih DeviceSignalsLinuxEvt.cpp
endif
SOURCES+= DeviceAnalogSensors.h DeviceAnalogSensors.ih DeviceAnalogSensors.cpp
SOURCES+= DeviceDigitalSensors.h DeviceDigitalSensors.ih DeviceDigitalSensors.cpp
SOURCES+= DeviceI2CBus.h DeviceI2CBus.ih DeviceI2CBus.cpp
SOURCES+= DeviceSerial.h DeviceSerial.ih DeviceSerial.cpp
# SOURCES+= DeviceOdometer.h DeviceOdometer.ih DeviceOdometer.cpp
# SOURCES+= DeviceGPS.h DeviceGPS.ih DeviceGPS.cpp
SOURCES+= DeviceInCabin.h DeviceInCabin.ih DeviceInCabin.cpp
SOURCES+= ATcontrol.h ATcontrol.ih ATcontrol.cpp
SOURCES+= DeviceTime.h DeviceTime.ih DeviceTime.cpp
SOURCES+= FileWriterCommandHandler.h FileWriterCommandHandler.ih FileWriterCommandHandler.cpp
SOURCES+= PollEventGenerator.h PollEventGenerator.ih PollEventGenerator.cpp

noinst_LTLIBRARIES	= libdevices.la
libdevices_la_SOURCES	= $(SOURCES)
libdevices_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @CCEXT2_CFLAGS@ @LOG4CPP_CFLAGS@ @SYSFS_CFLAGS@
libdevices_la_LDFLAGS 	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @CCEXT2_LIBS@ @LOG4CPP_LIBS@ @SYSFS_LIBS@
libdevices_la_LIBADD	= te/libte.la \
				atgps/libatgps.la \
				gprs/libgprs.la \
				wsproxy/libwsproxy.la
# 				arduino/libardu.la

