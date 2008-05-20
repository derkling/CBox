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


#ifndef _WSPROXYCOMMANDHANDLER_H
#define _WSPROXYCOMMANDHANDLER_H


#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/CommandHandler.h>
#include <controlbox/base/Querible.h>
#include <cc++/thread.h>
#include <controlbox/base/Configurator.h>
#include <queue>
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/devices/DeviceGPS.h>
#include <controlbox/devices/DeviceOdometer.h>
#include <controlbox/devices/DeviceAnalogSensors.h>


// Forward declaration
//class EndPoint;
#include "EndPoint.h"

/// @todo Features and extensions:
/// <ul>
///	<li>
/// </ul>


/// The size of the 'idAutista' field
#define WSPROXY_STRSIZE_IDA	15
#define WSPROXY_DEFAULT_IDA	"UNKNOWNA"
/// The size of the 'idMotriceSize' field
#define WSPROXY_STRSIZE_IDM	15
#define WSPROXY_DEFAULT_IDM	"UNKNOWNM"
/// The size of the 'idSemirimorchioSize' field
#define WSPROXY_STRSIZE_IDS	15
#define WSPROXY_DEFAULT_IDS	"UNKNOWNS"
/// The size of the 'CIM' field
#define WSPROXY_DEFAULT_CIM	"UNKNOWNCIM"
/// The size of a timestamp
#define WSPROXY_TIMESTAMP_SIZE	25

/// The first id for EndPoint configuration params lables
#define WSPROXY_EP_FIRST_ID	0
/// The maximun number of configurables EndPoints
#define WSPROXY_EP_MAXNUM	3

#define DEFAULT_DUMP_QUEUE_FILEPATH	"./wsUploadQueue.dump"


namespace controlbox {
namespace device {

/// A WSProxyCommandHandler is a CommandHandler able to
/// interact with a remote WebService in order to upload
/// data to it and get responses.<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>idAutista</b> - <i>Default: UNKNOWNA</i><br>
///		The truck driver identifier<br>
///		Size: WSPROXY_STRSIZE_IDA
///	</li>
///	<li>
///		<b>idMotrice</b> - <i>Default: UNKNOWNM</i><br>
///		The truck tractor identifier<br>
///		Size: WSPROXY_STRSIZE_IDM
///	</li>
///	<li>
///		<b>idSemirimorchio</b> - <i>UNKNOWNR</i><br>
///		The truck driver identifier<br>
///		Size: WSPROXY_STRSIZE_IDS
///	</li>
///	<li>
///		<b>dumpQueueFilePath</b> - <i>DEFAULT_DUMP_QUEUE_FILEPATH</i><br>
///		The file to use for uploadQueue messages dump on system reboots<br>
///	</li>
/// </ul>
/// @see CommandHandler
class WSProxyCommandHandler : public comsys::CommandHandler, public Querible, public ost::PosixThread  {

//------------------------------------------------------------------------------
//				PUBLIC TYPES
//------------------------------------------------------------------------------
public:

    /// The source of data to be uploaded
    /// NOTE from specifications 4.3 this type include Server-to-server
    ///		IDs: we support only the typed that could be generated by our
    ///		device.
    enum idSource {
        WS_SRC_CONC = 0,	///< Concentratore
        WS_SRC_ICD		///< In Cabin Device (PocketPC)
    };
    typedef enum idSource t_idSource;

    typedef char t_timeStamp[WSPROXY_TIMESTAMP_SIZE+1];

    //TODO complete the product ID list
    enum idProduct {
    	WS_PID_DIESEL = 0,
    	WS_PID_BLUDIESEL,
    	WS_PID_PETROL,
    	WS_PID_BLUSUPER,
    	WS_PID_GPL,
    	WS_PID_BITUMEN,
    	WS_PID_JETFEUL
    };
    typedef enum idProduct t_idProduct;

    struct product {
    	short onuCode;
    	const char *name;
    };
    typedef struct product t_product;

protected:

    typedef list<EndPoint *> t_EndPoints;

//------------------------------------------------------------------------------
//				PRIVATE TYPES
//------------------------------------------------------------------------------
protected:

    /// The (only) instance of this class.
    static WSProxyCommandHandler * d_instance;

    std::string d_name;

    /// Total number of messages received and queued for upload
    unsigned int d_msgCount;

    //TODO complete the product list.
    /// The product list.
    static const t_product d_products[];


