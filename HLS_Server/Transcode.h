#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Muxer.h"

#define STREAM_DURATION   10.0
//#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

#define SCALE_FLAGS SWS_BICUBIC

class Transcoder
{

public:

	int init(std::string Filename);
	int run();
	int close();

private:

	AVPacket packet = { 0, };
	int vidx = 0, aidx = 0;
	long long nGap = 0;
	long long nGap2 = 1;

	OutputStream video_st = { 0 }, audio_st = { 0 };
	const char *filename;
	AVOutputFormat *fmt;
	AVFormatContext *oc;
	AVCodec *audio_codec = NULL, *video_codec = NULL;
	int ret;
	int have_video = 0, have_audio = 0;
	int encode_video = 0, encode_audio = 0;
	AVDictionary *opt = NULL;
	int i = 0;
	AVFormatContext* ctx = avformat_alloc_context();
};

