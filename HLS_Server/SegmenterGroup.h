#pragma once

#include "ISegmenter.hpp"
#include "ClientFormat.h"

class SegmenterGroup
{
public:

	SegmenterGroup();
	SegmenterGroup(bool UseAudio, bool UseTranscoding, int Interval, AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
	~SegmenterGroup();

	bool CreateSeg();
	bool ChangeRunningSeg();

	std::vector<ISegmenter*> Seg[2] = {};
	QQ* FrameQ;

private:

	const AVCodecID VCo;
	const AVCodecID ACo;
	const bool UseAudio ,UseTranscoding ;
	const int Interval;
	int Ref;
	int RunningSegIndex;

};

