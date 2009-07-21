//******************************************************************************
//*************  Copyright (C) 2006 - Patrick Bellasi **************************
//******************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//******************************************************************************
//******************** Module information **************************************
//**
//** Project:       ControlBox (0.1)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
//**
//******************************************************************************
//******************** Revision history ****************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************

#ifndef _UTILITY_H
#define _UTILITY_H

#include <controlbox/config.h>
#include <controlbox/base/Exception.h>

// System call utilities
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

// Log4Cpp Logging Utilities
#include <log4cpp/Portability.hh>
#ifdef LOG4CPP_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#ifdef LOG4CPP_HAVE_SYSLOG
#include <log4cpp/SyslogAppender.hh>
#endif
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/NDC.hh>


/// Handler's services number request range
/// To each handler that expose services must be associated
/// a non overlapping range of service ID. This define the
/// range width of ID associated to each handler.
/// The effective range for each Handler is defined
/// tacking into account it's deviceCode and is defined as
/// follow:<br>
/// from: deviceCode*SERVICES_RANGE<br>
/// to: deviceCode*SERVICES_RANGE+SERVICES_RANGE<br>
/// <i>NOTE</i> the service IDs going from 0 to SERVICES_RANGE-1
/// are reserved for <i>broadcast</i> messages. <b>Broadcast messages</b>
/// are message not related to only a specific device but that each device
/// could decide if to accept ore ignore. Thos kind of messages are used i.e
/// to delivery the same message or /elaboration requesto to multiple
/// services providers.
/// @see deviceCode
#define SERVICES_RANGE 100

// #define C_WHITE(_msg_)  "\033[1;37m"##__msg__##"\033[0m"
// #define C_LGRAY(_msg_)  "\033[37m"##__msg__##"\033[0m"
// #define C_GRAY(_msg_)   "\033[1;30m"##__msg__##"\033[0m"
// #define C_BLACK(_msg_)  "\033[30m"##__msg__##"\033[0m"
// #define C_RED(_msg_)    "\033[31m"##__msg__##"\033[0m"
// #define C_LRED(_msg_)   "\033[1;31m"##__msg__##"\033[0m"
// #define C_GREEN(_msg_)  "\033[32m"##__msg__##"\033[0m"
// #define C_LGREEN(_msg_) "\033[1;32m"##__msg__##"\033[0m"
// #define C_BROWN(_msg_)  "\033[33m"##__msg__##"\033[0m"
// #define C_YELLOW(_msg_) "\033[1;33m"##__msg__##"\033[0m"
// #define C_BLUE(_msg_)   "\033[34m"##__msg__##"\033[0m"
// #define C_LBLUE(_msg_)  "\033[1;34m"##__msg__##"\033[0m"
// #define C_PURPLE(_msg_) "\033[35m"##__msg__##"\033[0m"
// #define C_PINK(_msg_)   "\033[1;35m"##__msg__##"\033[0m"
// #define C_CYAN(_msg_)   "\033[36m"##__msg__##"\033[0m"
// #define C_LCYAN(_msg_)  "\033[1;36m"##__msg__##"\033[0m"

extern bool useColors;


