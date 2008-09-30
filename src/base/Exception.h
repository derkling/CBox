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


#ifndef _EXCEPTION_H
#define _EXCEPTION_H


#include <string>
using namespace std;


namespace controlbox {
namespace exceptions{

/// The class Exception and its subclasses indicate conditions that a
/// reasonable application might want to catch.
class Exception {
protected:
    /// A descriptive string for the exception
    string d_message;

public:

    /// Create a new Exception.
    Exception(const string & message) :
            d_message(message) {}

    /// Return a descriptive string for the current Exception object
    inline const string & getMessage() {
        return d_message;
    }

};

/// RuntimeException is the parent class of those exceptions that can be
/// thrown during the normal operation of the process.
class RuntimeException : public Exception {
public:

    /// Create a new Exception.
    RuntimeException(const string & message = "RuntimeException") :
            Exception(message) {}

};

/// Signals that an I/O exception of some sort has occurred. This class
/// is the general class of exceptions produced by failed or interrupted
/// I/O operations.
class IOException : public Exception {
public:

    /// Create a new Exception.
    IOException(const string & message = "IOException") :
            Exception(message) {}

};

/// Thrown when an application attempts to...
class ProtocolException : public RuntimeException {
public:

    /// Create a new Exception.
    ProtocolException(const string & message = "ProtocolException") :
            RuntimeException(message) {}

};

/// Thrown when an application attempts to...
class IllegalArgException : public RuntimeException {
public:

    /// Create a new Exception.
    IllegalArgException(const string & message = "IllegalArgException") :
            RuntimeException(message) {};

};

/// Thrown when an application attempts to...
class InitializationException : public RuntimeException {
public:

    InitializationException(const string & message = "InitializationException") :
            RuntimeException(message) {};

};

/// Thrown when an application attempts to...
class OutOfMemoryException : public RuntimeException {
public:

    OutOfMemoryException(const string & message = "OutOfMemoryException") :
            RuntimeException(message) {};

};

/// Thrown when the required Command's parameter is not present
class UnknowedParamException : public RuntimeException {
public:

    UnknowedParamException(const string & message = "UnknowedParamException") :
            RuntimeException(message) {};

};

/// Thrown when an application attempts to...
class IllegalCommandException : public IOException {
public:

    /// Create a new Exception.
    IllegalCommandException(const string & message = "IllegalCommandException") :
            IOException(message) {};

};

/// Thrown when an application attempts to...
class NACKException : public IOException {
public:

    NACKException(const string & message = "NACKException") :
            IOException(message) {};

};

/// Thrown when an application attempts to...
class TimeoutException : public IOException {
public:

    TimeoutException(const string & message = "TimeoutException") :
            IOException(message) {};

};

/// Thrown when an application attempts to...
class ChecksumErrorException : public IOException {
public:

    ChecksumErrorException(const string & message = "ChecksumErrorException") :
            IOException(message) {};

};

/// Thrown when an application attempts to...
class SerialConfigurationException : public IOException {
public:

    SerialConfigurationException(const string & message = "SerialConfigurationException") :
            IOException(message) {};


};

/// Thrown when an application attempts to...
class SerialDeviceException : public IOException {
public:

    SerialDeviceException(const string & message = "SerialDeviceException") :
            IOException(message) {};


};


}// namespace exceptions
}// namespace controlbox

#endif
