#pragma once
//
//  ffmpegdecoder.h
//  ffmpeg RTSP Client
//
//  Created by Yonghye Kwon on 9/14/18.
//  Copyright (c) Freeeeeeeeeeeeeee
//

#ifndef ffmpegdecoder_H
#define ffmpegdecoder_H

#include <iostream>
#include <string>

#include <thread>
#include <mutex>
#include <chrono>

extern "C"
{
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavdevice/avdevice.h>

}

class FFmpegDecoder
{
public:
	FFmpegDecoder(std::string, bool);
	~FFmpegDecoder();

	int connect();
	int decode();
	void Stop() { Stopped = true; }

	bool isConncected() const { return bConnected; }
	const char* GetURL() const { return path.c_str(); }
	AVCodecID GetCodec() const { return pCodec->id; }

	std::mutex mtx;

private:

	void destroy();

	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	//AVFrame *pFrame;// , *pFrameBGR;
	AVPacket *packet;
	uint8_t *outBuffer;
	SwsContext *imgConvertCtx;

	int videoStream;
	int audioStream;

	std::string path;
	//Q ÇÊ¿ä

	bool Stopped;
	bool bConnected;
};

#endif