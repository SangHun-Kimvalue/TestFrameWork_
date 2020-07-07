#pragma once
#include "MediaFrame.hpp"
#define DECODE_FAIL_COUNT 60

typedef enum DecoderType {
	DECODE_NONE, SW_FFMPEG, HW_FFMPEG, HW_NVDEC, HW_INTEL
}DT;

__interface IDecoder
{
public:
	
	virtual int Init();
	virtual int Init(int SI, void* StreamInfomation);
	virtual int Decode(MediaFrame*) = 0;
	virtual int Release() = 0;

};
