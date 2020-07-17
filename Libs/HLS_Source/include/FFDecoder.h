#pragma once
#include <iostream>
#include "IDecoder.h"

extern "C"
{
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavdevice/avdevice.h>
}

class FFmpegDecoder : public IDecoder
{
public:
	FFmpegDecoder();
	~FFmpegDecoder();

	// NotUse : Init(int SI, AVFormatContext*);
	int Init() { return 0; }
	int Init(int SI, void*);
	int Decode(MediaFrame*);
	int Release();

private:

	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVCodecID pCodecID;

	FI F_info;
	int m_SI;
	int FailCount;
	//AVFrame *pFrame;// , *pFrameBGR;
	//AVPacket *packet;

};
