#include "convert.h"

int Aresampling(AVFrame* inFrame, StreamContext* stream_ctx, FILE* dst_file);

static int get_format_from_sample_fmt(const char **fmt,
	enum AVSampleFormat sample_fmt)
{
	int i;
	struct sample_fmt_entry {
		enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
	} sample_fmt_entries[] = {
		{ AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
		{ AV_SAMPLE_FMT_S16, "s16be", "s16le" },
		{ AV_SAMPLE_FMT_S32, "s32be", "s32le" },
		{ AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
		{ AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
	};
	*fmt = NULL;

	for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
		struct sample_fmt_entry *entry = &sample_fmt_entries[i];
		if (sample_fmt == entry->sample_fmt) {
			*fmt = AV_NE(entry->fmt_be, entry->fmt_le);
			return 0;
		}
	}

	fprintf(stderr,
		"Sample format %s not supported as output format\n",
		av_get_sample_fmt_name(sample_fmt));
	return AVERROR(EINVAL);
}

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(pkt, *time_base, st->time_base);
	pkt->stream_index = st->index;

	/* Write the compressed frame to the media file. */
	//log_packet(fmt_ctx, pkt);
	return av_interleaved_write_frame(fmt_ctx, pkt);
}

static int Muxing(bool encode_video, bool encode_audio, int64_t Vpkt_pts, int64_t Apkt_pts, AVPacket* Vpkt, AVPacket* Apkt, bool Stop) {

	//while (encode_video && encode_audio) {
		/* select the stream to encode */
		if (encode_video &&
			(!encode_audio || av_compare_ts(Vpkt_pts, Vstream_ctx->enc_ctx->time_base,
				Apkt_pts, Astream_ctx->enc_ctx->time_base) <= 0)) {

			if (av_compare_ts(Vpkt_pts, Vstream_ctx->enc_ctx->time_base,
				STREAM_DURATION, { 1, 1 }) > 0)
				//break;
			encode_video = !write_frame(ofmt_ctx, &Vstream_ctx->enc_ctx->time_base, Vstream_ctx->out_stream, Vpkt);
			//encode_video = !write_video_frame(oc, &Vstream_ctx->out_stream);
		}
		else {
			encode_audio = !write_frame(ofmt_ctx, &Astream_ctx->enc_ctx->time_base, Astream_ctx->out_stream, Apkt);
			//encode_audio = !write_audio_frame(stream_ctx->ofmt_ctx, &stream_ctx->out_stream);
		}
	//}

	return 0;
}

void Work() {

	AVPacket *Apacket = NULL;
	AVPacket *Vpacket = NULL;
	AVFrame* frame = NULL;
	
	Apacket = av_packet_alloc();
	Vpacket = av_packet_alloc();

	clock_t start = clock();
	clock_t end = 0;

	int count = 0;
	float loopend = 0;
	int data_size = 0;
	bool Stop = false;
	std::thread Muxing_Stop;
	FILE*dst_file;
	fopen_s(&dst_file, "resample_test.avi", "wb");
	if (!dst_file) {
		fprintf(stderr, "Could not open destination file %s\n", "resample_test");
		exit(1);
	}

	while (1) {
		loopend = end - start;
		if (loopend > 10000) {
			Stop = true;
			break;
		}
		bool Vencode = false;
		bool Aencode = false;

		//if (Apacket->stream_index == Vpacket->stream_index)
		//{
			//error = av_read_frame(Vstream_ctx->ifmt_ctx, Vpacket);
			//if (error < 0) {
			//	std::cout << "EOF" << std::endl;
			//	break;
			//}
			//else Vencode = true;

			error = av_read_frame(Astream_ctx->ifmt_ctx, Apacket);
			if (error < 0) {
				std::cout << "EOF" << std::endl;
				break;
			}
			else Aencode = true;

			frame = av_frame_alloc();
			if (!frame) {
				break;
			}

			error = avcodec_send_packet(Astream_ctx->dec_ctx, Apacket);
			if (error < 0) {
				error_pro(error, "avcodec_send_packet again");
				break;
			}

			error = avcodec_receive_frame(Astream_ctx->dec_ctx, frame);
			if (error == -11) {
				error_pro(error, "avcodec_send_packet again");
				av_packet_unref(Apacket);
				continue;
			}
			else if (error < 0) {
				error_pro(error, "avcodec_send_packet again");
				break;
			}

			fbuffer.push_back(frame);
			buffer.push_back(frame->data[0]);

			//Apacket->

			error = Aresampling(frame, Astream_ctx, dst_file);
			if (error < 0) {
				error_pro(error, "avcodec_send_packet again");
				break;
			}

			int Apkt_pts = Apacket->pts;
			int Vpkt_pts = Vpacket->pts;
			int Apkt_dts = Apacket->dts;
			int Vpkt_dts = Vpacket->dts;

			//Muxing_Stop = std::thread(Muxing, );
			//error = Muxing(Vencode, Aencode, Vpkt_pts, Apkt_pts, Vpacket, Apacket, Stop);
			//if (error < 0) {
			//	error_pro(error, "Muxing Error");
			//	break;
			//}

			//Apacket->stream_index = Astream_ctx->out_stream->index;
			//Vpacket->stream_index = Vstream_ctx->out_stream->index;

			// Write_File
			error = write_frame(ofmt_ctx, &Astream_ctx->enc_ctx->time_base, Astream_ctx->out_stream, Apacket);
			if (error < 0) {
				error_pro(error, "av_interleaved_write_frame error");
				av_packet_unref(Vpacket);
			}
			//Apacket->stream_index = 0;
			//Apacket->pts = pkt_pts;
			//Apacket->dts = pkt_dts;
		//}

		end = clock();
		count++;

	}

	fclose(dst_file);
	av_write_trailer(ofmt_ctx);

	//av_packet_unref(Apacket);
	//av_packet_unref(Vpacket);
	//av_frame_free(&frame);

	return;
}


int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	int Vmain_error = 0;
	int Amain_error = 0;
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	const char* outfile = "Muxing_out_test.mp3";

