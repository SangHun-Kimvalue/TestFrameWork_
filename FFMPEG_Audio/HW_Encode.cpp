
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <time.h>

extern"C" {
#include "libavformat/avformat.h"
#include "libavformat/avio.h"

#include "libavcodec/avcodec.h"

#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavdevice/avdevice.h"
#include "libswresample/swresample.h"
}
void error_pro(int error, const char* msg);
static int width, height;
static AVBufferRef *hw_device_ctx = NULL;

static int set_hwframe_ctx(AVCodecContext *ctx, AVBufferRef *hw_device_ctx)
{
	AVBufferRef *hw_frames_ref;
	AVHWFramesContext *frames_ctx = NULL;
	int err = 0;

	if (!(hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx))) {
		fprintf(stderr, "Failed to create VAAPI frame context.\n");
		return -1;
	}
	frames_ctx = (AVHWFramesContext *)(hw_frames_ref->data);
	frames_ctx->format = AV_PIX_FMT_CUDA;
	frames_ctx->sw_format = AV_PIX_FMT_YUV420P;
	frames_ctx->width = width;
	frames_ctx->height = height;
	frames_ctx->initial_pool_size = 20;
	if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0) {
		//fprintf(stderr, "Failed to initialize VAAPI frame context."
		//	"Error code: %s\n", av_err2str(err));
		av_buffer_unref(&hw_frames_ref);
		error_pro(err, "av_hwframe_ctx_init");
		return err;
	}
	ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
	if (!ctx->hw_frames_ctx)
		err = AVERROR(ENOMEM);

	av_buffer_unref(&hw_frames_ref);
	return err;
}

static int encode_write(AVCodecContext *avctx, AVFrame *frame, FILE *fout)
{
	int ret = 0;
	AVPacket enc_pkt;

	av_init_packet(&enc_pkt);
	enc_pkt.data = NULL;
	enc_pkt.size = 0;

	if ((ret = avcodec_send_frame(avctx, frame)) < 0) {
		//fprintf(stderr, "Error code: %s\n", av_err2str(ret));
		goto end;
	}
	while (1) {
		ret = avcodec_receive_packet(avctx, &enc_pkt);
		if (ret) {
			error_pro(ret, "");
			av_packet_unref(&enc_pkt);
			break;
		}

		enc_pkt.stream_index = 0;
		ret = fwrite(enc_pkt.data, enc_pkt.size, 1, fout);
		std::cout << "Success " << std::endl;

		av_packet_unref(&enc_pkt);
	}

end:
	ret = ((ret == AVERROR(EAGAIN)) ? 0 : -1);
	return ret;
}

void error_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;
	//fprintf(stderr, "Error %s %s\n", errstr, msg);

	return;
}

