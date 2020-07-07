#include "LiveRTSPClient.h"

int main(int argc, char** argv) {
	// Begin by setting up our usage environment:

	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	//const char* URI = "";
	//const char* URI = "rtsp://192.168.0.40/0";
	const char* URI = "rtsp://admin:1234@192.168.0.73/video1";
	const char* ProgName = "Client_Test";
	RTSP_Client* rtspClient = LiveRTSPClient::createNew(*env, URI, RTSP_CLIENT_VERBOSITY_LEVEL, ProgName, 0);

	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	//rtspClient->Initialize(URI, ProgName);

	rtspClient->MakeRequest("OPTIONS");
	rtspClient->MakeRequest("DESCRIBE");
	rtspClient->MakeRequest("SETUP");
	//rtspClient->MakeRequest("PLAY");
	//rtspClient->MakeRequest("TEARDOWN");

	// All subsequent activity takes place within the event loop:


	system("pause");
	// This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.
	
	delete rtspClient;
	env->reclaim();
	return 0;

}