namespace controlbox {

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;
typedef char		s8;
typedef short		s16;
typedef long		s32;

enum exitCode_ {
    OK,
    GENERIC_ERROR,
    OUT_OF_MEMORY,
    BUFFER_OVERFLOW,
    CONVERSION_ERROR,
    CS_DISPATCH_FAILURE,
    DB_DEVICE_NOT_EXIST,
    DB_DEVICE_DUPLICATE,
    DB_REGISTRATION_FAILURE,
    QR_REGISTRY_NOT_FOUND,
    QR_MISSING_QUERIBLE,
    QR_QUERY_DUPLICATE,
    QR_QUERY_NOT_EXIST,
    HR_HANDLER_DUPLICATE,
    HR_HANDLER_NOT_PRESENT,
    HR_QUERYMODE_NOT_SUPPORTED,
    INT_DEV_FAILED,
    INT_LINE_UNK,
    INT_ALREADY_DEFINED,
    INT_WFI_FAILED,
    INT_SELECT_FAILED,
    INT_SIGNAL_ERROR,
    INT_NO_HANDLER,
    INT_ACBAT_SWITCH,
    I2C_DEV_FAILED,
    I2C_DEV_NOT_OPEN,
    I2C_DEV_ACCESS_ERROR,
    ODO_CONVERSION_ERROR,
    ATCONTROL_TTY_OPEN_FAILURE,
    ATCONTROL_REGISTRY_NOT_FOUND,
    GEN_NO_DISPATCHER,
    GEN_NOT_ENABLED,
    DIS_SUSPENDED,
    DIS_COMMAND_NOT_SUPPORTED,
    GEN_THREAD_STARTED,
    WS_EP_NOT_SUPPORTED,
    WS_REGISTRY_NOT_FOUND,
    WS_MISSING_COMMAND_PARAM,
    WS_TRYING_UPLOAD,
    WS_DATA_QUEUED,
    WS_UPLOAD_FAULT,
    WS_FORMAT_ERROR,
    WS_XML_PARSE_ERROR,
    WS_INVALID_DATA,
    WS_LINK_DOWN,
    WS_MEM_FAILURE,
    WS_LOCAL_COMMAND,
    WS_POLLER_UPDATE_NOT_NEEDED,
    GPS_CONFIGURATION_FAILURE,
    GPS_TTY_OPEN_FAILURE,
    GPIO_ATTR_OPEN_FAILURE,
    GPIO_ATTR_NODEV,
    GPIO_ATTR_READ_NOAUTH,
    GPIO_ATTR_READ_FAILURE,
    GPIO_ATTR_WRITE_NOAUTH,
    GPIO_ATTR_WRITE_FAILURE,
    GPRS_DEVICE_NOT_PRESENT,
    GPRS_DEVICE_POWER_ON_FAILURE,
    GPRS_DEVICE_POWER_OFF_FAILURE,
    GPRS_DEVICE_RESET_FAILURE,
    GPRS_NETLINK_UNDEFINED,
    GPRS_NETLINK_PARSE_ERROR,
    GPRS_NETLINK_NOT_SUPPORTED,
    GPRS_TTY_OPEN_FAILURE,
    GPRS_TTY_NOT_OPENED,
    GPRS_TTY_SWITCH_FAILED,
    GPRS_TTY_TIMEOUT,
    GPRS_TTY_NOT_ARRIVED,
    GPRS_TTY_MODEM_NOT_RESPONDING,
    GPRS_TTY_MODEM_CONFIGURATION_FAILED,
    GPRS_AT_NOT_SUPPORTED,
    GPRS_AT_RESPONCE_KO,
    GPRS_AT_ESCAPE_FAILED,
    GPRS_PARSER_NOT_RUNNING,
    GPRS_NETWORK_UNREGISTERED,
    GPRS_NETWORK_REGDENIED,
    GPRS_LINK_NOT_CONFIGURED,
    GPRS_LINK_DOWN,
    GPRS_PDPCTX_FAILED,
    GPRS_CONNECT_FAILURE,
    GPRS_PPPD_PIPE_FAILURE,
    GPRS_PPPD_FAILURE,
    GPRS_PPPD_LOGFILE_FAILURE,
    GPRS_PPPD_NOT_RUNNING,
    GPRS_PPPD_SUSPEND_FAILED,
    GPRS_PPPD_RESUME_FAILED,
    GPRS_PPPD_KILL_FAILED,
    GPRS_PPPD_UNKNOWED_SENTENCE,
    GPRS_SMS_SEND_FAILED,
    GPRS_SIGNAL_LEVEL_FAILED,
    GPRS_GPRS_STATUS_FAILED,
    GPRS_SOCKET_CREATION_FAILED,
    GPRS_SOCKET_BINDING_FAILED,
    GPRS_SOCKET_RESOLVING_FAILED,
    GPRS_API_SELECT_FAILED,
    GPRS_API_SELECT_TIMEOUT,
    GPRS_API_ATSEND_FAILED,
    GPRS_API_ATRECV_FAILED,
    GPRS_API_ATRECV_ERRORS,
    GPRS_API_GPRS_UP_FAILED,
    GPRS_API_GPRS_UP_PARAMS_FAILED,
    GPRS_API_PARSE_ERROR,
    GPRS_RESOLVER_CONFIGURE_FAILED,
    GPRS_ROUTING_CONFIGURE_FAILED,
    GPRS_API_GPRS_DOWN_FAILED,
    GPRS_RESET_REQUIRED,
    AS_I2C_OPEN_FAILED,
    AS_UNDEFINED_SENSOR,
    DS_PORT_READ_ERROR,
    DS_NO_NEW_EVENTS,
    DS_VALUE_CONVERSION_FAILED,
    DS_PORT_UPDATE_FAILED,
    ARDU_PORT_READ_ERROR,
    ARDU_VALUE_CONVERSION_FAILED,
    ARDU_PORT_UPDATE_FAILED,
    ARDU_EVENT_UNDEFINED,
    ARDU_EVENT_DISABLED,
    ATGPS_PORT_READ_ERROR,
    ATGPS_NO_NEW_EVENTS,
    ATGPS_CMD_UNDEF,
    ATGPS_VALUE_CONVERSION_FAILED,
    ATGPS_PORT_UPDATE_FAILED,
    ATGPS_EVENT_UNDEFINED,
    ATGPS_EVENT_DISABLED,
    TE_TTY_OPEN_FAILURE,
    TE_CONNECT_FAILURE,
    TE_NAK_RECEIVED,
    TE_NO_NEW_EVENTS,
    TE_NOT_RESPONDING,
    TE_RESTART_DOWNLOAD,
    TE_EVENT_NOT_PARSED,
    TE_CMD_NOT_SUPPORTED,
    TTY_OPEN_FAILURE,
    TTY_NOT_OPEN,
    TTY_NOT_RESPONDING,
};
typedef enum exitCode_ exitCode;

/// Define a unique identifier forma each deviceCode
/// A device is an entity that could provide some kind
/// of services. Each device must conform to an interface,
/// the deviceCode is so an identifier of services exported.
enum deviceCode_ {
    ALL_DEVICES = 0,	/// messages adressed to any sort of device
    UNDEF,			/// any specific device
    FILEWRITER,
    WSPROXY,
    DEVICE_TE,
    DEVICE_GPRS,
    DEVICE_GPS,
    DEVICE_AS,
    DEVICE_DS
};
typedef enum deviceCode_ deviceCode;

/// Define the services common to all devices
/// This values could be used as Command type
/// for command adressed to multiple services providers.
/// A service provider could ignore this kind of service
/// requests, or otherwise honor them and produce some kind
/// of side effect.<br>
/// The maximun number of common services is upper limited
/// by the value of the SERVICES_RANGE define.
/// Devices providing some kind of common services shuld
/// declare them and explicity define what they will do
/// into their documentation.
/// @see SERVICES_RANGE
/// @see deviceCode
enum commonService_ {
    APPEND_DATA = 0	/// Require to append all meaningful data as param of the current command
};
typedef enum commonService_ commonService;

class Utils {

public:
	static std::string strFormat(const char* stringFormat, ...);
	static exitCode b64enc(const char *in, size_t inlen, char *out, size_t outlen);
	static exitCode b64enc(const char *in, size_t inlen, char **out, size_t &outlen);
	static exitCode b64dec(const char *inbuf, char *outbuf, size_t & out_len);

};


/** @addtogroup LoggingMacros Logging macros
@{
*/

#if 1
#define MARKLINE if (1) {						\
	cout << "MARK: " << __FILE__ << ":" << __LINE__ << endl;	\
}
#else
#define LOGLINE if (0) {};
#endif

