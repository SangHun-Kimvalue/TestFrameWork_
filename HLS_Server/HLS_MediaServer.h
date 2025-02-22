#pragma once

#include "SegmenterGroup.h"				//Muxing 및 Transcoding
#include "SourceManager.h"				//RTSP 실제 클라이언트
#include "FileManager.h"				//output 관리자 (폴더 생성 및 삭제)	//싱글톤

//#include "Transcode.h"			//for Debug
#include "FFSegmenter.h"	//for debug

#include <vector>

typedef struct BIND_LIST {

	std::string URL = "";
	CT Type = CT_NOT_DEFINED;

	SegmenterGroup* Seg = nullptr;
	QQ DataQ = nullptr;
	FileManager* FileM = nullptr;
	clock_t Updatetime = clock();

}SBL;

class HLS_MediaServer {			//분배 및 멀티 클라이언트 관리

public:
	HLS_MediaServer(CRITICAL_SECTION m_disLock);
	~HLS_MediaServer();

	//UUID RequestWithoutUUID(CT Type, std::string URL);
	std::string RequestWithUUID(CT Type, std::string URL, int Bitrate = 0);
	bool RequestWithFile(std::string URL, MFT filetype, int resol = 1080);

	bool CreateSet(CT Type, std::string URL, int Interval = 5, int Bitrate = 0);
	bool DoWorkSBL(std::string URL);
	bool StopWorkSBL(std::string URL);
	bool DeleteSet(std::string URL);

	const CLI GetClientInfo(CT Type, std::string URL) const;
	//bool ChangeBitrate(std::string URL, UUID uuid);

	std::vector<SBL*>* GetSBLList();
	//라프텔
private:

	bool CreateClient(CT Type, std::string URL, int Interval = 5);
	SBL* Get_SBL(std::string URL);
	bool Request_m3u8(SBL* m_sbl);
	bool Request_ts(SBL* m_sbl, int resol);

private:

	CRITICAL_SECTION m_disLock;
	std::vector<SBL*> SBLL;
	SourceManager* SourceM;

	//Muxer* debug_;
	//FFSegmenter* debug_;
};
