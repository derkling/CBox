# Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([src/cboxtest.cpp])

# Cross-compilation options
AC_CANONICAL_SYSTEM

# Automake configuration
AM_INIT_AUTOMAKE(controlbox, 0.1)
AM_CONFIG_HEADER(src/config.h)

# Language configuration
AC_LANG_CPLUSPLUS
AC_DISABLE_STATIC

# Platform-specific Makefile setup
EXTRA_LIBS=
case "${host}" in
        *-*-solaris*)   platform=SUN_OS
			EXTRA_LIBS="-lxnet -lsocket -lnsl"
			;;
        *-*-sysv5*)     platform=UNIXWARE ;;
        *-*-UnixWare*)  platform=UNIXWARE ;;
        *-*-unixware*)  platform=UNIXWARE ;;
	*-*-sco3.2v5*)  platform=OPENSERVER
	                EXTRA_LIBS="-lsocket"
			;;
        *-*-linux*)     platform=LINUX
			;;
	*-*-cygwin*)	platform=CYGWIN
			;;
        *-*-freebsd*)   platform=FREEBSD ;;
        *-*-irix*)      platform=IRIX ;;
        *-*-aix*)       platform=AIX
                      case "${host}" in
                         *-*-aix4.1*)   osver=AIX41;;
                         *-*-aix4.2*)   osver=AIX42;;
                         *-*-aix4.3*)   osver=AIX43;;
                         *-*-aix4.*)    osver=AIX43;;
                      esac;;
        *-sequent-*)    platform=PTX ;;
        *-*-hp*)        platform=HP_UX ;
                        case "${host}" in
                            *-*-hpux11*)  osver=HPUX11;;
                            *-*-hpux10*)  osver=HPUX10;;
                            *)            osver=HPUX11;;
                        esac;;
        *-*-mvs*)       platform=OS390 ;;
        *-*-os400*)     platform=OS400 ;;
        *-*-OS400*)     platform=OS400 ;;
        *-*-osf*)	platform=TRU64 ;;
        *-apple-*)	platform=MACOSX ;;
	*-nto-qnx)	platform=QNX
			EXTRA_LIBS="-lsocket"
			;;
        *)              platform=UNKNOWN ;;
esac
AC_SUBST(EXTRA_LIBS)
AC_SUBST(platform)

#-----[ Check for Standard Libraries Presence ]---------------------------------

# Checks for programs
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_INSTALL

# Checks for standard typedefs, structures, and compiler characteristics
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for standard libraries
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([strchr strdup strstr])

# Checks for standard header files
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h thread.h])

# Checks for standard library functions
AC_FUNC_MKTIME
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([sscanf])

#-----[ Check for User Options ]------------------------------------------------

# the debug build options adds symbols to compiler output (-g for g++)
AC_ARG_ENABLE(release,
	AC_HELP_STRING([--enable-release], [whetever to build a release version (default no)]),
		[release=true],
		[release=false])
AM_CONDITIONAL(CONTROLBOX_DEBUG, test x$release = xfalse)
if test x$release = xfalse; then
	AC_DEFINE([CONTROLBOX_DEBUG], [2], [Enable debug code])
	CONTROLBOX_CFLAGS="-g -O0"
else
	CONTROLBOX_CFLAGS="-Os"
fi

# Test using LAN connection instead of GPRS
AC_ARG_ENABLE(uselan,
	AC_HELP_STRING([--enable-uselan], [whetever to use LAN connection (default not)]),
		[uselan=true],
		[uselan=false])
AM_CONDITIONAL(USE_LAN, test x$uselan = xtrue)
if test x$uselan = xtrue; then
	AC_DEFINE([CONTROLBOX_USELAN], [], [Use LAN network instead of GPRS])
fi

