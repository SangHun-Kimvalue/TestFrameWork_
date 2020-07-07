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
} StreamContext;
static StreamContext *stream_ctx;

static AVFormatContext *ifmt_ctx;
AVFormatContext *ofmt_ctx = NULL;

std::list<uint8_t*> buffer;
std::list<AVFrame*> fbuffer;


static int open_output_file(const char *filename);
static int open_input_file();
void printAudioFrameInfo(const AVCodecContext* codecContext, const AVFrame* frame);

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

AVStream *out_stream = NULL;

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

	static AVFormatContext *ofmt_ctx;
	ofmt_ctx = NULL;
	AVStream* ostream = NULL;
	error = open_input_file();
	if (error) {
		error_pro(error, "input file error");
		return error;
	}
	error = open_output_file(encode_outfilename);
	if (error) {
		error_pro(error, "input file error");
		return error;
	}

	packet = av_packet_alloc();

	clock_t start = clock();
	clock_t end = 0;

	int count = 0;
	float loopend = 0;
	int data_size = 0;

	error = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, encode_outfilename);
	if (!ofmt_ctx) {
		//av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		//return AVERROR_UNKNOWN;
		std::cout << "avformat_alloc_output_context2" << std::endl;
		return -1;
	}
	//out_stream = avformat_new_stream(ofmt_ctx, NULL);
	//if (!out_stream) {
	//	std::cout << "avformat_new_stream" << std::endl;
	//	return -1;
	//}
	error = avcodec_parameters_from_context(out_stream->codecpar, stream_ctx->enc_ctx);
	if (error < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", 0);
		return error;
	}

	while (1) {
		loopend = end - start;
		if (loopend > 10000) {
			break;
		}

		if ((error = av_read_frame(ifmt_ctx, packet)) < 0) {
			std::cout << "EOF" << std::endl;
			break;
		}
		else {
			////pkt_stream_index = packet->stream_index;
			//int pkt_pts = packet->pts;
			//int pkt_dts = packet->dts;
			//
			//if (packet->stream_index == 0)
			//{
			//	packet->stream_index = out_stream->index;
			//	av_write_frame(ofmt_ctx, packet);
			//
			//	packet->stream_index = 0;
			//	packet->pts = pkt_pts;
			//	packet->dts = pkt_dts;
			//}
			////av_free_packet(packet);

			error = avcodec_send_packet(stream_ctx->dec_ctx, packet);
			if (error == -11) {
				error_pro(error, "avcodec_send_packet again");
				av_packet_unref(packet);
				continue;
			}
			else if (error < 0) {
				error_pro(error, "avcodec_send_packet again");
				break;
			}
			else {
				frame = av_frame_alloc();
				if (!frame) {
					break;
				}

				error = avcodec_receive_frame(stream_ctx->dec_ctx, frame);
				if (error == -11) {
					error_pro(error, "avcodec_send_packet again");
					av_packet_unref(packet);
					continue;
				}
				else if (error < 0) {
					error_pro(error, "avcodec_send_packet again");
					break;
				}

				fbuffer.push_back(frame);
				buffer.push_back(frame->data[0]);
			}

			end = clock();
			count++;
		}
	}
	
	printAudioFrameInfo(stream_ctx->dec_ctx, frame);
	
	wav* m_wav = new wav;

	m_wav->save_init(outfilename, packet->duration, frame->sample_rate, 32, 1);	//const char* filename, int duration, int smaplerate, int bit_rate, int channel

	m_wav->save(buffer, frame->linesize[0]);
	
	m_wav->close();



	av_free(stream_ctx->dec_ctx);
	av_packet_unref(packet);
	av_frame_free(&frame);
	fclose(cp);

	return 0;
}


static int open_input_file() {

	//AVDictionary* options = NULL;

	/////////////////////////////////////////////////////
	avdevice_register_all();

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

	//int ASI = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &)
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

	//av_dump_format(ifmt_ctx, 0, conv.c_str(), 0);
	return 0;
}

