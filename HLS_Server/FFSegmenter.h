#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ClientFormat.h"
#include "ISegmenter.hpp"
//#include "Muxer.h"
#include "Transcode.h"

extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

//#define STREAM_FRAME_RATE 30

//typedef struct OutputStream {
//	AVStream *st;
//	AVCodecContext *enc;
//
//	/* pts of the next frame that will be generated */
//	int64_t next_pts;
//	int samples_count;
//
//	AVFrame *frame;
//	AVFrame *tmp_frame;
//
//	float t, tincr, tincr2;
//
//	struct SwsContext *sws_ctx;
//	struct SwrContext *swr_ctx;
//} OutputStream;

class FFSegmenter : public ISegmenter
{

public:

	int Init();
	int Run();
	int Close();
	int Stop();
	bool TimeCheck(ISegmenter* mx);

public:
	FFSegmenter();
	FFSegmenter(std::string Filename, ST SegType, QQ* DataQ, bool UseAudio, int Interval, bool Needtranscoding,
		AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
	~FFSegmenter() {}

private:

	int SetOpt();
	void Muxing(FFSegmenter* mx);
	int Wirte_Header(AVFormatContext* pFormatCtx, AVDictionary* opt);
	int Wirte_Trailer(AVFormatContext* pFormatCtx);

	//int write_frame(AVFormatContext *fmt_ctx, AVPacket* pkt, OutputStream* ost);
	int write_frame(AVFormatContext *fmt_ctx, MediaFrame* MF, OutputStream* ost);
	void add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
	AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
	void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/);
	void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/);
	void close_stream(AVFormatContext *oc, OutputStream *ost);

	int write_audio_frame(AVFormatContext *oc, OutputStream *ost, AVPacket *pkt, AVFrame* frame);

public:

	bool Use_Trans;
	std::string Filename;

private:

	clock_t StartingTime;
	clock_t CheckTime;

	AVDictionary *avdic = nullptr;
	QQ* DataQ;
	AVOutputFormat *fmt;
	AVFormatContext *pOutFormatCtx;
	OutputStream video_st = { NULL }, audio_st = { NULL };

	const AVCodecID VCo;
	const AVCodecID ACo;
	AVCodec* VC = nullptr, *AC = nullptr;

	const bool UseAudio;// , UseTranscoding;
	const bool NeedTrans;
	const int Interval;

	int TargetWidth = 0;
	int TargetHeight = 0;

	std::thread DoMuxthr;
	std::thread TimeCheckthr;

	ST SegType = ST_NOT_DEFINE;

	bool Stopped;

};