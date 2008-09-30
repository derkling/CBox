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


#include "Sampi550.ih"

namespace controlbox {
namespace device {


Sampi550::t_teCmd Sampi550::d_teCmd[] = {
	{ // Command 28: download next event marked as "downloadable"
	{0x02, 0x30, 0x31, 0x31, 0x32, 0x38, 0x30, 0x32, 0x30, 0x30, 0x30, 0x43, 0x0}, 12},
	{ // Command 29: mark an event as downloaded
	{0x02, 0x30, 0x31, 0x31, 0x32, 0x39, 0x30, 0x37, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x36, 0x42, 0x0}, 17},
};

const char *Sampi550::eventTypeLable[] = {
	"Empty",
	"Power on",
	"Power off",
	"P.off pulses",
	"Error",
	"Loading",
	"Delivery",
	"Undefined"
	"Leakage",
	"Deleted load",
	"HW test",
	"Undefined",
	"Crash",
	"Reset",
	"Batch",
};


Sampi550::Sampi550(std::string const & logName) :
	DeviceTE(DeviceTE::SAMPI550, logName) {

	LOG4CPP_DEBUG(log, "Sampi550(std::string const &)");


}

Sampi550::~Sampi550() {

	LOG4CPP_DEBUG(log, "Destroying the Sampi550 device");

}

exitCode
Sampi550::checksum(const char * buf, unsigned len, char chksum[3]) {
	unsigned sum = 0;
	unsigned index = 0;

// 	printf("\n\n");
	for ( ; index<len; index++) {
// 		printf("%02X+", (unsigned)buf[index]);
		sum += (unsigned)buf[index];
	}
// 	printf("\n\n");

	// NOTE:  (value % 256) == (value & 0xFF)
	sprintf(chksum, "%02X",  (sum & 0xFF));

	// Swapping the two bytes...
	chksum[0] ^= chksum[1];
	chksum[1] ^= chksum[0];
	chksum[0] ^= chksum[1];

	LOG4CPP_DEBUG(log, "len [%d], sum [%d, 0x%X], checksum [%s]", len, sum, sum, chksum);
	return OK;


}

bool
Sampi550::verifyChecksum(const char * buf, unsigned len) {
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
Sampi550::queryNewEvent(char * p_event, int & len) {
	exitCode result = OK;

	// Locking TTY mux for this transaction
	d_tty->detachedMode(false);

	LOG4CPP_INFO(log, "==> CMD_28 (Read next available event) [%s]", d_teCmd[CMD_28].cmd+1);

	// Sending (pre)defined Command 28
	d_tty->sendSerial(d_teCmd[CMD_28].cmd, d_teCmd[CMD_28].len);

	// Reading back TE responce
	len = SAMPI550_MAX_RSP_LEN;
	result = d_tty->readSerial(p_event, len);

	// Releasgin TTY mux
	d_tty->detachedMode(true);

	return result;

}

exitCode
Sampi550::markMessageDownloaded(u16 loc, char * resp, int & len) {
	exitCode result = OK;
	char frame29[17];
	char chksum[] = "\0\0\0";

	// Starting from message template
	memcpy(frame29, d_teCmd[CMD_29].cmd, 17);

	// Updating event location to mark
	sprintf(frame29+10, "%04u", (unsigned)loc);

	// Computing checksum and appending to message
	checksum((const char *)frame29, 15, chksum);
	sprintf(frame29+15, "%2s", chksum);

	LOG4CPP_DEBUG(log, "==> CMD_29 [%s]", frame29+1);

	// Locking TTY mux for this transaction
	d_tty->detachedMode(false);

	// Sending (pre)defined Command 28
	d_tty->sendSerial(frame29, d_teCmd[CMD_29].len);

	// Reading back TE responce
	len = SAMPI550_MAX_RSP_LEN;
	result = d_tty->readSerial(resp, len);

printf("1\n");
	// Releasing TTY mux
	d_tty->detachedMode(true);
printf("2\n");
	return result;

}

exitCode
Sampi550::downloadEvents(t_eventList & eventList) {
	exitCode result;
	char buff[SAMPI550_MAX_RSP_LEN];
	char coded[512];
	int len;
	t_teFrame28_rx * frame28;
	u16 evtLocation;
	t_event * l_event;
	unsigned downloadedRecords = 0;
	bool uploadCompleted = false;
	bool chksumOk = false;

	do {

		// Sleep 2sec
		LOG4CPP_WARN(log, "Waiting 2sec...");
		::sleep(2);

		// Send CMD28 (download first event marked as downloadable)
		// Read Event
		result = queryNewEvent(buff, len);
		if ( result!=OK ) {
			LOG4CPP_WARN(log, "Failed quering TE for new events");
			continue;
		}

		// Verify checksum (continue if failed)
		chksumOk = verifyChecksum((const char *)buff, len);
		if ( !chksumOk ) {
			LOG4CPP_WARN(log, "Communication errors, checksum mismatch");
			continue;
		}

		frame28 = (t_teFrame28_rx *)buff;

		// if NOT FOUND: break (NO NEW EVENTS);
		if ( buff[4]==0 ) { //frame28->found==0 ) {
			LOG4CPP_INFO(log, "No new events to download");
			break;
		}

		evtLocation = (buff[6]<<8)+buff[7];//frame28->num;
		uploadCompleted = (buff[5]); //frame28->upd;

		LOG4CPP_INFO(log, "New event [(%hu)%15s:%04u]",
			(u8)frame28->data[0],
			eventTypeLable[frame28->data[0]],	// Event type
			(u16)evtLocation);			// Event location
// 			*((u16*)((frame28->data)+1)));		// Event number

		// Convert in base64
		// We don't convert leading ACK byte and final 2 checksum bytes
		// const char *p_in, size_t p_inlen, char *p_out, size_t p_outlen
		Utils::b64enc(buff+1, len-(1+3), coded, 512);
		LOG4CPP_DEBUG(log, "Base64 coded event (len: %d) [%s]",
					strlen(coded), coded);

		// Building new event and appending to list
		l_event = new t_event();
		l_event->type = UNDEF;
		l_event->event = std::string(coded);
		eventList.push_back(l_event);
		downloadedRecords++;

		// Sleep 2sec
		LOG4CPP_WARN(log, "Waiting 2sec...");
		::sleep(2);

		do {
			// Send CMD29 (mark message as downloaded)
			// Read marking result
// 			result = markMessageAsDownloaded(evtLocation, buff, len);
// FIXME if we use the function here upper insted of this inlined code
			char frame29[17];
			char chksum[] = "\0\0\0";

			// Starting from message template
			memcpy(frame29, d_teCmd[CMD_29].cmd, 17);

			// Updating event location to mark
			sprintf(frame29+10, "%04u", (unsigned)evtLocation);

			// Computing checksum and appending to message
			checksum((const char *)frame29, 15, chksum);
			sprintf(frame29+15, "%2s", chksum);

			LOG4CPP_INFO(log, "==> CMD_29 (Mark event %u as downloaded) [%s]",
				(unsigned)evtLocation, frame29+1);

			// Locking TTY mux for this transaction
			d_tty->detachedMode(false);

			// Sending (pre)defined Command 28
			d_tty->sendSerial(frame29, d_teCmd[CMD_29].len);

			// Reading back TE responce
			len = SAMPI550_MAX_RSP_LEN;
			result = d_tty->readSerial(buff, len);

			// Releasing TTY mux
			d_tty->detachedMode(true);

			if ( result!=OK ) {
				LOG4CPP_WARN(log, "Failed quering TE for marking downloaded message");
				chksumOk = false;
				continue;
			}

			// Verify checksum (break on success)
			chksumOk = verifyChecksum((const char *)buff, len);
			if ( chksumOk ) {
				LOG4CPP_DEBUG(log, "Event [%04u] marked as downloaded", (unsigned)evtLocation);
			} else {
				LOG4CPP_WARN(log, "Communication errors, checksum mismatch");
				// Sleep 2sec
				LOG4CPP_WARN(log, "Waiting 2sec...");
				::sleep(2);
			}

		} while ( !chksumOk ); // Marking confirmed

		// if MORE events: continue
		//	else break (NO MORE EVENTS)
		if ( uploadCompleted ) {
			LOG4CPP_WARN(log, "No more events to download");
		}

	} while (!uploadCompleted); // There are more events to donwload

	return OK;

}


exitCode
Sampi550::formatEvent(t_event const & p_event, comsys::Command & cmd) {
	std::ostringstream sbuf("");
	const char * evt;

	evt = p_event.event.c_str();

// 	sbuf.str("");
	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x2A;
	cmd.setParam( "dist_evtType", sbuf.str());
	cmd.setParam("dist_evtData", evt+1);
	LOG4CPP_INFO(log, "RAW SampiTE_550, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				evt+4);

	return OK;

}

}// namespace device
}// namespace controlbox