# cross compile for the CRIS-AXIS platform
#AC_ARG_ENABLE(cris,
#	AC_HELP_STRING([--enable-cris],
#		[whetever to crosscompile for the cris platform]),
#		[cris=true],
#		[cris=false])
#AM_CONDITIONAL(CRIS_COMPILE, test x$cris = xtrue)

if test x$host = xcris-axis-linux-gnu; then
	cris="true"
	AC_DEFINE([CONTROLBOX_CRIS], [], [Enable CRIS specific code])
else
	cris="false"
fi
AM_CONDITIONAL(CRIS_COMPILE, test x$cris = xtrue)

#-----[ Check for Specific Libraries Presence ]---------------------------------

AC_PATH_PROG(PKG_CONFIG, pkg-config)
AC_MSG_NOTICE([Using pkgconfig files into: $PKG_CONFIG_PATH])

# Check for log4cpp
PKG_CHECK_MODULES(LOG4CPP, log4cpp >= 0.3)

# Check for ccgnu2
PKG_CHECK_MODULES(CCGNU2, libccgnu2 >= 1.3)

# Check for ccext2
PKG_CHECK_MODULES(CCEXT2, libccext2 >= 1.3)

## Check for gsoap
#PKG_CHECK_MODULES(GSOAP, gsoap++ >= 2.7)



#-----[ Handling CrossCompilation Options ]-------------------------------------
#SOAPCPP_COMPILER=`pkg-config --variable=exec_prefix gsoap++`
if test x$cris = xfalse; then
	AXIS_CONFIG=
	GSOAP_SDK=
	CONTROLBOX_CFLAGS=
	CONTROLBOX_LDFLAGS=
else
	AXIS_CONFIG="include \$(AXIS_TOP_DIR)/tools/build/Rules.axis"
	GSOAP_SDK=
	CONTROLBOX_CFLAGS="-I\$(AXIS_KERNEL_DIR)/include"
	CONTROLBOX_LDFLAGS=
fi


#-----[ Applying configuration ]------------------------------------------------
AC_SUBST(CONTROLBOX_CFLAGS)
AC_SUBST(CONTROLBOX_LDFLAGS)

AC_SUBST(LOG4CPP_CFLAGS)
AC_SUBST(LOG4CPP_LIBS)

AC_SUBST(CCGNU2_CFLAGS)
AC_SUBST(CCGNU2_LIBS)

AC_SUBST(CCEXT2_CFLAGS)
AC_SUBST(CCEXT2_LIBS)

#AC_SUBST(GSOAP_CFLAGS)
#AC_SUBST(GSOAP_LIBS)
#AC_SUBST(GSOAP_SDK)
#AC_SUBST(SOAPCPP_COMPILER)

AC_SUBST(AXIS_CONFIG)


AC_OUTPUT(Makefile \
 src/Makefile \
 src/base/Makefile \
 src/base/comsys/Makefile \
 src/devices/Makefile \
 src/devices/te/Makefile \
 src/devices/gprs/Makefile \
 src/devices/atgps/Makefile \
 src/devices/arduino/Makefile \
 src/devices/wsproxy/Makefile \
)

AC_MSG_NOTICE([Using log4cpp configuration: CFLAGS="${LOG4CPP_CFLAGS}", LIBS="${LOG4CPP_LIBS}"])
AC_MSG_NOTICE([Using commoncpp2 configuration: CFLAGS="${CCGNU2_CFLAGS}", LIBS="${CCGNU2_LIBS}"])
#AC_MSG_NOTICE([Using gSOAP configuration: CFLAGS="${GSOAP_CFLAGS}", LIBS="${GSOAP_LIBS}"])
#AC_MSG_NOTICE([Using gSOAP compiler located on: $SOAPCPP_COMPILER]/bin)
if test $cris = "true"; then
	AC_MSG_NOTICE([System configured for cross-compilation with target: $host])
	AC_MSG_NOTICE([Running 'make cris-axis-linux-gnu' to enable cross-compilation... ])
	make cris-axis-linux-gnu
fi
