#pragma once

//////////////
// INCLUDES //
//////////////
//#include "Debug_logClass.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
}
#include <stdio.h>

enum DecoderType
{
	CPU, DXVA2, CUVID
};

class Decoder_f {
public:
	Decoder_f(const int select) : HA_Usage((DecoderType)select), d_end(false), 
		pFormatCtx(0), pVideoCodecCtx(0), pVideoCodec(0), ConvertCycle(0), PacketReadCycle(0)
	, realFPS(0){
	}
	~Decoder_f() {
	}

	const bool Open(const string inputFile);
	const bool Open_HA(const string inputFile);
	bool CloseFile();
	AVFrame* ReadPacket();

	float ConvertCycle;
	float PacketReadCycle;
	bool d_end;
	float realFPS;

	AVBufferRef *hw_device_ctx = NULL;
	AVFormatContext* pFormatCtx;
	AVCodecContext* pVideoCodecCtx;
	const DecoderType HA_Usage;
	int Megabitrate;

private:

	AVCodec* pVideoCodec;
	AVPacket packet;
	AVFrame *pVFrame;
	AVFrame* RGB_frame;
	int VSI;

	const enum AVPixelFormat *HW_format;
	clock_t move_method_start;
	clock_t move_method_end;

	int Decode();
	bool HA_Decode();
	//AVHWDeviceType HA_Select();

	AVFrame* Con_yuv_YUV420P(AVFrame* YUV_frame);
	AVFrame* Con_yuv_RGB(AVFrame* YUV_frame);
	bool SaveBMP2(char* filename, byte* pImage, int width, int height);

};