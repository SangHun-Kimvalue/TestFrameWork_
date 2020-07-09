#pragma once
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <cpprest/uri.h>
#include <cpprest/asyncrt_utils.h>

#include "HLS_MediaServer.h"
#include "HLS_Server.hpp"
#include "httplib.h"
#include <Windows.h>
#include <future>
#include <map>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace http::experimental::listener;
using namespace std;
using namespace httplib;

class HttpServer
{
public:
	HttpServer();
	~HttpServer();

	void Start();

public:


private:

	UUID RequestWithoutUUID(string URL);
	std::string RequestWithUUID(string* URL);

	CT ParseURLtoType(string URL);
	UUID ParseURLtoUUID(string *URL);
	MFT ParseURLtoFile(string *URL);
	int ParseURLtoBitrate(string *URL);

	int ConnectToClient(string URL);

	web::http::experimental::listener::http_listener* m_pListener;
	void HandeHttpGet(web::http::http_request msg);
	unsigned int WrapCheck();

private:

	std::wstring m_uri;
	std::wstring m_ip;
	unsigned int m_port;
	HANDLE m_checkEvent;
	CRITICAL_SECTION m_disLock;
	std::future<unsigned int> m_checkFuture;
	http_listener m_listener;
	std::map<std::string, SBL*> m_distributions;


	HLS_MediaServer* MediaS;




	Server svr;
};

