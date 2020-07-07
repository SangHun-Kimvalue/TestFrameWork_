#include "Muxer.h"

const char* merr_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;

	return errstr;
}

void Muxer::DoWork() {

	DoMux = std::thread([&]() { Muxing(this); });

	return;
}

bool Muxer::InitMux() {
	
	VC = nullptr;
	AC = nullptr;

	pOutFormatCtx = avformat_alloc_context();

	avformat_alloc_output_context2(&pOutFormatCtx, NULL, "hls", Filename.c_str());
	if (!pOutFormatCtx) {
		printf("Could not deduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(&pOutFormatCtx, NULL, "mpeg", Filename.c_str());
	}
	if (!pOutFormatCtx)
		return 1;
	
	fmt = pOutFormatCtx->oformat;

	if (fmt->video_codec != AV_CODEC_ID_NONE) {
		add_stream(&video_st, pOutFormatCtx, &VC, fmt->video_codec);
	}
	if (fmt->audio_codec != AV_CODEC_ID_NONE && UseAudio == true) {
		add_stream(&audio_st, pOutFormatCtx, &AC, fmt->audio_codec);
	}

	open_video(pOutFormatCtx, VC, &video_st);
	//   HLS Option 찾아봐야함.
	
	if (UseAudio)
		open_audio(pOutFormatCtx, AC, &audio_st);

	/* open the output file, if needed */
	if (!(fmt->flags & AVFMT_NOFILE)) {
		int ret = avio_open(&pOutFormatCtx->pb, Filename.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			//fprintf(stderr, "Could not open '%s': %s\n", filename,
			//	av_err2str(ret));
			return 1;
		}
	}

	return true;
}


int Muxer::write_frame(AVFormatContext *fmt_ctx, AVPacket* pkt)
{
	int ret = 0;

	//// send the frame to the encoder
	//ret = avcodec_send_frame(c, frame);
	//if (ret < 0) {
	//	return -1;
	//}

	//while (ret >= 0) {
		//AVPacket pkt = { 0 };

		//ret = avcodec_receive_packet(c, &pkt);
		//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		//	break;
		//else if (ret < 0) {
		//	return -1;
		//}

		/* rescale output packet timestamp values from codec to stream timebase */
		//av_packet_rescale_ts(&pkt, c->time_base, st->time_base);
		//pkt.stream_index = st->index;

		/* Write the compressed frame to the media file. */
		ret = av_interleaved_write_frame(fmt_ctx, pkt);
		av_packet_unref(pkt);
		if (ret < 0) {
			return -1;
		}
	//}

	return ret == AVERROR_EOF ? 1 : 0;
}

void Muxer::add_stream(OutputStream *ost, AVFormatContext *oc,
	AVCodec **codec,
	enum AVCodecID codec_id)
{
	AVCodecContext *c;
	int i;

	/* find the encoder */
	*codec = avcodec_find_encoder(codec_id);
	if (!(*codec)) {
		fprintf(stderr, "Could not find encoder for '%s'\n",
			avcodec_get_name(codec_id));
		exit(1);
	}

	ost->st = avformat_new_stream(oc, NULL);
	if (!ost->st) {
		fprintf(stderr, "Could not allocate stream\n");
		exit(1);
	}
	ost->st->id = oc->nb_streams - 1;
	c = avcodec_alloc_context3(*codec);
	if (!c) {
		fprintf(stderr, "Could not alloc an encoding context\n");
		exit(1);
	}
	ost->enc = c;

	switch ((*codec)->type) {
	case AVMEDIA_TYPE_AUDIO:
		c->sample_fmt = (*codec)->sample_fmts ?
			(*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
		c->bit_rate = 64000;
		c->sample_rate = 44100;
		if ((*codec)->supported_samplerates) {
			c->sample_rate = (*codec)->supported_samplerates[0];
			for (i = 0; (*codec)->supported_samplerates[i]; i++) {
				if ((*codec)->supported_samplerates[i] == 44100)
					c->sample_rate = 44100;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		c->channel_layout = AV_CH_LAYOUT_STEREO;
		if ((*codec)->channel_layouts) {
			c->channel_layout = (*codec)->channel_layouts[0];
			for (i = 0; (*codec)->channel_layouts[i]; i++) {
				if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
					c->channel_layout = AV_CH_LAYOUT_STEREO;
			}
		}
		c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
		ost->st->time_base = { 1, c->sample_rate };
		break;

	case AVMEDIA_TYPE_VIDEO:
		c->codec_id = codec_id;

		c->bit_rate = 400000;
		/* Resolution must be a multiple of two. */
		c->width = 1920;
		c->height = 1080;
		/* timebase: This is the fundamental unit of time (in seconds) in terms
		 * of which frame timestamps are represented. For fixed-fps content,
		 * timebase should be 1/framerate and timestamp increments should be
		 * identical to 1. */
		ost->st->time_base ={ 1, STREAM_FRAME_RATE };
		c->time_base = ost->st->time_base;

		c->gop_size = 12; /* emit one intra frame every twelve frames at most */
		c->pix_fmt = AV_PIX_FMT_YUV420P;
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
			/* just for testing, we also add B-frames */
			c->max_b_frames = 2;
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			 * This does not happen with normal video, it just happens here as
			 * the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
		}
		break;

	default:
		break;
	}

	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}


AVFrame* Muxer::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
	AVFrame *picture;
	int ret;

	picture = av_frame_alloc();
	if (!picture)
		return NULL;

	picture->format = pix_fmt;
	picture->width = width;
	picture->height = height;

	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(picture, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate frame data.\n");
		exit(1);
	}

	return picture;
}

void Muxer::open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/)
{
	int ret;
	AVCodecContext *c = ost->enc;
	//AVDictionary *opt = NULL;

	//av_dict_copy(&opt, opt_arg, 0);

	/* open the codec */
	ret = avcodec_open2(c, codec, NULL);
	//ret = avcodec_open2(c, codec, &opt);
	//av_dict_free(&opt);
	if (ret < 0) {
		std::cout << "Invalid argument error in Video AVCodec Open" << std::endl;
		//fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
		exit(1);
	}

	/* allocate and init a re-usable frame */
	ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
	if (!ost->frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}

	/* If the output format is not YUV420P, then a temporary YUV420P
	 * picture is needed too. It is then converted to the required
	 * output format. */
	ost->tmp_frame = NULL;
	if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
		ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
		if (!ost->tmp_frame) {
			fprintf(stderr, "Could not allocate temporary picture\n");
			exit(1);
		}
	}

	/* copy the stream parameters to the muxer */
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		exit(1);
	}
}