	Vmain_error = Video_Init(outfile);
	if (Vmain_error) {
		std::cout << "Error in Video_Init" << std::endl;
	}

	Amain_error = Audio_Init(outfile);
	if (Amain_error) {
		std::cout << "Error in Audio_Init" << std::endl;
	}
	
	if (Amain_error == 0) {
		Work();
	}

	fclose(cp);

	return 0;
}

static int Vopen_input_file(const char* inputfilename) {

	AVFormatContext* ifmt_ctx = NULL;

	//av_dict_set(&options, "list_devices", "true", 0);
	//AVInputFormat *iformat = av_find_input_format("dshow");
	//if (iformat == NULL) {
	//	printf("iformat is NULL\n");
	//}

	std::string conv;
	std::wstring wchar = L"test.264";
	//std::wstring wchar = L"input_test.wav";
	int size = wchar.size();
	convert_unicode_to_utf8_string(conv, wchar.c_str(), size);
	//std::string filename = "audio=마이크 배열(Realtek High Definition Audio)";

	ifmt_ctx = avformat_alloc_context();
	error = avformat_open_input(&ifmt_ctx, conv.c_str(), NULL, NULL);
	//error = avformat_open_input(&ifmt_ctx, conv.c_str(), iformat, NULL);
	if (error < 0) {
		error_pro(error, errstr);
		std::cout << conv.c_str() << std::endl;
		std::cout << "avformat_open_input Error : " << errstr << std::endl;
		return -1;
	}

	error = avformat_find_stream_info(ifmt_ctx, NULL);
	if (error < 0) {
		error_pro(error, "avformat_find_stream_info");
		return error;
	}

	Vstream_ctx = (StreamContext*)av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*Vstream_ctx));
	if (!Vstream_ctx) {
		std::cout << "av_mallocz_array Error" << std::endl;
		return -1;
	}
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream *stream = ifmt_ctx->streams[i];
		AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
		AVCodecContext *codec_ctx;
		if (!dec) {
			std::cout << "avcodec_find_decoder" << std::endl;
			return -1;
		}

		codec_ctx = avcodec_alloc_context3(dec);
		if (!codec_ctx) {
			std::cout << "avcodec_alloc_context3" << std::endl;
			return -1;
		}

		error = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
		if (error < 0) {
			error_pro(error, "avcodec_parameters_to_context");
			return -1;
		}

		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
				codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
			/* Open decoder */
			error = avcodec_open2(codec_ctx, dec, NULL);
			if (error < 0) {
				error_pro(error, "avcodec_open2");
				return -1;
			}
		}
		Vstream_ctx[i].dec_ctx = codec_ctx;
	}
	Vstream_ctx->ifmt_ctx = ifmt_ctx;
	//av_dump_format(ifmt_ctx, 0, conv.c_str(), 0);

	return 0;
}

