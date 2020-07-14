#include "Transcode.h"

const char* error_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;

	return errstr;
}

static int cnt;

static int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
	AVStream *st, AVFrame *frame)
{
	int ret;

	// send the frame to the encoder
	ret = avcodec_send_frame(c, frame);
	if (ret < 0) {
		//fprintf(stderr, "Error sending a frame to the encoder: %s\n",
		//	av_err2str(ret));
		//exit(1);
	}

	while (ret >= 0) {
		AVPacket pkt = { 0 };

		ret = avcodec_receive_packet(c, &pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0) {
			//	fprintf(stderr, "Error encoding a frame: %s\n", av_err2str(ret));
			//	exit(1);
		}

		/* rescale output packet timestamp values from codec to stream timebase */
		av_packet_rescale_ts(&pkt, c->time_base, st->time_base);
		pkt.stream_index = st->index;

		/* Write the compressed frame to the media file. */
		//log_packet(fmt_ctx, &pkt);
		ret = av_interleaved_write_frame(fmt_ctx, &pkt);
		av_packet_unref(&pkt);
		if (ret < 0) {
			//fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
			exit(1);
		}
	}

	return ret == AVERROR_EOF ? 1 : 0;
}

/* Add an output stream. */
static void add_stream(OutputStream *ost, AVFormatContext *oc,
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
		c->width = 352;
		c->height = 288;
		/* timebase: This is the fundamental unit of time (in seconds) in terms
		 * of which frame timestamps are represented. For fixed-fps content,
		 * timebase should be 1/framerate and timestamp increments should be
		 * identical to 1. */
		ost->st->time_base = { 1, STREAM_FRAME_RATE };
		c->time_base = ost->st->time_base;

		c->gop_size = 12; /* emit one intra frame every twelve frames at most */
		c->pix_fmt = STREAM_PIX_FMT;
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

/**************************************************************/
/* audio output */

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
	uint64_t channel_layout,
	int sample_rate, int nb_samples)
{
	AVFrame *frame = av_frame_alloc();
	int ret;

	if (!frame) {
		fprintf(stderr, "Error allocating an audio frame\n");
		exit(1);
	}

	frame->format = sample_fmt;
	frame->channel_layout = channel_layout;
	frame->sample_rate = sample_rate;
	frame->nb_samples = nb_samples;

	if (nb_samples) {
		ret = av_frame_get_buffer(frame, 0);
		if (ret < 0) {
			fprintf(stderr, "Error allocating an audio buffer\n");
			exit(1);
		}
	}

	return frame;
}

static void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
	AVCodecContext *c;
	int nb_samples;
	int ret;
	AVDictionary *opt = NULL;

	c = ost->enc;

	/* open it */
	av_dict_copy(&opt, opt_arg, 0);
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);
	if (ret < 0) {
		//fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
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

	ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout,
		c->sample_rate, nb_samples);
	ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
		c->sample_rate, nb_samples);

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

/**************************************************************/
/* video output */

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
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

static void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
	int ret;
	AVCodecContext *c = ost->enc;
	AVDictionary *opt = NULL;

	av_dict_copy(&opt, opt_arg, 0);

	/* open the codec */
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);
	if (ret < 0) {
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

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
	avcodec_free_context(&ost->enc);
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
}

/**************************************************************/
/* media file output */