    /// Define a message to be uploaded to a WebService.
    /// Each message could be upladed to more than one EndPoint
    struct wsData {
	unsigned int msgCount;			///< local message ID (used for local debugging)
	unsigned int endPoint;			///< endPoint mask
	t_idSource idSrc;			///< Source ID
	t_timeStamp tx_date;			///< Trasmission data
	t_timeStamp rx_date;			///< Reception data
	t_timeStamp cx_date;			///< Creation data
	char rx_lat[9];				///< Latitude at reception time
	char rx_lon[10];			///< Longitude at reception time
	std::ostringstream cmm;			///< Command common data
	std::ostringstream msg;			///< Command specific data
	EndPoint::t_epRespList respList;	///< Responces received by EndPoints
    };
    typedef struct wsData t_wsData;

    typedef list<t_wsData *> t_uploadList;

    /// A pointer to a command data parser function.
    /// It shuold be defined a command parser for each command type we
    /// understand. The command parser is a routine able to interpreter
    /// the information carried by the command and performing necessary
    /// elaboration. If the command is a "local command", meaning a command
    /// carring informations adressed to the WSProxy object itself (like
    /// network state notifications or other kind of events of interest
    /// for the WSProxy), the parser shuold perform any useful action and
    /// return a null pointer for the t_wsData param. Otherwise, if
    /// the command carry some information to be uploaded using throught
    /// the endPoints: the command parser is in charge to create and
    /// correctly initialize a t_wsData object, using the pointer that receive
    /// as input params and define the endPoint to witch the message shuld be
    /// forwarded.
    typedef exitCode (WSProxyCommandHandler::*t_cmdParserPtr)(t_wsData **, comsys::Command &);

    /// A mapping between supported command types and command data parser
    typedef map<comsys::Command::t_cmdType, t_cmdParserPtr> t_cmdParser;



//------------------------------------------------------------------------------
//				PRIVATE MEMBERS
//------------------------------------------------------------------------------
protected:
    /// The Configurator to use for getting configuration params
    Configurator & d_configurator;

    /// Loaded EndPoints
    t_EndPoints d_endPoints;

//---------------------------------------------------[ DEVICES ]----------------
    /// The DeviceTime to use
    DeviceTime * d_devTime;

    /// The GPS device to use.
    DeviceGPS * d_devGPS;

    /// The ODO device to use.
    DeviceOdometer * d_devODO;

    /// The Analog Sensors device to use
    DeviceAnalogSensors * d_devAS;

    /// The CAN Bus device to use
    //DeviceCAN const * d_devCAN;


//---------------------------------------------------[ STATE ]------------------

    /// The last received MTC message
    /// The code of the last TIP to MTC (concentratore) message successfully
    /// received.


    /// The last know GPS fix state.
    /// This value is updated by means of GPS_FIX_STATUS_UPDATE commands
    /// @see services
    DeviceGPS::t_fixStatus d_gpsFixStatus;

    /// The last know NET link state.
    /// This value is updated by means of NET_LINK_STATUS_UPDATE commands
    /// @see services
    DeviceGPRS::t_netStatus d_netStatus;

    /// Associate each Command's infoType to a t_wsData struct builder function
    t_cmdParser d_cmdParser;

    /// List of messages waiting to be uploaded.
    /// Messages ready to be uploaded are queued into this data structore.
    /// A dedicated thread, defined by this class, is in charge to monitor this
    /// data structore and, once the network link is up, upload queued SOAP
    /// messages as soon as possible.
    t_uploadList d_uploadList;

    /// The filepath for the file to use for uploadQueue dump and persistence
    std::string dumpQueueFilePath;

    /// Control Access to sensible data structures
    ost::Mutex d_wsAccess;

    /// Set to true once we want to terminate the SOAP messages upload thread.
    bool d_doExit;

//------------------------------------------------------------------------------
//				PUBLIC METHODS
//------------------------------------------------------------------------------
public:

    static WSProxyCommandHandler * getInstance(std::string const & logName = "WSProxy");

    /// Class destructor.
    ~WSProxyCommandHandler();

    /// The default notify routine.
    /// The implementation in that class does nothing, just generate
    /// log if level is <= DEBUG
    exitCode notify();

    /// Command notify routine.
    /// The specified command
    /// @param command the Command to process
    /// @return OK on success Command's dumping,
    ///		WS_INCOMPLETE_MESSAGE if the Command is missing some required param.
    /// @throw exceptions::IllegalCommandException never throwed by this
    ///			implementation.
    exitCode notify(comsys::Command * cmd)
    throw (exceptions::IllegalCommandException);

//-----[ Query interface ]------------------------------------------------------
    exitCode query(Querible::t_query & query);

