#include "HttpServer.h"
#include <time.h>

HttpServer::HttpServer() {

	InitializeCriticalSection(&m_disLock);
	m_checkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	bool UseHLS = true;
	if(UseHLS)
		MediaS = new HLS_MediaServer();
	//ConnectToClient("rtsp://admin:1234@192.168.0.70/video1");
}

HttpServer::~HttpServer() {

}

std::wstring HttpServer::GetServerIP() {

	std::wstring whostname = CCommonInfo::GetInstance()->ReadIniFile(L"Streamer", L"SERVERIP", L"127.0.0.1");
	std::string ahostname = "";

#define INFO_BUFFER_SIZE 32767
	TCHAR  infoBuf[INFO_BUFFER_SIZE];
	DWORD  bufCharCount = INFO_BUFFER_SIZE;

	// Get and display the name of the computer.
	if (!GetComputerName(infoBuf, &bufCharCount))
		wprintf(TEXT("GetComputerName"));
	wprintf(TEXT("\nComputer name:      %s"), infoBuf);

	//// Get and display the user name.
	//if (!GetUserName(infoBuf, &bufCharCount))
	//	wprintf(TEXT("GetUserName"));
	//wprintf(TEXT("\nUser name:          %s"), infoBuf);
	whostname = infoBuf;
	ahostname.assign(whostname.begin(), whostname.end());
	char szPath[128] = "";
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	gethostname(szPath, sizeof(szPath));
	printf("%s", szPath);
	struct addrinfo hints, *res;
	struct in_addr addr;
	int err;
	char buf[20] = {};
	WSAData data;
	WSAStartup(MAKEWORD(2, 0), &data);

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	
	if ((err = getaddrinfo(ahostname.c_str(), NULL, &hints, &res)) != 0) {
		printf("error in Getaddrinfo (ServerIP) - %d\n", err);
		CCommonInfo::GetInstance()->WriteLog("ERROR", "error in Getaddrinfo(ServerIP) - %d\n", err);
		return L"";
	}

	addr.S_un = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.S_un;
	inet_ntop(AF_INET, &addr.s_addr, buf, sizeof(buf));

	std::cout << "ServerIP Get - " << buf << std::endl;
	CCommonInfo::GetInstance()->WriteLog("INFO", "Server IP Find - %s", buf);

	freeaddrinfo(res);
	WSACleanup();

	std::wstring WServerIP = L"";
	std::string ServerIP = buf;
	
	WServerIP.assign(ServerIP.begin(), ServerIP.end());
	
	return WServerIP;
}

void HttpServer::Start() {
	
	m_ip = GetServerIP();
	//m_ip = L"0.0.0.0";
	uri_builder mmuri(m_ip);
	auto addr = mmuri.to_uri().to_string();
	m_port = (unsigned int)CCommonInfo::GetInstance()->ReadIniFile(L"Streamer", L"SERVERPORT", 8077);
	m_uri = m_ip + L":" + std::to_wstring(m_port);
	
	//m_ip = L"localhost";
	//m_port = 7878;
	//if (!m_pCfg->LoadConfig(U("config.json")))
	//	return;
	//
	//m_ip = m_pCfg->_ip;
	//m_port = m_pCfg->_port;

	WCHAR uri[MAX_PATH];
	swprintf_s(uri, MAX_PATH, L"http://%ws:%hu/", m_ip.c_str(), m_port);
	m_uri = uri;

	m_pListener = new web::http::experimental::listener::http_listener(m_uri);
	
	try
	{
		m_pListener->support(web::http::methods::GET, std::bind(&HttpServer::HandeHttpGet, this, std::placeholders::_1));
		m_pListener->open().wait();
		//open().wait();
		ucout << L"start lisen on " << m_pListener->uri().to_string() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return;
	}
	
	if (!m_checkFuture.valid())
	{
		m_checkFuture = std::async(&HttpServer::WrapCheck, this);
	}

}

