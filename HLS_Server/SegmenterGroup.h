#pragma once

#include "ISegmenter.hpp"
#include "ClientFormat.h"

class SegmenterGroup
{
public:

	SegmenterGroup() :UseAudio(false), UseTranscoding(false), Interval(5), VCo(AV_CODEC_ID_NONE), ACo(AV_CODEC_ID_NONE), Ref(0)
	{}
	SegmenterGroup(bool UseAudio, bool UseTranscoding, int Interval, AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE)
	:UseAudio(UseAudio), UseTranscoding(UseTranscoding), Interval(Interval), VCo(VCo), ACo(ACo), Ref(0){}
	~SegmenterGroup() {}

	bool CreateSeg() {}
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

