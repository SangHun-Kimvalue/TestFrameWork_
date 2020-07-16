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

typedef struct BitrateSupportTable {

	int Width;
	int Height;
	int Bitrate;

	BitrateSupportTable(int i) {
		switch (i) {
		case 0:
		default:
			Width = 1920;
			Height = 1080;
			Bitrate = 400000;
			break;
		case 1:
			Width = 1280;
			Height = 720;
			Bitrate = 250000;
			break;
		case 2:
			Width = 720;
			Height = 480;
			Bitrate = 140000;
			break;
		case 3:
			Width = 640;
			Height = 360;
			Bitrate = 100000;
			break;
		}
	}
}BST;

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
	int Run(std::shared_ptr<MediaFrame> Frame);
	//int Run(MediaFrame* Frame);
	int Close();
	int Stop();

public:
	FFSegmenter();
	FFSegmenter(std::string Filename, ST SegType, int index, bool UseAudio, int Interval, bool Needtranscoding,
		AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
	~FFSegmenter();

private:

	int SetOpt();
	//void Muxing(std::shared_ptr<MediaFrame> Frame);
	int Wirte_Header(AVFormatContext* pFormatCtx, AVDictionary* opt);
	int Wirte_Trailer(AVFormatContext* pFormatCtx);

	int inner_encode(AVFrame *frame, AVPacket *pkt, AVCodecContext* c,int *got_packet);
	int write_frame(AVFormatContext *fmt_ctx, std::shared_ptr<MediaFrame> MF, OutputStream* ost);
	void add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
	AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
	void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/);
	void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/);
	void close_stream(AVFormatContext *oc, OutputStream *ost);
	
	int SWScaling(AVCodecContext* c);
	int SWScaling_Init(AVCodecContext* c);

	void TimeCheck(FFSegmenter* SG);

public:
	
private:

	struct SwsContext *sws_ctx;
	enum AVPixelFormat src_pix_fmt = AV_PIX_FMT_NONE, dst_pix_fmt = AV_PIX_FMT_YUV420P;

	clock_t loopTime;
	clock_t CheckTime;

	AVDictionary *avdic = nullptr;
	AVOutputFormat *fmt;
	AVFormatContext *pOutFormatCtx;
	AVFormatContext *ifmt_ctx;
	OutputStream video_st = { NULL }, audio_st = { NULL };

	const AVCodecID VCo;
	const AVCodecID ACo;

	AVCodec* VC = nullptr, *AC = nullptr;

	const bool UseAudio;// , UseTranscoding;
	const bool Encoding;
	const int Interval;
	const BST m_BT;
	const ST SegType = ST_NOT_DEFINE;

	std::thread TimeCheckthr;

	bool HeaderTail;

};