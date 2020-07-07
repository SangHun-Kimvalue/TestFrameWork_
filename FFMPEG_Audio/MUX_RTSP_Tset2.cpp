// Roughly based on: https://ffmpeg.org/doxygen/trunk/muxing_8c-source.html

#include <Windows.h>
#include <chrono>
#include <thread>
#include <tchar.h>
#include <iostream>

extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavformat\avio.h>
#include <libswscale\swscale.h>
#include <libavutil\time.h>
#include <libavutil/opt.h>
}

using namespace std;

static int video_is_eof;

#define STREAM_DURATION   20
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT   AV_PIX_FMT_YUV420P /* default pix_fmt */ //AV_PIX_FMT_NV12;
#define VIDEO_CODEC_ID AV_CODEC_ID_H264

/* video output */
static AVFrame *frame;
static AVFrame* src_picture, dst_picture;

/* Add an output stream. */
static AVStream *add_stream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, AVCodecContext*& c)
{
	//AVCodecContext *c;
	AVStream *st;

	int i;

	/* find the encoder */
	*codec = avcodec_find_encoder(codec_id);
	if (!(*codec)) {
		fprintf(stderr, "Could not find encoder for '%s'\n",
			avcodec_get_name(codec_id));
		exit(1);
	}

	st = avformat_new_stream(oc, NULL);
	if (!st) {
		fprintf(stderr, "Could not allocate stream\n");
		exit(1);
	}
	st->id = oc->nb_streams - 1;
	c = avcodec_alloc_context3(*codec);
	if (!c) {
		fprintf(stderr, "Could not alloc an encoding context\n");
		exit(1);
	}

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
		st->time_base = { 1, c->sample_rate };
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
		st->time_base = { 1, STREAM_FRAME_RATE };
		c->time_base = st->time_base;

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

	return st;
}

AVFrame* alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
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
	ret = av_frame_get_buffer(picture, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate frame data.\n");
		exit(1);
	}

	return picture;
}

static int open_video(AVFormatContext *oc, AVCodec *codec, AVStream *st ,AVCodecContext* c)
{
	int ret;

	//AVCodecContext *c = st->codec;
	int error = avcodec_parameters_to_context(c, st->codecpar);

	/* open the codec */
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Could not open video codec.\n", avcodec_get_name(c->codec_id));
	}
	else {

		/* allocate and init a re-usable frame */
		frame = av_frame_alloc();
		if (!frame) {
			av_log(NULL, AV_LOG_ERROR, "Could not allocate video frame.\n");
			ret = -1;
		}
		else {
			frame->format = c->pix_fmt;
			frame->width = c->width;
			frame->height = c->height;

			/* Allocate the encoded raw picture. */
			frame = alloc_picture(c->pix_fmt, c->width, c->height);
			
		}
	}

	return ret;
}

/* Prepare a dummy image. */
static void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
{
	int x, y, i;

	i = frame_index;

	/* Y */
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;

	/* Cb and Cr */
	for (y = 0; y < height / 2; y++) {
		for (x = 0; x < width / 2; x++) {
			pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
			pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
		}
	}
}

const char* error_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;

	return errstr;
}

int inner_encode(AVCodecContext* enc_ctx, AVFrame *frame, AVPacket *pkt, int *got_packet) {

	int m_error = 0;
	*got_packet = 0;
	av_packet_unref(pkt);
	av_init_packet(pkt);
	pkt->data = NULL;
	pkt->size = 0;

	m_error = avcodec_send_frame(enc_ctx, frame);
	if (m_error < 0) {
		error_pro(m_error, "avcodec_send_frame error");
		return m_error;
	}

	m_error = avcodec_receive_packet(enc_ctx, pkt);
	if (!m_error)
		*got_packet = 1;
	else if (m_error == AVERROR(EAGAIN)) {
		return 0;
	}
	else if (m_error < 0) {
		error_pro(m_error, "avcodec_receive_packet error");
		return m_error;
	}

	return m_error;
}


static int write_video_frame(AVFormatContext *oc, AVStream *st, int frameCount, AVCodecContext *c)
{
	int ret = 0;	
	//AVCodecContext *c = NULL;
	//AVCodecContext *c = st->codec;
	int error = avcodec_parameters_to_context(c, st->codecpar);

	fill_yuv_image(frame, frameCount, c->width, c->height);

	AVPacket pkt = { 0 };
	int got_packet;
	av_init_packet(&pkt);

	/* encode the image */
	frame->pts = frameCount;
	ret = inner_encode(c, frame, &pkt, &got_packet);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error encoding video frame.\n");
	}
	else {
		if (got_packet) {
			pkt.stream_index = st->index;
			pkt.pts = av_rescale_q_rnd(pkt.pts, c->time_base, st->time_base, AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			ret = av_write_frame(oc, &pkt);

			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Error while writing video frame.\n");
			}
		}
	}

	return ret;
}

