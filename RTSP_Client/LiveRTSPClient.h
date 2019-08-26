#pragma once

#include "Client.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#define REQUEST_STREAMING_OVER_TCP False
#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

struct StreamClientState {
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

//// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
//UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
//	return env << "[URL:\"" << rtspClient.url() << "\"]: ";
//}
//
//// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
//UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
//	return env << subsession.mediumName() << "/" << subsession.codecName();
//}

class LiveRTSPClient : public Client
{
public:
	LiveRTSPClient();
	~LiveRTSPClient();

	virtual void Release();
	virtual void Run();
	virtual const char* Get_URL();
	virtual const char* Get_Name();
	virtual bool Initialize(const char* URI, const char* ProgName);
	virtual void Restart();
	bool SetLoopSatus(bool Status);


	char* URL;

private:
	int portNumber;
	int httpTunnelingPort;
	//FFmpegH264Encoder * m_Encoder;
	char quit;

	TaskScheduler    *scheduler;
	UsageEnvironment *env;
	RTSPClient	 *m_Client;


	char eventLoopWatchVariable;
};


