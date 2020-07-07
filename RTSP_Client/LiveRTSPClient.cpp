#include "LiveRTSPClient.h"

LiveRTSPClient* m_Live;
unsigned long m_startTime;

//void shutdownStream(RTSPClient* rtspClient);
//void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterOPTION(RTSPClient* rtspClient, int resultCode, char* resultString);

char* createSessionString(char const* sessionId) {
	char* sessionStr;
	if (sessionId != NULL) {
		sessionStr = new char[20 + strlen(sessionId)];
		sprintf(sessionStr, "Session: %s\r\n", sessionId);
	}
	else {
		sessionStr = strDup("");
	}
	return sessionStr;
}

Boolean isAbsoluteURL(char const* url) {
	// Assumption: "url" is absolute if it contains a ':', before any
	// occurrence of '/'
	while (*url != '\0' && *url != '/') {
		if (*url == ':') return True;
		++url;
	}

	return False;
}

//void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
//	Boolean success = False;
//
//	std::cout <<  "Started playing session" << std::endl;
//	m_startTime = timeGetTime();
//	//m_Live->m_SAlive = true;
//	//m_Live->GetParameter();
//
//	std::cout << "\nProgram Name : " <<m_Live->Get_Name() << std::endl;
//	std::cout << "\nURL : " <<m_Live->Get_URL() << std::endl;
//	std::cout << "\nStatus : " <<m_Live->Get_Status() << std::endl;
//
//	//m_Live->startAlive();
//	success = True;
//	shutdownStream((RTSPClient*)rtspClient);
//	delete[] resultString;
//
//}
//
//void continueAfterOPTION(RTSPClient* rtspClient, int resultCode, char* resultString) {
//
//	m_Live->MakeRequest("DESCRIBE");
//	//rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
//
//}
//
//void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
//	
//	//m_Live->GetParameter();
//	m_Live->MakeRequest("PLAY");
//	
//}
//
//void shutdownStream(RTSPClient* rtspClient) {
//
//	m_Live->MakeRequest("TEARDOWN");
//	std::cout << "Closing the stream.\n" << std::endl;
//	//Medium::close(rtspClient);
//	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
//	m_Live->SetLoopSatus(false);
//	
//}
//
//void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
//
//
//	if (resultCode != 0) {
//		std::cerr<< "Failed to get a SDP description: " << resultString << std::endl;
//		delete[] resultString;
//		return;
//	}
//
//	rtspClient->sendSetupCommand(continueAfterSETUP);
//
//	return;
//
//}

LiveRTSPClient* LiveRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
	return new LiveRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

LiveRTSPClient::LiveRTSPClient(UsageEnvironment& env, char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
	: RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1), env(&env)
	, Use_Getparam(false), Pharse_SDP(false), ClientPort(0), ServerPort(0), eventLoopWatchVariable(0), havesession(false)
	, state(SERROR), handlestate(SERROR), Looping(true)
{

	m_URL = strDup(rtspURL);
	m_ProgName = strDup(applicationName);
	//GetParent(parent);
	
	start();
	
	Sleep(10);
}

LiveRTSPClient::~LiveRTSPClient()
{
	Release();
}

Boolean LiveRTSPClient::handleDESCRIBEResponse(char const* sdp) {
	
		if (sdp == "") {
			std::cerr << "Failed to get a SDP description: " << std::endl;
			return False;
		}

		SDP = strDup(sdp);
		std::cout << "Got a SDP description:\n" << SDP << std::endl;

		// Create a media session object from this SDP description:
		scs.session = MediaSession::createNew(*env, SDP);
		

		if (scs.session == NULL) {
			std::cout << "Failed to create a MediaSession object from the SDP description: " << std::endl;
			return False;
		}
		else if (!scs.session->hasSubsessions()) {
			std::cout << "This session has no media subsessions (i.e., no \"m=\" lines)\n" << std::endl;
			//return False;
		}
		
		// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
		// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
		// (Each 'subsession' will have its own data source.)
		scs.iter = new MediaSubsessionIterator(*scs.session);
		havesession = true;
		const char* track = scs.session->controlPath();
		//const char* track = scs.session->
		
		sessiontrack = track;
	
		std::string sessionDescription = scs.session->sessionDescription();
		std::string mediaSessionType = scs.session->mediaSessionType();
		std::string sessionName = scs.session->sessionName();
		//ClientPort = scs.subsession->clientPortNum();
		//std::string Cname = scs.session->CNAME();

		if (sessionDescription != "" && mediaSessionType != "" && sessionName != "") {
			Pharse_SDP = true;
		}
	
		handlestate = SDESCRIBE;

	return True;
}

