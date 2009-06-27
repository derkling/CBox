# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

@AXIS_CONFIG@

## This is the URL for the WebService retrivial
# Formato dati 4.2
#WSDLURL	= http://130.251.5.85/WSTELECONTROLLO/Concentratore.asmx?WSDL
# Formato dati 4.3
WSDLURL		= http://130.251.5.85/WSTELECONTROLLO/Concentratore43.asmx?WSDL
## The binary 'wsdl2h' complete path
#WSDL2H		= @SOAPCPP_COMPILER@/bin/wsdl2h
WSDL2H		= ./wsdl2h
## The binary 'soapcpp2' complete path
#SOAP		= @SOAPCPP_COMPILER@/bin/soapcpp2
SOAP		= ./soapcpp2
## The gSOAP import folder path
#SOAP_FLAGS	= -L -C -I$(GSOAP_SDK)/import
## The soapcpp2 compiler options:
# -L      don't generate soapClientLib/soapServerLib
# -C      generate client-side code only
SOAP_FLAGS	= -2 -L -C

INCLUDES	= -I@top_srcdir@ 

noinst_LTLIBRARIES	= libgsoapruntime.la libwsproxy.la
noinst_HEADERS 		= concentratore.h stdsoap2.h stlvector.h

## Building gSOAP WebService Runtime Library
## This library is needed in order to share gSOAP runtime Code between
## WSProxy and (eventually) WebService
libgsoapruntime_la_SOURCES	= stdsoap2.cpp
libgsoapruntime_la_CXXFLAGS	= @CONTROLBOX_CFLAGS@
#libgsoapruntime_la_CXXFLAGS	= @CONTROLBOX_CFLAGS@ @GSOAP_CFLAGS@
#libgsoapruntime_la_LDFLAGS	= @GSOAP_LIBS@

## Building gSOAP WebService Proxy library
libwsproxy_la_SOURCES 	= soapH.h soapC.cpp \
				soapStub.h soapClient.cpp \
				soapConcentratoreSoapProxy.h \
				WSProxyCommandHandler.h WSProxyCommandHandler.ih WSProxyCommandHandler.cpp \
				EndPoint.h EndPoint.ih EndPoint.cpp \
				FileEndPoint.h FileEndPoint.ih FileEndPoint.cpp \
				DistEndPoint.h DistEndPoint.ih DistEndPoint.cpp
libwsproxy_la_CXXFLAGS	= $(CONTROLBOX_CFLAGS) @CCGNU2_CFLAGS@ @CCEXT2_CFLAGS@ @LOG4CPP_CFLAGS@
libwsproxy_la_LDFLAGS	= $(CONTROLBOX_LDFLAGS) @CCGNU2_LIBS@ @CCEXT2_CFLAGS@ @LOG4CPP_CFLAGS@
libwsproxy_la_LIBADD	= libgsoapruntime.la

########################################################################
## Makefile rules for building the C++ Proxy library and Server
########################################################################

## From the WSDL produce the header file required by the gSOAP
##  stub and skeleton compiler
concentratore.h :
	$(WSDL2H) -o concentratore.h $(WSDLURL)

## The Proxy lirary sources
SOAP_CPP_SRC = soapH.h soapC.cpp soapStub.h soapClient.cpp soapServer.cpp \
		ConcentratoreSoap.nsmap ConcentratoreSoap.wsdl \
		soapConcentratoreSoapProxy.h

## Proxy library gSOAP required files
$(SOAP_CPP_SRC) : concentratore.h
	$(SOAP) $(SOAP_FLAGS) $<

## CleanUp rule
CLEANFILES = $(SOAP_CPP_SRC) *.o *.req.xml *.res.xml *.wsdl *.nsmap *.xsd *Proxy\.h *~

########################################################################

