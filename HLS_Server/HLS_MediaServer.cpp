#include "HLS_MediaServer.h"
#define ___DEBUG

HLS_MediaServer::HLS_MediaServer(CRITICAL_SECTION m_disLock) :m_disLock(m_disLock){

	SourceM = new SourceManager();
	//FileM = new FileManager();

	SBL* It = new SBL({ "", CT_NOT_DEFINED, nullptr, nullptr, nullptr, clock() });
	SBLL.push_back(It);
	
}

HLS_MediaServer::~HLS_MediaServer() {


}


std::vector<SBL*>* HLS_MediaServer::GetSBLList() {
	return &SBLL;
}

bool HLS_MediaServer::CreateClient(CT Type, std::string URL, int Interval) {

	CLI TempCLI = {};
	TempCLI.Type = Type;
	TempCLI.URL = URL;
	TempCLI.Interval = Interval;

	GetClientValue GC = { Type, URL };

	//If return 1 == Already had, 0 == Create Success
	int Check = SourceM->Create_Client(TempCLI);
	TempCLI = SourceM->GetClientInfo(GC);
	if (Check == 0) {
		std::cout << "Create Client Success - URL : " << URL.c_str() << std::endl;
		SourceM->PrintInfo(GC);
		return true;
	}
	else if (Check == 1) {
		std::cout << "Already has Client  - URL : " << URL.c_str() << std::endl;
		return true;
	}
	else {
		std::cout << "Create Client Failed - URL : " << URL.c_str() << std::endl;
		return false;
	}

	//CCommonInfo::GetInstance()->WriteLog("INFO", "Success create SBL - %s", URL.c_str());

	return false;
}

bool HLS_MediaServer::CreateSet(CT Type, std::string URL, int Interval, int Bitrate) {

	bool Check = false;
	std::string Filename = "Error";
	GetClientValue GC = { Type, URL };
	SBL* AddNewSBL = nullptr;

	Check = CreateClient(Type, URL/*, Interval*/);
	if (!Check) {
		//std::cout << " Error - in Create Set SBL - URL :  " << URL << std::endl;
		CCommonInfo::GetInstance()->WriteLog("ERROR", " Error - in Create Set SBL - URL : %s", URL.c_str());
		return false;
	}
	else{

		AddNewSBL = Get_SBL(URL);

		if (AddNewSBL == nullptr) {
			AddNewSBL = new SBL();

			CLI tempinfo = SourceM->GetClientInfo(GC);
			GC.Type = tempinfo.Type;
			//GC.uuid = tempinfo.uuid;
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
			//AddNewSBL->ClientUUID = tempinfo.uuid;
			//AddNewSBL->DataQ = SourceM->GetFrameQ(GC).get();
			AddNewSBL->DataQ = SourceM->GetFrameQ(GC);

#ifdef __DEBUG
			debug_ = new Muxer("TestFile.m3u8", AddNewSBL->DataQ, UseAudio, Interval, VCo, ACo);
#else
			//ST SegType, std::string Filename, bool UseAudio, bool UseTranscoding, int Interval, QQ* DataQ, AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE
			AddNewSBL->Seg = new SegmenterGroup(ST_FFSW, UseAudio, UseTranscoding, Interval, AddNewSBL->DataQ, VCo, ACo);
#endif
			AddNewSBL->FileM = new FileManager(AddNewSBL->URL);
			char** DirPath = nullptr;
			DirPath = AddNewSBL->FileM->GetDirPath();
			AddNewSBL->Seg->SetPath(DirPath);
			AddNewSBL->Seg->CreateSeg();

			EnterCriticalSection(&m_disLock);
			SBLL.push_back(AddNewSBL);
		
			//debug_ = nullptr;
			//debug_ = new FFSegmenter("TestFile.m3u8", ST_FFSW, AddNewSBL->DataQ, UseAudio, Interval, VCo, ACo);

			Check = DoWorkSBL(AddNewSBL->URL);
			AddNewSBL->Updatetime = clock();
			LeaveCriticalSection(&m_disLock);

			CCommonInfo::GetInstance()->WriteLog("INFO", "Success create SBL - %s", URL.c_str());

			return true;
		}
		else {
			EnterCriticalSection(&m_disLock);
			Filename = DoWorkSBL(AddNewSBL->URL);
			AddNewSBL->Updatetime = clock();
			LeaveCriticalSection(&m_disLock);
			return true;
		}

	}
	//UUID temp = SourceM->GetUUID(URL);

	return false;
}