Boolean LiveRTSPClient::handleOPTIONSResponse(char const* publicParamsStr) {

	std::string param = strDup(publicParamsStr);
	int find = 0;
	
	find = param.find("GET_PARAMETER");

	if (find != 0) {
		Use_Getparam = true;
	}


	handlestate = SOPTION;

	return True;
}

Boolean LiveRTSPClient::handleSETUPResponse(char const* sessionParamsStr, char const* transportParamsStr, Boolean streamUsingTCP) {

	if (sessionParamsStr == "" || sessionParamsStr == NULL) {
		return False;
	}

	char* serverAddressStr = NULL;
	unsigned char rtpChannelId, rtcpChannelId;
	rtpChannelId = rtcpChannelId = 0xFF;
	Boolean res = parseTransportParams(transportParamsStr, serverAddressStr, ServerPort, rtpChannelId, rtcpChannelId);
	if (res == False)
		return res;

	char* temp = strDup(sessionParamsStr);
	std::string SessionId = strtok(temp, ";");
	strtok(NULL, "=");
	std::string timeouttemp = strtok(NULL, "");

	SetfLastSessionId(SessionId.c_str());

	timeout = std::stoi(timeouttemp);

	SetSessionTimeoutParameter(timeout);

	handlestate = SSETUP;

	return True;
 }

Boolean LiveRTSPClient::handlePLAYResponse(MediaSession& session, char const* scaleParamsStr, const char* speedParamsStr,
	char const* rangeParamsStr, char const* rtpInfoParamsStr) {
	handlestate = SPLAY;
	return True;
 }

Boolean LiveRTSPClient::handlePLAYResponse(MediaSession& session, MediaSubsession& subsession,
	char const* scaleParamsStr, const char* speedParamsStr,
	char const* rangeParamsStr, char const* rtpInfoParamsStr) {
	handlestate = SPLAY;
	return True;
 }

void LiveRTSPClient::SetTrack(const char* input) {
	sessiontrack = input;
}

void LiveRTSPClient::SetURL(const char* input) {

	m_URL = const_cast<char*>(input);
}


void LiveRTSPClient::CompareCommand(std::string Command) {

	if (Command == "OPTIONS") {
		state = SOPTION;
	}
	else if (Command == "DESCRIBE") {
		state = SDESCRIBE;
	}
	else if (Command == "SETUP") {
		state = SSETUP;
	}
	else if (Command == "PLAY") {
		state = SPLAY;
	}
	else if (Command == "TEARDOWN") {
		state = SEXIT;
	}
	else {
		state = SERROR;
	}
}

bool LiveRTSPClient::MakeRequest(const char* Command) {

	CompareCommand(Command);

	switch (state) {

	case SOPTION: {

		sendOptionsCommand();
		break;
	} 
	case SDESCRIBE: {

		sendDescribeCommand();
		break;
	}
	case SSETUP: {
		
		std::string URL = m_URL;

		if (Pharse_SDP == true) {
			sendSetupCommand();
			scs.session->sessionDescription();
			scs.session->mediaSessionType();
			scs.session->sessionName();
		}
		else {
			SetTrack("track1");
			SetURL("URL");
			sendSetupCommand();
		}
		break;
	}
	case SPLAY: {

		sendPlayCommand();
		break;
	}   
	case SEXIT: {

		sendTeardownCommand();
		break;
	}
	case SERROR: {

		std::cerr << "Error Command" << std::endl;
		break;
		return false;
	}
	}
	int count = 0;
	while (1) {

		if (state == handlestate) {
			return true;
		}
		else if (count > 500) {
			return false;
		}
		count++;
		Sleep(1);
	}

	return true;
}

