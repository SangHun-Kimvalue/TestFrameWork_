#pragma once

#include "FFSegmenter.h"
#include "ClientFormat.h"
#include <ppl.h>
#include <thread>
#include <Windows.h>

using namespace concurrency;

#define SegmengerCount 1

class SegmenterGroup
{
public:

	SegmenterGroup();
	SegmenterGroup(ST SegType, bool UseAudio, bool UseTranscoding, int Interval,
		QQ DataQ, AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
	~SegmenterGroup();

	bool CreateSeg();
	//bool ChangeRunningSeg(int Bitrate);
	int Run(std::string URL);
	int Stop();

	bool SetPath(char** Path);

public:

	ISegmenter* Seg[2] = {};
	QQ* FrameQ;
	ST SegType = ST_NOT_DEFINE;
	const int SegCount = SegmengerCount;

private:

	int ParseBitrateToIndex(std::string URL);
	ISegmenter* SetType(ST SegType, std::string Filename, int i);
	void DoWork(SegmenterGroup* SG);
	void Notify(std::shared_ptr<MediaFrame>);
	void TimeCheck(SegmenterGroup* SG);

private:

	const AVCodecID VCo;
	const AVCodecID ACo;
	const bool UseAudio ,UseTranscoding ;
	const int Interval;
	std::string Filename;

	std::thread Nofitythr;
	std::thread TimeCheckthr;

	clock_t loopTime;
	clock_t CheckTime;

	char* PathDir[SegmengerCount];

	int RunningSegIndex;
	QQ DataQ = nullptr;

	bool Running = false;
	int Runindex = 0;
	
};

