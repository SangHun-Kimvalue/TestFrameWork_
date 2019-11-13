#include <Windows.h>
#include <iostream>
//#include "audiorw.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
//#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
}

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096
char errstr[256];
int error = 0;

void error_pro(int error, const char* msg) {

	av_strerror(error, errstr, 256);
	std::cout << "Error" << errstr << " " << msg << std::endl;
	//fprintf(stderr, "Error %s %s\n", errstr, msg);

	return;
}

/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec *codec)
{
	const uint64_t *p;
	uint64_t best_ch_layout = 0;
	int best_nb_channels = 0;

	if (!codec->channel_layouts)
		return AV_CH_LAYOUT_STEREO;

	p = codec->channel_layouts;
	while (*p) {
		int nb_channels = av_get_channel_layout_nb_channels(*p);

		if (nb_channels > best_nb_channels) {
			best_ch_layout = *p;
			best_nb_channels = nb_channels;
		}
		p++;
	}
	return best_ch_layout;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec *codec)
{
	const int *p;
	int best_samplerate = 0;

	if (!codec->supported_samplerates)
		return 44100;

	p = codec->supported_samplerates;
	while (*p) {
		if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
			best_samplerate = *p;
		p++;
	}
	return best_samplerate;
}

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt)
{
	const enum AVSampleFormat *p = codec->sample_fmts;

	while (*p != AV_SAMPLE_FMT_NONE) {
		if (*p == sample_fmt)
			return 1;
		p++;
	}
	return 0;
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
		"sample format %s is not supported as output format\n",
		av_get_sample_fmt_name(sample_fmt));
	return -1;
}


static int decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame,
	FILE *outfile)
{
	int i, ch;
	int ret, data_size;
	char errstr[256];
	int error = 0;

	/* send the packet with the compressed data to the decoder */
	error = avcodec_send_packet(dec_ctx, pkt);
	if (error < 0) {
		error_pro(error, "Error submitting the packet to the decoder\n");
		av_strerror(error, errstr, 256);
		fprintf(stderr, "Error submitting the packet to the decoder\n");
		return -1;
	}

	/* read all the output frames (in general there may be any number of them */
	while (error >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_strerror(ret, errstr, 256);
			return -1;
		}
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n"); 
			av_strerror(ret, errstr, 256);
			return -1;
		}
		data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
		if (data_size < 0) {
			/* This should not occur, checking just for paranoia */
			fprintf(stderr, "Failed to calculate data size\n");
			av_strerror(ret, errstr, 256);
			return -1;
		}
		//데이터 저장
		for (i = 0; i < frame->nb_samples; i++)
			for (ch = 0; ch < dec_ctx->channels; ch++)
				fwrite(frame->data[ch] + data_size * i, 1, data_size, outfile);
	}
}

