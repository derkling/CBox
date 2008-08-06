//***************************************************************************************
//*************  Copyright (C) 2006 - Patrick Bellasi ***********************************
//***************************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//***************************************************************************************
//******************** Module information ***********************************************
//**
//** Project:       ControlBox (0.1)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
//**
//***************************************************************************************
//******************** Revision history *************************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------------------------- -----------------------------
//**
//**
//***************************************************************************************


#ifndef _COMMAND_H
#define _COMMAND_H


#include <controlbox/base/Utility.h>
#include <controlbox/base/Exception.h>
#include <controlbox/base/Device.h>
#include <map>

#include <sstream>

#define INSERT_PARAM(_labe_, _value_)							\
	do {										\
		d_params.insert (pair<std::string, t_cmdParam>(_lable_,_value_));	\
	} while (0);

namespace controlbox {
namespace comsys {

/// A Core Communication System Command Interface.
/// Core Communication System Commands are used to transport elaboration requests
/// and elaboration results among different system components.
/// Each Command as a type defining the service requested. Valid commands type are
/// defined as enum values by CommandHandlers.
class Command {

public:

    /// The Command Type.
    /// Meaningful values for that type are related to each device.
    typedef unsigned int t_cmdType;


protected:

    enum paramType {
        PT_INT = 0,
        PT_FLOAT,
        PT_STRING
    };
    typedef enum paramType t_paramType;

    union paramValue {
        long i;
        double d;
        std::string * s;
    };
    typedef union paramValue t_paramValue;

    struct cmdParam {
        t_paramType type;
        t_paramValue value;
    };
    typedef struct cmdParam t_cmdParam;

    /// Map param lables to value.
    typedef multimap<std::string, t_cmdParam> t_mapValue;


    /// The command type
    t_cmdType d_cmdType;

    /// The device class producing the message
    Device::t_deviceType d_devType;

    /// The device identifier within the device class
    Device::t_deviceId d_devId;

    /// Other (optional) command params.
    t_mapValue d_params;

    /// The command priority
    /// This priority could be used define the upload queue where the command
    /// will be inserted, and thus its upload priority
    /// @see WSProxyCommandHandler
    unsigned short d_prio;

    /// Logger
    /// Use this logger reference, related to the 'log' category, to log your messages
    log4cpp::Category & log;


public:

    /// Create a new Command for a defined device
    /// @param cmdType the command type, meaningful values for this param are usually
    ///		exported by Handlers with the enum type named services. Commands
    ///		objects usually doesn't associate a meaning to this value; the
    ///		meaning is knowed only by the receiving Handler.
    /// @param devType the "device class" to witch the Command is adressed, meaningful values are
    ///		defined by deviceCode.
    /// @param devId a device identifier within the device class defined by device.
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs.command."
    /// @see deviceCode
    /// @see Handler
    static Command * getCommand(t_cmdType const & cmdType, Device::t_deviceType devType, Device::t_deviceId devId, std::string const & logName = "Generic");

    ~Command() {};

    /// Set the device type identifier.
    /// @param devType the device type identifier
    inline exitCode setDevice(Device::t_deviceType const & devType);

    /// Set the device identifier
    /// @param devId the device identifier.
    inline exitCode setDeviceId(Device::t_deviceId const & devId);

    void setPrio(unsigned short prio);

    unsigned short getPrio(void) const;

    /// Set a string command parameter.
    /// Use this to associate a string parameter to the command
    /// @param lable the param name
    /// @param value the param string value
    /// @param override whatever override the current definition of this param if
    ///			already present (default true)
    exitCode setParam(std::string const & lable, std::string const & value, bool override = true);

    /// Set an int command parameter.
    /// Use this to associate a string parameter to the command
    /// @param lable the param name
    /// @param value the param int value
    /// @param override whatever override the current definition of this param if
    ///			already present (default true)
    exitCode setParam(std::string const & lable, int value, bool override = true);

    /// Set a float command parameter.
    /// Use this to associate a float parameter to the command
    /// @param lable the param name
    /// @param value the param float valueFLOAT
    /// @param override whatever override the current definition of this param if
    ///			already present (default true)
    exitCode setParam(std::string const & lable, double value, bool override = true);

