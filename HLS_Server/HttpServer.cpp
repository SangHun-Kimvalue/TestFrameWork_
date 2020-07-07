#include "HttpServer.h"
#include <time.h>

HttpServer::HttpServer() {
	MediaS = new HLS_MediaServer();
	std::string* URL = new std::string("rtsp://admin:1234@192.168.0.73/video1");
	std::string* URLWithID = new std::string("rtsp://admin:1234@192.168.0.73/video1.m3u8?id=550e8400-e29b-41d4-a716-446655440000");
	std::string* URLWithID2 = new std::string("rtsp://admin:1234@192.168.0.73/video1.m3u8?id=550e8400-e29b-41d4-a716-446655440000");
	
	UUID ClientUUID = RequestWithoutUUID(*URL);
	std::string Filename = RequestWithUUID(URLWithID);

	clock_t Start = clock();
	clock_t End = clock();

	RequestWithoutUUID(*URL);
	RequestWithUUID(URLWithID2);


	while (1) {
		Sleep(1);

		End = clock();
		
		if ((End - Start) > 5000) {

			bool Check = MediaS->DeleteSet(URL->c_str(), ClientUUID);
			break;
		}
	}

	//ConnectToClient("rtsp://admin:1234@192.168.0.70/video1");
}

HttpServer::~HttpServer() {

}

CT HttpServer::ParseURLtoType(std::string URL) {

	CT Type = CT_NOT_DEFINED;
	size_t pos = 0;
	std::string Temp = "";

	pos = URL.find("://", 0);
	Temp = URL.substr(0, pos);

	if (Temp.compare("rtsp") == 0) {

		bool LIVE = (bool)CCommonInfo::GetInstance()->ReadIniFile(L"Converter", L"RTSPTYPE", 0);
		Type  =  (LIVE) ? CT_RTSP_LIVE_CLIENT : CT_RTSP_FF_CLIENT;
		return Type;
	}
	Type = (Temp.compare("rtmp") == 0) ? CT_RTMP_CLIENT : CT_NOT_DEFINED;
	Type = (Temp.compare("local") == 0) ? CT_LOCAL_FILE_CLIENT : CT_NOT_DEFINED;
	Type = (Temp.compare("hls") == 0) ? CT_HLS_CLIENT : CT_NOT_DEFINED;

	return Type;
}

UUID HttpServer::RequestWithoutUUID(std::string URL) {
	//CLI CLI_temp = {};		//private 필요
	CT Type = ParseURLtoType(URL);

	//CLI_temp.Type = CT_RTSP_FF_CLIENT;
	//CLI_temp.URL = URL;
	//CLI_temp.Index = 0;
	//CLI_temp.TransportType = TT_TCP;
	//UuidCreate(&CLI_temp.uuid);
	//CLI_temp.Interval = 10;

	UUID Responseuuid = MediaS->RequestWithoutUUID(Type, URL);
	//MediaS->CreateClient(CT_RTSP_FF_CLIENT, URL);

	//unsigned char * str = nullptr;
	//UuidToStringA(&Responseuuid, &str);

	//std::string s((char*)str);

	//RpcStringFreeA(&str);
	//std::cout << s.c_str() << std::endl;

	return Responseuuid;
}

UUID HttpServer::ParseURLtoUUID(std::string *URL) {

	UUID uuid = {};		//8 - 4 - 4 - 4 - 12
	size_t pos = 0;
	std::string UUIDTemp = "";

	pos = URL->find("?id=", 0);
	if (pos == 0){}

		if (pos == string::npos) {
		return {};
	}

	UUIDTemp = URL->substr(pos);
	UUIDTemp.erase(0, 4);
	URL->erase(pos);

	//pos = UUIDTemp.find("-", 0);
	//std::string FirstSection = UUIDTemp.substr(0, pos).c_str();
				
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

	if (err == 0)
		return uuid;
	else {
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Could not generating UUID - %s", UUIDTemp.c_str());
		return {};
	}
}

int HttpServer::ParseURLtoBitrate(std::string *URL) {

	return 0;
}

MFT HttpServer::ParseURLtoFile(std::string *URL) {

	std::string FileTemp = "";
	MFT FileType = MFT_NOT_DEFINED;
	size_t Deletepos = 0;

	if (URL->find(".m3u8") != string::npos){
		FileType = MFT_M3U8;
		Deletepos = URL->find(".m3u8");
	}
	else if (Deletepos = URL->find(".ts") != string::npos) {
		FileType = MFT_TS;
		//Deletepos = URL->find(".ts", 0);
	}
	else if (Deletepos = URL->find(".avi") != string::npos) {
		FileType = MFT_AVI;
		//Deletepos = URL->find(".avi", 0);
	}
	else if (Deletepos = URL->find(".mkv") != string::npos) {
		FileType = MFT_MKV;
		//Deletepos = URL->find(".mkv", 0);
	}
	else if (Deletepos = URL->find(".mp4") != string::npos) {
		FileType = MFT_MP4;
		//Deletepos = URL->find(".mp4", 0);
	}
	else {
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Could not generating File Type - %s", URL->c_str());
		FileType = MFT_NOT_DEFINED;
	}

	if (FileType != MFT_NOT_DEFINED) {

		URL->erase(Deletepos);
	}

	return FileType;
}

std::string HttpServer::RequestWithUUID(std::string* URL){
	
	CT Type = ParseURLtoType(*URL);

	UUID uuid = ParseURLtoUUID(URL);
	MFT TempMFT = ParseURLtoFile(URL);
	if (TempMFT == MFT_NOT_DEFINED)
		return "";

	std::string FileURI = MediaS->RequestWithUUID(Type, *URL, uuid);

	return FileURI;
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

