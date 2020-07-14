#pragma once

#include "FFSegmenter.h"
#include "ClientFormat.h"

#define SegmengerCount 2
typedef enum BitrateSupportTable {
	BT_1080 = 1080, BT_720 = 720
}BT;

class SegmenterGroup
{
public:

	SegmenterGroup();
	SegmenterGroup(ST SegType, std::string Filename, bool UseAudio, bool UseTranscoding, int Interval,
		QQ* DataQ, AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
	~SegmenterGroup();

	bool CreateSeg();
	//bool ChangeRunningSeg(int Bitrate);
	int Run(std::string URL);
	int Stop(std::string URL);

public:

	ISegmenter* Seg[2] = {};
	QQ* FrameQ;
	ST SegType = ST_NOT_DEFINE;

private:

	int ParseBitrateToIndex(std::string URL);
	ISegmenter* SetType(ST SegType, std::string Filename);

private:

	const AVCodecID VCo;
	const AVCodecID ACo;
	const bool UseAudio ,UseTranscoding ;
	const int Interval;
	const std::string Filename;

	int RunningSegIndex;
	QQ* DataQ = nullptr;

	bool Running = false;
	int Runindex = 0;
	
};

