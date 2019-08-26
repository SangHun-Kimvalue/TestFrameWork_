#pragma once
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <iostream>

// Forward function definitions:
struct StreamClientState {
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
	return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
	return env << subsession.mediumName() << "/" << subsession.codecName();
}

#define REQUEST_STREAMING_OVER_TCP False

class m_RTSP_Client : public RTSPClient{
public:
	static m_RTSP_Client* createNew(UsageEnvironment& env,
		char const* rtspURL,
		int verbosityLevel = 0,
		char const* applicationName = NULL,
		portNumBits tunnelOverHTTPPortNum = 0);

	StreamClientState scs;

	//char* SDP;

protected:
	m_RTSP_Client(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
	// called only by createNew();
	virtual ~m_RTSP_Client();
	
private:

};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