static int Vopen_output_file(const char* outputfilename) {

	//AVFormatContext* ofmt_ctx = NULL;
	AVCodecContext* enc_ctx = NULL;
	AVCodec* encoder = NULL;
	AVStream* out_stream = NULL;
	AVCodecID codec_id = AV_CODEC_ID_H264;

	error = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, outputfilename);
	if (!ofmt_ctx) {
		//av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		//return AVERROR_UNKNOWN;
		std::cout << "avformat_alloc_output_context2" << std::endl;
		return -1;
	}

	encoder = avcodec_find_encoder(codec_id);
	if (!encoder) {
		std::cout << "encoder is invalid" << std::endl;
		return -1;
	}
	
	out_stream = avformat_new_stream(ofmt_ctx, NULL);
	if (!out_stream) {
		std::cout << "avformat_new_stream is invalid" << std::endl;
		return -1;
	}

	out_stream->id = ofmt_ctx->nb_streams - 1;

	enc_ctx = avcodec_alloc_context3(encoder);
	if (!enc_ctx) {
		std::cout << "avcodec_alloc_context3 is invalid" << std::endl;
		return -1;
	}

	Vstream_ctx->enc_ctx = enc_ctx;

	enc_ctx->codec_id = codec_id;

	enc_ctx->bit_rate = 400000;
	/* Resolution must be a multiple of two. */
	enc_ctx->width = 352;
	enc_ctx->height = 288;
	/* timebase: This is the fundamental unit of time (in seconds) in terms
	 * of which frame timestamps are represented. For fixed-fps content,
	 * timebase should be 1/framerate and timestamp increments should be
	 * identical to 1. */
	out_stream->time_base = { 1, STREAM_FRAME_RATE };
	enc_ctx->time_base = out_stream->time_base;

	enc_ctx->gop_size = 12; /* emit one intra frame every twelve frames at most */
	enc_ctx->pix_fmt = STREAM_PIX_FMT;
	if (enc_ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
		/* just for testing, we also add B-frames */
		enc_ctx->max_b_frames = 2;
	}
	if (enc_ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		 * This does not happen with normal video, it just happens here as
		 * the motion of the chroma plane does not match the luma plane. */
		enc_ctx->mb_decision = 2;
	}
	
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	error = avcodec_open2(enc_ctx, encoder, NULL);
	if (error < 0) {
		error_pro(error, "video avcodec_open2 error");
		return error;
	}

	error = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
	if (error < 0) {
		error_pro(error, "video avcodec_parameters_from_context error");
		return error;
	}


	Vstream_ctx->enc_ctx = enc_ctx;
	//Vstream_ctx->ofmt_ctx = ofmt_ctx;
	Vstream_ctx->out_stream = out_stream;

	return 0;
}