    /// Return the command type.
    /// @return the command type
    Command::t_cmdType type() const;

    /// Return the device type.
    /// The device type identifier is null if not otherwise specified
    /// @return the device type type.
    Device::t_deviceType device() const;

    /// Return the device identifier.
    /// The device identifier is null if not otherwise specified
    /// @return the device id
    Device::t_deviceId deviceId() const;

    /// Return the number of command parameters.
    /// @return the number of params
    unsigned int paramCount() const;

    /// Return the number of command parameters having the lable specified.
    /// @param lable the lable's param to count
    /// @return the number of params with lable specified
    unsigned int paramCount(std::string const & lable) const;

    /// Return the required command parameter.
    /// The param whose lable is specified is returned. If the param is a
    /// multi-value parameter (same lable for multiple values) by default the first
    /// occurrence of that lable is returned, otherwise the specified on (if exist).
    /// If the required param is not present an exception is throwed.
    /// @param lable the param's lable to return
    /// @param pos starting from index 1, the param's ocurrence to return (by default the first one)
    /// @return the string value of the required parameter
    /// @throw exceptions::UnknowedParamException if the required parameter is not present.
    long getIParam(std::string const & lable, unsigned int pos = 1)
    throw (exceptions::UnknowedParamException);

    /// Return the required command parameter.
    /// The param whose lable is specified is returned. If the param is a
    /// multi-value parameter (same lable for multiple values) by default the first
    /// occurrence of that lable is returned, otherwise the specified on (if exist).
    /// If the required param is not present an exception is throwed.
    /// @param lable the param's lable to return
    /// @param pos starting from index 1, the param's ocurrence to return (by default the first one)
    /// @return the string value of the required parameter
    /// @throw exceptions::UnknowedParamException if the required parameter is not present.
    double getFParam(std::string const & lable, unsigned int pos = 1)
    throw (exceptions::UnknowedParamException);

    /// Return the required command parameter.
    /// The param whose lable is specified is returned. If the param is a
    /// multi-value parameter (same lable for multiple values) by default the first
    /// occurrence of that lable is returned, otherwise the specified on (if exist).
    /// If the required param is not present an exception is throwed.
    /// @note this method always return a stringified version of the param,
    ///		even if it's type is a number
    /// @param lable the param's lable to return
    /// @param pos starting from index 1, the param's ocurrence to return (by default the first one)
    /// @return the string value of the required parameter
    /// @throw exceptions::UnknowedParamException if the required parameter is not present.
    std::string param(std::string const & lable, unsigned int pos = 1)
    throw (exceptions::UnknowedParamException);

    /// Verify if the specified param is defined.
    bool hasParam(std::string const & lable);


    exitCode xmlDump(std::string & xmlCommand);


protected:

    /// Create a new Command for a defined device
    /// @param cmdType the command type, meaningful values for this param are usually
    ///		exported by Handlers with the enum type named services. Commands
    ///		objects usually doesn't associate a meaning to this value; the
    ///		meaning is knowed only by the receiving Handler.
    /// @param devType the "device class" to witch the Command is adressed, meaningful values are
    ///		defined by deviceCode.
    /// @param devId a device identifier within the device class defined by device.
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    /// @see deviceCode
    /// @see Handler
    Command (t_cmdType const & cmdType, Device::t_deviceType devType, Device::t_deviceId devId, std::string const & logName);

    inline exitCode eraseParam(std::string const & lable);

    inline exitCode setTheParam(std::string const & lable, t_cmdParam const & param, bool override);

    inline t_cmdParam find(std::string const & lable, unsigned int pos)
    throw (exceptions::UnknowedParamException);

    /// Convert from string to native T type.
    /// I.e. to convert 'str' to an int 'i':
    ///   fromString<int>(i, str, std::dec)
    /// @param t the convertion result container
    /// @param s the string to convert
    /// @param f the base for the converted value
    /// 	(it could be: std::hex, std::dec or std::oct)
    template <typename T>
    static bool fromString(T & t, std::string const & s, std::ios_base & (*f)(std::ios_base&)) {
        std::istringstream iss(s);
        return !(iss >> f >> t).fail();
    };

};

} //namespace comsys
} //namespace controlbox
#endif