Boolean LiveRTSPClient::setRequestFields(RequestRecord* request,
	char*& cmdURL, Boolean& cmdURLWasAllocated,
	char const*& protocolStr,
	char*& extraHeaders, Boolean& extraHeadersWereAllocated) 
{
	float originalScale;
	// Set various fields that will appear in our outgoing request, depending upon the particular command that we are sending.

	if (strcmp(request->commandName(), "DESCRIBE") == 0) {
		extraHeaders = (char*)"Accept: application/sdp\r\n";
	}
	else if (strcmp(request->commandName(), "OPTIONS") == 0) {
		// If we're currently part of a session, create a "Session:" header (in case the server wants this to indicate
		// client 'liveness); this makes up our 'extra headers':
		extraHeaders = createSessionString(sessionId);
		extraHeadersWereAllocated = True;
	}
	else if (strcmp(request->commandName(), "ANNOUNCE") == 0) {
		extraHeaders = (char*)"Content-Type: application/sdp\r\n";
	}
	else if (strcmp(request->commandName(), "SETUP") == 0) {
		MediaSubsession& subsession = *request->subsession();
		Boolean streamUsingTCP = (request->booleanFlags() & 0x1) != 0;
		Boolean streamOutgoing = (request->booleanFlags() & 0x2) != 0;
		Boolean forceMulticastOnUnspecified = (request->booleanFlags() & 0x4) != 0;

		std::string URL = cmdURL;
		int Ssize = URL.length();
		if (sessiontrack == "*")
			sessiontrack = "track1";
		suffix = sessiontrack;

		//for (int i = Ssize; i > 7; --i) {
		//	if (URL[i] == ':') {
		//
		//		URL.erase(i);
		//		URL = URL + suffix;
		//		break;
		//	}
		//}

		prefix = URL.c_str();

		if (isAbsoluteURL(suffix.c_str())) {
			prefix = separator = "";
		}
		else {
			unsigned prefixLen = prefix.length();
			separator = (prefixLen == 0 || prefix[prefixLen - 1] == '/' || suffix[0] == '/') ? "" : "/";
		}

		char const* transportFmt;
		transportFmt = "Transport: RAW/RAW/UDP%s%s%s=%d-%d\r\n";

		char const *urltemp = prefix.c_str();
		char const *separatortemp = separator.c_str();
		char const *suffixtemp = suffix.c_str();

		cmdURL = new char[strlen(urltemp) + strlen(separatortemp) + strlen(suffixtemp) + 1];
		cmdURLWasAllocated = True;
		
		sprintf(cmdURL, "%s%s%s", urltemp, separatortemp, suffixtemp);

		if (scs.session != NULL) {
			if (scs.session->hasSubsessions())
				originalScale = 0;//originalScale = request->subsession()->scale();
			else
				originalScale = 0;
		}
		else {
			originalScale = 0;
		}

		// Construct a "Transport:" header.
		char const* transportTypeStr;
		char const* modeStr = streamOutgoing ? ";mode=receive" : "";
		// Note: I think the above is nonstandard, but DSS wants it this way
		char const* portTypeStr;
		portNumBits rtpNumber, rtcpNumber;
	
		// normal RTP streaming
		//unsigned connectionAddress = subsession.connectionEndpointAddress();
		Boolean requestMulticastStreaming = False;
		//= IsMulticastAddress(connectionAddress) || (connectionAddress == 0 && forceMulticastOnUnspecified);
		transportTypeStr = requestMulticastStreaming ? ";multicast" : ";unicast";
		portTypeStr = requestMulticastStreaming ? ";port" : ";client_port";
		if (ClientPort != 0) {
			rtpNumber = ClientPort++;
			rtcpNumber = ClientPort++;
		}
		else {
			rtpNumber = 0;
			rtcpNumber = 0;
		}

		unsigned transportSize = strlen(transportFmt)
			+ strlen(transportTypeStr) + strlen(modeStr) + strlen(portTypeStr) + 2 * 5 /* max port len */;
		char* transportStr = new char[transportSize];
		sprintf(transportStr, transportFmt,
			transportTypeStr, modeStr, portTypeStr, rtpNumber, rtcpNumber);

		// When sending more than one "SETUP" request, include a "Session:" header in the 2nd and later commands:
		char* sessionStr = createSessionString(sessionId);

		// Optionally include a "Blocksize:" string:
		const char* blocksizeStr = "";// createBlocksizeString(streamUsingTCP);

		//The "Transport:" and "Session:" (if present) and "Blocksize:" (if present) headers
		//make up the 'extra headers':
		extraHeaders = new char[transportSize + strlen(sessionStr) + strlen(blocksizeStr)];
		extraHeadersWereAllocated = True;
		sprintf(extraHeaders, "%s%s%s", transportStr, sessionStr, blocksizeStr);

		delete[] transportStr; 
		delete[] sessionStr; 
	}
	else { // "PLAY", "PAUSE", "TEARDOWN", "RECORD", "SET_PARAMETER", "GET_PARAMETER"
	// First, make sure that we have a RTSP session in progress
		if (sessionId == NULL && (strcmp(request->commandName(), "PLAY") != 0) && (strcmp(request->commandName(), "TEARDOWN") != 0)) {
			envir().setResultMsg("No RTSP session is currently in progress\n");
			return False;
		}

		std::string URL = cmdURL;
		int Ssize = URL.length();
		if (sessiontrack == "*")
			sessiontrack = "track1";
		suffix = sessiontrack;

		for (int i = Ssize; i > 7; --i) {
			if (URL[i] == ':') {

				URL.erase(i);
				URL = URL + suffix;
				break;
			}
		}

		prefix = URL.c_str();

		if (isAbsoluteURL(suffix.c_str())) {
			prefix = separator = "";
		}
		else {
			unsigned prefixLen = prefix.length();
			separator = (prefixLen == 0 || prefix[prefixLen - 1] == '/' || suffix[0] == '/') ? "" : "/";
		}

		char const *urltemp = prefix.c_str();
		char const *separatortemp = separator.c_str();
		char const *suffixtemp = suffix.c_str();

		cmdURL = new char[strlen(urltemp) + strlen(separatortemp) + strlen(suffixtemp) + 1];
		cmdURLWasAllocated = True;

		sprintf(cmdURL, "%s%s%s", urltemp, separatortemp, suffixtemp);

		if (scs.session != NULL) {
			if (scs.session->hasSubsessions())
				originalScale = 0; //originalScale = request->subsession()->scale();
			else
				originalScale = 0;
		}
		else {
			originalScale = 0;
		}

		if (strcmp(request->commandName(), "PLAY") == 0) {
			// Create possible "Session:", "Scale:", "Speed:", and "Range:" headers;
			// these make up the 'extra headers':
			char* sessionStr = createSessionString(sessionId);
			extraHeaders = new char[strlen(sessionStr) + 1];
			extraHeadersWereAllocated = True;
			sprintf(extraHeaders, "%s", sessionStr/*, speedStr, rangeStr*/);
			delete[] sessionStr; ; //delete[] speedStr; delete[] rangeStr;
		}
		else {
			// Create a "Session:" header; this makes up our 'extra headers':
			extraHeaders = createSessionString(sessionId);
			extraHeadersWereAllocated = True;
		}
	}

	return True;
}