int Transcoder::init(std::string Filename) {

	filename = Filename.c_str();

	AVDictionary *dicts = NULL;
	//int rc = av_dict_set(&dicts, "rtsp_transport", "tcp", 0); // default udp. Set tcp interleaved mode
	//if (rc < 0)
	//{
	//	return EXIT_FAILURE;
	//}
	//
	//if (avformat_open_input(&ctx, "rtsp://admin:1234@192.168.0.73/video1", NULL, &dicts) != 0) {
	//	return EXIT_FAILURE;
	//}

	/* allocate the output media context */
	//avformat_alloc_output_context2(&oc, NULL, "hls", "playlist.m3u8"); // apple hls. If you just want to segment file use "segment"
	//if (!oc) {
	//	printf("Could not deduce output format from file extension: using MPEG.\n");
	//	avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
	//}
	//if (!oc)
	//	return 1;

	//fmt = oc->oformat;

	/* Add the audio and video streams using the default format codecs
	 * and initialize the codecs. */
	if (fmt->video_codec != AV_CODEC_ID_NONE) {
		add_stream(&video_st, oc, &video_codec, fmt->video_codec);
		have_video = 1;
		encode_video = 1;
	}
	if (fmt->audio_codec != AV_CODEC_ID_NONE) {
		add_stream(&audio_st, oc, &audio_codec, fmt->audio_codec);
		have_audio = 1;
		encode_audio = 1;
	}

	/* Now that all the parameters are set, we can open the audio and
	 * video codecs and allocate the necessary encode buffers. */
	if (have_video)
		open_video(oc, video_codec, &video_st, opt);

	if (have_audio)
		open_audio(oc, audio_codec, &audio_st, opt);

	int k = (1 << 4); // omit endlist
	int j = (1 << 1);  // delete segment. 

	av_opt_set(oc->priv_data, "hls_segment_filename", "file%03d.ts", 0);
	av_opt_set_int(oc->priv_data, "hls_list_size", 3, 0);
	av_opt_set_int(oc->priv_data, "hls_time", 3, 0);
	av_opt_set_int(oc->priv_data, "hls_flags", k | j, 0);

	av_dump_format(oc, 0, filename, 1);

	/* open the output file, if needed */
	if (!(fmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			//fprintf(stderr, "Could not open '%s': %s\n", filename,
			//	av_err2str(ret));
			return 1;
		}
	}
}

int Transcoder::run()
{

	/* Write the stream header, if any. */
	ret = avformat_write_header(oc, &opt);
	if (ret < 0) {
		//fprintf(stderr, "Error occurred when opening output file: %s\n",
		//	av_err2str(ret));
		return 1;
	}

	//while (encode_video || encode_audio) {
	//	/* select the stream to encode */
	//	if (encode_video &&
	//		(!encode_audio || av_compare_ts(video_st.next_pts, video_st.enc->time_base,
	//			audio_st.next_pts, audio_st.enc->time_base) <= 0)) {
	//		encode_video = !write_video_frame(oc, &video_st);
	//		cnt++;
	//	}
	//	else {
	//		encode_audio = !write_audio_frame(oc, &audio_st);
	//	}
	//}


	for (int i = 0; i < ctx->nb_streams; i++)
	{
		if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			vidx = i;
		if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			aidx = i;
	}

	while (true)
	{
		av_init_packet(&packet);
		int nRecvPacket = av_read_frame(ctx, &packet);

		if (packet.stream_index == vidx) // video frame
		{
			//vstream->id = vidx;
			packet.pts = av_rescale_q(nGap, { 1, 10000 }, oc->streams[packet.stream_index]->time_base);
			nGap += 333;
			cnt++;

		}
		else if (packet.stream_index == aidx) // audio frame
		{
			//astream->id = aidx;
			packet.pts = av_rescale_q(nGap2, { 1, 10000 }, oc->streams[packet.stream_index]->time_base);
			nGap2 += 666;
		}

		packet.dts = packet.pts;// generally, dts is same as pts. it only differ when the stream has b-frame
		//av_write_frame(oc, &packet);
		av_interleaved_write_frame(oc, &packet);
		av_packet_unref(&packet);

		std::cout << "Recording " << cnt << std::endl;

		if (cnt > 150) // 
			break;
	}

	/* Write the trailer, if any. The trailer must be written before you
	 * close the CodecContexts open when you wrote the header; otherwise
	 * av_write_trailer() may try to use memory that was freed on
	 * av_codec_close(). */
	int error = av_write_trailer(oc);

	return 0;
}

int Transcoder::close() {

	/* Close each codec. */
	if (have_video)
		close_stream(oc, &video_st);
	if (have_audio)
		close_stream(oc, &audio_st);

	if (!(fmt->flags & AVFMT_NOFILE))
		/* Close the output file. */
		avio_closep(&oc->pb);

	avio_closep(&oc->pb);
	/* free the stream */
	avformat_free_context(oc);

	return 0;
}