    inline std::string name() const {
        return d_name;
    };



//------------------------------------------------------------------------------
//				PRIVATE METHODS
//------------------------------------------------------------------------------
protected:

    /// Create a new WSProxyCommandHandler.
    /// @param append   whether to truncate the file or just append to it if it
    ///		already exists. Defaults to 'true'.
    WSProxyCommandHandler(std::string const & logName = "WSProxy");


    /// Initialize configuration params.
    /// This method provide to (pre)load all required params checking
    /// for their value or iniializing with default values.
    /// @return OK on success.
    inline exitCode preloadParams();

//    /// Actives EndPoint mask;
//     inline unsigned int epEnabled();

    /// Load the configured EndPoints
    inline exitCode loadEndPoints();

    /// Link dependant devices.
    /// This method provide to get referencies to required device.
    /// @return OK on success.
    inline exitCode linkDependencies();

    /// Return the list of loaded EndPoints
    inline std::string listEndPoint();

    /// Initialize command parsers
    /// Command parsers encapsulate the code able to decode a command
    /// extracing all required params in order to prduce a t_wsData
    /// structure ready to be uploaded by EndPoints.
    /// This method is called by the constructor to initialize the runtime
    /// support for command parser selection.
    inline exitCode initCommandParser();


    /// Return the actual CIM
    inline std::string getCIM(void);

    /// Return the actual MTC
    inline std::string getMTC(void);


    /// Fill-in wsData common section.
    /// @param wsData a pointer to a t_wsData: a new wsData will be created
    ///		by the method, initialized and returned to the caller.
    /// @return a new t_wsData object ready to be customized with command
    ///		specific data.
    inline exitCode fillCommonSection(t_wsData * wsData, comsys::Command & cmd);


    /// Upload SOAP message's Thread body.
    /// This method is the thread code in charge to effectively deliver SOAP
    /// messages to the remote WebService.
    /// SOAP messages ready to be delivered are thaken from the d_uploadList
    /// data structure. This thread is usually sleeping and does it's work only
    /// when it's signalled by some-one.
    /// Interesting events that shuld signal that thread are:
    /// <ul>
    ///	<li>
    ///		a new SOAP message is ready to be uploaded (while the network link is up).
    ///		On this case the thread allow to immediatly upload of the new message.
    ///	</li>
    ///	<li>
    ///		a network link up event has been signalled to an object of this class.
    ///		The thread must be awaked in order to immediatlly upload any eventually
    ///		queued SOAP message present into the d_uploadList queue.
    ///	</li>
    /// </ul>
    void run(void);

    /// A derived method to handle asynchronous I/O requests delivered to the specified thread.
    /// This method override the default implementation provided by the base class ost::PosixThread
    /// and simply resume the current thread once an asynchronous I/O requests is catched.
    /// Asynchronous I/O requests shuld be notified to this thread on new messages ready to be sent
    /// while network link is down, network link coming.
    void onPolling(void);

    /// A derived method to handle Hangup signals delivered to the specified thread.
    void onHangup(void);

    /// A derived method to handle Abort delivered to the specified thread.
    void onException(void);

    /// Try to immediatly upload the specified message.
    /// This method queue the specified message into the upload queue
    /// and, if the network link is up, signal the upload thread in order
    /// to handle immediatly the queued SOAP message dispatching.
    /// @param message the SOAP message to upload
    /// @return WS_TRYING_UPLOAD if the network link is up and the upload thread will try
    ///			an immediatly upload,
    ///		WS_QUEUED_GSOAPMSG if the network link is down and the message has been queued for
    ///			delayed update
    /// NOTE: on error queueMsg exit code is returned
    exitCode uploadMsg(t_wsData & wsData);

    /// Queue a SOAP message to be uploaded to the WebService.
    /// SOAP messages ready to be uploaded to the WebService are simply
    /// queued into the upload queue by this method that also notify
    /// the upload theread about new data ready to be uploaded.
    /// @param message the gSOAP message to upload
    exitCode queueMsg(t_wsData & wsData);

    /// Check EndPoint piggybacked commands and trigger suitable options.
    exitCode checkEpCommands(EndPoint::t_epRespList &respList);

    /// Effectively upload a message to the associated EndPoints
    /// This method is the MUTEX protected access to EndPoints.
    /// NOTE: in the threaded version of upload queue the mutex is not beeded because the
    ///		exclusive access to the WebService is granted by the uniqueness
    ///		of the thread calling this method.
    /// @return OK on upload success
    /// 		WS_UPLOAD_FAULT on upload fault tue to communication problems with the WS
    ///		WS_INVALID_DATA if the WebService complain about either invalid data
    ///			format or SOAP message wrong format
    ///		WS_LINK_DOWN the network link is down: unable to upload SOAP message
    /// @note This class leave message untouched.
    exitCode callEndPoints(t_wsData & msg);

