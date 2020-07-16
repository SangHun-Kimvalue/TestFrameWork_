#pragma once
#include "MediaFrame.hpp"
#define DECODE_FAIL_COUNT 60

typedef enum DecoderType {
	DT_DECODE_NONE, DT_SW_FFMPEG, DT_HW_FFMPEG, DT_HW_NVDEC, DT_HW_INTEL
}DT;

__interface IDecoder
{
public:
	
	virtual int Init();
	virtual int Init(int SI, void* StreamInfomation);
	virtual int Decode(MediaFrame*) = 0;
	virtual int Release() = 0;

};
