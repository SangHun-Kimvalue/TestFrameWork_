#include <Windows.h>
#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <list>

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include "wav.h"

char errstr[256];
int error = 0;

typedef struct StreamContext {
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
	AVFormatContext *ifmt_ctx;
	AVFormatContext *ofmt_ctx = NULL;
	AVStream *out_stream = NULL;

} StreamContext;
static StreamContext *stream_ctx;



static int open_output_file(const char *filename);
static int open_input_file();
static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt);

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

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(pkt, *time_base, st->time_base);
	pkt->stream_index = st->index;

	/* Write the compressed frame to the media file. */
	//log_packet(fmt_ctx, pkt);
	return av_interleaved_write_frame(fmt_ctx, pkt);
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);
	AVPacket *packet = NULL;
	AVFrame* frame = NULL;
	FILE* f;

	const char* outfilename = "output.wav";
	const char* encode_outfilename = "encode_output.wav";
	
	error = open_input_file();
	if (error) {
		error_pro(error, "input file error");
		return error;
	}
	error = open_output_file(encode_outfilename);
	if (error) {
		error_pro(error, "output file error");
		return error;
	}

	packet = av_packet_alloc();

	clock_t start = clock();
	clock_t end = 0;

	int count = 0;
	float loopend = 0;
	int data_size = 0;

	//error = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, encode_outfilename);
	//if (!ofmt_ctx) {
	//	//av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
	//	//return AVERROR_UNKNOWN;
	//	std::cout << "avformat_alloc_output_context2" << std::endl;
	//	return -1;
	//}
	////out_stream = avformat_new_stream(ofmt_ctx, NULL);
	////if (!out_stream) {
	////	std::cout << "avformat_new_stream" << std::endl;
	////	return -1;
	////}
	//error = avcodec_parameters_from_context(out_stream->codecpar, stream_ctx->enc_ctx);
	//if (error < 0) {
	//	av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", 0);
	//	return error;
	//}

	while (1) {
		loopend = end - start;
		if (loopend > 10000) {
			break;
		}

		if ((error = av_read_frame(stream_ctx->ifmt_ctx, packet)) < 0) {
			std::cout << "EOF" << std::endl;
			break;
		}
		else {
			int pkt_pts = packet->pts;
			int pkt_dts = packet->dts;
			
			if (packet->stream_index == 0)
			{
				packet->stream_index = stream_ctx->out_stream->index;
				error = write_frame(stream_ctx->ofmt_ctx, &stream_ctx->enc_ctx->time_base, stream_ctx->out_stream, packet);
				if (error < 0) {
					error_pro(error, "av_interleaved_write_frame error");
					av_packet_unref(packet);
				}
				packet->stream_index = 0;
				packet->pts = pkt_pts;
				packet->dts = pkt_dts;
			}
	
			end = clock();
			count++;
		}
	}

	av_write_trailer(stream_ctx->ofmt_ctx);

	av_free(stream_ctx->ofmt_ctx);
	av_free(stream_ctx->dec_ctx);
	av_free(stream_ctx->enc_ctx);
	av_packet_unref(packet);
	av_frame_free(&frame);
	fclose(cp);

	return 0;
}

static int open_input_file() {

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

	stream_ctx = (StreamContext*)av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*stream_ctx));
	if (!stream_ctx) {
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
		stream_ctx[i].dec_ctx = codec_ctx;
	}
	stream_ctx->ifmt_ctx = ifmt_ctx;
	//av_dump_format(ifmt_ctx, 0, conv.c_str(), 0);
	return 0;
}

static int open_output_file(const char *filename)
{
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder = NULL;
	AVFormatContext* ofmt_ctx = NULL;
	AVStream* out_stream = NULL;
	
	int ret;
	
	dec_ctx = stream_ctx->dec_ctx;
	error = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
	if (!ofmt_ctx) {
		//av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		//return AVERROR_UNKNOWN;
		std::cout << "avformat_alloc_output_context2" << std::endl;
		return -1;
	}
	encoder = avcodec_find_encoder(dec_ctx->codec_id);
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
	stream_ctx->enc_ctx = enc_ctx;

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
	
	stream_ctx->enc_ctx = enc_ctx;

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

	stream_ctx->ofmt_ctx = ofmt_ctx;
	stream_ctx->out_stream = out_stream;

	return 0;
}
