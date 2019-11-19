#pragma once
#include <Windows.h>
#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <list>
#include <thread>

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
//#include <libswscale/swscale.h>
//#include <libswresample/swresample.h>
}

#include "wav.h"

#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

#define SCALE_FLAGS SWS_BICUBIC

char errstr[256];
int error = 0;

typedef struct StreamContext {
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
	AVFormatContext *ifmt_ctx;
	AVStream *out_stream = NULL;
} StreamContext;
static StreamContext *Astream_ctx;
static StreamContext *Vstream_ctx;
AVFormatContext *ofmt_ctx = NULL;

static int Vopen_input_file(const char *filename);
static int Vopen_output_file(const char *filename);
static int Aopen_output_file(const char *filename);
static int Aopen_input_file();
static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt);
static int Muxing(bool encode_video, bool encode_audio, int64_t Vpkt_pts, int64_t Apkt_pts, AVPacket* Vpkt, AVPacket* Apkt);
void Work();
int Video_Init(const char* outfile);
int Audio_Init(const char* outfile);
void Close();


DWORD convert_unicode_to_utf8_string(__out std::string& utf8, __in const wchar_t* unicode, __in const size_t unicode_size);
void error_pro(int error, const char* msg);