static void fill_yuv_image(AVFrame *pict, int frame_index,
	int width, int height)
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
	ret = av_frame_get_buffer(picture, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate frame data.\n");
		exit(1);
	}

	return picture;
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow) {
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	int size, err;
	FILE *fin = NULL, *fout = NULL;
	AVFrame *sw_frame = NULL, *hw_frame = NULL;
	AVCodecContext *avctx = NULL;
	AVCodec *codec = NULL;
	const char *enc_name = "h264_nvenc";

	clock_t tstart = 0;
	clock_t tend = 0;
	clock_t start = 0;
	clock_t end = 0;

	//if (argc < 5) {
	//	fprintf(stderr, "Usage: %s <width> <height> <input file> <output file>\n", argv[0]);
	//	return -1;
	//}
	int i = 0;
	width = 1920;
	height = 1080;
	size = width * height;
	//err = fopen_s(&fin, "input.avi", "r");
	//if (err < 0) {
		//fprintf(stderr, "Fail to open input file : %s\n", strerror(errno));
	//	return -1;
	//}
	err = fopen_s(&fout, "output.264", "w+");
	if (err < 0){
		//fprintf(stderr, "Fail to open output file : %s\n", strerror(errno));
		err = -1;
		goto close;
	}

	err = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_CUDA,
		NULL, NULL, 0);
	if (err < 0) {
		error_pro(err, "");
		//fprintf(stderr, "Failed to create a VAAPI device. Error code: %s\n", av_err2str(err));
		goto close;
	}

	if (!(codec = avcodec_find_encoder_by_name(enc_name))) {
		fprintf(stderr, "Could not find encoder.\n");
		error_pro(err, "");
		err = -1;
		goto close;
	}

	if (!(avctx = avcodec_alloc_context3(codec))) {
		err = AVERROR(ENOMEM);
		goto close;
	}

	avctx->width = width;
	avctx->height = height;
	avctx->time_base = { 1, 30 };
	avctx->framerate = { 30, 1 };
	avctx->sample_aspect_ratio = { 1, 1 };
	avctx->pix_fmt = AV_PIX_FMT_CUDA;
	//avctx->pix_fmt = AV_PIX_FMT_YUV420P;				//AV_PIX_FMT_YUV420P12BE
	avctx->gop_size = 30;// I-frame per second

	avctx->width = width;
	avctx->height = height;
	//avctx->pix_fmt = AV_PIX_FMT_YUV420P;

	//avctx->colorspace = AVCOL_SPC_BT709;
	//avctx->color_range = AVCOL_RANGE_MPEG;
	//avctx->rc_buffer_size = 4096000;

	avctx->i_quant_factor = (float)0.71;                        // qscale factor between P and I frames
	avctx->qcompress = (float)0.6;								//0.0 => CBR, 1.0 => CQP. Recommended default: -qcomp 0.60
	avctx->bit_rate = 4096000;// targetBitrate << 10;  // ex) targetBitrate=1000 -> (shifting << 10) -> 1024000
	avctx->bit_rate_tolerance = avctx->bit_rate * av_q2d(avctx->time_base);
	avctx->rc_max_rate = avctx->bit_rate;
	avctx->rc_min_rate = avctx->bit_rate;

	//avctx_ctx->out_stream->time_base = Vstream_ctx->enc_ctx->framerate;

	avctx->codec_type = AVMEDIA_TYPE_VIDEO;
	avctx->skip_frame = AVDISCARD_NONREF;
	avctx->skip_loop_filter = AVDISCARD_ALL;
	avctx->skip_idct = AVDISCARD_ALL;
	avctx->idct_algo = 1;

	avctx->max_qdiff = 4;                                      // maximum quantizer difference between frames
	avctx->max_b_frames = 0;
	avctx->refs = 1;                                            // number of reference frames

	/* set hw_frames_ctx for encoder's AVCodecContext */
	if ((err = set_hwframe_ctx(avctx, hw_device_ctx)) < 0) {
		fprintf(stderr, "Failed to set hwframe context.\n");
		goto close;
	}

	if ((err = avcodec_open2(avctx, codec, NULL)) < 0) {
		//fprintf(stderr, "Cannot open video encoder codec. Error code: %s\n", av_err2str(err));
		goto close;
	}

	start = clock();

	while (1) {
		//if (!(sw_frame = av_frame_alloc())) {
		//	err = AVERROR(ENOMEM);
		//	goto close;
		//}
		sw_frame = alloc_picture(AV_PIX_FMT_YUV420P, width, height);
		if (!sw_frame) {
			fprintf(stderr, "Could not allocate video frame\n");
			exit(1);
		}
		/* read data into software frame, and transfer them into hw frame */
		sw_frame->width = width;
		sw_frame->height = height;
		sw_frame->format = AV_PIX_FMT_YUV420P;
		//if ((err = av_frame_get_buffer(sw_frame, 32)) < 0)
		//	goto close;
		//if ((err = fread((uint8_t*)(sw_frame->data[0]), size/ 2, 1, fin)) <= 0)
		//	break;
		//if ((err = fread((uint8_t*)(sw_frame->data[1]), size / 4, 1, fin)) <= 0)
		//	break;
		//if ((err = fread((uint8_t*)(sw_frame->data[2]), size / 4, 1, fin)) <= 0)
		//	break;
		int check2 = av_frame_make_writable(sw_frame);
		if (check2 < 0)
			return -1;
		fill_yuv_image(sw_frame, i++, width, height);

		if (!(hw_frame = av_frame_alloc())) {
			err = AVERROR(ENOMEM);
			goto close;
		}
		tstart = clock();
		if ((err = av_hwframe_get_buffer(avctx->hw_frames_ctx, hw_frame, 0)) < 0) {
			//fprintf(stderr, "Error code: %s.\n", av_err2str(err));
			goto close;
		}
		if (!hw_frame->hw_frames_ctx) {
			err = AVERROR(ENOMEM);
			goto close;
		}
		if ((err = av_hwframe_transfer_data(hw_frame, sw_frame, 0)) < 0) {
			//fprintf(stderr, "Error while transferring frame data to surface."
			//	"Error code: %s.\n", av_err2str(err));
			goto close;
		}

		if ((err = (encode_write(avctx, hw_frame, fout))) < 0) {
			fprintf(stderr, "Failed to encode.\n");
			goto close;
		}
		tend = clock();
		std::cout << tend - tstart << std::endl;
		av_frame_free(&hw_frame);
		av_frame_free(&sw_frame);
		end = clock();

		if ((end - start) > 15 * 1000)
			break;
	}

	/* flush encoder */
	err = encode_write(avctx, NULL, fout);
	if (err == AVERROR_EOF)
		err = 0;

close:
	if (fin)
		fclose(fin);
	if (fout)
		fclose(fout);
	av_frame_free(&sw_frame);
	av_frame_free(&hw_frame);
	avcodec_free_context(&avctx);
	av_buffer_unref(&hw_device_ctx);

	return err;

}