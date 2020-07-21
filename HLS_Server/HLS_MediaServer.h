#pragma once

#include "SegmenterGroup.h"				//Muxing �� Transcoding
#include "SourceManager.h"				//RTSP ���� Ŭ���̾�Ʈ
#include "FileManager.h"				//output ������ (���� ���� �� ����)	//�̱���

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

class HLS_MediaServer {			//�й� �� ��Ƽ Ŭ���̾�Ʈ ����

public:
	HLS_MediaServer();
	~HLS_MediaServer();

	//uuid �� �������� Ŭ���̾�Ʈ �ʱ� ��û m3u8���Ͽ� ���� uuid�� Ŭ���̾�Ʈ���� �����ؾ���.
	int CreateClient(CT Type, UUID &uuid, std::string URL, int Interval = 5);
	//uuid �޾ƿ;���?

	UUID RequestWithoutUUID(CT Type, std::string URL);
	std::string RequestWithUUID(CT Type, std::string URL, UUID uuid, int Bitrate = 0);
	bool RequestWithFile(UUID uuid, MFT filetype, int resol = 1080);

	//uuid �� ������ Ŭ���̾�Ʈ ��û
	std::string CreateSet(CT Type, std::string URL, UUID uuid, int Interval = 5, int Bitrate = 0);
	//bool IsConnected(std::string URL, UUID uuid);

	std::string DoWorkSBL(std::string URL, UUID uuid);
	bool StopWorkSBL(std::string URL, UUID uuid);
	bool DeleteSet(std::string URL, UUID uuid);

	bool ChangeBitrate(std::string URL, UUID uuid);

	//������
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
