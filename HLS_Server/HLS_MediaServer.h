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
	UUID ClientUUID = {};

	SegmenterGroup* Seg = nullptr;
	QQ DataQ = nullptr;
	FileManager* FileM = nullptr;

}SBL;

class HLS_MediaServer {			//분배 및 멀티 클라이언트 관리

public:
	HLS_MediaServer();
	~HLS_MediaServer();

	//uuid 를 미포함한 클라이언트 초기 요청 m3u8파일에 의한 uuid를 클라이언트에게 전달해야함.
	int CreateClient(CT Type, UUID &uuid, std::string URL, int Interval = 5);
	//uuid 받아와얗라듯?

	UUID RequestWithoutUUID(CT Type, std::string URL);
	std::string RequestWithUUID(CT Type, std::string URL, UUID uuid, int Bitrate = 0);
	bool RequestWithFile(UUID uuid, MFT filetype, int resol = 1080);

	//uuid 를 포함한 클라이언트 요청
	std::string CreateSet(CT Type, std::string URL, UUID uuid, int Interval = 5, int Bitrate = 0);
	//bool IsConnected(std::string URL, UUID uuid);

	std::string DoWorkSBL(std::string URL, UUID uuid);
	bool StopWorkSBL(std::string URL, UUID uuid);
	bool DeleteSet(std::string URL, UUID uuid);

	bool ChangeBitrate(std::string URL, UUID uuid);

	//라프텔
private:

	SBL* Get_SBL(UUID uuid);
	bool Request_m3u8(SBL* m_sbl);
	bool Request_ts(SBL* m_sbl, int resol);

private:

	std::vector<SBL*> SBLL;
	SourceManager* SourceM;

	//Muxer* debug_;
	//FFSegmenter* debug_;
};
