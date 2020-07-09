#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ClientFormat.h"
#include "ISegmenter.hpp"
#include "Muxer.h"
#include "Transcode.h"

#define STREAM_FRAME_RATE 30

class FFSegmenter : public ISegmenter
{

public:

	int Init();
	int Run();
	int Close();
	int Stop();

public:
	FFSegmenter();
	FFSegmenter(std::string Filename, QQ* DataQ, bool UseAudio, int Interval,
		AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
	~FFSegmenter() {}
	
	int GetQSize();

private:

	int SetOpt();
	void Muxing(FFSegmenter* mx);
	int Wirte_Header(AVFormatContext* pFormatCtx, AVDictionary* opt);
	int Wirte_Trailer(AVFormatContext* pFormatCtx);

	int write_frame(AVFormatContext *fmt_ctx, AVPacket* pkt);
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

	AVDictionary *avdic = nullptr;
	QQ* DataQ;
	AVOutputFormat *fmt;
	AVFormatContext *pOutFormatCtx;
	OutputStream video_st = { NULL }, audio_st = { NULL };

	const AVCodecID VCo;
	const AVCodecID ACo;
	AVCodec* VC = nullptr, *AC = nullptr;

	const bool UseAudio;// , UseTranscoding;
	const int Interval;

	int TargetWidth = 0;
	int TargetHeight = 0;

	std::thread DoMux;

	bool Stopped;

};