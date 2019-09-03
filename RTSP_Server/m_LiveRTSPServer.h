#pragma once

#include <UsageEnvironment.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>

#include "RTSPServer.hh"
#include "RTSPCommon.hh"
#include "RTSPRegisterSender.hh"
#include "Base64.hh"
#include "m_RTSPServer.h"
//#include "LiveServerMediaSubsession.h"
#include "H264VideoFileServerMediaSubsession.hh"
#include "VideoRTPSink.hh"
#include "H264VideoRTPSource.hh"


#include <string>

class LiveRTSPServer : public m_RTSPServer, public RTSPServer
{
public:

	static LiveRTSPServer* createNew(UsageEnvironment& env, Port ourPort = 554);

	~LiveRTSPServer();

	virtual char const* allowedCommandNames();

	virtual ClientConnection* createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr);
	virtual ClientSession* createNewClientSession(u_int32_t sessionId);

	virtual void Release();
	virtual bool Initialize(int port);
	virtual void Run();
	virtual std::string GetURL();
	virtual void Restart();
	virtual std::string  GetStreamName();

protected:

	char* rtspURLPrefix(int clientSocket = -1) const;
	LiveRTSPServer(UsageEnvironment& env,
		int ourSocket, Port ourPort,
		UserAuthenticationDatabase* authDatabase,
		unsigned reclamationSeconds);


private: // redefined virtual functions
	virtual Boolean isRTSPServer() const;
	char* rtspURL(int clientSocket = -1) const;
	char* rtspURL(const char* serverMediaSession, int clientSocket = -1) const;
	char* rtspURL(ServerMediaSession const* serverMediaSession, int clientSocket = -1) const;

