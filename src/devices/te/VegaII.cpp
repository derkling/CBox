//*********************************************************************
//*************  Copyright (C) 2006        DARICOM  ********************
//*********************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** DARICOM The programs may be used and/or copied only with the
//** written permission from DARICOM or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//*********************************************************************
//******************** Module information *****************************
//**
//** Project:       ProjectName (ProjectCode/Version)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Developer
//** Creation date:  21/06/2006
//**
//*********************************************************************
//******************** Revision history *******************************
//** Revision date       Comments                           Responsible
//** -------- ---------- ---------------------------------- -----------
//**
//**
//*********************************************************************


#include "VegaII.ih"

namespace controlbox {
namespace device {

VegaII::t_teCmd VegaII::d_teCmd[] = {
	{ // Command 31: master ACK
	{0x02, 0x33, 0x31, 0x36, 0x36, 0x0}, 5},
	{ // Command 32: master NAK
	{0x02, 0x33, 0x32, 0x37, 0x36, 0x0}, 5},
	{ // Command 34: download new event
	{0x02, 0x33, 0x34, 0x39, 0x36, 0x0}, 5},
};

const char *VegaII::eventTypeLable[] = {
	"Accensione",
	"Spegnimento",
	"Ingresso programmazione",
	"Uscita proggrammazione",
	"Mancanza consenso",
	"Presenza HW-Key",
	"Mancanza HW-Key",
	"Allarme di sistema",
	"Allarme misuratore 1",
	"Allarme misuratore 2",
	"Inserimento piano di carico",
	"Annullamanto piano di carico",
	"Inizio scarico misuratore 1",
	"Inizio scarico misuratore 2",
	"Ripresa scarico dopo stop misuratore 1",
	"Ripresa scarico dopo stop misuratore 2",
	"Fine fase scarico misuratore 1",
	"Fine fase scarico misuratore 2",
	"Stop scarico da operatore misuratore 1",
	"Stop scarico da operatore misuratore 2",
	"Stop scarico per mancanza di alimentazione",
	"Stop scarico per scomparto vuoto misuratore 1",
	"Stop scarico per scomparto vuoto misuratore 2",
	"Inizio svuotamento prodotto misuratore 1",
	"Inizio svuotamento prodotto misuratore 2",
	"Ripresa scarico da nuovo scomparto misuratore 1",
	"Ripresa scarico da nuovo scomparto misuratore 2",
	"Fine svuotamento prodotto misuratore 1",
	"Fine svuotamento prodotto misuratore 2",
	"Stampa report carico",
	"Stampa report scarico misuratore 1",
	"Stampa report scarico misuratore 2",
	"Estrazione cartellino",
	"Temperature dei prodotti al carico",
	"Stampante non disponibile",
	"Mancanza carta stampante",
	"Auto-accensione",
	"Trafilamento misuratore 1",
	"Trafilamento misuratore 2",
	"Stampa annullata tramite codice sblocco",
	"Reset del log eventi",
	"Reset del log parametri",
	"Quantità scaricate e perdite",
	"Temperatura dei prodotti allo scarico",
	"Apertura valvole di fondo",
	"Default cliente",
	"Programmazione parametri da seriale"
};
#define VEGAII_EVENTS_COUNT sizeof(eventTypeLable)/sizeof(char*)

const char *VegaII::nakCodeLable[] = {
	"Unknowen",
	"Code not existing",
	"Wrong lenght",
	"Chksum errato",
	"Evento non disponibile",
	"Log modifica parametri non disponibile",
	"Prodotto inesistente",
	"Scomparto inesistente",
	"Quantità pianificata fuori dal limite caricabile per lo scomparto"
	"Programmazione",
	"Piano di carico",
	"Prodotto impostato",
	"Quantità impostata",
	"Modalità programmazione",
	"Indeice parametro inesistente",
	"Richiesta numero parametrio non disponibile",
	"Formato data/ora non corretto",
	"Non esiste un messaggio non conformato da master da poter inviare"
};

VegaII::VegaII(std::string const & logName) :
	DeviceTE(DeviceTE::VEGAII, logName) {
	LOG4CPP_DEBUG(log, "VegaII(std::string const &)");
}

VegaII::~VegaII() {
	LOG4CPP_DEBUG(log, "Destroying the VegaII device");
}



inline exitCode
VegaII::checksum(const char * buf, unsigned len, char chksum[3]) {
	unsigned sum = 0;
	unsigned index = 0;

	for ( ; index<len; index++) {
		sum += buf[index];
	}

	// NOTE:  (value % 256) == (value & 0xFF)
	sprintf(chksum, "%02X",  (sum & 0xFF));

	// On trasmission we need to SWAP the two bytes
	chksum[0] ^= chksum[1];
	chksum[1] ^= chksum[0];
	chksum[0] ^= chksum[1];

	LOG4CPP_DEBUG(log, "len [%d], sum [%d, 0x%X], checksum [%s]", len, sum, sum, chksum);
	return OK;
}

bool
VegaII::verifyChecksum(const char * buf, unsigned len) {
	char rxChksum[] = "\0\0\0";
	char myChksum[] = "\0\0\0";
	bool match = false;

#ifdef DARICOMDEBUG
	if ( d_forceDownload ) return true;
#endif

	memcpy(rxChksum, buf+len-3, 2);
	checksum(buf, len-3, myChksum);

	LOG4CPP_DEBUG(log, "Checksum rx [%s], computed [%s]", rxChksum, myChksum);

	match = strncasecmp(rxChksum, myChksum, 2);
	if ( match ) {
		LOG4CPP_WARN(log, "Checksum mismatch on TE data download");
	}

	return !match;

}


exitCode
VegaII::queryNewEvent(char * p_event, int & len) {
	exitCode result = OK;
	bool chksumOk = false;

	LOG4CPP_DEBUG(log, "==> CMD_34 (New events?) [%s]", d_teCmd[CMD_34].cmd+1);
	d_tty->sendSerial(d_teCmd[CMD_34].cmd, d_teCmd[CMD_34].len);

	do {
		// Reading back TE responce (CMD64 or CMD62)
		len = VEGAII_MAX_RSP_LEN;
		result = d_tty->readSerial(p_event, len);
		if ( result != OK ) {
			break;
		}

		// Verify checksum (continue if failed)
		chksumOk = verifyChecksum((const char *)p_event, len);
		if ( !chksumOk ) {
			// Sending master NAK
			LOG4CPP_DEBUG(log, "==> CMD_32 (Master NAK) [%s]", d_teCmd[CMD_32].cmd+1);
			d_tty->sendSerial(d_teCmd[CMD_32].cmd, d_teCmd[CMD_32].len);
		}

	} while ( !chksumOk );

	return result;

}

exitCode
VegaII::sendACK() {
	exitCode result = OK;

	LOG4CPP_DEBUG(log, "==> CMD_31 (Master ACK) [%s]", d_teCmd[CMD_31].cmd+1);

	// Sending (pre)defined Command 34
	d_tty->sendSerial(d_teCmd[CMD_31].cmd, d_teCmd[CMD_31].len);

	return result;

}

exitCode
VegaII::checkNAK(char * p_event) {
	char sNakCode[] = "\0\0\0";
	unsigned nakCode;

	// Check if received MSG62
	if ( p_event[1] == '6' &&
		p_event[2] == '2') {

		sNakCode[0] = p_event[3];
		sNakCode[1] = p_event[4];
		sscanf(sNakCode, "%u", &nakCode);

		if ( nakCode == 0x04 ) {
			return TE_NO_NEW_EVENTS;
		}

		// Refer to pag. 52 for code details
		LOG4CPP_DEBUG(log, "<== MSG_62 (Slave NAK) [(0x%02X): %s]",
			nakCode, nakCodeLable[nakCode]);

		return TE_NAK_RECEIVED;
	}

	return OK;
}

exitCode
VegaII::logEvent(char * p_event) {
	char rxEvent[] = "\0\0\0";
	unsigned l_event = 0;
	unsigned l_eventCount = VEGAII_EVENTS_COUNT;

	rxEvent[0] = p_event[3];
	rxEvent[1] = p_event[4];
	sscanf(rxEvent, "%u", &l_event);

	if ( l_event<l_eventCount ) {
		LOG4CPP_INFO(log, "New event [%s]", eventTypeLable[l_event]);
	} else {
		LOG4CPP_INFO(log, "New event [UNKNOWEN]");
	}

	return OK;
}

exitCode
VegaII::downloadEvents(t_eventList & eventList) {
	exitCode result = OK;
	int len = 0;
	char buff[SAMPI550_MAX_RSP_LEN];
	t_event * l_event = 0;
	unsigned downloadedRecords = 0;

	// Locking TTY mux for this transaction
	d_tty->detachedMode(false);

	do {
		// Send CMD34 (event request message)
		result = queryNewEvent(buff, len);
		if ( result!=OK ) {
			LOG4CPP_WARN(log, "Failed quering TE for new events");
			break;
		}

		result = checkNAK(buff);
		if ( result==TE_NAK_RECEIVED ) {
			// Trying to download messages one more time
			// FIXME this could be a problem if the communication
			//	always fails... we fall into an endless loop.
			LOG4CPP_WARN(log, "Unhandleded slave NAK code");
			break;
		}
		if ( result==TE_NO_NEW_EVENTS ) {
			LOG4CPP_DEBUG(log, "No new events");
			if ( downloadedRecords>0 ) {
				result = OK;
			}
			break;
		}

		//Removing leading checksum and line terminator....
		buff[len-3] = 0;

		logEvent(buff);

		// Building new event and appending to list
		l_event = new t_event();
		l_event->type = UNDEF;
		l_event->event = std::string(buff);
		eventList.push_back(l_event);
		downloadedRecords++;

		sendACK();

		// Sleep 2sec
		::sleep(1);

	} while (true);

	// Releasgin TTY mux
	d_tty->detachedMode(true);

	return result;
}


exitCode
VegaII::formatEvent(t_event const & p_event, comsys::Command & cmd) {
	std::ostringstream sbuf("");
	const char * evt;

	evt = p_event.event.c_str();

	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x2B;
	cmd.setParam( "dist_evtType", sbuf.str());
	cmd.setParam("dist_evtData", evt+1);
	LOG4CPP_DEBUG(log, "RAW VegaII, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				evt+4);

	return OK;

}

}// namespace device
}// namespace controlbox
