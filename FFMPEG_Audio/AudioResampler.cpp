#include "stdafx.h"
#include "AudioResampler.h"

AudioResampler::AudioResampler()
{
	src_nb_channels =0;
	src_nb_samples = 0;
	src_ch_layout =  0;
	src_rate = 		 0;
	src_data = 		 0;
	src_linesize = 	 0;

	dst_nb_channels = 0;
	dst_nb_samples = 0;
	max_dst_nb_samples = 0;
	dst_ch_layout = AV_CH_LAYOUT_STEREO;
	dst_rate = 44100;
	dst_sample_fmt = AV_SAMPLE_FMT_FLTP;
	dst_data = NULL;
	dst_linesize = 0;

}

AudioResampler::AudioResampler(AVCodecContext* dec_ctx, AVCodecContext* enc_ctx)
{
	swr_ctx = swr_alloc();

	av_opt_set_int(swr_ctx, "in_channel_count", dec_ctx->channels, 0);
	av_opt_set_int(swr_ctx, "in_channel_layout", dec_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", dec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", dec_ctx->sample_fmt, 0);
	av_opt_set_int(swr_ctx, "out_channel_count", enc_ctx->channels, 0);
	av_opt_set_int(swr_ctx, "out_channel_layout", enc_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", enc_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", enc_ctx->sample_fmt, 0);

	swr_init(swr_ctx);

	// prepare pFrameAudio
	if (!(pAvFrameBuffer = av_frame_alloc())) {
		av_log(NULL, AV_LOG_ERROR, "Alloc frame failed\n");
		return ;
	}
}

AudioResampler::~AudioResampler()
{
	src_data = NULL;
	dst_data = NULL;
	swr_free(&swr_ctx);
	swr_ctx = NULL;
	av_freep(pAvFrameBuffer);
}

int AudioResampler::Init(AVCodecContext* dec_ctx, AVCodecContext* enc_ctx) {
	int ret = 0;

	/* create resampler context */
	swr_ctx = swr_alloc();
	if (!swr_ctx) {
		fprintf(stderr, "Could not allocate resampler context\n");
		exit(1);
	}

	/* set options */
	av_opt_set_int(swr_ctx, "in_channel_layout", dec_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "in_channel_count", dec_ctx->channels, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", dec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", dec_ctx->sample_fmt, 0);

	av_opt_set_int(swr_ctx, "out_channel_layout", enc_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "out_channel_count", enc_ctx->channels, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", enc_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", enc_ctx->sample_fmt, 0);

	/* initialize the resampling context */
	if ((ret = swr_init(swr_ctx)) < 0) {
		fprintf(stderr, "Failed to initialize the resampling context\n");
		exit(1);
	}
	return 0;
}

//dst_ch_layout = AV_CH_LAYOUT_STEREO;  dst_rate = 44100;  dst_sample_fmt = AV_SAMPLE_FMT_FLTP;
int AudioResampler::Init(AVFrame* inFrame, AVCodecContext* dec_ctx, AVCodecContext* enc_ctx) {
	int ret = 0;
	
	
	swr_ctx = swr_alloc();

	av_opt_set_int(swr_ctx, "in_channel_count", dec_ctx->channels, 0);
	av_opt_set_int(swr_ctx, "in_channel_layout", dec_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", dec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", dec_ctx->sample_fmt, 0);

	av_opt_set_int(swr_ctx, "out_channel_count", enc_ctx->channels, 0);
	av_opt_set_int(swr_ctx, "out_channel_layout", enc_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", enc_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", enc_ctx->sample_fmt, 0);

	swr_init(swr_ctx);

	if (inFrame == NULL) {
		std::cout << "Frame is NULL" << std::endl;
		return -1;
	}

	src_nb_channels = dec_ctx->channels;
	src_nb_samples = inFrame->nb_samples;
	src_ch_layout = dec_ctx->channel_layout;
	src_rate = dec_ctx->sample_rate;
	src_sample_fmt = dec_ctx->sample_fmt;
	src_data = inFrame->data;
	src_linesize = 0;

	dst_nb_channels = enc_ctx->channels;
	dst_nb_samples = 0;
	max_dst_nb_samples = 0;
	//dst_ch_layout = AV_CH_LAYOUT_MONO;
	dst_ch_layout = enc_ctx->channel_layout;
	dst_rate = enc_ctx->sample_rate;
	//dst_rate = enc_ctx->bit_rate;
	//dst_rate = 44100;
	dst_sample_fmt = enc_ctx->sample_fmt;
	//av_freep(&dst_data[0]);
	dst_linesize = 0;

	src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
	m_error = av_samples_alloc_array_and_samples(&src_data, &src_linesize, src_nb_channels,
		src_nb_samples, src_sample_fmt, 0);
	if (m_error < 0) {
		fprintf(stderr, "Could not allocate source samples");
	    return -1;
	}

	///* compute the number of converted samples: buffering is avoided
	// * ensuring that the output buffer will contain at least all the
	// * converted input samples */
	max_dst_nb_samples = dst_nb_samples =
		av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
	

	return 0;
}