CT HttpServer::ParseURLtoType(std::string URL) {

	CT Type = CT_NOT_DEFINED;
	size_t pos = 0;
	std::string Temp = "";

	pos = URL.find("://", 0);
	if (pos == string::npos) {
		return CT_NOT_DEFINED;
	}
	Temp = URL.substr(0, pos);

	if (Temp.compare("rtsp") == 0) {
		bool LIVE = false;
		std::wstring temp = CCommonInfo::GetInstance()->ReadIniFile(L"Streamer", L"RTSPTYPE", L"ffmpeg");
		size_t find = temp.find(L"live");
		if (find = std::wstring::npos)
			LIVE = true;

		Type  =  (LIVE) ? CT_RTSP_LIVE_CLIENT : CT_RTSP_FF_CLIENT;
		return Type;
	}
	Type = (Temp.compare("rtmp") == 0) ? CT_RTMP_CLIENT : CT_NOT_DEFINED;
	Type = (Temp.compare("local") == 0) ? CT_LOCAL_FILE_CLIENT : CT_NOT_DEFINED;
	Type = (Temp.compare("hls") == 0) ? CT_HLS_CLIENT : CT_NOT_DEFINED;

	return Type;
}

UUID HttpServer::ParseURLtoUUID(std::string *URL) {

	UUID uuid = {};		//8 - 4 - 4 - 4 - 12
	size_t pos = 0;
	std::string UUIDTemp = "";

	pos = URL->find("uuid[", 0);
	if (pos == 0){}

		if (pos == string::npos) {
		return {};
	}

	UUIDTemp = URL->substr(pos);
	UUIDTemp.erase(0, 5);
	
	//uuid[550e8400-e29b-41d4-a716-446655440000]
	URL->erase(pos, 42);
	
	unsigned long p0;
	int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;

	int err = sscanf_s(UUIDTemp.c_str(), "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);

	uuid.Data1 = p0;
	uuid.Data2 = p1;
	uuid.Data3 = p2;
	uuid.Data4[0] = p3;
	uuid.Data4[1] = p4;
	uuid.Data4[2] = p5;
	uuid.Data4[3] = p6;
	uuid.Data4[4] = p7;
	uuid.Data4[5] = p8;
	uuid.Data4[6] = p9;
	uuid.Data4[7] = p10;

	if (err >= 0)
		return uuid;
	else {
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Could not generating UUID - %s", UUIDTemp.c_str());
		return {};
	}
}

int HttpServer::ParseURLtoBitrate(std::string URL) {
	
	std::string FileTemp = "";
	size_t Deletepos = 0;
	int Bitrate = 0;

	if ((Deletepos = URL.find("1080_")) != string::npos) {
		Bitrate = 1080;
		//Deletepos = URL->find("1080_");
		//URL.erase(Deletepos, 5);
	}
	else if ((Deletepos = URL.find("720_")) != string::npos) {
		Bitrate = 720;
		//Deletepos = URL->find("720_");
		//URL.erase(Deletepos, 4);
	}
	//else if ((Deletepos = URL->find("480_")) != string::npos) {
	//	Bitrate = 480;
	//	//Deletepos = URL->find("480_");
	//	URL->erase(Deletepos, 4);
	//}

	return Bitrate;
}

MFT HttpServer::ParseURLtoFile(std::string URL, string &FileNumber) {

	std::string FileTemp = "";
	MFT FileType = MFT_NOT_DEFINED;
	size_t Deletepos = -1;

	if ((Deletepos = URL.find(".m3u8")) != string::npos){
		FileType = MFT_M3U8;
		//Deletepos = URL.find(".m3u8");
	}
	else if ((Deletepos = URL.find(".ts")) != string::npos) {
		FileType = MFT_TS;
		//Deletepos = URL->find(".ts", 0);
	}
	else if ((Deletepos = URL.find(".avi")) != string::npos) {
		FileType = MFT_AVI;
		//Deletepos = URL->find(".avi", 0);
	}
	else if ((Deletepos = URL.find(".mkv")) != string::npos) {
		FileType = MFT_MKV;
		//Deletepos = URL->find(".mkv", 0);
	}
	else if ((Deletepos = URL.find(".mp4")) != string::npos) {
		FileType = MFT_MP4;
		//Deletepos = URL->find(".mp4", 0);
	}
	else {
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Could not generating File Type - %s", URL.c_str());
		FileType = MFT_NOT_DEFINED;
	}
	if(Deletepos != -1){
		for (int i = Deletepos; i > Deletepos - 4; i--) {
			if (URL.at(i) == '/') {
				FileNumber = URL.substr(i, Deletepos - i);
				if (FileNumber == "") {
					return FileType = MFT_NOT_DEFINED;
				}
			}
		}
	}

	//if (FileType != MFT_NOT_DEFINED) {
	//
	//	URL->erase(Deletepos);
	//}

	return FileType;
}

bool HttpServer::ResponseFile(MFT FileType, string &FileNumber, web::http::http_request msg) {
	
	if (FileType == MFT_M3U8) {

		std::string path = FileNumber + ".m3u8";

		auto fullPath = utility::conversions::usascii_to_utf16(path);
		try
		{
			concurrency::streams::fstream::open_istream(fullPath, std::ios::in | std::ios::binary).then(
				[=](concurrency::streams::istream is) {

				msg.reply(web::http::status_codes::OK, is, appMpegUrl);

			}
			).get();
		}
		catch (const std::exception& e)
		{
			std::vector<std::pair<utility::string_t, web::json::value>> kv;
			kv.push_back({ U("info"),web::json::value::string(U("m3u8 not already ok")) });
			web::json::value v = web::json::value::object(kv);

			msg.reply(web::http::status_codes::RequestTimeout, v);
			return false;
		}
	}
	else if (FileType == MFT_TS) {

		std::string path = FileNumber + ".ts";

		auto fullPath = utility::conversions::usascii_to_utf16(path);
		try
		{
			concurrency::streams::fstream::open_istream(fullPath, std::ios::in | std::ios::binary).then(
				[=](concurrency::streams::istream is) {

				msg.reply(web::http::status_codes::OK, is, videoMP2T);

			}
			).get();
		}
		catch (const std::exception& e)
		{
			std::vector<std::pair<utility::string_t, web::json::value>> kv;
			kv.push_back({ U("error"),web::json::value::string(U("request this file some error happened")) });
			web::json::value v = web::json::value::object(kv);
			msg.headers().add(U("Connection"), U("close"));
			msg.reply(web::http::status_codes::NotFound, v);
			std::wcout << e.what() << "ts file exception" << std::endl;
			return false;
		}
	}
	if (FileType == MFT_NOT_DEFINED) {

		std::vector<std::pair<utility::string_t, web::json::value>> kv;
		kv.push_back({ U("error"),web::json::value::string(U("request this file some error happened")) });
		web::json::value v = web::json::value::object(kv);
		msg.headers().add(U("Connection"), U("close"));
		msg.reply(web::http::status_codes::NotFound, v);
		ucout << L"Error not include file - " << msg.to_string() << std::endl;

		return false;
	}

	return true;
}

std::string HttpServer::RequestWithURL(std::string URL, string &ConnectURL, string &FileType, string &FileNumber, web::http::http_request msg){
	
	CT Type = ParseURLtoType(URL);
	//int Bitrate = ParseURLtoBitrate(URL);
	MFT TempMFT = ParseURLtoFile(URL, FileNumber);
	

	std::string FileURI = "";
	bool Check = MediaS->CreateSet(Type, URL);
	//std::string FileURI = MediaS->RequestWithUUID(Type, URL);
	if (Check) {
		//먼저 Seg랑 클라이언트랑 활성화 시키고 시간 갱신 시켜주고 들어가야함.
		Check = ResponseFile(TempMFT, FileNumber, msg);
	}

}

void HttpServer::handle_error(pplx::task<void>& t)
{
	try
	{
		t.get();
	}
	catch (...)
	{
		CCommonInfo::GetInstance()->WriteLog("ERROR ", "UNKOWN ERROR");
		// Ignore the error, Log it if a logger is available 
	}
}	

bool HttpServer::CheckClientExist(std::string URL, CT Type) {

	CLI temp = {};
	temp = MediaS->GetClientInfo(Type, URL);
	if (temp.Type == CT_NOT_DEFINED) {
		return true;
	}
	else
		return false;
}

//bool HttpServer::CreateClient(CT Type, std::string URL) {
//	
//	bool Check = CheckClientExist(URL, Type);
//	if (Check) {
//		int intcheck = MediaS->CreateSet((CT)0, URL);
//		if (intcheck >= 0) {
//			return true;
//		}
//		else {
//			std::cout << "Create Client Failed - URL : " << URL.c_str() << std::endl;
//			CCommonInfo::GetInstance()->WriteLog("ERROR", "Create Client is Failed - %s", URL.c_str());
//			return false;
//		}
//	}
//	else {
//		return false;
//	}
//
//}

const char* HttpServer::ParsePercentEncodingA(wstring &URL, string &Temp) {
	
	// { = %7B    /   } = %7D
	// [ \ %5B    /	  ] = %5D

	size_t fpos5 = URL.find(L"%5B");
	if(fpos5 != string::npos)
		URL.replace(fpos5, 3, L"[");
	
	size_t epos5 = URL.find(L"%5D");
	if (epos5 != string::npos)
		URL.replace(epos5, 3, L"]");

	size_t fpos7 = URL.find(L"%7B");
	if (fpos7 != string::npos)
		URL.replace(fpos5, 1, L"{");

	size_t epos7 = URL.find(L"%7D");
	if (epos7 != string::npos)
		URL.replace(epos5, 1, L"{");

	Temp = "";
	Temp.assign(URL.begin(), URL.end());

	std::cout << "Parse string - " << Temp.c_str() << std::endl;

	return Temp.c_str();
}

void HttpServer::ParsetoURL(string URL, string &ConnectURL, string &FileType, string &Filename) {

	size_t urlpos = URL.find("video_url");
	string temp = URL.substr(urlpos+10);
	urlpos = temp.find("]");
	temp.erase(urlpos);
	ConnectURL = temp.c_str();

	size_t filepos = URL.find("]/");
	temp = URL.substr(filepos + 2);

	FileType = temp.c_str();

	return;
}

void HttpServer::HandeHttpGet(web::http::http_request msg) {


	//std::string* URL = new std::string("rtsp://admin:1234@192.168.0.73/video1");
	//std::string* URLWithID = new std::string("rtsp://admin:1234@192.168.0.73/video1.m3u8?id=550e8400-e29b-41d4-a716-446655440000");
	//std::string* URLWithID2 = new std::string("rtsp://admin:1234@192.168.0.73/video1.m3u8?1080_?id=550e8400-e29b-41d4-a716-446655440000");
	//std::string URL2(*URL);
	//unsigned char * str = nullptr;

	std::string URL = utility::conversions::to_utf8string(msg.relative_uri().to_string());
	std::wstring action = msg.absolute_uri().path();

	std::wcout << action.c_str() << std::endl;
	

	size_t pos = action.find(L"/hls");
	if (pos != string::npos){

		ParsePercentEncodingA(action, URL);
		CCommonInfo::GetInstance()->WriteLog("INFO", "Request recieved - %s ", URL.c_str());

		string ConnectURL = "";
		string FileURL = "";
		string FileNumber = "";
		ParsetoURL(URL, ConnectURL, FileURL, FileNumber);
		
		if (FileURL.length() < 4) {

			std::vector<std::pair<utility::string_t, web::json::value>> kv;
			kv.push_back({ U("error"),web::json::value::string(U("request this file some error happened")) });
			web::json::value v = web::json::value::object(kv);
			msg.headers().add(U("Connection"), U("close"));
			msg.reply(web::http::status_codes::NotFound, v);
			ucout << L"Error not include file - "<< msg.to_string() << std::endl;
			return;
		}

		bool Check = false;
		std::string Filename = RequestWithURL(URL, ConnectURL, FileURL, FileNumber, msg);


		ucout << msg.to_string() << std::endl;
		msg.reply(status_codes::OK, U("GET"));
	}

	//MediaS = new HLS_MediaServer();
	//UUID ClientUUID = RequestWithoutUUID(*URL);
	//std::string Filename = RequestWithUUID(URLWithID);
	//
	//clock_t Start = clock();
	//clock_t End = clock();
	//
	//RequestWithoutUUID(*URL);
	////RequestWithUUID(URLWithID2);
	//
	//while (1) {
	//	Sleep(1);
	//
	//	End = clock();
	//
	//	if ((End - Start) > 10000) {
	//
	//		//bool Check = MediaS->DeleteSet(URL->c_str(), ClientUUID);
	//		break;
	//	}
	//}

	//MediaS->DeleteSet(URL->c_str(), ClientUUID);

	//std::string g;
	//cin >> g;	getchar();

	return;
}

unsigned int HttpServer::WrapCheck()
{
	DWORD dwRet;
	while (true)
	{
		dwRet = WaitForSingleObject(m_checkEvent, 10 * 1000);
		if (dwRet == WAIT_TIMEOUT)
		{
			EnterCriticalSection(&m_disLock);
			auto it = m_distributions.begin();
			while (it != m_distributions.end())
			{
				//if (it->second->GetClientCount() == 0)
				//{
				//	delete it->second;
				//	it = m_distributions.erase(it);
				//	std::clog << "remove distribution" << std::endl;
				//	continue;
				//}
				++it;
			}
			LeaveCriticalSection(&m_disLock);
		}
		else
			break;
	}
	return 0;
}

void response() {
	//std::wstring  strName = L"c:\\Temp\\" + action;// TestAlert - 1550057065.jpg";
	//
	//concurrency::streams::file_stream<unsigned char>::open_istream(strName).then([=]
	//(pplx::task<concurrency::streams::basic_istream<unsigned char>>  prevTask) {
	//	try {
	//		auto fileStream = prevTask.get(); // got ready file stream
	//		http_response resp;             // prepare response with OK status
	//		resp.set_body(fileStream);
	//		resp.set_status_code(status_codes::OK);
	//		resp.headers().set_content_type(U("image/jpeg"));
	//		return message.reply(resp).then([fileStream, message, strName](pplx::task<void> prevResp) {
	//			try {
	//				fileStream.close();
	//				//INFO("GET: " + strName + " file send to client");
	//				if (prevResp.is_done())message.reply(status_codes::OK);
	//			}
	//			catch (const std::system_error& e) {
	//				//ERR("GET: file stream err");
	//				//ERR(e.what());
	//				message.reply(status_codes::BadRequest);
	//			}
	//		});
	//
	//	}
	//	catch (const std::system_error& e) {
	//		//ERR("GET: task err");
	//		//ERR(e.what());
	//		return pplx::task_from_result();
	//	}
	//}).wait();
}

//void HttpServer::Load_Init();
//bool HttpServer::URL_Parse();
//#include <chrono>
//#include <cstdio>
//#include <httplib.h>
//
//#define SERVER_CERT_FILE "./cert.pem"
//#define SERVER_PRIVATE_KEY_FILE "./key.pem"
//
//using namespace httplib;
//
//std::string dump_headers(const Headers &headers) {
//	std::string s;
//	char buf[BUFSIZ];
//
//	for (auto it = headers.begin(); it != headers.end(); ++it) {
//		const auto &x = *it;
//		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
//		s += buf;
//	}
//
//	return s;
//}
//
//std::string log(const Request &req, const Response &res) {
//	std::string s;
//	char buf[BUFSIZ];
//
//	s += "================================\n";
//
//	snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
//		req.version.c_str(), req.path.c_str());
//	s += buf;
//
//	std::string query;
//	for (auto it = req.params.begin(); it != req.params.end(); ++it) {
//		const auto &x = *it;
//		snprintf(buf, sizeof(buf), "%c%s=%s",
//			(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
//			x.second.c_str());
//		query += buf;
//	}
//	snprintf(buf, sizeof(buf), "%s\n", query.c_str());
//	s += buf;
//
//	s += dump_headers(req.headers);
//
//	s += "--------------------------------\n";
//
//	snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
//	s += buf;
//	s += dump_headers(res.headers);
//	s += "\n";
//
//	if (!res.body.empty()) { s += res.body; }
//
//	s += "\n";
//
//	return s;
//}
//
//int main(void) {
//#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
//	SSLServer svr(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
//#else
//	Server svr;
//#endif
//
//	if (!svr.is_valid()) {
//		printf("server has an error...\n");
//		return -1;
//	}
//
//	svr.Get("/", [=](const Request & /*req*/, Response &res) {
//		res.set_redirect("/hi");
//	});
//
//	svr.Get("/hi", [](const Request & /*req*/, Response &res) {
//		res.set_content("Hello World!\n", "text/plain");
//	});
//
//	svr.Get("/slow", [](const Request & /*req*/, Response &res) {
//		std::this_thread::sleep_for(std::chrono::seconds(2));
//		res.set_content("Slow...\n", "text/plain");
//	});
//
//	svr.Get("/dump", [](const Request &req, Response &res) {
//		res.set_content(dump_headers(req.headers), "text/plain");
//	});
//
//	svr.Get("/stop",
//		[&](const Request & /*req*/, Response & /*res*/) { svr.stop(); });
//
//	svr.set_error_handler([](const Request & /*req*/, Response &res) {
//		const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
//		char buf[BUFSIZ];
//		snprintf(buf, sizeof(buf), fmt, res.status);
//		res.set_content(buf, "text/html");
//	});
//
//	svr.set_logger([](const Request &req, const Response &res) {
//		printf("%s", log(req, res).c_str());
//	});
//
//	svr.listen("localhost", 8080);
//
//	return 0;
//}
