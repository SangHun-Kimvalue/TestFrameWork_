#pragma once
#include "ClientFormat.h"
//#include "Transcode.h"

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

#define STREAM_FRAME_RATE 30
// a wrapper around a single output AVStream
typedef struct OutputStream {
	AVStream *st;
	AVCodecContext *enc;
	AVCodecContext *dec;

	/* pts of the next frame that will be generated */
	int64_t next_pts;
	int samples_count;

	AVFrame *frame;
	AVFrame *tmp_frame;

	float t, tincr, tincr2;

	struct SwsContext *sws_ctx;
	struct SwrContext *swr_ctx;
} OutputStream;

class Muxer
{
public:
	Muxer() : Filename(""), UseAudio(false), Interval(5), VCo(AV_CODEC_ID_NONE), ACo(AV_CODEC_ID_NONE), DataQ(nullptr), Stopped(false) {}
	Muxer(std::string Filename ,QQ DataQ, bool UseAudio, int Interval, 
		AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE)
		:UseAudio(UseAudio), Interval(Interval),
		VCo(VCo), ACo(ACo), DataQ(DataQ), Stopped(false), Filename(Filename) {
		
		InitMux();
		//DoWork();
	}
	~Muxer(){

		Stopped = true;

		if (DoMux.joinable())
			DoMux.join();

		DataQ = nullptr;

		close_stream(pOutFormatCtx, &video_st);

		if(UseAudio)
			close_stream(pOutFormatCtx, &audio_st);

		if (!(fmt->flags & AVFMT_NOFILE))
			/* Close the output file. */
			avio_closep(&pOutFormatCtx->pb);
		avformat_free_context(pOutFormatCtx);
	}

	void DoWork();
	int GetQSize();
	void StopWork();

public:

	std::string Filename;


private:

	bool InitMux();
	int SetOpt();
	void Muxing(Muxer* mx);

private:

	AVDictionary *avdic = nullptr;
	QQ DataQ;
	AVOutputFormat *fmt;
	AVFormatContext *pOutFormatCtx;
	OutputStream video_st = { 0 }, audio_st = { 0 };

	const AVCodecID VCo;
	const AVCodecID ACo;
	AVCodec* VC = nullptr, *AC = nullptr;

	const bool UseAudio;// , UseTranscoding;
	const int Interval;

	int TargetWidth = 0;
	int TargetHeight = 0;

	int Wirte_Header(AVFormatContext* pFormatCtx, AVDictionary* opt);
	int Wirte_Trailer(AVFormatContext* pFormatCtx);

	int write_frame(AVFormatContext *fmt_ctx, AVPacket* pkt);
	void add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
	AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
	void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/);
	void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/);
	void close_stream(AVFormatContext *oc, OutputStream *ost);

	int write_audio_frame(AVFormatContext *oc, OutputStream *ost, AVPacket *pkt, AVFrame* frame);

	std::thread DoMux;

	bool Stopped;
};