public: // should be protected, but some old compilers complain otherwise
  // The state of a TCP connection used by a RTSP client:
	class RTSPClientSession; // forward
	class RTSPClientConnection : public GenericMediaServer::ClientConnection {
	public:

	protected: // redefined virtual functions:
		virtual void handleRequestBytes(int newBytesRead);

	protected:
		RTSPClientConnection(LiveRTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr);
		virtual ~RTSPClientConnection();

		friend class LiveRTSPServer;
		friend class RTSPClientSession;

		// Make the handler functions for each command virtual, to allow subclasses to reimplement them, if necessary:
		virtual void handleCmd_OPTIONS();
		// You probably won't need to subclass/reimplement this function; reimplement "RTSPServer::allowedCommandNames()" instead.
		virtual void handleCmd_GET_PARAMETER(char const* fullRequestStr); // when operating on the entire server
		virtual void handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr);
		// You probably won't need to subclass/reimplement this function;
		//     reimplement "RTSPServer::weImplementREGISTER()" and "RTSPServer::implementCmd_REGISTER()" instead.
		virtual void handleCmd_bad();
		virtual void handleCmd_notSupported();
		virtual void handleCmd_notFound();
		virtual void handleCmd_sessionNotFound();
		virtual void handleCmd_unsupportedTransport();
		// Support for optional RTSP-over-HTTP tunneling:
		virtual Boolean parseHTTPRequestString(char* resultCmdName, unsigned resultCmdNameMaxSize,
			char* urlSuffix, unsigned urlSuffixMaxSize,
			char* sessionCookie, unsigned sessionCookieMaxSize,
			char* acceptStr, unsigned acceptStrMaxSize);
		Boolean authenticationOK(char const* cmdName, char const* urlSuffix, char const* fullRequestStr);


	protected:
		void resetRequestBuffer();
		void closeSocketsRTSP();
		static void handleAlternativeRequestByte(void*, u_int8_t requestByte);
		void handleAlternativeRequestByte1(u_int8_t requestByte);
		void changeClientInputSocket(int newSocketNum, unsigned char const* extraData, unsigned extraDataSize);

		// Shortcuts for setting up a RTSP response (prior to sending it):
		void setRTSPResponse(char const* responseStr);
		void setRTSPResponse(char const* responseStr, u_int32_t sessionId);
		void setRTSPResponse(char const* responseStr, char const* contentStr);
		void setRTSPResponse(char const* responseStr, u_int32_t sessionId, char const* contentStr);

		LiveRTSPServer& fOurRTSPServer; // same as ::fOurServer
		int& fClientInputSocket; // aliased to ::fOurSocket
		int fClientOutputSocket;
		Boolean fIsActive;
		unsigned char* fLastCRLF;
		unsigned fRecursionCount;
		char const* fCurrentCSeq;
		Authenticator fCurrentAuthenticator; // used if access control is needed
		char* fOurSessionCookie; // used for optional RTSP-over-HTTP tunneling
		unsigned fBase64RemainderCount; // used for optional RTSP-over-HTTP tunneling (possible values: 0,1,2,3)
	};

	// The state of an individual client session (using one or more sequential TCP connections) handled by a RTSP server:
	class RTSPClientSession : public GenericMediaServer::ClientSession {
	protected:
		RTSPClientSession(LiveRTSPServer& ourServer, u_int32_t sessionId);
		virtual ~RTSPClientSession();

		friend class RTSPServer;
		friend class LiveRTSPServer;
		friend class RTSPClientConnection;

		// Make the handler functions for each command virtual, to allow subclasses to redefine them:
		virtual void handleCmd_SETUP(RTSPClientConnection* ourClientConnection,
			char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr);
		virtual void handleCmd_withinSession(RTSPClientConnection* ourClientConnection,
			char const* cmdName,
			char const* urlPreSuffix, char const* urlSuffix,
			char const* fullRequestStr);
		virtual void handleCmd_TEARDOWN(RTSPClientConnection* ourClientConnection,
			ServerMediaSubsession* subsession);
		virtual void handleCmd_PLAY(RTSPClientConnection* ourClientConnection,
			ServerMediaSubsession* subsession, char const* fullRequestStr);
		virtual void handleCmd_PAUSE(RTSPClientConnection* ourClientConnection,
			ServerMediaSubsession* subsession);
		virtual void handleCmd_GET_PARAMETER(RTSPClientConnection* ourClientConnection,
			ServerMediaSubsession* subsession, char const* fullRequestStr);
		//virtual void handleCmd_SET_PARAMETER(RTSPClientConnection* ourClientConnection,
		//	ServerMediaSubsession* subsession, char const* fullRequestStr);
	protected:
		void deleteStreamByTrack(unsigned trackNum);
		void reclaimStreamStates();
		Boolean isMulticast() const { return fIsMulticast; }

		// Shortcuts for setting up a RTSP response (prior to sending it):
		void setRTSPResponse(RTSPClientConnection* ourClientConnection, char const* responseStr) { ourClientConnection->setRTSPResponse(responseStr); }
		void setRTSPResponse(RTSPClientConnection* ourClientConnection, char const* responseStr, u_int32_t sessionId) { ourClientConnection->setRTSPResponse(responseStr, sessionId); }
		void setRTSPResponse(RTSPClientConnection* ourClientConnection, char const* responseStr, char const* contentStr) { ourClientConnection->setRTSPResponse(responseStr, contentStr); }
		void setRTSPResponse(RTSPClientConnection* ourClientConnection, char const* responseStr, u_int32_t sessionId, char const* contentStr) { ourClientConnection->setRTSPResponse(responseStr, sessionId, contentStr); }

	protected:
		LiveRTSPServer& fOurRTSPServer; // same as ::fOurServer
		Boolean fIsMulticast, fStreamAfterSETUP;
		unsigned char fTCPStreamIdCount; // used for (optional) RTP/TCP
		Boolean usesTCPTransport() const { return fTCPStreamIdCount > 0; }
		unsigned fNumStreamStates;
		struct streamState {
			ServerMediaSubsession* subsession;
			int tcpSocketNum;
			void* streamToken;
		} *fStreamStates;
	};

	private:

		friend class RTSPClientConnection;
		friend class RTSPClientSession;

		UserAuthenticationDatabase* fAuthDB;
		UsageEnvironment *env;
		std::string StreamName;
		int StreamCount;
		int RSTP_Port;
		std::string FileName;
};