int utility_init_output_frame(AVFrame **frame, int frame_size, int
	channels, int sample_fmt,
	int sample_rate) {

	int error;
	if (!(*frame = av_frame_alloc())) {
		return AVERROR_EXIT;
	}

	(*frame)->nb_samples = frame_size;
	(*frame)->channels = channels;
	(*frame)->channel_layout = av_get_default_channel_layout(channels);
	(*frame)->format = sample_fmt;
	(*frame)->sample_rate = sample_rate;

	if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
		av_frame_free(frame);
		return error;
	}
	return 0;
}

//return bufsize
int AudioResampler::Resampling(AVFrame* inFrame, AVCodecContext* dec_ctx, AVCodecContext* enc_ctx) {
	
	int ret = 0;
	//AVFrame* outFrame = av_frame_alloc();
	//src_data = inFrame->data;

	//dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) +
	//	src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
	//if (dst_nb_samples > max_dst_nb_samples) {		//없어도 될듯?
	//	av_freep(&outFrame->data[0]);
	//	ret = av_samples_alloc(outFrame->data, &dst_linesize, dst_nb_channels,
	//		dst_nb_samples, dst_sample_fmt, 1);
	//	if (ret < 0)
	//		return NULL;
	//	//break;
	//	max_dst_nb_samples = dst_nb_samples;
	//}
	//
	////* convert to destination format */
	//ret = swr_convert(swr_ctx, outFrame->data, dst_nb_samples, (const uint8_t **)src_data, src_nb_samples);
	//if (ret < 0) {
	//	fprintf(stderr, "Error while converting\n");
	//	return NULL;
	//}

	dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) + src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);

	/* buffer is going to be directly written to a rawaudio file, no alignment */
	dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
	m_error = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 0);
	if (m_error < 0) {
		fprintf(stderr, "Could not allocate destination samples");
		return NULL;
	}

	int linesize;
	ret = av_samples_alloc(dst_data, &linesize,	enc_ctx->channels, dst_nb_samples, dst_sample_fmt, 0);
	//ret = av_samples_alloc(dst_data, &linesize,	enc_ctx->channels, enc_ctx->frame_size, dst_sample_fmt, 0);
	if (ret < 0) {
		std::cout << "sample_alloc Error" << std::endl;
		return NULL;
	}
	enc_ctx->frame_size;
	ret = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t**)inFrame->extended_data, src_nb_samples);
	//ret = swr_convert(swr_ctx, dst_data, enc_ctx->frame_size, (const uint8_t**)inFrame->extended_data, enc_ctx->frame_size);
	if (ret < 0) {
		std::cout << "sample_alloc Error" << std::endl;
		return NULL;
	}
	
	pAvFrameBuffer = NULL;
	//pAvFrameBuffer = av_frame_alloc();
	int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 1);
	if (dst_bufsize < 0) {
		fprintf(stderr, "Could not get sample buffer size\n");
		return NULL;
	}
	//utility_init_output_frame(&pAvFrameBuffer, enc_ctx->frame_size, enc_ctx->channels, enc_ctx->sample_fmt, enc_ctx->sample_rate);
	//utility_init_output_frame(&pAvFrameBuffer, dst_bufsize, enc_ctx->channels, enc_ctx->sample_fmt, enc_ctx->sample_rate);
	//utility_init_output_frame(&pAvFrameBuffer, dst_nb_samples, enc_ctx->channels, enc_ctx->sample_fmt, enc_ctx->sample_rate);
	//pAvFrameBuffer->data[0] = *dst_data;
	//pAvFrameBuffer->nb_samples = dst_nb_samples;
	//pAvFrameBuffer->pts = inFrame->pts;
	//pAvFrameBuffer->linesize[0] = dst_linesize;

	ret = avcodec_fill_audio_frame(pAvFrameBuffer, dst_nb_channels, enc_ctx->sample_fmt, *dst_data, dst_bufsize, 0);
	if (ret < 0) {
		std::cout << "avcodec_fill_audio_frame Error" << std::endl;
		return -1;
	}

	return dst_bufsize;

	//int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
	//	ret, dst_sample_fmt, 1);
	//if (dst_bufsize < 0) {
	//	fprintf(stderr, "Could not get sample buffer size\n");
	//	return -1;
	//}
	//printf("t:%f in:%d out:%d\n", t, src_nb_samples, ret);
	//fwrite(dst_data[0], 1, dst_bufsize, dst_file);

	//} while (t < 10);

	//if ((ret = get_format_from_sample_fmt(&fmt, dst_sample_fmt)) < 0)
	//	return -1;
	//fprintf(stderr, "Resampling succeeded. Play the output file with the command:\n"
	//	"ffplay -f %s -channel_layout %"PRId64" -channels %d -ar %d %s\n",
	//	fmt, dst_ch_layout, dst_nb_channels, dst_rate, dst_filename);
	
	//return NULL;
}