unsigned LiveRTSPClient::sendPlayCommand(responseHandler* responseHandler) {
	return RTSPClient::sendRequest(new RequestRecord(++fCSeq, "PLAY", responseHandler, 0));
}

unsigned LiveRTSPClient::sendPlayCommand(Authenticator* authenticator) {
	if (fCurrentAuthenticator < authenticator) fCurrentAuthenticator = *authenticator;
	RTSPClient::sendRequest(new RequestRecord(++fCSeq, "PLAY"));

	return 0;
}

unsigned LiveRTSPClient::sendSetupCommand(Authenticator* authenticator) {
	if (fCurrentAuthenticator < authenticator) fCurrentAuthenticator = *authenticator;
	RTSPClient::sendRequest(new RequestRecord(++fCSeq, "SETUP"));

	return 0;
}

unsigned LiveRTSPClient::sendTeardownCommand(Authenticator* authenticator) {
	if (fCurrentAuthenticator < authenticator) fCurrentAuthenticator = *authenticator;
	RTSPClient::sendRequest(new RequestRecord(++fCSeq, "TEARDOWN"));

	return 0;
}

unsigned LiveRTSPClient::sendOptionsCommand(Authenticator* authenticator) {
	RTSPClient::sendRequest(new RequestRecord(++fCSeq, "OPTIONS"));
	return 0;
}