static void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt,
	FILE *output)
{
	int ret;

	/* send the frame for encoding */
	ret = avcodec_send_frame(ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending the frame to the encoder\n");
		exit(1);
	}

	/* read all the available output packets (in general there may be any
	 * number of them */
	while (ret >= 0) {
		ret = avcodec_receive_packet(ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error encoding audio frame\n");
			exit(1);
		}

		fwrite(pkt->data, 1, pkt->size, output);
		av_packet_unref(pkt);
	}
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	//const char* filename = "input_test.mp3";
	const char* filename = "output_test.mp3";
	//const char* wfilename = "wtest.wav";
	
	//const char *outfilename = "wtest.wav";
	AVCodec *codec;
	//AVCodecContext *c = NULL;
	AVFormatContext *fmtc = NULL;
	//AVCodecParserContext *parser = NULL;
	int len, ret;
	FILE *f, *outfile;
	uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	//uint8_t *data;
	//size_t   data_size;
	AVPacket *pkt;
	//AVFrame *decoded_frame = NULL;
	AVDictionary* options = NULL;
	//enum AVSampleFormat sfmt;
	int n_channels = 0;
	//const char *fmt = "";
	int ASI = 0;

	///////////////////////////////////////////////////////////////////////
	AVCodecContext *ec = NULL;
	AVCodec *ecodec = NULL;
	AVFrame* frame = NULL;
	uint16_t *samples;

	avdevice_register_all();
	filename = filename;
	//outfilename = wfilename;

	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	if (iformat == NULL) {
		printf("iformat is NULL\n");
	}

	pkt = av_packet_alloc();

	fmtc = avformat_alloc_context();
	std::string conv;
	std::wstring wchar = L"audio=마이크 배열(Realtek High Definition Audio)";
	//std::wstring wchar = L"example.mp2";
	int size = wchar.size();
	convert_unicode_to_utf8_string(conv, wchar.c_str(), size);
	//std::string filename = "audio=마이크 배열(Realtek High Definition Audio)";
	
	error = avformat_open_input(&fmtc, conv.c_str(), iformat, NULL);
	//error = avformat_open_input(&fmtc, "example.mp2", NULL, NULL);
	//if (error < 0) {
	//	av_strerror(error, errstr, sizeof(errstr));
	//	std::cout << conv.c_str() << std::endl;
	//	std::cout << "avformat_open_input Error : " << errstr << std::endl;
	//	return -1;
	//}

	//파일 디코딩 용
	//ret = avformat_open_input(&fmtc, filename, NULL, NULL);
	//if (ret < 0) {
	//	av_strerror(ret, errstr, 256);
	//	return ret;
	//}

	//error = avformat_find_stream_info(fmtc, NULL);
	//if (error < 0) {
	//	error_pro(error, "avformat_find_stream_info");
	//	return error;
	//}

	ecodec = avcodec_find_encoder(AV_CODEC_ID_MP2);
	if (!ecodec) {
		printf("find_encoder error.\n");
		return -1;
	}

	ec = avcodec_alloc_context3(ecodec);
	if (!ec) {
		printf("alloc_context error.\n");
		return -1;
	}

	ec->bit_rate = 64000;//44100

	ec->sample_fmt = AV_SAMPLE_FMT_S16;
	error = check_sample_fmt(ecodec, ec->sample_fmt);
	if (error == 0) {
		//fprintf(stderr, "Encoder does not support sample format %s",
		//	av_get_sample_fmt_name(ec->sample_fmt));
		error_pro(error, av_get_sample_fmt_name(ec->sample_fmt));
		return -1;
	}
	/* select other audio parameters supported by the encoder 인코딩 필수 작업 */
	ec->sample_rate = select_sample_rate(ecodec);
	ec->channel_layout = select_channel_layout(ecodec);
	ec->channels = av_get_channel_layout_nb_channels(ec->channel_layout);
	
	error = avcodec_open2(ec, ecodec, NULL);
	if (error < 0) {
		error_pro(error, "Could not open codec");
		//av_strerror(error, errstr, 256);
		//fprintf(stderr, "Could not open codec\n");
		return error;
	}

	fopen_s(&f, filename, "wb");
	if (!f) {
		fprintf(stderr, "Could not open file %s\n", filename);
		return -1;
	}

	/* packet for holding encoded output */
	pkt = av_packet_alloc();
	if (!pkt) {
		fprintf(stderr, "could not allocate the packet\n");
		return -1;
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "could not allocate the frame\n");
		return -1;
	}
	frame->nb_samples = ec->frame_size;
	frame->format = ec->sample_fmt;
	frame->channel_layout = ec->channel_layout;

	error = av_frame_get_buffer(frame, NULL);
	if (error < 0) {
		error_pro(error, "could not av_frame_get_buffer");
		//av_strerror(error, errstr, sizeof(errstr));
		//fprintf(stderr, "could not av_frame_get_buffer\n");
		return error;
	}

	int buffer_size = av_samples_get_buffer_size(NULL, ec->channels, ec->frame_size,
		ec->sample_fmt, 0);

	samples = (uint16_t*)av_malloc(buffer_size);
	if (!samples) {
		fprintf(stderr, "Could not allocate %d bytes for samples buffer\n",
			buffer_size);
		exit(1);
	}

	/* setup the data pointers in the AVFrame */
	error = avcodec_fill_audio_frame(frame, ec->channels, ec->sample_fmt,
		(const uint8_t*)samples, buffer_size, 0);
	if (error < 0) {
		fprintf(stderr, "Could not setup audio frame\n");
		exit(1);
	}

	float tan = 0;
	float tincr = 2 * M_PI * 440.0 / ec->sample_rate;
	for (int i = 0; i < 200; i++) {
		error = av_frame_make_writable(frame);
		if (error < 0) {
			error_pro(error, "could not av_frame_make_writable");
			//av_strerror(error, errstr, sizeof(errstr));
			//fprintf(stderr, "could not av_frame_get_buffer\n");
			return error;
		}

		samples = (uint16_t*)frame->data[0];

		for (int j = 0; j < ec->frame_size; j++) {
			samples[2 * j] = (int)(sin(tan) * 10000);

			for (int k = 1; k < ec->channels; k++)
				samples[2 * j + k] = samples[2 * j];
			tan += tincr;
		}
		encode(ec, frame, pkt, f);
	}

	/* flush the encoder */
	encode(ec, NULL, pkt, f);

	fclose(f);

	av_frame_free(&frame);
	av_packet_free(&pkt);
	avcodec_free_context(&ec);

	//ASI = av_find_best_stream(fmtc, AVMEDIA_TYPE_AUDIO, -1, -1, &ecodec, 0);
	//if (ASI < 0) {
	//	fprintf(stderr, "Codec not found\n");
	//	exit(1);
	//}

	////* find the MPEG audio decoder */
	//codec = avcodec_find_decoder(fmtc->streams[ASI]->codecpar->codec_id);
	//if (!codec) {
	//	fprintf(stderr, "Codec not found\n");
	//	exit(1);
	//}
	//
	//c = avcodec_alloc_context3(codec);
	//if (!c) {
	//	fprintf(stderr, "Could not allocate audio codec context\n");
	//	exit(1);
	//}
	//
	////* open it */
	//error = avcodec_open2(c, codec, NULL);
	//if (error < 0) {
	//	av_strerror(error, errstr, sizeof(errstr));
	//	fprintf(stderr, "Could not open codec\n");
	//	exit(1);
	//}
	//
	//fopen_s(&f, filename, "rb");
	//if (!f) {
	//	fprintf(stderr, "Could not open %s\n", filename);
	//	exit(1);
	//}
	//
	//fopen_s(&outfile, outfilename, "wb");
	//if (!outfile) {
	//	av_free(c);
	//	exit(1);
	//}
	//
	//parser = av_parser_init(codec->id);
	//if (!parser) {
	//	fprintf(stderr, "Parser not found\n");
	//	exit(1);
	//}
	//
	////* decode until eof */
	//data = inbuf;
	//data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);
	//
	//
	//if (pkt->stream_index == ASI)
	//{
	//	while (data_size > 0) {
	//		if (!decoded_frame) {
	//			if (!(decoded_frame = av_frame_alloc())) {
	//				fprintf(stderr, "Could not allocate audio frame\n");
	//				exit(1);
	//			}
	//		}
	//
	//		ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
	//			data, data_size,
	//			AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	//		if (ret < 0) {
	//			fprintf(stderr, "Error while parsing\n");
	//			exit(1);
	//		}
	//		data += ret;
	//		data_size -= ret;
	//
	//		if (pkt->size) {
	//			if (av_read_frame(fmtc, pkt) >= 0) {
	//				if (ASI == pkt->stream_index) {
	//					/*error = avcodec_send_packet(c, pkt);
	//					if (error < 0) {
	//						if (error == AVERROR_EOF)
	//							break;
	//
	//						av_packet_unref(pkt);
	//						return false;
	//					}*/
	//					decode(c, pkt, decoded_frame, outfile);
	//				}
	//			}
	//			//decode(c, pkt, decoded_frame, outfile);
	//		}
	//		if (data_size < AUDIO_REFILL_THRESH) {
	//			memmove(inbuf, data, data_size);
	//			data = inbuf;
	//			len = fread(data + data_size, 1,
	//				AUDIO_INBUF_SIZE - data_size, f);
	//			if (len > 0)
	//				data_size += len;
	//		}
	//	}
	//
	//	/* flush the decoder */
	//	pkt->data = NULL;
	//	pkt->size = 0;
	//	decode(c, pkt, decoded_frame, outfile);
	//
	//	/* print output pcm infomations, because there have no metadata of pcm */
	//	sfmt = c->sample_fmt;
	//
	//	if (av_sample_fmt_is_planar(sfmt)) {
	//		const char *packed = av_get_sample_fmt_name(sfmt);
	//		printf("Warning: the sample format the decoder produced is planar "
	//			"(%s). This example will output the first channel only.\n",
	//			packed ? packed : "?");
	//		sfmt = av_get_packed_sample_fmt(sfmt);
	//	}
	//
	//	n_channels = c->channels;
	//	if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
	//		goto end;
	//}
	//printf("Play the output audio file with the command:\n"
	//	"ffplay -f %s -ac %d -ar %d %s\n",
	//	fmt, n_channels, c->sample_rate,
	//	outfilename);
	//end:
	//fclose(outfile);
	//fclose(f);
	//fclose(cp);
	//
	//avcodec_free_context(&c);
	//av_parser_close(parser);
	//av_frame_free(&decoded_frame);
	//av_packet_free(&pkt);
	

	return 0;
}
