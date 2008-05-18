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


#ifndef _QUERIBLE_H
#define _QUERIBLE_H

#include <controlbox/base/Utility.h>
#include <map>

/// @todo Features and extensions:
/// <ul>
///	<li>
/// </ul>

/// Query flags is Read-Write
#define		QST_RW	0x3
/// Query flags is Read-Only
#define		QST_RO	0x2
/// Query flags is Write-Only
#define		QST_WO	0x1

/// Export a query to the QueryRegistry and define the handler method.
/// This macro provide suitable query export definition by both updating
///	the QueryRegistry and defining the local handler
#define EXPORT_QUERY(ID, HANDLER, LABLE, DESCR, VALUES, FLAGS)	\
	if ( true ) {						\
		d_hR->setHandler(ID, HANDLER);			\
		registerQuery(ID, LABLE, DESCR, VALUES, FLAGS);	\
	}

#define FORMAT_STRING(FORMAT, ...)				\
	if (true ) {						\
		Utils::strFormat(FORMAT, ## __VA_ARGS__);	\
	}

#define APPEND_STRING(STRING, FORMAT, ...)				\
	if (true ) {							\
		STRING += Utils::strFormat(FORMAT, ## __VA_ARGS__);	\
	}

#define RETURN_VALUE(QUERY, FORMAT, ...)				\
	if (true ) {							\
		QUERY.value = Utils::strFormat(FORMAT, ## __VA_ARGS__);	\
		QUERY.responce = true;					\
	}

namespace controlbox {

/// A Querible provide remote query support.
/// An object implementing this interface must export a
/// public method suitable to be used to remotely query
/// or send commands to it.<br>
/// @note This class is <i>abstract</i> and should be derived in order to
///	actually implement an object suitable to export query
///	on a QueryRegistry
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>[param]</b> - <i>Default: [default value]</i><br>
///		[description]<br>
///		Size: [size]
///	</li>
/// </ul>
/// @see QueryRegistry
class Querible {

//------------------------------------------------------------------------------
//				PUBLIC TYPES
//------------------------------------------------------------------------------
public:

    /// The identifier of the required param.
    typedef std::string t_queryName;

    /// A numeric code associated to a query.
    /// This code is meaningful only to the Querible, that
    /// could set it into the query description in order to
    /// handle query request more efficiently
    typedef unsigned int t_queryID;

    /// The remote invocation mode.
    enum _queryType {
        QM_QUERY = 0,		///< Query on a member value
        QM_VALUES,		///< Query on a member suitable values
        QM_SET			///< Set a member value
    };
    typedef enum _queryType t_queryType;

    /// Bitmask of supported queryType.
    typedef unsigned short t_querySupportedType;

    /// The value associated.
    /// @note only set mode invocation could have a value associated.
    typedef std::string t_queryValue;

    //---[ START ] Query registration entry
    struct _queryDescription {
        t_queryID		id;
        t_queryName		name;
        t_querySupportedType	flags;			//TODO: renema as flags
        std::string		description;
        std::string		supportedValues;
    };
    typedef struct _queryDescription t_queryDescription;
    //-----[ END ]

    //---[ START ] Query performing
    /// A query.
    /// @param value in a QM_SET query is the value to be setted, in QM_QUERY query
    ///	is the value returned (to be considered valid only if responce is set true).
    ///	Either is the query type: if an error occour performing a query, this
    ///	field should contain a failure reason description.
    /// @param responce set true if 'value' contain a valid responce, false otherwise
    /// @note on errors performing any kind of query this field should be set
    ///	to a description of what's happened and the called routine should return
    ///	an exitCode != OK in order to notify that error.
    struct _query {
        t_queryDescription const * descr;
        t_queryType type;
        t_queryValue value;
        bool responce;
    };
    typedef struct _query t_query;
    //-----[ END ]

//------------------------------------------------------------------------------
//				PRIVATE TYPES
//------------------------------------------------------------------------------
protected:

    /// A generic handler registry implementation.
    /// An handler registry is a container of pointers to local
    /// member functions. This class clould be used to simplify
    /// (and perhaps optimize) the dispatching of queries to
    /// query-type specific member functions
    template <typename aQuerible>
    class handlerRegistry {

    public:
        typedef exitCode (aQuerible::*t_pHandler)(t_query & query);

    protected:
        typedef map<Querible::t_queryID, t_pHandler> t_handlerRegistry;
        typedef pair<Querible::t_queryID, t_pHandler> t_handlerRegistryEntry;

    protected:
        aQuerible * d_pQ;
        t_handlerRegistry d_handlers;

    public:
        /// Create a new handler registry.
        handlerRegistry(aQuerible * pQ) :
                d_pQ(pQ) {}

        ~handlerRegistry() {
            d_handlers.clear();
        }

        /// Register a new handler.
        /// @return OK on success,
        ///	HR_HANDLER_DUPLICATE if an handler with the specified
        ///	id is already present into the registry.
        inline
        exitCode setHandler(Querible::t_queryID id, t_pHandler pH) {

            if ( d_handlers.find(id) != d_handlers.end() ) {
                return HR_HANDLER_DUPLICATE;
            }

            d_handlers[id] = pH;
            return OK;
        }

        inline
        exitCode call(short id, t_query & query) {
            if ( d_handlers.find(id) == d_handlers.end() ) {
                return HR_HANDLER_NOT_PRESENT;
            }
            return (d_pQ->*(d_handlers.find(id)->second))(query);
        }

    };


//------------------------------------------------------------------------------
//				PRIVATE MEMBERS
//------------------------------------------------------------------------------
protected:


//------------------------------------------------------------------------------
//				PUBLIC METHODS
//------------------------------------------------------------------------------
public:
    /// Remote query invocation.
    /// This method execute the required query (if supported) returning
    /// an eventual result. Anyway the method shoud return an exit code
    /// informing the caller about the query execution success.
    /// @return OK on query successfully executed.
    /// @note if the query must return a result, that is expected to be
    ///	into the 'value' query's field with 'responce' field set to true.
    virtual exitCode query(t_query & query) = 0;

    /// Return the Querible descriptive name.
    virtual std::string name() const = 0;

//------------------------------------------------------------------------------
//				PRIVATE METHODS
//------------------------------------------------------------------------------
protected:

    /// Register supported query.
    /// This method should be implemented to export supported query
    /// by registering them into the queryRegistry
    /// @see queryRegistry;
    virtual exitCode exportQuery() = 0;

    exitCode registerQuery(t_queryID id,
                                    t_queryName name,
                                    std::string const & description = "Undefined",
                                    std::string const & values = "Undefined",
                                    t_querySupportedType flags = QST_RW
                                   );

//------------------------------------------------------------------------------
//				Command Parsers
//------------------------------------------------------------------------------


};

}
#endif