std::string HLS_MediaServer::RequestWithUUID(CT Type, std::string URL, int Bitrate) {

	std::string m3u8FileURI = "";
	bool Check = false;
	Check = CreateSet(Type, URL);

	return m3u8FileURI;
}

//bool HLS_MediaServer::ChangeBitrate(std::string URL, UUID uuid) {
//	return true;
//}

bool HLS_MediaServer::Request_m3u8(SBL* m_sbl) {
	
	//m_sbl->Seg->Getm3u8()
	
	return true;
}

bool HLS_MediaServer::Request_ts(SBL* m_sbl, int resol) {

	
	//m_sbl->Seg->ChangeRunningSeg(/*resol*/);

	return true;
}

bool HLS_MediaServer::RequestWithFile(std::string URL, MFT filetype, int resol) {
	
	bool Check = false;

	SBL* sbl = Get_SBL(URL);
	if (sbl == nullptr) {
		//unsigned char* str = nullptr;
		//UuidToStringA(&uuid, &str);
		//std::cout << "Could not find in SBL list - " << str << std::endl;
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
	
bool HLS_MediaServer::DeleteSet(std::string URL) {
	
	SBL* Getto = Get_SBL(URL);
	if (Getto == nullptr) {
		//std::cout << "Not included in list SBL " << std::endl;
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Not included in list SBL " );
		return false;
	}
	EnterCriticalSection(&m_disLock);
	for (int i = 0; i < SBLL.size(); i++) {
		if (SBLL.at(i)->URL == URL) {
			SBLL.erase(SBLL.begin()+i);
		}
	}
	LeaveCriticalSection(&m_disLock);
	//true면 지우고 false면 참조 감소
	bool Check = StopWorkSBL(URL);

	//Segment 지우는 작업
	Getto->Seg->Stop();
	Getto->Seg->~SegmenterGroup();
	//Getto->FileM->~FileManager();
	delete Getto;//Getto = nullptr;

	CCommonInfo::GetInstance()->WriteLog("INFO", "Success delete SBL - %s", URL.c_str());

	return true;
}

const CLI  HLS_MediaServer::GetClientInfo(CT Type, std::string URL) const {
	GetClientValue GC = { Type, URL };
	return SourceM->GetClientInfo(GC);
}

bool HLS_MediaServer::DoWorkSBL(std::string URL) {

	SBL* Getto = Get_SBL(URL);
	GetClientValue GC = { Getto->Type, URL };
	CLI WorkInfo = SourceM->GetClientInfo(GC);

	int Check = SourceM->DoWorkClient({ WorkInfo.Type, WorkInfo.URL });
	if (Check == 1) {

#ifdef __DEBUG
		debug_->DoWork();
#else
		Getto->Seg->Run(URL);
#endif
		//unsigned char* str;
		//UuidToStringA(&WorkInfo.uuid, &str);
		//string str_array(reinterpret_cast<char const *>(str));
		//std::string url_uuid = WorkInfo.URL.c_str();	
		//url_uuid = url_uuid + " / ";	
		//url_uuid  = url_uuid + str_array;
		CCommonInfo::GetInstance()->WriteLog("INFO", "It Works - %s", URL.c_str());
		return true;
	}
	/*else if (Check > 1) {

#ifdef __DEBUG
		debug_->DoWork();
#else
		Getto->Seg->Run(URL);
#endif
		std::cout << "SBL is already work - " << WorkInfo.URL.c_str() << std::endl;
	}*/

	else {
		CCommonInfo::GetInstance()->WriteLog("ERROR", "Error in DoWork - %s", WorkInfo.URL.c_str());
		return false;
	}

	//Getto->Seg->DoWork();
	//std::string FileURI = Getto->FileM->DoWork();

	return false;
}

bool HLS_MediaServer::StopWorkSBL(std::string URL) {

	SBL* Getto = Get_SBL(URL);
	GetClientValue GC = { CT(0), URL };

#ifdef __DEBUG
	debug_->StopWork();
#else
	int ref = Getto->Seg->Stop();
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

SBL* HLS_MediaServer::Get_SBL(std::string URL) {

	EnterCriticalSection(&m_disLock);
	SBL* Getto = nullptr;
	int size = SBLL.size();
	if (size == 0) {
		return nullptr;
	}

	for (int i = 0; i < size; i++) {
		if (SBLL.at(i)->URL == URL) {
			Getto = SBLL.at(i);
		}
	}
	LeaveCriticalSection(&m_disLock);

	return Getto;
}