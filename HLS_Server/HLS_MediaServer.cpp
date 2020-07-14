#include "HLS_MediaServer.h"
#define ___DEBUG

HLS_MediaServer::HLS_MediaServer() {

	SourceM = new SourceManager();
	//FileM = new FileManager();
	
}

HLS_MediaServer::~HLS_MediaServer() {


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

	//CCommonInfo::GetInstance()->WriteLog("INFO", "Success create SBL - %s", URL.c_str());

	return Check;
}

UUID HLS_MediaServer::RequestWithoutUUID(CT Type, std::string URL) {

	UUID ReturnUUID = {};

	int Check = CreateClient(Type, ReturnUUID, URL);

	return ReturnUUID;
}

//uuid 를 포함한 클라이언트 요청
std::string HLS_MediaServer::CreateSet(CT Type, std::string URL, UUID uuid, int Interval, int Bitrate) {

	int Check = false;
	std::string Filename = "";
	GetClientValue GC = { Type, uuid, URL };

	Check = CreateClient(Type, uuid, URL, Interval);
	if (Check < -1) {
		//std::cout << " Error - in Create Set SBL - URL :  " << URL << std::endl;
		CCommonInfo::GetInstance()->WriteLog("ERROR", " Error - in Create Set SBL - URL : %s", URL.c_str());
		return "Error";
	}
	else if (Check == 0) {
		CCommonInfo::GetInstance()->WriteLog("ERROR", " Error - Create new Client - URL : %s", URL.c_str());
		return "Create";
	}
	else if (Check == 1) {

		SBL* AddNewSBL = nullptr;

		AddNewSBL = Get_SBL(uuid);

		if (AddNewSBL == nullptr) {
			AddNewSBL = new SBL();

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
			AddNewSBL->ClientUUID = tempinfo.uuid;
			AddNewSBL->DataQ = SourceM->GetFrameQ(GC).get();

#ifdef __DEBUG
			debug_ = new Muxer("TestFile.m3u8", AddNewSBL->DataQ, UseAudio, Interval, VCo, ACo);
#else
			//ST SegType, std::string Filename, bool UseAudio, bool UseTranscoding, int Interval, QQ* DataQ, AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE
			AddNewSBL->Seg = new SegmenterGroup(ST_FFSW, "TestFile.m3u8", UseAudio, UseTranscoding, Interval, AddNewSBL->DataQ, VCo, ACo);
#endif
			AddNewSBL->FileM = new FileManager();

			SBLL.push_back(AddNewSBL);

			//debug_ = nullptr;
			//debug_ = new FFSegmenter("TestFile.m3u8", ST_FFSW, AddNewSBL->DataQ, UseAudio, Interval, VCo, ACo);

			Filename = DoWorkSBL(AddNewSBL->URL, AddNewSBL->ClientUUID);

			CCommonInfo::GetInstance()->WriteLog("INFO", "Success create SBL - %s", URL.c_str());

			return Filename;
		}
		else {
			Filename = DoWorkSBL(AddNewSBL->URL, AddNewSBL->ClientUUID);
		}

	}
	//UUID temp = SourceM->GetUUID(URL);

	return Filename;
}

std::string HLS_MediaServer::RequestWithUUID(CT Type, std::string URL, UUID uuid, int Bitrate) {

	std::string m3u8FileURI = "";

	m3u8FileURI = CreateSet(Type, URL, uuid);

	return m3u8FileURI;
}

bool HLS_MediaServer::ChangeBitrate(std::string URL, UUID uuid) {
	return true;
}

bool HLS_MediaServer::Request_m3u8(SBL* m_sbl) {
	
	//m_sbl->Seg->Getm3u8()
	
	return true;
}

bool HLS_MediaServer::Request_ts(SBL* m_sbl, int resol) {

	
	//m_sbl->Seg->ChangeRunningSeg(/*resol*/);

	return true;
}

bool HLS_MediaServer::RequestWithFile(UUID uuid, MFT filetype, int resol) {
	
	bool Check = false;

	SBL* sbl = Get_SBL(uuid);
	if (sbl == nullptr) {
		unsigned char* str = nullptr;
		UuidToStringA(&uuid, &str);
		std::cout << "Could not find in SBL list - " << str << std::endl;
		return false;
	}

	if (filetype == MFT_M3U8) {
		Check = Request_m3u8(sbl);
	}
	else if (filetype == MFT_TS) {
		Check = Request_ts(sbl, resol);
	}
	else if (filetype == MFT_MKV) {
		//Check = Request_ts();
	}
	else if (filetype == MFT_MP4) {
		//Check = Request_ts();
	}

	return Check;
}

bool HLS_MediaServer::DeleteSet(std::string URL, UUID uuid) {
	
	SBL* Getto = Get_SBL(uuid);
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

std::string HLS_MediaServer::DoWorkSBL(std::string URL, UUID uuid) {

	SBL* Getto = Get_SBL(uuid);
	GetClientValue GC = { Getto->Type, Getto->ClientUUID, "" };
	CLI WorkInfo = SourceM->GetClientInfo(GC);

	int Check = SourceM->DoWorkClient({ WorkInfo.Type, WorkInfo.uuid, WorkInfo.URL });
	if (Check == 1) {

#ifdef __DEBUG
		debug_->DoWork();
#else
		Getto->Seg->Run(URL);
#endif
		CCommonInfo::GetInstance()->WriteLog("INFO", "It Works - %s", WorkInfo.URL.c_str());
	}
	else if (Check > 1) {

#ifdef __DEBUG
		debug_->DoWork();
#else
		Getto->Seg->Run(URL);
#endif
		std::cout << "SBL is already work - " << WorkInfo.URL.c_str() << std::endl;
	}
	else {
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Error in DoWork - %s", WorkInfo.URL.c_str());
	}

	//Getto->Seg->DoWork();
	//std::string FileURI = Getto->FileM->DoWork();

	return "";
}

bool HLS_MediaServer::StopWorkSBL(std::string URL, UUID uuid) {

	SBL* Getto = Get_SBL(uuid);
	GetClientValue GC = { CT(0), uuid, URL };

#ifdef __DEBUG
	debug_->StopWork();
#else
	int ref = Getto->Seg->Stop(URL);
#endif

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

SBL* HLS_MediaServer::Get_SBL(UUID uuid) {

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