void printAudioFrameInfo(const AVCodecContext* codecContext, const AVFrame* frame)
{
	// See the following to know what data type (unsigned char, short, float, etc) to use to access the audio data:
	// http://ffmpeg.org/doxygen/trunk/samplefmt_8h.html#af9a51ca15301871723577c730b5865c5
	std::cout << "Audio frame info:\n"
		<< "  Sample count: " << frame->nb_samples << '\n'
		<< "  Channel count: " << codecContext->channels << '\n'
		<< "  Format: " << av_get_sample_fmt_name(codecContext->sample_fmt) << '\n'
		<< "  Bytes per sample: " << av_get_bytes_per_sample(codecContext->sample_fmt) << '\n'
		<< "  Is planar? " << av_sample_fmt_is_planar(codecContext->sample_fmt) << '\n'
		<< "  Duration? " << frame->pkt_duration << '\n'
		<< "  Sample rate? " << frame->sample_rate << '\n'
		<< "  Frame length? " << (1000 / double(frame->sample_rate)*double(frame->nb_samples)) << '\n';


	std::cout << "frame->linesize[0] tells you the size (in bytes) of each plane\n";

	if (codecContext->channels > AV_NUM_DATA_POINTERS && av_sample_fmt_is_planar(codecContext->sample_fmt))
	{
		std::cout << "The audio stream (and its frames) have too many channels to fit in\n"
			<< "frame->data. Therefore, to access the audio data, you need to use\n"
			<< "frame->extended_data to access the audio data. It's planar, so\n"
			<< "each channel is in a different element. That is:\n"
			<< "  frame->extended_data[0] has the data for channel 1\n"
			<< "  frame->extended_data[1] has the data for channel 2\n"
			<< "  etc.\n";
	}
	else
	{
		std::cout << "Either the audio data is not planar, or there is enough room in\n"
			<< "frame->data to store all the channels, so you can either use\n"
			<< "frame->data or frame->extended_data to access the audio data (they\n"
			<< "should just point to the same data).\n";
	}

	std::cout << "If the frame is planar, each channel is in a different element.\n"
		<< "That is:\n"
		<< "  frame->data[0]/frame->extended_data[0] has the data for channel 1\n"
		<< "  frame->data[1]/frame->extended_data[1] has the data for channel 2\n"
		<< "  etc.\n";

	std::cout << "If the frame is packed (not planar), then all the data is in\n"
		<< "frame->data[0]/frame->extended_data[0] (kind of like how some\n"
		<< "image formats have RGB pixels packed together, rather than storing\n"
		<< " the red, green, and blue channels separately in different arrays.\n";
}

static int open_output_file(const char *filename)
{

	AVStream *in_stream;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder = NULL;
	AVPacket* pkt;
	AVFrame* frame;
	int ret;
	unsigned int i;
	static AVFormatContext *ofmt_ctx;

	ofmt_ctx = NULL;
	
	dec_ctx = stream_ctx[0].dec_ctx;

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

	enc_ctx = avcodec_alloc_context3(encoder);
	if (!enc_ctx) {
		fprintf(stderr, "Could not allocate audio codec context\n");
		exit(1);
	}

	in_stream = ifmt_ctx->streams[0];
	


	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		out_stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream) {
			std::cout << "avformat_new_stream" << std::endl;
			return -1;
		}

		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		
				enc_ctx->sample_rate = dec_ctx->sample_rate;
				enc_ctx->channel_layout = dec_ctx->channel_layout;
				enc_ctx->channels = 1;
				enc_ctx->sample_fmt = encoder->sample_fmts[0];
				enc_ctx->time_base = { 1, enc_ctx->sample_rate };


			ret = avcodec_open2(enc_ctx, encoder, NULL);
			if (ret < 0) {
				error_pro(ret, "avcodec_open2");
				return -1;
			}
			ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
			if (ret < 0) {
				error_pro(ret, "avcodec_parameters_from_context");
				return -1;
			}

			out_stream->time_base = enc_ctx->time_base;
			stream_ctx[i].enc_ctx = enc_ctx;
		}
		else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
			std::cout << "codec_type is strange" << std::endl;
			return -1;
		}
		else {
			/* if this stream must be remuxed */
			ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
			if (ret < 0) {
				error_pro(ret, "avcodec_parameters_copy");
				return -1;
			}
			out_stream->time_base = in_stream->time_base;
		}
	}

	stream_ctx->enc_ctx = enc_ctx;
	return 0;
}