    inline WSProxyCommandHandler::t_wsData * newWsData(t_idSource src = WS_SRC_CONC);

    /// Securely release memory on wsData delete.
    /// Provide a secure way to release all memory associated to the
    /// specified wsData message
    /// @param message the SOAP message to release
    inline exitCode wsDataRelease(t_wsData * wsData);

    /// Flush upload queue to file.
    /// Flush the upload queue entry into the specified file.
    /// @param filepath the path of the file into witch data must be saved
    /// @return OK on success
    exitCode flushUploadQueueToFile() {};

    /// Load upload queue from file.
    /// Load the upload queue entry from the specified file.
    /// @param filepath the path of the file from witch data must be loaded
    /// @return OK on success
    exitCode loadUploadQueueFromFile() {};

//-----[ Query interface ]------------------------------------------------------

    exitCode exportQuery();

//------------------------------------------------------------------------------
//				Command Parsers
//------------------------------------------------------------------------------


//-----[ Local Commands ]-------------------------------------------------------

    /// GPS_FIX_UPDATE
    exitCode cp_gpsFixUpdate(t_wsData ** wsData, comsys::Command & cmd);

    /// GPRS_STATUS_UPDATE
    exitCode cp_gprsStatusUpdate(t_wsData ** wsData, comsys::Command & cmd);


//-----[ Remote Commands ]------------------------------------------------------

    /// Build a wsData using DIST protocol.
    /// In order to support this interface, cmd should contains a "dist_event"
    /// parameter, that rapresent a valid payload for the variable part of
    /// the wsData message, and a "timestamp" parameter that rapresent the
    /// event generation time
    exitCode formatDistEvent(t_wsData ** wsData, comsys::Command & cmd, t_idSource src);

    /// SEND_POLL_DATA: dati a frequenza periodica
    exitCode cp_sendPollData(t_wsData ** wsData, comsys::Command & cmd);
    /// SEND_GENERIC_DATA: Dati non pre-codificati inviati dall'autista
    exitCode cp_sendGenericData(t_wsData ** wsData, comsys::Command & cmd);
    /// SEND_CODED_EVENT: Eventi pre-codificat inviato dall'autista
    exitCode cp_sendCodedEvent(t_wsData ** wsData, comsys::Command & cmd);
    /// DIGITAL_SENSORS_EVENT: Eventi generati da sensori digitali
    exitCode cp_sendDSEvent(t_wsData ** wsData, comsys::Command & cmd);
    /// SEND_TE_EVENT: Eventi generati dalla Testate Elettronica
    exitCode cp_sendTEEvent(t_wsData ** wsData, comsys::Command & cmd);
    /// ODOMETER_EVENT: Eventi generati dall'odometro
    exitCode cp_sendOdoEvent(t_wsData ** wsData, comsys::Command & cmd);


    /*
    	/// Handle SEND_DATA Command with infoType: POLL_DATA
    	static exitCode service_uploadData_pollData(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_ACCENSIONE
    	static exitCode service_uploadData_eventAccensione(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_CARICO
    	static exitCode service_uploadData_eventCarico(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_EROGAZIONE
    	static exitCode service_uploadData_eventErogazione(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_SPEGNIMENTO
    	static exitCode service_uploadData_eventSpegnimento(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_ACPORTELLONE
    	static exitCode service_uploadData_eventACPortellone(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_SCOMPARTOVUOTO
    	static exitCode service_uploadData_eventScompartoVuoto(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_TRABOCCAMENTO
    	static exitCode service_uploadData_eventTraboccamento(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_MANUAL
    	static exitCode service_uploadData_eventManual(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_MANUAL_PRECOD
    	static exitCode service_uploadData_eventManualPrecod(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: STATE
    	static exitCode service_uploadData_state(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_RICONCILIAZIONE
    	static exitCode service_uploadData_eventRiconciliazione(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_STACCOBATTERIA
    	static exitCode service_uploadData_eventStaccobatteria(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_ACCICLOCHIUSO
    	static exitCode service_uploadData_eventACCicloChiuso(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );

    	/// Handle SEND_DATA Command with infoType: EVENT_ACCASSETTAPNEUMATICA
    	static exitCode service_uploadData_eventACCassettaPenumatica(WSProxyCommandHandler & wspch, _ns1__uploadData & message,  Command & command );
    */

};


}// namespace device
}// namespace controlbox
#endif

