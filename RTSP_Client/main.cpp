#include "LiveRTSPClient.h"
static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

int main(int argc, char** argv) {
	// Begin by setting up our usage environment:

	const char* URI = "rtsp://192.168.0.40:8554";
	//const char* URI = "rtsp://admin:1234@192.168.0.70/video1";
	const char* ProgName = "Client_Test";

	Client* rtspClient = new LiveRTSPClient();

	++rtspClientCount;

	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	rtspClient->Initialize(URI, ProgName);
	rtspClient->Run();

	// All subsequent activity takes place within the event loop:

	// This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.
	rtspClient->Release();

	return 0;

}