#pragma once

#include "GenericMediaServer.hh"
#include "RTSPCommon.hh"
#include "RTSPRegisterSender.hh"
#include "Base64.hh"
#include <GroupsockHelper.hh>

class Connect_Handler : public GenericMediaServer
{
public:

	static Connect_Handler* createNew(UsageEnvironment& env, Port ourPort = 554,
		UserAuthenticationDatabase* authDatabase = NULL,
		unsigned reclamationSeconds = 65);

	Connect_Handler(UsageEnvironment& env,
		int ourSocket, Port ourPort,
		UserAuthenticationDatabase* authDatabase,
		unsigned reclamationSeconds);

	~Connect_Handler();

	char* rtspURL(int clientSocket = -1) const;
	char* rtspURL(const char* serverMediaSession, int clientSocket = -1) const;
	char* rtspURL(ServerMediaSession const* serverMediaSession, int clientSocket = -1) const;

	char* rtspURLPrefix(int clientSocket = -1) const;

	virtual char const* allowedCommandNames();
	virtual UserAuthenticationDatabase* getAuthenticationDatabaseForCommand(char const* cmdName);
	virtual Boolean specialClientAccessCheck(int clientSocket, struct sockaddr_in& clientAddr,
		char const* urlSuffix);
	// a hook that allows subclassed servers to do server-specific access checking
	// on each client (e.g., based on client IP address), without using digest authentication.
	virtual Boolean specialClientUserAccessCheck(int clientSocket, struct sockaddr_in& clientAddr,
		char const* urlSuffix, char const *username);
	// another hook that allows subclassed servers to do server-specific access checking
	// - this time after normal digest authentication has already taken place (and would otherwise allow access).
	// (This test can only be used to further restrict access, not to grant additional access.)

	UserAuthenticationDatabase* setAuthenticationDatabase(UserAuthenticationDatabase* newDB);
	// Changes the server's authentication database to "newDB", returning a pointer to the old database (if there was one).
	// "newDB" may be NULL (you can use this to disable authentication at runtime, if desired).

	virtual ClientConnection* createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr);
	virtual ClientSession* createNewClientSession(u_int32_t sessionId);

private: // redefined virtual functions
	virtual Boolean isRTSPServer() const;

public: // should be protected, but some old compilers complain otherwise
  // The state of a TCP connection used by a RTSP client:
	class RTSPClientSession; // forward
	class RTSPClientConnection : public GenericMediaServer::ClientConnection {
	public:

	protected: // redefined virtual functions:
		virtual void handleRequestBytes(int newBytesRead);

	protected:
		RTSPClientConnection(Connect_Handler& ourServer, int clientSocket, struct sockaddr_in clientAddr);
		virtual ~RTSPClientConnection();

		friend class Connect_Handler;
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

		Connect_Handler& fOurRTSPServer; // same as ::fOurServer
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
		RTSPClientSession(Connect_Handler& ourServer, u_int32_t sessionId);
		virtual ~RTSPClientSession();

		friend class RTSPServer;
		friend class Connect_Handler;
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
		Connect_Handler& fOurRTSPServer; // same as ::fOurServer
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
		//static void incomingConnectionHandlerHTTP(void*, int /*mask*/);
		//void incomingConnectionHandlerHTTP();

		//void noteTCPStreamingOnSocket(int socketNum, RTSPClientSession* clientSession, unsigned trackNum);
		//void unnoteTCPStreamingOnSocket(int socketNum, RTSPClientSession* clientSession, unsigned trackNum);
		//void stopTCPStreamingOnSocket(int socketNum);

		friend class RTSPClientConnection;
		friend class RTSPClientSession;
		//int fHTTPServerSocket; // for optional RTSP-over-HTTP tunneling
		//Port fHTTPServerPort; // ditto
		//HashTable* fClientConnectionsForHTTPTunneling; // maps client-supplied 'session cookie' strings to "RTSPClientConnection"s
		  // (used only for optional RTSP-over-HTTP tunneling)
		//HashTable* fTCPStreamingDatabase;
		// maps TCP socket numbers to ids of sessions that are streaming over it (RTP/RTCP-over-TCP)
		//unsigned fRegisterOrDeregisterRequestCounter;
		UserAuthenticationDatabase* fAuthDB;
		//Boolean fAllowStreamingRTPOverTCP; // by default, True
};

