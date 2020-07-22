#pragma once
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <cpprest/uri.h>
#include <cpprest/asyncrt_utils.h>

#include "HLS_MediaServer.h"
#include "HLS_Server.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winsock.h>
#include <Windows.h>
#include <future>

#if defined(_MSC_VER)
#ifdef _WIN64
using ssize_t = __int64;
#else
using ssize_t = int;
#endif

#if _MSC_VER < 1900
#define snprintf _snprintf_s
#endif
#endif // _MSC_VER
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "cryptui.lib")
#endif
#ifdef _DEBUG
#pragma comment(lib,"cpprest_2_10d.lib")
#else
#pragma comment(lib,"cpprest_2_10.lib")
#endif

using namespace utility;
using namespace web;
using namespace web::http;
using namespace http::experimental::listener;
using namespace std;
//using namespace httplib;

class HttpServer
{
public:
	HttpServer();
	~HttpServer();

	void Start();

public:

private:

	bool CheckClientExist(std::string URL, CT Type = CT_NOT_DEFINED);
	bool CreateClient(CT Type, std::string URL);

	UUID RequestWithoutUUID(string URL);
	std::string RequestWithUUID(string* URL);

	CT ParseURLtoType(string URL);
	UUID ParseURLtoUUID(string *URL);
	MFT ParseURLtoFile(string URL);
	int ParseURLtoBitrate(string URL);

	int ConnectToClient(string URL);

	void ParsetoURL(string URL, string &ConnectURL, string &FileType);
	const char* ParsePercentEncodingA(wstring &URL, string &Temp);
	std::wstring GetServerIP();
	void handle_error(pplx::task<void>& t);
	void HandeHttpGet(web::http::http_request msg);
	unsigned int WrapCheck();

private:

	std::wstring m_uri;
	std::wstring m_ip;
	unsigned short m_port;
	HANDLE m_checkEvent;
	CRITICAL_SECTION m_disLock;
	std::future<unsigned int> m_checkFuture;
	//http_listener m_listener;
	
	std::map<std::string, SBL*> m_distributions;
	
	web::http::experimental::listener::http_listener* m_pListener;

	HLS_MediaServer* MediaS;

	//Server svr;
};