#if !defined(LOG4CPP_UNLIKELY)
#if __GNUC__ >= 3
/**
Provides optimization hint to the compiler
to optimize for the expression being false.
@param expr boolean expression.
@returns value of expression.
*/
#define LOG4CPP_UNLIKELY(expr) __builtin_expect(expr, 0)
#else
/**
Provides optimization hint to the compiler
to optimize for the expression being false.
@param expr boolean expression.
@returns value of expression.
**/
#define LOG4CPP_UNLIKELY(expr) expr
#endif
#endif


/**
Logs a method marker to a specified category with the DEBUG level.
NOTE: This macro will generata code only if compiling with debug simbol
	enabled; otherwise it will be converted simply to empty code.

@param category the category to be used.
@param message the message string to log.
*/
#if CONTROLBOX_DEBUG > 1
# define LOG4CPP_MARK(category) { \
			if (LOG4CPP_UNLIKELY(category.isDebugEnabled())) {\
				category.log(::log4cpp::Priority::DEBUG,  "MRK: %s,%d - %s", __FILE__, __LINE__, __FUNCTION__); \
			}\
		}
# define LOG4CPP_MARKP(category, format, ...) { \
			if (LOG4CPP_UNLIKELY(category.isDebugEnabled())) {\
				category.log(::log4cpp::Priority::DEBUG,  "MRK: %s,%d - %s" format "\n", __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); \
			}\
		}
