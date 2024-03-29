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


#include "Sampi500.ih"

namespace controlbox {
namespace device {

Sampi500::t_teCmd Sampi500::d_teCmd[] = {
	{ // Command 54: start trasmission
	{0x02, 0x30, 0x30, 0x30, 0x35, 0x34, 0x42, 0x46, 0x0}, 8 },
	{ // Command 54: ACK last received responce
	{0x06, 0x30, 0x30, 0x30, 0x35, 0x34, 0x46, 0x46, 0x0}, 8 },
};

Sampi500::Sampi500(std::string const & logName) :
	DeviceTE(DeviceTE::SAMPI500, logName) {

	LOG4CPP_DEBUG(log, "Sampi500(std::string const &)");


}

Sampi500::~Sampi500() {

	LOG4CPP_DEBUG(log, "Destroying the Sampi500 device");

}


inline exitCode
Sampi500::write(t_cmdType type) {

	switch (type) {
	case C54_STX:
		LOG4CPP_DEBUG(log, "WRITE STX");
		break;
	case C54_ACK:
		LOG4CPP_DEBUG(log, "WRITE ACK");
		break;
	default:
		LOG4CPP_WARN(log, "Command not supported");
		return TE_CMD_NOT_SUPPORTED;
	}

	return d_tty->sendSerial(d_teCmd[type].cmd, d_teCmd[type].len);

}

inline unsigned
Sampi500::read(char * resp, int len) {

	LOG4CPP_DEBUG(log, "Reading serial...");

	if ( d_tty->readSerial(resp, len) == OK ) {
		return len;
	}

	// By default we don't read any byte on any error
	return 0;
}

inline exitCode
Sampi500::checksum(const char * buf, unsigned len, char chksum[3], bool tx) {
	unsigned sum = 0;
	unsigned index = 0;

	for ( ; index<len; index++) {
		sum += buf[index];
	}

	// NOTE:  (value % 256) == (value & 0xFF)
	sprintf(chksum, "%02X",  (sum & 0xFF));

	// On trasmission we need to SWAP the two bytes
	if (tx) {
		chksum[0] ^= chksum[1];
		chksum[1] ^= chksum[0];
		chksum[0] ^= chksum[1];
	}

	LOG4CPP_DEBUG(log, "len [%d], sum [%d, 0x%X], checksum [%s]", len, sum, sum, chksum);
	return OK;
}

bool
Sampi500::verifyChecksum(const char * buf, unsigned len) {
	char rxChksum[] = "\0\0\0";
	char myChksum[] = "\0\0\0";
	bool match = false;

#ifdef DARICOMDEBUG
	if ( d_forceDownload ) return true;
#endif

	memcpy(rxChksum, buf+len-2, 2);
	checksum(buf, len-2, myChksum, false);

	LOG4CPP_DEBUG(log, "Checksum rx [%s], computed [%s]", rxChksum, myChksum);

	match = strncasecmp(rxChksum, myChksum, 2);
	if ( match ) {
		LOG4CPP_WARN(log, "Checksum mismatch on TE data download");
	}

	return !match;

}

inline Sampi500::t_teResps
Sampi500::checkResponce(const char * buf) {

	// checking NAK
	if ( buf[0] == NAK ) {
		switch (buf[7]) {
			case '1':
				LOG4CPP_WARN(log, "Checksum errro");
				return NAK_CHECKSUM_ERROR;
			case '2':
				LOG4CPP_WARN(log, "Missing terminator (0x0D)");
				return NAK_MISSING_TERMINATOR;
			case '3':
				LOG4CPP_INFO(log, "No new events to download");
				return NAK_NO_NEW_EVENT;
			case '8':
				LOG4CPP_WARN(log, "Comunication start errro");
				return NAK_START_ERROR;
			default:
				LOG4CPP_WARN(log, "Unknowen NAK received");
				return NAK_UNKNOWEN;
		}
	}

	return ACK;

}

inline std::string
Sampi500::verboseNak(t_teResps resp) {

	switch (resp) {
		case NAK_NO_NEW_EVENT:
			return "No new events to download";
		case NAK_CHECKSUM_ERROR:
			return "Checksum error on received message";
		case NAK_MISSING_TERMINATOR:
			return "CR missing";
		case NAK_START_ERROR:
			return "Communication start char error";
		default:
			return "Unknowed NAK reason";
	}

}

inline DeviceTE::t_eventType
Sampi500::getEventType(char * buf) {
	char eventCode;

	eventCode = buf[8];

	LOG4CPP_DEBUG(log, "Event type: [%c]", eventCode);

	switch (eventCode) {
		case 'A':
			return POWERUP;
		case 'S':
			return SHUTDOWN;
		case 'C':
			return LOAD;
		case 'O':
			return DOWNLOAD;
		default:
			return UNDEF;
	}

}
/*
exitCode
Sampi500::downloadEvents(t_eventList & eventList) {
	unsigned len, index;
	short retry = DEVICE_TE_SAMPI500_RX_RETRY+1;
	char buf[DEVICE_TE_SAMPI500_RX_BUFFER];
	Sampi500::t_teResps resp;
	unsigned device;
	unsigned remRecords;
	t_event * event;
	bool readOk = false;
	EVENT_CODE(evtLable);

	LOG4CPP_DEBUG(log, "Sampi500::downloadEvents(t_eventList & eventList)");

	do {
		if ( ! (--retry) )
			break;

		// Sending download command #54
		write(C54_STX);
		// Reading first responce and verifying checksum
		len = read(buf, DEVICE_TE_SAMPI500_RX_BUFFER);

		//NOTE if len==0: nothing readed or device not responding...
		if (len)
			readOk = verifyChecksum(buf, len-1);

	} while ( !readOk );

	if (!retry) {
		LOG4CPP_WARN(log, "TE not responding, retrying later");
		return TE_NOT_RESPONDING;
	}

	// NOTE: if this is not an ACK we should really come back and FIX THE CODE
	resp = checkResponce(buf);
	if ( resp > NAK_NO_NEW_EVENT ) {
		LOG4CPP_FATAL(log, "Communication error: %s", verboseNak(resp).c_str());
		return TE_NAK_RECEIVED;
	}

	if ( resp == NAK_NO_NEW_EVENT ) {
		LOG4CPP_DEBUG(log, "No new events to download from SAMPI500 device/s");
		return TE_NO_NEW_EVENTS;
	}

	// Now we should get the number of following records...
	sscanf(buf+1, "%3u%4u", &device, &remRecords);
	LOG4CPP_INFO(log, "Downloading %u new events from SAMPI500 device [%u]", remRecords, device);

	// Sending ACK
	write(C54_ACK);

	// Downloading events
	index = 0;
	while ( ++index <= remRecords ) {

		// Reading next message
		len = read(buf, DEVICE_TE_SAMPI500_RX_BUFFER);
		retry = DEVICE_TE_SAMPI500_RX_RETRY;
		while ( !len && --retry) {
			LOG4CPP_DEBUG(log, "Retrying download...");
			len = read(buf, DEVICE_TE_SAMPI500_RX_BUFFER);
		};
		if (!len) {
			LOG4CPP_WARN(log, "Device not responding: download ABORTED");
			return TE_RESTART_DOWNLOAD;
		}
		// Verifying message checksum
		if ( !verifyChecksum(buf, len-1) ) {
			LOG4CPP_WARN(log, "Checksum error on downloading message [%d]", index);
//TODO Aborting download to restart from this record
			return TE_RESTART_DOWNLOAD;
		}

		// Queue event (removing all un-necessary fields)
		event = new t_event();
		event->type = getEventType(buf);
		buf[len-3] = 0; //Removing leading checksum and line terminator....

//NOTE: we don't remove the leading address, record number and event type because is used by the
//	RAW event formatter
// 		event->event = std::string(buf+4);
		event->event = std::string(buf);

		LOG4CPP_DEBUG(log, "EVT [%c], [%s]",
					evtLable[event->type],
					(event->event).c_str());

		eventList.push_back(event);

		// Sending ACK
		write(C54_ACK);

		LOG4CPP_DEBUG(log, "%d more event/s to download", remRecords-index);
	}

	return OK;

}
*/

exitCode
Sampi500::downloadEvents(t_eventList & eventList) {
	unsigned len;
	unsigned short retry;
	char buf[DEVICE_TE_SAMPI500_RX_BUFFER];
	Sampi500::t_teResps resp;
	unsigned downloadedRecords = 0;
	t_event * l_event;
	EVENT_CODE(evtLable);
	exitCode result = OK;

	d_tty->detachedMode(false);

	// Sending download command #54
	LOG4CPP_DEBUG(log, "Send command 54");
	write(C54_STX);

	do {
		// Reading next message
		len = read(buf, DEVICE_TE_SAMPI500_RX_BUFFER);
		retry = d_retry+1;
		while ( !len && --retry) {
			LOG4CPP_DEBUG(log, "Retrying download...");
			len = read(buf, DEVICE_TE_SAMPI500_RX_BUFFER);
		};
		if (!len) {
			LOG4CPP_WARN(log, "Device not responding: download ABORTED");
			// If the TE does not respond we wait until the next download time
			result = TE_NO_NEW_EVENTS;
			goto download_end;
		}

		// Verifying message checksum
		if ( !verifyChecksum(buf, len-1) ) {
			LOG4CPP_WARN(log, "Checksum error on downloading message [%d]", downloadedRecords+1);
			//TODO Aborting download to restart from this record
			result = TE_RESTART_DOWNLOAD;
			goto download_end;
		}

		// NOTE: if this is not an ACK we should really come back and FIX THE CODE
		resp = checkResponce(buf);
		if ( resp > NAK_NO_NEW_EVENT ) {
			LOG4CPP_FATAL(log, "Communication error: %s", verboseNak(resp).c_str());
			result = TE_NAK_RECEIVED;
			goto download_end;
		}

		if ( resp == NAK_NO_NEW_EVENT ) {
			LOG4CPP_DEBUG(log, "No more events from SAMPI500");
			if (downloadedRecords) {
				LOG4CPP_DEBUG(log, "Notify events to be uploaded");
				result = OK;
				goto download_end;
			} else {
				result = TE_NO_NEW_EVENTS;
				goto download_end;
			}
		}

		// Queue event (removing all un-necessary fields)
		l_event = new t_event();
		l_event->type = getEventType(buf);
		buf[len-3] = 0; //Removing leading checksum and line terminator....

	//NOTE: we don't remove the leading address, record number and event type because is used by the
	//	RAW event formatter
	// 	l_event->event = std::string(buf+4);
		l_event->event = std::string(buf);
		LOG4CPP_DEBUG(log, "EVT [%c], [%s]",
				evtLable[l_event->type],
				(l_event->event).c_str());

		eventList.push_back(l_event);
		downloadedRecords++;

		// Sending ACK
		write(C54_ACK);

	} while (true);

	LOG4CPP_INFO(log, "%d new events downloaded", downloadedRecords);

download_end:

	d_tty->detachedMode(true);
	return result;

}




exitCode Sampi500::saveEvent(t_eventList & eventList, std::string msg) {
	return OK;
}

exitCode
Sampi500::formatEvent(t_event const & p_event, comsys::Command & cmd) {
	exitCode result = OK;
	EVENT_CODE(evtLable);

//NOTE: event.event has the initial address and event type too...
//	this is used by the default (RAW) formatter, others formatter should ignore
//	the first 4 bytes...

	LOG4CPP_DEBUG(log, "===>   TE event to format %c [%s]",
				evtLable[p_event.type],
				p_event.event.c_str());

// 	switch ( event.type ) {
// 		case DeviceTE::POWERUP:
// 			result = formatEvent_PowerUp(event.event.c_str(), cmd);
// 			break;
// 		case DeviceTE::SHUTDOWN:
// 			result = formatEvent_Shutdown(event.event.c_str(), cmd);
// 			break;
// 		case DeviceTE::LOAD:
// 			result = formatEvent_Load(event.event.c_str(), cmd);
// 			break;
// 		case DeviceTE::DOWNLOAD:
// 			result = formatEvent_Download(event.event.c_str(), cmd);
// 			break;
// 		default:
			result = formatEvent_Raw(p_event.event.c_str(), cmd);
			return TE_EVENT_NOT_PARSED;
// 	}

	return result;

}

// Any RAW record downloaded by the TE
inline exitCode
Sampi500::formatEvent_Raw(const char * evt, comsys::Command & cmd) {
// 	const char *evt = event.event.c_str();
	std::ostringstream sbuf("");

	sbuf.str("");
	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x29;
	cmd.setParam( "dist_evtType", sbuf.str());
// 	cmd.setParam("dist_evtType", 0x29);
	cmd.setParam("dist_evtData", evt+1);
	LOG4CPP_DEBUG(log, "RAW Sampi500_500, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				evt+4);

	return OK;
}

// Record downloaded: ADDTTTTTTTTCCCCGGGGGGGGBBBBBBBB
//                       \------/    \------/\------/
//                        DTime       Gasoil  Benzina
// Format: nn{ccvvvvvvvv}
inline exitCode
Sampi500::formatEvent_PowerUp(const char * evt, comsys::Command & cmd) {
	// Only 'Gasolio' (00) and 'Benzina' (02)
	char buf[] = "0200XXXXXXXX02YYYYYYYY\0";
	std::ostringstream sbuf("");

	memcpy(buf+4, evt+8+15, 8);
	memcpy(buf+14, evt+8+23, 8);

	sbuf.str("");
	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x02;
	cmd.setParam( "dist_evtType", sbuf.str());
// 	cmd.setParam("dist_evtType", 0x02);
	cmd.setParam("dist_evtData", buf);
	LOG4CPP_DEBUG(log, "A   Sampi500_500, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				buf);

	return OK;
/*
// 	std::string format;
// 	unsigned short index;
// 	std::ostringstream formattedCounters("");
// 	std::ostringstream formattedRecord("");
//
//
// 	format = d_config.param("device_te_recordFormat_A", DEVICETE_DEFAULT_RECORDFORMAT_A, true);
//
// 	LOG4CPP_DEBUG(log, "Record A, using format [%s]", format.c_str());
//
// 	index = 0; setfill('0');
// 	while ( index < format.size() ) {
// 		index++;
// 		formattedCounters << setw(2) << hex << format.substr(index,1);
// 		formattedCounters << event.event.substr(15+8*index, 8);
// 	}
//
// 	formattedRecord << setw(2) << index;
// 	formattedRecord << formattedCounters;
//
// 	return formattedRecord.str();*/

}

// Record downloaded: SDDTTTTTTTTRRRRGGGGGGGGBBBBBBBB
//                       \------/    \------/\------/
//                        DTime       Gasoil  Benzina
// nn{ccvvvvvvvv}
inline exitCode
Sampi500::formatEvent_Shutdown(const char * evt, comsys::Command & cmd) {
	// Only 'Gasolio' (00) and 'Benzina' (02)
	char buf[] = "0200XXXXXXXX02YYYYYYYY\0";
	std::ostringstream sbuf("");

	memcpy(buf+4, evt+8+15, 8);
	memcpy(buf+14, evt+8+23, 8);

	sbuf.str("");
	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x05;
	cmd.setParam( "dist_evtType", sbuf.str());
// 	cmd.setParam("dist_evtType", 0x05);
	cmd.setParam("dist_evtData", buf);
	LOG4CPP_DEBUG(log, "S   Sampi500_500, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				buf);

	return OK;
}




// Record downloaded: CDDTTTTTTTTRRRRKKKKKKKKNN[9*Qp]
//                       \------/    \------/\/\----/
//                        DTime       Load#  |   Product Qty
//                                         Compart#
// Qp: QQQQQQQQCCTTTT
// kkkknn{sccqqqqtttt}
inline exitCode
Sampi500::formatEvent_Load(const char * evt, comsys::Command & cmd) {
	unsigned short i;
	int num;
	char buf[] = "KKKKNN1CCQQQQTTTT2CCQQQQTTTT3CCQQQQTTTT4CCQQQQTTTT5CCQQQQTTTT6CCQQQQTTTT7CCQQQQTTTT8CCQQQQTTTT9CCQQQQTTTT\0";
	std::ostringstream sbuf("");

	memcpy(buf, evt+8+19, 4);
	memcpy(buf+4, evt+8+23, 2);
	sscanf(buf+4, "%2d", &num);
	LOG4CPP_DEBUG(log, "Parsing [%d] compartment", num);
	for (i=0; i<num; i++) {
		memcpy(buf+7+(i*11), evt+8+33+(i*14), 2);
		memcpy(buf+9+(i*11), evt+8+29+(i*14), 4);
		memcpy(buf+13+(i*11), evt+8+35+(i*14), 4);
	}
	buf[6+(i*11)] = 0;

	sbuf.str("");
	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x03;
	cmd.setParam( "dist_evtType", sbuf.str());
// 	cmd.setParam("dist_evtType", 0x03);
	cmd.setParam("dist_evtData", buf);
	LOG4CPP_DEBUG(log, "C   Sampi500_500, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				buf);

	return OK;

/*
// 	std::ostringstream formattedCounters("");
// 	std::ostringstream formattedRecord("");
// 	unsigned short nextRecord;
// 	unsigned short index;
//
//
// 	index = 0; setfill('0');
// 	nextRecord = 25+index*14;
// 	while ( nextRecord < event.event.size() ) {
// 		formattedCounters << setw(2) << hex << event.event.substr(nextRecord+8, 2);
// 		formattedCounters << setw(4) << hex << event.event.substr(nextRecord+4, 4);
// 		formattedCounters << setw(4) << hex << event.event.substr(nextRecord+10, 4);
// 		index++;
// 		nextRecord += 14;
// 	}
//
// 	formattedRecord << setw(2) << index;
// 	formattedRecord << formattedCounters;*/

}


// Record downloaded: ODDTTTTTTTTRRRRHHHHHHHHKKKKKKKKLLLLMMMMhhhhhhhhssSSssSS
//                       \------/    \------/\------/
//                        DTime       Dwnl#   Load#
//
// Qp: QQQQQQQQCCTTTT
// hhhhkkkkccppppeeeevvvvvvvvabfgnn{sqqqqtttt}
// IN RECORD:
// OUT RECORD: ccppppeeeevvvvvvvvabfgnn{sqqqqtttt}
inline exitCode
Sampi500::formatEvent_Download(const char * evt, comsys::Command & cmd) {
	unsigned short i;
	char cbuf[] = "HHHHKKKKCCPPPPEEEEVVVVVVVVABFGNNSQQQQTTTTSQQQQTTTTSQQQQTTTTSQQQQTTTTSQQQQTTTTSQQQQTTTTSQQQQTTTTSQQQQTTTTSQQQQTTTT\0";
	std::ostringstream sbuf("");
	unsigned int mask, pos;
	unsigned char num, tmp;

	memcpy(cbuf, evt+8+19, 4);
	memcpy(cbuf+4, evt+8+23, 4);
	memcpy(cbuf+8, evt+8+161, 2);
	memcpy(cbuf+10, evt+8+75, 4);
	memcpy(cbuf+14, evt+8+83, 4);
	memcpy(cbuf+18, evt+8+63, 8);

	memcpy(cbuf+26, evt+8+48, 1);
	memcpy(cbuf+27, evt+8+50, 1);
	memcpy(cbuf+28, evt+8+52, 1);
	memcpy(cbuf+29, evt+8+54, 1);

	sscanf(evt+8+123, "%2d", &mask);
	LOG4CPP_DEBUG(log, "Parsing product mask [0x%02X]", mask);
	for (num=0, pos=0x1, i=0; i<16; i++) {
		LOG4CPP_DEBUG(log, "num [%d], mask [0x%02X], pos [0x%02X], mask&pos [0x%02X]", num, mask, pos, mask&pos);
		if (mask & pos) {
			sprintf(cbuf+32+(num*9), "%1u", (num+1) & 0xF );
			memcpy(cbuf+33+(num*9), evt+8+123+(num*4), 4);
			memcpy(cbuf+37+(num*9), evt+8+87+(num*4), 4);
			num++;
		}
		pos <<= 1;
	}

	tmp = cbuf[32];
	sprintf(cbuf+30, "%02u", num & 0xFF);
	cbuf[32] = tmp;
	*(cbuf+32+(num*9)) = 0;


	sbuf.str("");
	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x04;
	cmd.setParam( "dist_evtType", sbuf.str());
// 	cmd.setParam("dist_evtType", 0x04);
	cmd.setParam("dist_evtData", cbuf);
	LOG4CPP_DEBUG(log, "O   Sampi500_500, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				cbuf);

	return OK;

/*
// 	std::ostringstream formattedCounters("");
// 	std::ostringstream formattedRecord("");
//
// 	setfill('0');
// 	formattedCounters << setw(4) << hex << event.event.substr(71, 4);
// 	formattedCounters << setw(4) << hex << event.event.substr(79, 4);
// 	formattedCounters << setw(8) << hex << event.event.substr(63, 8);
// 	formattedCounters << setw(1) << hex << event.event.substr(49, 1);
// 	formattedCounters << setw(1) << hex << event.event.substr(53, 1);
// 	formattedCounters << setw(1) << hex << event.event.substr(51, 1);
// 	formattedCounters << setw(1) << hex << event.event.substr(55, 1);*/

}



}// namespace device
}// namespace controlbox