bool AudioResampler::Resampling(const AVFrame *pAvFrameIn, AVCodecContext *dec_ctx, int frame_size, int &k0, AVCodecContext *enc_ctx) {
	
	swr_ctx = swr_alloc();
	av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
	av_opt_set_int(swr_ctx, "out_channel_layout", enc_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", pAvFrameIn->sample_rate, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", enc_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", dec_ctx->sample_fmt, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", enc_ctx->sample_fmt, 0);
	//av_opt_set_int(swr_ctx, "internal_sample_fmt", enc_ctx->sample_fmt, 0);

	swr_init(swr_ctx);
	// prepare pFrameAudio

	if (!(pAvFrameBuffer = av_frame_alloc())) {
		av_log(NULL, AV_LOG_ERROR, "Alloc frame failed\n");
		return false;
	}
	else {
		(pAvFrameBuffer)->format = dec_ctx->sample_fmt;
		(pAvFrameBuffer)->channels = dec_ctx->channels;
		(pAvFrameBuffer)->sample_rate = dec_ctx->sample_rate;
		(pAvFrameBuffer)->nb_samples = frame_size;
		int ret = av_frame_get_buffer(pAvFrameBuffer, 0);
		if (ret < 0) {
			char err[500];
			av_log(NULL, AV_LOG_ERROR, "get audio buffer failed: %s\n",
				av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret));
			return false;
		}
		(pAvFrameBuffer)->nb_samples = 0;
		(pAvFrameBuffer)->pts = pAvFrameIn->pts;
	}

	// copy input data to buffer
	int n_channels = pAvFrameIn->channels;
	int new_samples = min(pAvFrameIn->nb_samples - k0, frame_size - (pAvFrameBuffer)->nb_samples);
	int k1 = (pAvFrameBuffer)->nb_samples;

	if (pAvFrameIn->format == AV_SAMPLE_FMT_S16) {
		int16_t *d_in = (int16_t *)pAvFrameIn->data[0];
		d_in += n_channels * k0;
		int16_t *d_out = (int16_t *)(pAvFrameBuffer)->data[0];
		d_out += n_channels * k1;

		for (int i = 0; i < new_samples; ++i) {
			for (int j = 0; j < pAvFrameIn->channels; ++j) {
				*d_out++ = *d_in++;
			}
		}
	}
	else {
		printf("not handled format for audio buffer\n");
		return false;
	}

	(pAvFrameBuffer)->nb_samples += new_samples;
	k0 += new_samples;

	return true;
}