#else
# define LOG4CPP_MARK(category)
# define LOG4CPP_MARKP(category, format, ...)
#endif



/**
Logs a message to a specified category with the DEBUG level.
NOTE: This macro will generata code only if compiling with debug simbol
	enabled; otherwise it will be converted simply to empty code.

@param category the category to be used.
@param message the message string to log.
*/
/*
#undef LOG4CPP_DEBUG
#ifdef CONTROLBOX_DEBUG
	#define LOG4CPP_DEBUG(category, format, ...) { \
			if (LOG4CPP_UNLIKELY(category.isDebugEnabled())) {\
				std::ostringstream formatter("%s:%d - "); \
				category.log(::log4cpp::Priority::DEBUG, "%25s:%05d - " format, __FILE__, __LINE__, ## __VA_ARGS__); \
			}\
	}
#else
	#define LOG4CPP_DEBUG(category, format, ...)
#endif
*/
#undef LOG4CPP_DEBUG
#ifdef CONTROLBOX_DEBUG
	#define LOG4CPP_DEBUG(category, format, ...) \
			if (category.isDebugEnabled()) {\
				std::ostringstream formatter("%s:%d - "); \
				category.log(::log4cpp::Priority::DEBUG, "%25s:%05d - " format, __FILE__, __LINE__, ## __VA_ARGS__); \
			}
#else
	#define LOG4CPP_DEBUG(category, format, ...)
#endif

/**
Logs a message to a specified category with the INFO level.

@param category the category to be used.
@param message the message string to log.
*/
#undef LOG4CPP_INFO
#define LOG4CPP_INFO(category, format, ...) \
			if (category.isInfoEnabled()) {\
				if (useColors) \
					category.log(::log4cpp::Priority::INFO, "\033[32m" format "\033[0m", ## __VA_ARGS__); \
				else \
					category.log(::log4cpp::Priority::INFO, format, ## __VA_ARGS__); \
			}

/**
Logs a message to a specified category with the WARN level.

@param category the category to be used.
@param message the message string to log.
*/
#undef LOG4CPP_WARN
#define LOG4CPP_WARN(category, format, ...) \
			if (category.isWarnEnabled()) {\
				if (useColors) \
					category.log(::log4cpp::Priority::WARN, "\033[33m" format "\033[0m", ## __VA_ARGS__); \
				else \
					category.log(::log4cpp::Priority::WARN, format, ## __VA_ARGS__); \
			}

/**
Logs a message to a specified category with the ERROR level.

@param category the category to be used.
@param message the message string to log.
*/
#undef LOG4CPP_ERROR
#define LOG4CPP_ERROR(category, format, ...) \
			if (category.isErrorEnabled()) {\
				if (useColors) \
					category.log(::log4cpp::Priority::ERROR, "\033[31m" format "\033[0m", ## __VA_ARGS__); \
				else \
					category.log(::log4cpp::Priority::ERROR, format, ## __VA_ARGS__); \
			}

/**
Logs a message to a specified category with the CRIT level.

@param category the category to be used.
@param message the message string to log.
*/
#undef LOG4CPP_CRIT
#define LOG4CPP_CRIT(category, format, ...) \
			if (category.isCritEnabled()) {\
				if (useColors) \
					category.log(::log4cpp::Priority::CRIT, "\033[1;31m" format "\033[0m", ## __VA_ARGS__); \
				else \
					category.log(::log4cpp::Priority::CRIT, format, ## __VA_ARGS__); \
			}


/**
Logs a message to a specified category with the FATAL level.

@param category the category to be used.
@param message the message string to log.
*/
#undef LOG4CPP_FATAL
#define LOG4CPP_FATAL(category, format, ...) \
			if (category.isFatalEnabled()) {\
				if (useColors) \
					category.log(::log4cpp::Priority::FATAL, "\033[1;31m" format "\033[0m", ## __VA_ARGS__); \
				else \
					category.log(::log4cpp::Priority::FATAL, format, ## __VA_ARGS__); \
			}

/**
Disabled debug statements
*/
#define DLOG4CPP_DEBUG(category, format, ...)
#define DLOG4CPP_INFO(category, format, ...)
#define DLOG4CPP_WARN(category, format, ...)
#define DLOG4CPP_ERROR(category, format, ...)
#define DLOG4CPP_CRIT(category, format, ...)
#define DLOG4CPP_FATAL(category, format, ...)

} // controlbox namespace

#endif
