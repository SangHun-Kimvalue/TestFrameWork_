/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2016, Live Networks, Inc.  All rights reserved
// A demo application, showing how to create and run a RTSP client (that can potentially receive multiple streams concurrently).
//
// NOTE: This code - although it builds a running application - is intended only to illustrate how to develop your own RTSP
// client application.  For a full-featured RTSP client application - with much more functionality, and many options - see
// "openRTSP": http://www.live555.com/openRTSP/

#include "m_RTSP_Client.h"

// The main streaming routine (for each "rtsp://" URL):



int main(int argc, char** argv);
//void setupNextSubsession(RTSPClient* rtspClient);
//void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);
void shutdownStream(m_RTSP_Client* rtspClient);
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
//void streamTimerHandler(void* clientData);

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
	Boolean success = False;
	
	UsageEnvironment& env = rtspClient->envir(); // alias

	env << *rtspClient << "Started playing session";

	success = True;
	shutdownStream((m_RTSP_Client*)rtspClient);
	delete[] resultString;

}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {

	rtspClient->sendPlayCommand(continueAfterPLAY);
}

void shutdownStream(m_RTSP_Client* rtspClient) {
	UsageEnvironment& env = rtspClient->envir(); // alias

	rtspClient->sendTeardownCommand(NULL);
	env << *rtspClient << "Closing the stream.\n";
	Medium::close(rtspClient);
	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

	if (--rtspClientCount == 0) {
		// The final stream has ended, so exit the application now.
		// (Of course, if you're embedding this code into your own application, you might want to comment this out,
		// and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
		//exit(-1);
		rtspClient->SetLoopSatus(false);
	}
}

// Implementation of the RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		StreamClientState& scs = ((m_RTSP_Client*)rtspClient)->scs; // alias

		if (resultCode != 0) {
			env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
			delete[] resultString;
			break;
		}

		char* const sdpDescription = resultString;
		rtspClient->SDP = resultString;
		//env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

		scs.session = MediaSession::createNew(env, sdpDescription);
		if (scs.session == NULL) {
			env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
			break;
		}

		rtspClient->sendSetupCommand(continueAfterSETUP);

		return;
	} while (0);

	// An unrecoverable error occurred with this stream.
	//shutdownStream(rtspClient);

}

//void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL) {
//	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
//	// to receive (even if more than stream uses the same "rtsp://" URL).
//	ourRTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
//	if (rtspClient == NULL) {
//		env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
//		return;
//	}
//
//	++rtspClientCount;
//
//	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
//	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
//	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
//
//	rtspClient->sendOptionsCommand(0);
//	rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
//}

m_RTSP_Client* m_RTSP_Client::createNew(char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
	return new m_RTSP_Client(rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

bool m_RTSP_Client::SetLoopSatus(bool Status) {

}

m_RTSP_Client::m_RTSP_Client(char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
	: RTSPClient(*env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
	eventLoopWatchVariable = 0;
}

const char* m_RTSP_Client::Get_URL() const {
	return url();
}

const char* m_RTSP_Client::Get_Name() const {
	return getprogName();
}

void m_RTSP_Client::Initialize() {
	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);

	m_RTSP_Client::createNew(URI, RTSP_CLIENT_VERBOSITY_LEVEL, ProgName);
	if (rtspClient == NULL) {
		std::cerr << "\nRTSP_URL Error" << std::endl;
		return ;
	}
}

void m_RTSP_Client::Run() {
	sendOptionsCommand();
	sendDescribeCommand(continueAfterDESCRIBE);

	env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
}

void m_RTSP_Client::Release() {

	eventLoopWatchVariable = 1;
	env->reclaim();
	delete scheduler; scheduler = NULL;
	
}


m_RTSP_Client::~m_RTSP_Client() {

	Release();
}