static int Aopen_input_file() {

	//AVDictionary* options = NULL;

	/////////////////////////////////////////////////////
	avdevice_register_all();
	
	AVFormatContext* ifmt_ctx = NULL;


	//av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	if (iformat == NULL) {
		printf("iformat is NULL\n");
	}

	std::string conv;
	std::wstring wchar = L"audio=마이크 배열(Realtek High Definition Audio)";
	//std::wstring wchar = L"input_test.wav";
	int size = wchar.size();
	convert_unicode_to_utf8_string(conv, wchar.c_str(), size);
	//std::string filename = "audio=마이크 배열(Realtek High Definition Audio)";

	ifmt_ctx = avformat_alloc_context();
	//error = avformat_open_input(&ifmt_ctx, "input_test.wav", NULL, NULL);
	error = avformat_open_input(&ifmt_ctx, conv.c_str(), iformat, NULL);
	if (error < 0) {
		error_pro(error, errstr);
		std::cout << conv.c_str() << std::endl;
		std::cout << "avformat_open_input Error : " << errstr << std::endl;
		return -1;
	}

	error = avformat_find_stream_info(ifmt_ctx, NULL);
	if (error < 0) {
		error_pro(error, "avformat_find_stream_info");
		return error;
	}

	Astream_ctx = (StreamContext*)av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*Astream_ctx));
	if (!Astream_ctx) {
		std::cout << "av_mallocz_array Error" << std::endl;
		return -1;
	}
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream *stream = ifmt_ctx->streams[i];
		AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
		AVCodecContext *codec_ctx;
		if (!dec) {
			std::cout << "avcodec_find_decoder" << std::endl;
			return -1;
		}
		codec_ctx = avcodec_alloc_context3(dec);
		if (!codec_ctx) {
			std::cout << "avcodec_alloc_context3" << std::endl;
			return -1;
		}
		error = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
		if (error < 0) {
			error_pro(error, "avcodec_parameters_to_context");
			return -1;
		}
		/* Reencode video & audio and remux subtitles etc. */
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
				codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
			/* Open decoder */
			error = avcodec_open2(codec_ctx, dec, NULL);
			if (error < 0) {
				error_pro(error, "avcodec_open2");
				return -1;
			}
		}
		Astream_ctx[i].dec_ctx = codec_ctx;
	}
	Astream_ctx->ifmt_ctx = ifmt_ctx;
	//av_dump_format(ifmt_ctx, 0, conv.c_str(), 0);
	return 0;
}

static void fill_samples(double *dst, int nb_samples, int nb_channels, int sample_rate, double *t)
{
	int i, j;
	double tincr = 1.0 / sample_rate, *dstp = dst;
	const double c = 2 * M_PI * 440.0;

	/* generate sin tone with 440Hz frequency and duplicated channels */
	for (i = 0; i < nb_samples; i++) {
		*dstp = sin(c * *t);
		for (j = 1; j < nb_channels; j++)
			dstp[j] = dstp[0];
		dstp += nb_channels;
		*t += tincr;
	}
}

