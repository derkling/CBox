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


#ifndef _QUERYREGISTRY_H
#define _QUERYREGISTRY_H


#include <controlbox/base/Utility.h>
#include <controlbox/base/Querible.h>
#include <map>

namespace controlbox {

/// Class defining a QueryRegistry.
/// A QueryRegistry allow to associate lable to reference to
/// objects implementing Querible<br>
/// @note This class is <i>abstract</i> and should be derived in order to
///	actually implement an EndPoint.
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>[param]</b> - <i>Default: [default value]</i><br>
///		[description]<br>
///		Size: [size]
///	</li>
/// </ul>
/// @see Querible
class QueryRegistry  {
//------------------------------------------------------------------------------
//				PUBLIC TYPES
//------------------------------------------------------------------------------
  public:


//------------------------------------------------------------------------------
//				PRIVATE TYPES
//------------------------------------------------------------------------------
  private:

	typedef std::string t_queryName;
	typedef Querible * t_pQuerible;
	typedef Querible::t_queryDescription const * t_pQdesc;

	/// Registered query
	typedef map<t_queryName, t_pQuerible> t_queryRegistry;
	typedef pair<t_queryName, t_pQuerible> t_queryRegistryEntry;

	/// Query grouped by Querible.
	typedef multimap<t_pQuerible, t_pQdesc> t_queribleRegistry;
	typedef pair<t_pQuerible, t_pQdesc> t_queribleRegistryEntry;

//------------------------------------------------------------------------------
//				PRIVATE MEMBERS
//------------------------------------------------------------------------------
  protected:

	static QueryRegistry * d_instance;

	t_queryRegistry d_queryRegistry;

	t_queribleRegistry d_queribleRegistry;

	log4cpp::Category & log;

//------------------------------------------------------------------------------
//				PUBLIC METHODS
//------------------------------------------------------------------------------
  public:

	/// Get an instance of QueryRegistry
	/// QueryRegistry is a singleton class, this method provide
	/// a pointer to the (eventually just created) only one instance.
	static QueryRegistry * getInstance();


	/// Default destructor
	~QueryRegistry();

	/// Register a query.
	/// The pointer to a Querible object is saved into the registry and
	/// associated to the specified query descriptor.
	/// @param querible a pointer to a Querible object
	/// @param descr the query description
	/// @return OK on successfully registration,
	///	RI_QUERY_DUPLICATE on 'query' already present into the registry
	/// @note t_query values must be uniques into the registry.
	exitCode registerQuery(Querible * querible, t_pQdesc queryDescriptor);

	/// Unregister a query.
	/// Remove the specified query, and the associated querible, from the registry.
	/// @param query
	/// @param release set true (default) to delete the associated t_pQdesc
	/// @return OK on remove success,
	///	RI_QUERY_NOT_EXIST if the specified 'query' is not
	///	present into the registry
	exitCode unregisterQuery(t_queryName const & query, bool release = true);

	/// Get a pointer to the Querible exporting the specified query.
	/// Return a generic Querible pointer to
	/// the device that has registered the specified query<br>
	/// @return a Querible's pointer to the specified t_queryName if
	/// present into the registry, a zero-pointer if ther's no such
	/// a query registerd.
	Querible * getQuerible(t_queryName const & query);

	/// Get a pointer to the Query Descriptor for the specified query.
	/// @return a t_queryDescription's pointer to the specified t_queryName if
	/// present into the registry, a zero-pointer if ther's no such
	/// a query registerd.
	Querible::t_queryDescription const * getQueryDescriptor(t_queryName const & query);

	/// Get a pointer to the QueribleDescriptor exporting the specified query.
	/// @return a QueribleDescriptor's pointer to the specified t_queryName if
	/// present into the registry, a zero-pointer if ther's no such
	/// a query registerd.
	Querible::t_queryDescription const * getQueryDescriptor(t_queryName const & query, Querible * querible);

	/// Print registered query.
	/// This method return a string with a report of registerd query.
	/// @param query the query of interest
	/// @return if query is empty: the list of all registered queries,
	///	otherwise the list of query exported by the same Querible
	///	object exporting the specified query too.
	std::string printRegistry(t_queryName const & query = "", std::string const & radix = "AT+");

//------------------------------------------------------------------------------
//				PRIVATE METHODS
//------------------------------------------------------------------------------
  protected:

	/// Create a new QueryRegistry
	QueryRegistry(std::string const & logName = "QueryRegistry");


//------------------------------------------------------------------------------
//				Command Parsers
//------------------------------------------------------------------------------

};



}
#endif
 
