#include "FFSegmenter.h"

const char* fferr_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;

	return errstr;
}

FFSegmenter::FFSegmenter() : ISegmenter(0), SegType(ST_NOT_DEFINE), UseAudio(false), Interval(5),
VCo(AV_CODEC_ID_NONE), ACo(AV_CODEC_ID_NONE), Encoding(false), m_BT(BST(0)) {

	Filename = "";
}

FFSegmenter::FFSegmenter(std::string iFilename, ST SegType, int index, bool UseAudio, int Interval, bool Encoding,
	AVCodecID VCo, AVCodecID ACo)
	:ISegmenter(index), SegType(SegType), UseAudio(UseAudio), Interval(Interval),
	VCo(VCo), ACo(ACo), Encoding(Encoding), m_BT(BST(index)){

	Filename = iFilename;
	HeaderTail = false;
	Init();
	if (Encoding) {
	
	}
	else {
	}
	
}

FFSegmenter::~FFSegmenter() {
	Close();
}

int FFSegmenter::Init() {

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

	TimeCheckthr = std::thread([&]() { TimeCheck(this); });
	Wirte_Header(pOutFormatCtx, avdic);

	return 0;
}

int FFSegmenter::Run(std::shared_ptr <MediaFrame> Frame) {

	static int encode_video;
	static int encode_audio;
	static int count;
	
	CheckTime = clock();
	Running = true;

	if (HeaderTail && Running) {
		if (Frame->Info.StreamId == 0) {

			if (UseAudio) {
				if (av_compare_ts(video_st.next_pts, video_st.enc->time_base,
					audio_st.next_pts, audio_st.enc->time_base) > 0) {
					encode_audio = !write_frame(pOutFormatCtx, Frame, &audio_st);
				}
			}
			encode_video = !write_frame(pOutFormatCtx, Frame, &video_st);
		}
		else if (Frame->Info.StreamId == 1 && UseAudio == true) {

			encode_audio = !write_frame(pOutFormatCtx, Frame, &audio_st);
		}
		count++;
		std::cout << "\t\t Muxer Q size : " << count << std::endl;
		Sleep(1);
	}

	return 0;
}

void FFSegmenter::TimeCheck(FFSegmenter* SG) {

	int base_time = 1000 * 65;
	int Dif = 0;

	while (1) {
		if (Running) {
			SG->loopTime = clock();
			Dif = (SG->CheckTime - SG->loopTime);

			if (Dif < 0)
				Dif = Dif * -1;

			if (Dif > base_time) {
				//SG->Stop();
				Running = false;
				std::cout << "Time Checkout - " << std::endl;
				return;
			}
		}
		Sleep(1);
	}
	return;
}

int FFSegmenter::Close() {
	
	Stop();

	if (TimeCheckthr.joinable()) {
		TimeCheckthr.join();
	}

	close_stream(pOutFormatCtx, &video_st);

	if (UseAudio)
		close_stream(pOutFormatCtx, &audio_st);

	if (!(fmt->flags & AVFMT_NOFILE))
		/* Close the output file. */
		avio_closep(&pOutFormatCtx->pb);
	avformat_free_context(pOutFormatCtx);

	return 0;
}

int FFSegmenter::Stop() {

	Wirte_Trailer(pOutFormatCtx);
	return 0;

}

int FFSegmenter::inner_encode(AVFrame *frame, AVPacket *pkt, AVCodecContext* c, int *got_packet)
{
	int ret = 0;
	*got_packet = 0;

	av_init_packet(pkt);
	pkt->data = NULL;
	pkt->size = 0;

	ret = avcodec_send_frame(c, frame);
	if (ret < 0) {
		fferr_pro(ret, "Send_Frame Error");
		return ret;
	}

	ret = avcodec_receive_packet(c, pkt);
	if (!ret)
		*got_packet = 1;
	if (ret == AVERROR(EAGAIN))
		return 0;
	else if (ret == AVERROR_EOF) {
		std::cout << "Detected End of Files"<< std::endl;
		return ret;
	}
	else if (ret < 0) {
		return ret;
	}

	return ret;
}

