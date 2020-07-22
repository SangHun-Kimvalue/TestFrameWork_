#include "HttpServer.h"
#include <time.h>

HttpServer::HttpServer() {

	InitializeCriticalSection(&m_disLock);
	m_checkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//ConnectToClient("rtsp://admin:1234@192.168.0.70/video1");
}

HttpServer::~HttpServer() {

}

std::wstring HttpServer::GetServerIP() {

	std::wstring whostname = CCommonInfo::GetInstance()->ReadIniFile(L"Converter", L"SERVERIP", L"127.0.0.1");
	std::string ahostname = "";
	ahostname.assign(whostname.begin(), whostname.end());

	const char *hostname = ahostname.c_str();
	struct addrinfo hints, *res;
	struct in_addr addr;
	int err;
	char buf[20] = {};
	WSAData data;
	WSAStartup(MAKEWORD(2, 0), &data);

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	
	if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
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
	m_port = (unsigned int)CCommonInfo::GetInstance()->ReadIniFile(L"Converter", L"SERVERPORT", 8077);
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
		std::wstring temp = CCommonInfo::GetInstance()->ReadIniFile(L"Converter", L"RTSPTYPE", L"ffmpeg");
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

UUID HttpServer::RequestWithoutUUID(std::string URL) {

	CT Type = ParseURLtoType(URL);
	UUID Responseuuid = MediaS->RequestWithoutUUID(Type, URL);

	return Responseuuid;
}

//UUID 제거
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

//Bitrate 제거
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

//FileType 제거
MFT HttpServer::ParseURLtoFile(std::string URL) {

	std::string FileTemp = "";
	MFT FileType = MFT_NOT_DEFINED;
	size_t Deletepos = 0;

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
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Could not generating File Type - %s", URL->c_str());
		FileType = MFT_NOT_DEFINED;
	}

	//if (FileType != MFT_NOT_DEFINED) {
	//
	//	URL->erase(Deletepos);
	//}

	return FileType;
}

std::string HttpServer::RequestWithUUID(std::string* URL){
	
	CT Type = ParseURLtoType(*URL);

	UUID uuid = ParseURLtoUUID(URL);
	int Bitrate = ParseURLtoBitrate(*URL);
	MFT TempMFT = ParseURLtoFile(*URL);
	if (TempMFT == MFT_NOT_DEFINED) {
	}

	std::string FileURI = MediaS->RequestWithUUID(Type, *URL);
	size_t Error = FileURI.find("Error");
	if (Error == std::string::npos) {
		return "Error - In Create SBL Set";
	}
	size_t Create = FileURI.find("Create");
	if (Create == std::string::npos) {
		return "Error - Create new Client";
	}
	else {
		return FileURI;
	}
	
	//else {
	//	bool Check = MediaS->RequestWithFile(uuid, TempMFT, Bitrate);
	//}
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

bool HttpServer::CreateClient(CT Type, std::string URL) {
	
	bool Check = CheckClientExist(URL, Type);
	if (Check) {
		int intcheck = MediaS->CreateClient((CT)0, URL);
		if (intcheck >= 0) {
			return true;
		}
		else {
			std::cout << "Create Client Failed - URL : " << URL.c_str() << std::endl;
			CCommonInfo::GetInstance()->WriteLog("ERROR", "Create Client is Failed - %s", URL.c_str());
			return false;
		}
	}
	else {
		return false;
	}

}

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

void HttpServer::ParsetoURL(string URL, string &ConnectURL, string &FileType) {

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
		ParsetoURL(URL, ConnectURL, FileURL);
		
		if (FileURL.length() < 4) {

			std::vector<std::pair<utility::string_t, web::json::value>> kv;
			kv.push_back({ U("error"),web::json::value::string(U("request this file some error happened")) });
			web::json::value v = web::json::value::object(kv);
			msg.headers().add(U("Connection"), U("close"));
			msg.reply(web::http::status_codes::NotFound, v);
			ucout << L"Error not include file - "<< msg.to_string() << std::endl;

			return;
		}


		CT Type = CT_NOT_DEFINED;
		std::string Filename = "";

			bool ClientExist = CheckClientExist(ConnectURL);

			Type = ParseURLtoType(ConnectURL);
			//int Bitrate = ParseURLtoBitrate(&URL);
			MFT FileType = ParseURLtoFile(FileURL);

			bool Check = CreateClient(Type, ConnectURL);
			if (Check) {
				Filename = MediaS->CreateSet(Type, ConnectURL);
			}

			Filename = MediaS->CreateSet(Type, URL);
			//return to file

			Type = ParseURLtoType(URL);
			Check = CreateClient(Type, URL);
			//return to UUID


		//Filename return (response sending function)
		
		//std::string Filename = MediaS->CreateSet((CT)0, URL, ClientUUID);




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

int HttpServer::ConnectToClient(std::string URL) {

	//CLI CLI_temp = {};		//private 필요
	CT Type;
	size_t pos = 0;
	std::string Temp = "";

	pos = URL.find("://", pos);
	Temp = URL.substr(0, pos);

	if (Temp.compare("rtsp") == 0) {
		Type = CT_RTSP_FF_CLIENT;
	}

	//CLI_temp.Type = CT_RTSP_FF_CLIENT;
	//CLI_temp.URL = URL;
	//CLI_temp.Index = 0;
	//CLI_temp.TransportType = TT_TCP;
	//UuidCreate(&CLI_temp.uuid);
	//CLI_temp.Interval = 10;
		
	UUID Responseuuid = MediaS->RequestWithoutUUID(Type, URL);
	//MediaS->CreateClient(CT_RTSP_FF_CLIENT, URL);

	unsigned char * str = nullptr;
	UuidToStringA(&Responseuuid, &str);

	std::string s((char*)str);

	RpcStringFreeA(&str);
	std::cout << s.c_str() << std::endl;

	return 0;
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