static int Aresampling(AVFrame* inFrame, StreamContext* stream_ctx, FILE* dst_file) {
	int ret = 0;
	struct SwrContext *swr_ctx;
	int src_nb_channels = stream_ctx->enc_ctx->channels;
	int dst_nb_channels = 0;
	int src_nb_samples = 1024, dst_nb_samples, max_dst_nb_samples;
	int64_t src_ch_layout = AV_CH_LAYOUT_STEREO, dst_ch_layout = AV_CH_LAYOUT_STEREO;
	int src_rate = 44100, dst_rate = 44100;
	enum AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_S16, dst_sample_fmt = AV_SAMPLE_FMT_FLTP;
	uint8_t **src_data = inFrame->data, **dst_data = NULL;
	int src_linesize = inFrame->linesize[0];	int dst_linesize = 0;
	
	const char *fmt;

	//stream_ctx->enc_ctx->sam



	swr_ctx = swr_alloc();
	if (!swr_ctx) {
		fprintf(stderr, "Could not allocate resampler context\n");
		ret = AVERROR(ENOMEM);
		return -1;
	}

	/* set options */
	av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", src_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", src_sample_fmt, 0);

	av_opt_set_int(swr_ctx, "out_channel_layout", dst_ch_layout, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", dst_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);

	/* initialize the resampling context */
	if ((ret = swr_init(swr_ctx)) < 0) {
		fprintf(stderr, "Failed to initialize the resampling context\n");
		return -1;
	}

	src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
	ret = av_samples_alloc_array_and_samples(&src_data, &src_linesize, src_nb_channels,
		src_nb_samples, src_sample_fmt, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate source samples\n");
		return -1;
	}

	/* compute the number of converted samples: buffering is avoided
	 * ensuring that the output buffer will contain at least all the
	 * converted input samples */
	max_dst_nb_samples = dst_nb_samples =
		av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);

	/* buffer is going to be directly written to a rawaudio file, no alignment */
	dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
	ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels,
		dst_nb_samples, dst_sample_fmt, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate destination samples\n");
		return -1;
	}

	double t = 0;
	//do {
		/* generate synthetic audio */
		//fill_samples((double *)src_data[0], src_nb_samples, src_nb_channels, src_rate, &t);

		/* compute destination number of samples */
		dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) +
			src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
		if (dst_nb_samples > max_dst_nb_samples) {
			av_freep(&dst_data[0]);
			ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
				dst_nb_samples, dst_sample_fmt, 1);
			if (ret < 0)return -1;
				//break;
			max_dst_nb_samples = dst_nb_samples;
		}

		/* convert to destination format */
		ret = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)src_data, src_nb_samples);
		if (ret < 0) {
			fprintf(stderr, "Error while converting\n");
			return -1;
		}
		int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
			ret, dst_sample_fmt, 1);
		if (dst_bufsize < 0) {
			fprintf(stderr, "Could not get sample buffer size\n");
			return -1;
		}
		printf("t:%f in:%d out:%d\n", t, src_nb_samples, ret);
		fwrite(dst_data[0], 1, dst_bufsize, dst_file);
	//} while (t < 10);

	//if ((ret = get_format_from_sample_fmt(&fmt, dst_sample_fmt)) < 0)
	//	return -1;
	//fprintf(stderr, "Resampling succeeded. Play the output file with the command:\n"
	//	"ffplay -f %s -channel_layout %"PRId64" -channels %d -ar %d %s\n",
	//	fmt, dst_ch_layout, dst_nb_channels, dst_rate, dst_filename);

}