void Muxer::open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/)
{
	AVCodecContext *c;
	int nb_samples;
	int ret;
	//AVDictionary *opt = NULL;

	c = ost->enc;

	/* open it */
	//av_dict_copy(&opt, opt_arg, 0);
	ret = avcodec_open2(c, codec, NULL);
	//ret = avcodec_open2(c, codec, &opt);
	//av_dict_free(&opt);
	if (ret < 0) {
		std::cout << "Invalid argument error in Video AVCodec Open" << std::endl;
		exit(1);
	}

	/* init signal generator */
	ost->t = 0;
	ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
	/* increment frequency by 110 Hz per second */
	ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

	if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		nb_samples = 10000;
	else
		nb_samples = c->frame_size;

	ost->frame = av_frame_alloc();
		//alloc_audio_frame(/*c->sample_fmt, c->channel_layout,	c->sample_rate, nb_samples*/);
	ost->tmp_frame = av_frame_alloc();
	//ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
	//	c->sample_rate, nb_samples);

	/* copy the stream parameters to the muxer */
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);
	if (ret < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		exit(1);
	}

	/* create resampler context */
	ost->swr_ctx = swr_alloc();
	if (!ost->swr_ctx) {
		fprintf(stderr, "Could not allocate resampler context\n");
		exit(1);
	}

	/* set options */
	av_opt_set_int(ost->swr_ctx, "in_channel_count", c->channels, 0);
	av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
	av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(ost->swr_ctx, "out_channel_count", c->channels, 0);
	av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
	av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);

	/* initialize the resampling context */
	if ((ret = swr_init(ost->swr_ctx)) < 0) {
		fprintf(stderr, "Failed to initialize the resampling context\n");
		exit(1);
	}
}

void Muxer::close_stream(AVFormatContext *oc, OutputStream *ost)
{
	avcodec_free_context(&ost->enc);
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
}

//트랜스코딩용
static AVFrame *get_video_frame(OutputStream *ost)
{
	AVCodecContext *c = ost->enc;

	/* check if we want to generate more frames */
	if (av_compare_ts(ost->next_pts, c->time_base,
		10,{ 1, 1 }) > 0)			//interval필요
		return NULL;

	/* when we pass a frame to the encoder, it may keep a reference to it
	 * internally; make sure we do not overwrite it here */
	if (av_frame_make_writable(ost->frame) < 0)
		return nullptr;

	//if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
	//	/* as we only generate a YUV420P picture, we must convert it
	//	 * to the codec pixel format if needed */
	//	if (!ost->sws_ctx) {
	//		ost->sws_ctx = sws_getContext(c->width, c->height,
	//			AV_PIX_FMT_YUV420P,
	//			c->width, c->height,
	//			c->pix_fmt,
	//			SCALE_FLAGS, NULL, NULL, NULL);
	//		if (!ost->sws_ctx) {
	//			fprintf(stderr,
	//				"Could not initialize the conversion context\n");
	//			exit(1);
	//		}
	//	}
		/*fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
		sws_scale(ost->sws_ctx, (const uint8_t * const *)ost->tmp_frame->data,
			ost->tmp_frame->linesize, 0, c->height, ost->frame->data,
			ost->frame->linesize);*/
	//}
	//else {
		//fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
	//}
	//Get QQQQ


	ost->frame->pts = ost->next_pts++;

	return ost->frame;
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
//static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
//{
//	return write_frame(oc, ost->enc, ost->st);
//
//}

//트랜스코딩용
int Muxer::write_audio_frame(AVFormatContext *oc, OutputStream *ost, AVPacket *pkt, AVFrame* frame)
{
	AVCodecContext *c;
	//AVFrame *frame;
	int ret;
	int dst_nb_samples;

	c = ost->enc;

	//frame = get_audio_frame(ost);

	//if (frame) {
		/* convert samples from native format to destination codec format, using the resampler */
			/* compute destination number of samples */
		dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
			c->sample_rate, c->sample_rate, AV_ROUND_UP);
		av_assert0(dst_nb_samples == frame->nb_samples);

		/* when we pass a frame to the encoder, it may keep a reference to it
		 * internally;
		 * make sure we do not overwrite it here
		 */
		ret = av_frame_make_writable(ost->frame);
		if (ret < 0)
			exit(1);

		/* convert to destination format */
		ret = swr_convert(ost->swr_ctx,
			ost->frame->data, dst_nb_samples,
			(const uint8_t **)frame->data, frame->nb_samples);
		if (ret < 0) {
			fprintf(stderr, "Error while converting\n");
			exit(1);
		}
		frame = ost->frame;

		frame->pts = av_rescale_q(ost->samples_count, { 1, c->sample_rate }, c->time_base);
		ost->samples_count += dst_nb_samples;
	//}

	return write_frame(oc, pkt);
}

