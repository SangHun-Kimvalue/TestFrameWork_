#include "HLS_MediaServer.h"

HLS_MediaServer::HLS_MediaServer() {

	SourceM = new SourceManager();
	//FileM = new FileManager();
	
}

HLS_MediaServer::~HLS_MediaServer() {


}

UUID HLS_MediaServer::RequestWithoutUUID(CT Type, std::string URL) {

	UUID ReturnUUID = {};

	int Check = CreateClient(Type, ReturnUUID, URL);

	return ReturnUUID;
}

std::string HLS_MediaServer::RequestWithUUID(CT Type, std::string URL, UUID uuid) {

	std::string FileURI = "";
	FileURI = CreateSet(Type, URL, uuid);
	if (FileURI == "") {
		return "Error - In Create SBL Set";
	}

	return FileURI;
	
}

bool HLS_MediaServer::RequestWithFile(UUID uuid, std::string filename, int resol) {

	return true;
}

int HLS_MediaServer::CreateClient(CT Type, UUID &uuid, std::string URL, int Interval) {

	CLI TempCLI = {};
	TempCLI.Type = Type;
	TempCLI.URL = URL;
	TempCLI.uuid = uuid;
	TempCLI.Interval = Interval;

	GetClientValue GC = { Type, uuid, URL };

	//If return 1 == Already had, 0 == Create Success
	int Check = SourceM->Create_Client(TempCLI);
	if (Check == 0) {
		std::cout << "Create Client Success - URL : " << URL.c_str() << std::endl;
		SourceM->PrintInfo(GC);
	}
	else if (Check == 1) {
		std::cout << "Already has Client  - URL : " << URL.c_str() << std::endl;
	}
	else {
		std::cout << "Create Client Failed - URL : " << URL.c_str() << std::endl;
		return -1;
	}

	TempCLI = SourceM->GetClientInfo(GC);
	uuid = TempCLI.uuid;

	CCommonInfo::GetInstance()->WriteLog("INFO", "Success create SBL - %s", URL.c_str());

	return Check;
}

//uuid 를 포함한 클라이언트 요청
std::string HLS_MediaServer::CreateSet(CT Type, std::string URL, UUID uuid, int Interval) {

	int Check = false;
	GetClientValue GC = { Type, uuid, URL };

	Check = CreateClient(Type, uuid, URL, Interval);
	if (Check < 0) {
		//std::cout << " Error - in Create Set SBL - URL :  " << URL << std::endl;
		CCommonInfo::GetInstance()->WriteLog("INFO", " Error - in Create Set SBL - URL : %s", URL.c_str());
		return "";
	}
	else {

		SBL* AddNewSBL = new SBL();
		
		CLI tempinfo = SourceM->GetClientInfo(GC);
		GC.Type = tempinfo.Type;
		GC.uuid = tempinfo.uuid;
		GC.URL = tempinfo.URL;

		AVCodecID VCo = AV_CODEC_ID_NONE;
		AVCodecID ACo = AV_CODEC_ID_NONE;
		bool UseAudio = false, UseTranscoding = false;
		int Interval = tempinfo.Interval;

		VCo = tempinfo.VCodecID;
		if (UseAudio = tempinfo.IncludedAudio)
			ACo = tempinfo.ACodecID;

		UseTranscoding = tempinfo.Use_Transcoding;

		AddNewSBL->Type = Type;
		AddNewSBL->URL = URL;
		AddNewSBL->Seg = new SegmenterGroup(UseAudio, UseTranscoding, Interval, VCo, ACo);
		AddNewSBL->ClientUUID = tempinfo.uuid;
		AddNewSBL->DataQ = SourceM->GetFrameQ(GC).get();
		AddNewSBL->FileM = new FileManager();

		SBLL.push_back(AddNewSBL);

		//debug_ = nullptr;
		debug_ = new Muxer("TestFile.m3u8", AddNewSBL->DataQ, UseAudio, Interval, VCo, ACo);

		std::string Filename = DoWorkSBL(AddNewSBL->ClientUUID);

		CCommonInfo::GetInstance()->WriteLog("INFO", "Success create SBL - %s", URL.c_str());

		return Filename;
	}
	//UUID temp = SourceM->GetUUID(URL);

	return "";
}

bool HLS_MediaServer::DeleteSet(std::string URL, UUID uuid) {
	
	SBL* Getto = Getto_SBL(uuid);
	if (Getto == nullptr) {
		//std::cout << "Not included in list SBL " << std::endl;
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Not included in list SBL " );
		return false;
	}

	//true면 지우고 false면 참조 감소
	bool Check = StopWorkSBL(URL, uuid);
	if (Check) {
		//Segment 지우는 작업
		Getto->Seg->~SegmenterGroup();
		Getto->FileM->~FileManager();

		delete Getto;//Getto = nullptr;

		CCommonInfo::GetInstance()->WriteLog("INFO", "Success delete SBL - %s", URL.c_str());
	}
	else {
		//Getto->Seg->DecreaseRef();

	}

	return true;
}

std::string HLS_MediaServer::DoWorkSBL(UUID uuid) {

	SBL* Getto = Getto_SBL(uuid);
	GetClientValue GC = { Getto->Type, Getto->ClientUUID, "" };
	CLI WorkInfo = SourceM->GetClientInfo(GC);

	int Check = SourceM->DoWorkClient({ WorkInfo.Type, WorkInfo.uuid, WorkInfo.URL });
	if (Check > 0) {
		debug_->DoWork();
		CCommonInfo::GetInstance()->WriteLog("INFO", "It Works - %s", WorkInfo.URL.c_str());
	}

	//Getto->Seg->DoWork();
	//std::string FileURI = Getto->FileM->DoWork();

	return "";
}

bool HLS_MediaServer::StopWorkSBL(std::string URL, UUID uuid) {

	debug_->StopWork();

	GetClientValue GC = { CT(0), uuid, URL };
	int res = SourceM->DeleteClient(GC);

	if (res == 0) {
		CCommonInfo::GetInstance()->WriteLog("INFO", "Stop SBL Working - %s", URL.c_str());
		return true;
	}
	else {
		//SBL 멈추지 않고 Ref만 줄여야함
		CCommonInfo::GetInstance()->WriteLog("INFO", "Decrease Ref Count - %s count : %d", URL.c_str(), res);
		return false;
	}

	return false;
}

bool HLS_MediaServer::ChangeBitrate(std::string URL, UUID uuid) {
	return true;
}

bool HLS_MediaServer::Request_m3u8() {
	return true;
}

bool HLS_MediaServer::Request_ts() {
	return true;
}

SBL* HLS_MediaServer::Getto_SBL(UUID uuid) {

	SBL* Getto = nullptr;
	int size = SBLL.size();
	if (size == 0) {
		return nullptr;
	}

	for (int i = 0; i < size; i++) {
		if (SBLL.at(i)->ClientUUID == uuid) {
			Getto = SBLL.at(i);
		}
	}

	return Getto;
}