//int FFSegmenter::write_frame(AVFormatContext *fmt_ctx, AVPacket* pkt, OutputStream* ost)
int FFSegmenter::write_frame(AVFormatContext *fmt_ctx, std::shared_ptr<MediaFrame> MFrame, OutputStream* ost)
{
	int ret = 0, got_output = 0;
	static int count;

	MediaFrame* MF = new MediaFrame(MFrame.get());

	AVCodecContext* c = ost->enc;

	if (Encoding) {

		ret = inner_encode(MF->Frm, MF->Pkt, c, &got_output);
		if (ret < 0) {
			fferr_pro(ret, "Encoding Error");
			return ret;
		}
		if(got_output) {
			/* rescale output packet timestamp values from codec to stream timebase */
			//av_packet_rescale_ts(MF->Pkt, c->time_base, ost->st->time_base);
			MF->Pkt->stream_index = ost->st->index;
			if (MF->Pkt == nullptr) {
				std::cout << "Error Packet Encoding - " << fmt_ctx->url << std::endl;
				return -1;
			}
		}
	}
		/* Write the compressed frame to the media file. */
	ret = av_interleaved_write_frame(fmt_ctx, MF->Pkt);
	av_packet_unref(MF->Pkt);
	if (ret < 0) {
		fferr_pro(ret, "Error in write_frame");
		return -1;
	}
	else {
		count++;
		if (count > 500) {
			std::cout << "Success mux - " << count << std::endl;
			count = 0;
			Wirte_Trailer(fmt_ctx);
		}
	}

	return ret == AVERROR_EOF ? 1 : 0;
}

void FFSegmenter::add_stream(OutputStream *ost, AVFormatContext *oc,
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

		c->bit_rate = m_BT.Bitrate;
		/* Resolution must be a multiple of two. */
		c->width = m_BT.Width;
		c->height = m_BT.Height;
		/* timebase: This is the fundamental unit of time (in seconds) in terms
		 * of which frame timestamps are represented. For fixed-fps content,
		 * timebase should be 1/framerate and timestamp increments should be
		 * identical to 1. */
		ost->st->time_base = { 1, STREAM_FRAME_RATE };
		//ost->st->time_base = { 1, 90000 };
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


AVFrame* FFSegmenter::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
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

void FFSegmenter::open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/)
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

void FFSegmenter::open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost/*, AVDictionary *opt_arg*/)
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

void FFSegmenter::close_stream(AVFormatContext *oc, OutputStream *ost)
{
	avcodec_free_context(&ost->enc);
	av_frame_free(&ost->frame);
	av_frame_free(&ost->tmp_frame);
	sws_freeContext(ost->sws_ctx);
	swr_free(&ost->swr_ctx);
}