void Muxer::Muxing(Muxer* mx) {

	int encode_video = 0;
	int encode_audio = 0;

	Wirte_Header(pOutFormatCtx, avdic);

	while (!Stopped) {

		if (DataQ->size() > 0) {

			MediaFrame* Frame = nullptr;
			Frame = DataQ->pop();

			if(Frame->Info.StreamId == 0){
				
				if (UseAudio) {
					if (av_compare_ts(video_st.next_pts, video_st.enc->time_base,
						audio_st.next_pts, audio_st.enc->time_base) <= 0) {
						encode_video = !write_frame(pOutFormatCtx, Frame->Pkt);
					}
				}
				else
					encode_video = !write_frame(pOutFormatCtx, Frame->Pkt);
			}
			else if(Frame->Info.StreamId == 1 && UseAudio == true) {

				encode_audio = !write_frame(pOutFormatCtx, Frame->Pkt);
			}
		}

		//std::cout << "\t\t Muxer Q size : " << GetQSize() << std::endl;
		Sleep(1);

	}

	Wirte_Trailer(pOutFormatCtx);

	return;
}

void Muxer::StopWork() {
	Stopped = true;
	if (DoMux.joinable())
		DoMux.join();
}

int Muxer::GetQSize() {
	return DataQ->size();
}

int Muxer::SetOpt() {

	int ret = av_dict_set(&avdic, "max_delay", "50", 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set(&avdic, "segment_time", "5", 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set(&avdic, "segment_list_flags", "live", 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set(&avdic, "segment_list_size", "6", 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set(&avdic, "segment_warp", "0", 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set(&avdic, "hls_delete_threshold", "7", 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_count(avdic);

	ret = av_dict_set_int(&avdic, "max_delay", 50, 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set_int(&avdic, "segment_time", 5, 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set_int(&avdic, "segment_list_size", 6, 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set_int(&avdic, "segment_warp", 0, 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set_int(&avdic, "hls_delete_threshold", 7, 0);
	if (ret < 0) {
		return ret;
	}
	ret = av_dict_set(&avdic, "segment_list ", "1080", 0);
	if (ret < 0) {
		return ret;
	}

	//필요할듯
	//ret = av_dict_set(&avdic, "hls_start_number_source", "0", 0);
	//if (ret < 0) {
	//	return ret;
	//}
	//start-segments-with-iframe설명 이 필요합니다. 이름에서 알 수 있듯이 
	//이 옵션은 각 세그먼트가 I- 프레임으로 시작되도록 보장합니다. 이것이 왜 중요한가? 
	//스트림을 전환 할 때 비디오를 최대한 원활하게 재생합니다.
	//method
	//ffmpeg -re -i in.ts -f hls -method PUT http://example.com/live/out.m3u8

	ret = av_dict_count(avdic);

	return ret;
}

int Muxer::Wirte_Header(AVFormatContext* pFormatCtx, AVDictionary* opt) {

	int ret = SetOpt();
	if (ret >= 0) {
		std::cout << "Options Set up Success " << std::endl;
	}
	else {
		std::cout << "Error - Options Set up Failed - " << pFormatCtx->url << std::endl;
		return -1;
	}

	ret = avformat_write_header(pFormatCtx, &opt);
	if (ret < 0) {
		std::cout << "Error - avformat_write_header - " << pFormatCtx->url << std::endl;
		//fprintf(stderr, "Error occurred when opening output file: %s\n",
		//	av_err2str(ret));
		return -1;
	}

	return 0;
}

int Muxer::Wirte_Trailer(AVFormatContext* pFormatCtx){

	av_write_trailer(pFormatCtx);

	return 0;
}