static int rtspInterruptCallback(void *ctx)
{
	AVFormatContext* formatContext =
		reinterpret_cast<AVFormatContext*>(ctx);
	printf("In the callback on error\n");

	//clock_t wait_time = clock();
	//clock_t wait_time_end = 0;
	//while (1) {
	//	wait_time_end = clock();
	//	if((wait_time_end - wait_time ) > 100000)
	//		break;
	//
	//	Sleep(1);
	//	//return -1;
	//}

	return 0;
}

void custom_log(void *ptr, int level, const char* fmt, va_list vl) {

	//To TXT file
	FILE *fp = NULL;
	fopen_s(&fp, "av_log.txt", "a+");
	if (fp) {
		vfprintf(fp, fmt, vl);
		fflush(fp);
		fclose(fp);
	}

	//To Logcat
	//LOGE(fmt, vl);
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow) {
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	printf("starting...\n");
	const char *url = "rtsp://192.168.0.40:8554/test.sdp";
	//const char *url = "rtsp://192.168.33.19:1935/ffmpeg/0";
	av_log_set_level(AV_LOG_VERBOSE);
	av_log_set_callback(custom_log);

	AVFormatContext *outContext;
	AVStream *video_st;
	AVCodec *video_codec;
	AVCodecContext* c = NULL;
	int ret = 0, frameCount = 0;

	av_log_set_level(AV_LOG_DEBUG);
	//av_log_set_level(AV_LOG_TRACE);

	//av_register_all();
	//av_register_all();
	//avcodec_register_all();
	avformat_network_init();

	avformat_alloc_output_context2(&outContext, NULL, "rtsp", url);

	if (!outContext) {
		av_log(NULL, AV_LOG_FATAL, "Could not allocate an output context for '%s'.\n", url);
		return -1;
	}

	if (!outContext->oformat) {
		av_log(NULL, AV_LOG_FATAL, "Could not create the output format for '%s'.\n", url);
		return -1;
	}


	AVCodecID codec_id = VIDEO_CODEC_ID;
	video_st = add_stream(outContext, &video_codec, codec_id, c);

	av_opt_set(c->priv_data, "preset", "ultrafast", 0);
	av_opt_set(c->priv_data, "tune", "zerolatency", 0);



	/*video_st = avformat_new_stream(outContext, NULL);
	if (!video_st) {
		fprintf(stderr, "Could not allocate stream\n");
		exit(1);
	}
	video_st->id = outContext->nb_streams - 1;*/
	
	
	int m_error = avcodec_parameters_from_context(video_st->codecpar, c);
	if (m_error < 0) {
		error_pro(m_error, "Audio Could not copy the stream parameters\n");
		return m_error;
	}
	/* Now that all the parameters are set, we can open the video codec and allocate the necessary encode buffers. */
	if (video_st) {
		av_log(NULL, AV_LOG_DEBUG, "Video stream codec %s.\n ", avcodec_get_name(video_st->codecpar->codec_id));

		ret = open_video(outContext, video_codec, video_st, c);
		if (ret < 0) {
			av_log(NULL, AV_LOG_FATAL, "Open video stream failed.\n");
			return -1;
		}
	}
	else {
		av_log(NULL, AV_LOG_FATAL, "Add video stream for the codec '%s' failed.\n", avcodec_get_name(VIDEO_CODEC_ID));
		return -1;
	}

	av_dump_format(outContext, 0, url, 1);
	//outContext->oformat->flags = AVFMT_NOFILE;
	//m_error = avio_open(&outContext->pb, "rtsp", AVIO_FLAG_WRITE);
	//if (m_error < 0) {
	//	error_pro(m_error, "avio_open");
	//	return m_error;
	//}
	//outContext->interrupt_callback.callback = rtspInterruptCallback;
	//outContext->interrupt_callback.opaque = outContext;
	//outContext->flags = outContext->flags | AVFMT_FLAG_NONBLOCK;
	ret = avformat_write_header(outContext, NULL);
	if (ret != 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to connect to RTSP server for '%s'.\n", url);
		return -1;
	}

	printf("Press any key to start streaming...\n");
	getchar();

	auto startSend = std::chrono::system_clock::now();

	while (video_st) {
		frameCount++;
		auto startFrame = std::chrono::system_clock::now();

		ret = write_video_frame(outContext, video_st, frameCount, c);

		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Write video frame failed.\n", url);
			goto end;
		}

		auto streamDuration = std::chrono::duration_cast<chrono::milliseconds>(std::chrono::system_clock::now() - startSend).count();

		printf("Elapsed time %ldms, video stream pts %ld.\n", streamDuration, video_st->time_base);

		if (streamDuration / 1000.0 > STREAM_DURATION) {
			break;
		}
		else {
			auto frameDuration = std::chrono::duration_cast<chrono::milliseconds>(std::chrono::system_clock::now() - startFrame).count();
			std::this_thread::sleep_for(std::chrono::milliseconds((long)(1000.0 / STREAM_FRAME_RATE - frameDuration)));
		}
	}

	if (video_st) {
		//avcodec_close(video_st->codec);
		av_free(src_picture->data[0]);
		av_free(dst_picture.data[0]);
		av_frame_free(&frame);
	}

	avformat_free_context(outContext);

end:
	printf("finished.\n");

	getchar();

	return 0;
}