int FFSegmenter::SetOpt() {

	int ret = av_dict_set(&avdic, "segment_list_flags", "live", 0);
	if (ret < 0) {
		return ret;
	}

	ret = av_dict_count(avdic);

	//ret = av_dict_set_int(&avdic, "max_delay", 50, 0);
	//if (ret < 0) {
	//	return ret;
	//}
	//ret = av_dict_set_int(&avdic, "segment_time", 5, 0);
	//if (ret < 0) {
	//	return ret;
	//}
	//ret = av_dict_set_int(&avdic, "segment_list_size", 6, 0);
	//if (ret < 0) {
	//	return ret;
	//}
	//ret = av_dict_set_int(&avdic, "segment_warp", 0, 0);
	//if (ret < 0) {
	//	return ret;
	//}
	//ret = av_dict_set_int(&avdic, "hls_delete_threshold", 7, 0);
	//if (ret < 0) {
	//	return ret;
	//}
	//ret = av_dict_set(&avdic, "segment_list ", "1080", 0);
	//if (ret < 0) {
	//	return ret;
	//}

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

	//hls_ts_options options_list
	//Set output format options using a :-separated list of key = value parameters.Values containing : special characters must be escaped.

	int i = (1 << 4); // omit endlist
	int j = (1 << 1);  // delete segment. 
	//  libavformat/hlsenc.c 's description shows that no longer available files will be deleted but it doesnt works as described.

	char temp[256] = {};


	//sprintf_s(temp, sizeof(temp), "%d%s", Bitrate, UURL.c_str());
	////ret = av_opt_set(pOutFormatCtx->priv_data, "hls_segment_filename", temp, 0);
	//if (ret < 0) {
	//	fferr_pro(ret, "Set opt error ");
	//}


	//var_stream_map
	//	Map string which specifies how to group the audio, video and subtitle streams into different variant streams.
	//  The variant stream groups are separated by space.Expected string format is like this "a:0,v:0 a:1,v:1 ...."
	//  .Here a : , v : , s : are the keys to specify audio, video and subtitle streams respectively.
	//  Allowed values are 0 to 9 (limited just based on practical usage).
	//
	//	When there are two or more variant streams, the output filename pattern must contain the string "%v",
	//  this string specifies the position of variant stream index in the output media playlist filenames.
	//  The string "%v" may be present in the filename or in the last directory name containing the file.
	//  If the string is present in the directory name, then sub - directories are created after expanding the directory name pattern.
	//  This enables creation of variant streams in subdirectories.
	//
	//	ffmpeg - re - i in.ts - b : v : 0 1000k - b : v : 1 256k - b : a : 0 64k - b : a : 1 32k \
	//	- map 0:v - map 0 : a - map 0 : v - map 0 : a - f hls - var_stream_map "v:0,a:0 v:1,a:1" \
	//	http ://example.com/live/out_%v.m3u8
	//	This example creates two hls variant streams.The first variant stream will contain video stream of bitrate 1000k 
	//  and audio stream of bitrate 64k and the second variant stream will contain video stream of bitrate 256k and audio stream of 
	//  bitrate 32k.Here, two media playlist with file names out_0.m3u8 and out_1.m3u8 will be created.If you want something meaningful 
	//	text instead of indexes in result names, you may specify names for each or some of the variants as in the following example.
	//
	//	ffmpeg - re - i in.ts - b : v : 0 1000k - b : v : 1 256k - b : a : 0 64k - b : a : 1 32k \
	//	- map 0:v - map 0 : a - map 0 : v - map 0 : a - f hls - var_stream_map "v:0,a:0,name:my_hd v:1,a:1,name:my_sd" \
	//	http ://example.com/live/out_%v.m3u8
	//	This example creates two hls variant streams as in the previous one.But here, the two media playlist with file names out_my_hd.m3u8 
	//  and out_my_sd.m3u8 will be created.
	//
	//	ffmpeg - re - i in.ts - b : v : 0 1000k - b : v : 1 256k - b : a : 0 64k \
	//	- map 0:v - map 0 : a - map 0 : v - f hls - var_stream_map "v:0 a:0 v:1" \
	//	http ://example.com/live/out_%v.m3u8
	//	This example creates three hls variant streams.The first variant stream will be a video only stream with video bitrate 1000k, 
	//  the second variant stream will be an audio only stream with bitrate 64k and the third variant stream will be a video only stream 
	//  with bitrate 256k.Here, three media playlist with file names out_0.m3u8, out_1.m3u8 and out_2.m3u8 will be created.
	//
	//	ffmpeg - re - i in.ts - b : v : 0 1000k - b : v : 1 256k - b : a : 0 64k - b : a : 1 32k \
	//	- map 0:v - map 0 : a - map 0 : v - map 0 : a - f hls - var_stream_map "v:0,a:0 v:1,a:1" \
	//	http ://example.com/live/vs_%v/out.m3u8
	//	This example creates the variant streams in subdirectories.Here, the first media playlist is created 
	//  at http ://example.com/live/vs_0/out.m3u8 and the second one at http://example.com/live/vs_1/out.m3u8.
	//
	//ffmpeg - re - i in.ts - b : a : 0 32k - b : a : 1 64k - b : v : 0 1000k - b : v : 1 3000k  \
	//	- map 0:a - map 0 : a - map 0 : v - map 0 : v - f hls \
	//	- var_stream_map "a:0,agroup:aud_low a:1,agroup:aud_high v:0,agroup:aud_low v:1,agroup:aud_high" \
	//	- master_pl_name master.m3u8 \
	//	http://example.com/live/out_%v.m3u8
	//This example creates two audio only and two video only variant streams.In addition to the #EXT - X - STREAM - INF tag 
	// for each variant stream in the master playlist, #EXT - X - MEDIA tag is also added for the two audio only variant streams 
	// and they are mapped to the two video only variant streams with audio group names ’aud_low’ and ’aud_high’.
	//
	//	By default, a single hls variant containing all the encoded streams is created.
	//
	//	ffmpeg - re - i in.ts - b:a:0 32k - b : a : 1 64k - b : v : 0 1000k \
	//	- map 0:a - map 0 : a - map 0 : v - f hls \
	//	- var_stream_map "a:0,agroup:aud_low,default:yes a:1,agroup:aud_low v:0,agroup:aud_low" \
	//	- master_pl_name master.m3u8 \
	//	http://example.com/live/out_%v.m3u8
	//This example creates two audio only and one video only variant streams.In addition to the #EXT - X - STREAM - INF tag 
	//  for each variant stream in the master playlist, #EXT - X - MEDIA tag is also added for the two audio only variant streams
	//  and they are mapped to the one video only variant streams with audio group name ’aud_low’, and the audio group have default stat is NO or YES.
	//
	//	By default, a single hls variant containing all the encoded streams is created.
	//
	//	ffmpeg - re - i in.ts - b:a:0 32k - b : a : 1 64k - b : v : 0 1000k \
	//	- map 0:a - map 0 : a - map 0 : v - f hls \
	//	- var_stream_map "a:0,agroup:aud_low,default:yes,language:ENG a:1,agroup:aud_low,language:CHN v:0,agroup:aud_low" \
	//	- master_pl_name master.m3u8 \
	//	http://example.com/live/out_%v.m3u8
	//This example creates two audio only and one video only variant streams.In addition to the #EXT - X - STREAM - INF tag 
	//  for each variant stream in the master playlist, #EXT - X - MEDIA tag is also added for the two audio only variant streams 
	//  and they are mapped to the one video only variant streams with audio group name ’aud_low’, and the audio group have default stat is NO or YES,
	//  and one audio have and language is named ENG, the other audio language is named CHN.
	//
	//	By default, a single hls variant containing all the encoded streams is created.
	//
	//	ffmpeg - y - i input_with_subtitle.mkv \
	//	- b:v:0 5250k - c : v h264 - pix_fmt yuv420p - profile : v main - level 4.1 \
	//	- b:a:0 256k \
	//	- c:s webvtt - c : a mp2 - ar 48000 - ac 2 - map 0 : v - map 0 : a : 0 - map 0 : s : 0 \
	//	- f hls - var_stream_map "v:0,a:0,s:0,sgroup:subtitle" \
	//	- master_pl_name master.m3u8 - t 300 - hls_time 10 - hls_init_time 4 - hls_list_size \
	//	10 - master_pl_publish_rate 10 - hls_flags \
	//	delete_segments + discont_start + split_by_time . / tmp / video.m3u8
	//	This example adds #EXT - X - MEDIA tag with TYPE = SUBTITLES in the master playlist with webvtt subtitle group name
	//  ’subtitle’.Please make sure the input file has one text subtitle stream at least.

	//av_opt_set_int(pOutFormatCtx->priv_data, "hls_list_size", 5, 0);
	av_opt_set_int(pOutFormatCtx->priv_data, "hls_time", Interval, 0);
	//av_opt_set_int(pOutFormatCtx->priv_data, "hls_flags", i | j, 0);

	AVDictionaryEntry* e = NULL;
	while ((e = av_dict_get(avdic, "", e, AV_DICT_IGNORE_SUFFIX)))
		printf("Segmenter Options - %s %s\n", e->key, e->value);

	ret = av_dict_count(avdic);

	return ret;
}

int FFSegmenter::Wirte_Header(AVFormatContext* pFormatCtx, AVDictionary* opt) {

	if (HeaderTail == true) {
		return -1;
	}

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

	HeaderTail = true;
	Running = true;

	return 0;
}

int FFSegmenter::Wirte_Trailer(AVFormatContext* pFormatCtx) {

	if (HeaderTail == true) {
		av_write_trailer(pFormatCtx);
		HeaderTail = false;
		Running = false;
	}
	return 0;
}