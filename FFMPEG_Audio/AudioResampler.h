#pragma once

#include "stdafx.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

class AudioResampler
{
public:
	AudioResampler();
	AudioResampler(AVCodecContext* dec_ctx, AVCodecContext* enc_ctx);
	~AudioResampler();

	int Init(AVFrame* inFrame, AVCodecContext* dec_ctx, AVCodecContext* enc_ctx);
	int Init(AVCodecContext* dec_ctx, AVCodecContext* enc_ctx);
	int Resampling(AVFrame* inFrame, AVCodecContext* dec_ctx, AVCodecContext* enc_ctx);
	bool Resampling(const AVFrame *pAvFrameIn, AVCodecContext *dec_ctx, int frame_size, int &k0, AVCodecContext *enc_ctx);

	AVFrame* pAvFrameBuffer;

private:

	struct SwrContext *	swr_ctx;
	int src_nb_channels ;
	int dst_nb_channels;
	int src_nb_samples, dst_nb_samples, max_dst_nb_samples;
	int64_t src_ch_layout, dst_ch_layout;
	int src_rate, dst_rate;
	enum AVSampleFormat src_sample_fmt, dst_sample_fmt;
	uint8_t **src_data, **dst_data = NULL;
	int src_linesize;	
	int dst_linesize;

	int m_error;
};

