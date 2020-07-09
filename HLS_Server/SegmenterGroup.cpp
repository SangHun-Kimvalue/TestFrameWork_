#include "SegmenterGroup.h"


SegmenterGroup::SegmenterGroup() : UseAudio(false), UseTranscoding(false), Interval(5), VCo(AV_CODEC_ID_NONE), ACo(AV_CODEC_ID_NONE), Ref(0)
{

}

SegmenterGroup::SegmenterGroup(bool UseAudio, bool UseTranscoding, int Interval, AVCodecID VCo, AVCodecID ACo)
	:UseAudio(UseAudio), UseTranscoding(UseTranscoding), Interval(Interval), VCo(VCo), ACo(ACo), Ref(0) 
{


}

SegmenterGroup::~SegmenterGroup() {


}

bool CreateSeg() {

	return false;
}

bool ChangeRunningSeg() {

	return false;
}