unsigned LiveRTSPClient::sendDescribeCommand(Authenticator* authenticator) {
	if (fCurrentAuthenticator < authenticator) fCurrentAuthenticator = *authenticator;
	return sendRequest(new RequestRecord(++fCSeq, "DESCRIBE"));
}

//unsigned LiveRTSPClient::sendGetParameterCommand() {
//	//if (fCurrentAuthenticator < authenticator) fCurrentAuthenticator = *authenticator;
//	//MediaSession& session = nullptr;
//	responseHandler* responseHandler;
//	char const* parameterName = "";
//	// We assume that:
//	//    parameterName is NULL means: Send no body in the request.
//	//    parameterName is "" means: Send only \r\n in the request body.  
//	//    parameterName is non-empty means: Send "<parameterName>\r\n" as the request body.  
//	unsigned parameterNameLen = parameterName == NULL ? 0 : strlen(parameterName);
//	char* paramString = new char[parameterNameLen + 3]; // the 3 is for \r\n + the '\0' byte
//	if (parameterName == NULL) {
//		paramString[0] = '\0';
//	}
//	else {
//		sprintf(paramString, "%s\r\n", parameterName);
//	}
//
//	unsigned result = RTSPClient::sendRequest(new RequestRecord(++fCSeq, "GET_PARAMETER"));
//	//unsigned result = sendRequest(new RequestRecord(++fCSeq, "GET_PARAMETER", responseHandler, &session, NULL, False, 0.0, 0.0, 0.0, paramString));
//	delete[] paramString;
//	return result;
//}

void LiveRTSPClient::Restart() {

	const char* URL_temp = Get_URL();
	const char* Name_temp = Get_Name();

	Release();

	bool res = Initialize();
	if (res == true) {
		Run();
	}
	else {
		std::cerr << "\nInitialize Error" << std::endl;
	}
}

void LiveRTSPClient::Release() {

	eventLoopWatchVariable = 1;

	if (AliveThread.joinable())
		AliveThread.join();

}

const char* LiveRTSPClient::Get_URL() {
	
	if (m_URL == url())
		return url();
	else
		return m_URL;
}

const char* LiveRTSPClient::Get_Name() {
	return getprogName();
}

const char* LiveRTSPClient::Get_Status() {

	if (eventLoopWatchVariable == 0)
		return "Alive";
	else
		return "Pause";
}

const char* LiveRTSPClient::Get_SDP() {
	
	const char* temp_sdp = SDP;
	if (temp_sdp == "") {
		return "Nothing";
	}

	return temp_sdp;
}

bool LiveRTSPClient::Initialize() {

	m_Live = this;
	eventLoopWatchVariable = 0;

	return true;
}

////스레드 실행
//bool LiveRTSPClient::KeepAlive() {
//	
//	static int m_count;
//	timeout;
//
//
//	while (eventLoopWatchVariable == 0) {
//		if (m_SAlive == true) {
//
//			if (timeGetTime() >= (m_startTime + 5000)) {
//				m_startTime = timeGetTime();
//			}
//
//		}
//
//		Sleep(10);
//	}
//	
//	if (m_SAlive == true)
//		return true;
//	else
//		return false;
//}

void LiveRTSPClient::start() {

	AliveThread = std::thread(&LiveRTSPClient::Run, this);
}

void LiveRTSPClient::stop() {

	Looping = false;
}

void LiveRTSPClient::Run() {

	Initialize();
	
	eventLoopWatchVariable = 0;
	
	env->taskScheduler().doEventLoop(&eventLoopWatchVariable);

}

const unsigned LiveRTSPClient::Get_Timeout() {

	return timeout;
}

bool LiveRTSPClient::SetLoopSatus(bool Status) {
	
	if (Status == true) {
		eventLoopWatchVariable = 0;
	}
	else {
		eventLoopWatchVariable = 1;
		m_SAlive = false;
	}

	return Status;
}