static int Aopen_output_file(const char *filename)
{
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder = NULL;
	AVOutputFormat* tempoutfmt = NULL;
	AVStream* out_stream = NULL;
	
	int ret;
	
	dec_ctx = Astream_ctx->dec_ctx;
	error = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
	if (!ofmt_ctx) {
		//av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		//return AVERROR_UNKNOWN;
		std::cout << "avformat_alloc_output_context2" << std::endl;
		return -1;
	}
	

	//.으로 문자열을 나누어 확장자만 따와야함
	tempoutfmt = av_guess_format(NULL, filename, NULL);
	if (tempoutfmt == NULL) {
		tempoutfmt = av_guess_format("mp3", NULL, NULL);
	}
	ofmt_ctx->oformat = tempoutfmt;
	//ofmt_ctx->video_codec_id = Vstream_ctx->dec_ctx->codec_id;
	ofmt_ctx->audio_codec_id = Astream_ctx->dec_ctx->codec_id;

	encoder = avcodec_find_encoder(ofmt_ctx->oformat->audio_codec);
	if (!encoder) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	out_stream = avformat_new_stream(ofmt_ctx, NULL);
	if (!out_stream) {
		fprintf(stderr, "avformat_new_stream not found\n");
		exit(1);
	}
	out_stream->id = ofmt_ctx->nb_streams - 1;

	enc_ctx = avcodec_alloc_context3(encoder);
	if (!enc_ctx) {
		fprintf(stderr, "Could not allocate audio codec context\n");
		exit(1);
	}
	Astream_ctx->enc_ctx = enc_ctx;

	enc_ctx->sample_fmt = *encoder->sample_fmts;
	enc_ctx->bit_rate = 64000;
	enc_ctx->sample_rate = 44100;

	enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
	enc_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
	if ((encoder)->channel_layouts) {
		enc_ctx->channel_layout = (encoder)->channel_layouts[0];
		for (int i = 0; (encoder)->channel_layouts[i]; i++) {
			if ((encoder)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
				enc_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
		}
	}
	enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
	out_stream->time_base = { 1, enc_ctx->sample_rate };
	
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	
	Astream_ctx->enc_ctx = enc_ctx;

	ret = avcodec_open2(enc_ctx, encoder, NULL);
	if (ret < 0) {
		//fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
		error_pro(ret, "avcodec_open2");
		exit(1);
	}

	ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
	if (ret < 0) {
		fprintf(stderr, "Could not copy the stream parameters\n");
		exit(1);
	}

	ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
	if (ret < 0) {
		error_pro(ret, "avio_open");
		return ret;
	}

	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		error_pro(ret, "avformat_write_header");
		return ret;
	}

	//stream_ctx->ofmt_ctx = ofmt_ctx;
	Astream_ctx->out_stream = out_stream;

	return 0;
}


void Close() {

	av_free(ofmt_ctx);
	av_free(Astream_ctx->ifmt_ctx);
	av_free(Astream_ctx->dec_ctx);
	av_free(Astream_ctx->enc_ctx);
	av_free(Vstream_ctx->ifmt_ctx);
	av_free(Vstream_ctx->dec_ctx);
	av_free(Vstream_ctx->enc_ctx);

}


int Audio_Init(const char* outfile) {

	const char* outfilename = "output.avi";
	//const char* encode_outfilename = "encode_output.wav";
	const char* encode_outfilename = outfile;

	error = Aopen_input_file();
	if (error) {
		error_pro(error, "input file error");
		return error;
	}
	error = Aopen_output_file(encode_outfilename);
	if (error) {
		error_pro(error, "output file error");
		return error;
	}

	return 0;
}

int Video_Init(const char* outfile) {

	const char* infilename = "test.h264";
	//const char* encode_outfilename = "encode_output.wav";
	const char* encode_outfilename = outfile;

	error = Vopen_input_file(infilename);
	if (error) {
		error_pro(error, "input file error");
		return error;
	}
	error = Vopen_output_file(encode_outfilename);
	if (error) {
		error_pro(error, "output file error");
		return error;
	}

	return 0;
}


void error_pro(int error, const char* msg) {

	av_strerror(error, errstr, 256);
	std::cout << "Error" << errstr << " " << msg << std::endl;
	//fprintf(stderr, "Error %s %s\n", errstr, msg);

	return;
}

DWORD convert_unicode_to_utf8_string(__out std::string& utf8, __in const wchar_t* unicode, __in const size_t unicode_size) {
	DWORD error = 0;
	do {
		if ((nullptr == unicode) || (0 == unicode_size)) { error = ERROR_INVALID_PARAMETER; break; } utf8.clear();
		// // getting required cch. // 
		int required_cch = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, unicode, static_cast<int>(unicode_size), nullptr, 0, nullptr, nullptr);
		if (0 == required_cch)
		{
			error = ::GetLastError();
			break;
		} // // allocate. // 
		utf8.resize(required_cch); // // convert. // 
		if (0 == ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, unicode, static_cast<int>(unicode_size),
			const_cast<char*>(utf8.c_str()), static_cast<int>(utf8.size()), nullptr, nullptr))
		{
			error = ::GetLastError();
			break;
		}
	} while (false);
	return error;
}
