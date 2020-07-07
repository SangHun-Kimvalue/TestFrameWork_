#pragma once
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <cpprest/uri.h>
#include <cpprest/asyncrt_utils.h>

#include "HLS_MediaServer.h"
#include "HLS_Server.hpp"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace http::experimental::listener;
using namespace std;

class HttpServer
{
public:
	HttpServer();
	~HttpServer();

private:

	UUID RequestWithoutUUID(string URL);
	std::string RequestWithUUID(string* URL);

	CT ParseURLtoType(string URL);
	UUID ParseURLtoUUID(string *URL);
	MFT ParseURLtoFile(string *URL);
	int ParseURLtoBitrate(string *URL);

	int ConnectToClient(string URL);

	http_listener m_listener;
	HLS_MediaServer* MediaS;


};

