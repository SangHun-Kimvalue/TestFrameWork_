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

typedef struct StreamContext {
	AVCodecContext *dec_ctx;
	AVCodecContext *enc_ctx;
} StreamContext;
static StreamContext *stream_ctx;


static AVFormatContext *ifmt_ctx;


static int open_output_file(const char *filename);
static int open_input_file();
void printAudioFrameInfo(const AVCodecContext* codecContext, const AVFrame* frame);

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
	std::list<uint8_t*> buffer;
	//std::vecter<uint8_t> buffer;

	const char* outfilename = "output.wav";

	error = open_input_file();
	if (error) {
		error_pro(error, "input file error");
		return error;
	}

	//error = open_output_file(outfilename);
	//if (error) {
	//	error_pro(error, "open_output_file error");
	//	return error;
	//}

	packet = av_packet_alloc();

	clock_t start = clock();
	clock_t end = 0;

	int count = 0;
	float loopend = 0;
	int data_size = 0;

	while (1) {
		loopend = end - start;
		if (loopend > 10000) {
			break;
		}

		if ((error = av_read_frame(ifmt_ctx, packet)) < 0) {
			std::cout << "EOF" << std::endl;
			break;
		}

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

			//data_size = av_get_bytes_per_sample(stream_ctx->dec_ctx->sample_fmt);
			//if (data_size < 0) {
			//	/* This should not occur, checking just for paranoia */
			//	fprintf(stderr, "Failed to calculate data size\n");
			//	exit(1);
			//}

			buffer.push_back(frame->data[0]);
		}

		//stream_index = packet->stream_index;
		//type = ifmt_ctx->streams[packet->stream_index]->codecpar->codec_type;
		//타임 스탬프 동기화로 싱크 작업
		//av_packet_rescale_ts(&packet,
		//	ifmt_ctx->streams[stream_index]->time_base,
		//	stream_ctx[stream_index].dec_ctx->time_base);

		end = clock();
		count++;
	}
	
	printAudioFrameInfo(stream_ctx->dec_ctx, frame);
	
	wav* m_wav = new wav;
	//pkt->duration, frame->samplerate, 32, 1
	//m_wav->save_init("output.wav", 1024, 16000, 32, 1);	//const char* filename, int duration, int smaplerate, int bit_rate, int channel
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

//int _tssmain(int argc)
//{
//
//
//
//	av_register_all();
//
//	AVFrame* frame = av_frame_alloc();
//	if (!frame)
//	{
//		std::cout << "Error allocating the frame" << std::endl;
//		return 1;
//	}
//
//	// you can change the file name "01 Push Me to the Floor.wav" to whatever the file is you're reading, like "myFile.ogg" or
//	// "someFile.webm" and this should still work
//	AVFormatContext* formatContext = NULL;
//	if (avformat_open_input(&formatContext, "C:\\Users\\Emanuele\\Downloads\\I still have soul (HBO Boxing) New Motivational and Inspirational Videov.dts", NULL, NULL) != 0)
//	{
//		av_free(frame);
//		std::cout << "Error opening the file" << std::endl;
//		return 1;
//	}
//
//	if (avformat_find_stream_info(formatContext, NULL) < 0)
//	{
//		av_free(frame);
//		avformat_close_input(&formatContext);
//		std::cout << "Error finding the stream info" << std::endl;
//		return 1;
//	}
//
//	// Find the audio stream
//	AVCodec* cdc = nullptr;
//	int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
//	if (streamIndex < 0)
//	{
//		av_free(frame);
//		avformat_close_input(&formatContext);
//		std::cout << "Could not find any audio stream in the file" << std::endl;
//		return 1;
//	}
//
//	AVStream* audioStream = formatContext->streams[streamIndex];
//	AVCodecContext* codecContext = audioStream->codecpar;
//	codecContext->codec = cdc;
//
//	if (avcodec_open2(codecContext, codecContext->codec, NULL) != 0)
//	{
//		av_free(frame);
//		avformat_close_input(&formatContext);
//		std::cout << "Couldn't open the context with the decoder" << std::endl;
//		return 1;
//	}
//
//	std::cout << "This stream has " << codecContext->channels << " channels and a sample rate of " << codecContext->sample_rate << "Hz" << std::endl;
//	std::cout << "The data is in the format " << av_get_sample_fmt_name(codecContext->sample_fmt) << std::endl;
//
//	AVPacket readingPacket;
//	av_init_packet(&readingPacket);
//
//	// Read the packets in a loop
//	while (av_read_frame(formatContext, &readingPacket) == 0)
//	{
//		if (readingPacket.stream_index == audioStream->index)
//		{
//			AVPacket decodingPacket = readingPacket;
//
//			// Audio packets can have multiple audio frames in a single packet
//			while (decodingPacket.size > 0)
//			{
//				// Try to decode the packet into a frame
//				// Some frames rely on multiple packets, so we have to make sure the frame is finished before
//				// we can use it
//				int gotFrame = 0;
//				int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &decodingPacket);
//
//				if (result >= 0 && gotFrame)
//				{
//					decodingPacket.size -= result;
//					decodingPacket.data += result;
//
//					// We now have a fully decoded audio frame
//					printAudioFrameInfo(codecContext, frame);
//				}
//				else
//				{
//					decodingPacket.size = 0;
//					decodingPacket.data = nullptr;
//				}
//			}
//		}
//
//		// You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
//		av_free_packet(&readingPacket);
//	}
//
//	// Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
//	// is set, there can be buffered up frames that need to be flushed, so we'll do that
//	if (codecContext->codec->capabilities & AV_CODEC_CAP_DELAY)
//	{
//		av_init_packet(&readingPacket);
//		// Decode all the remaining frames in the buffer, until the end is reached
//		int gotFrame = 0;
//		while (avcodec_decode_audio4(codecContext, frame, &gotFrame, &readingPacket) >= 0 && gotFrame)
//		{
//			// We now have a fully decoded audio frame
//			printAudioFrameInfo(codecContext, frame);
//		}
//	}
//
//	// Clean up!
//	av_free(frame);
//	avcodec_close(codecContext);
//	avformat_close_input(&formatContext);
//
//	//_tprintf(_T("Done.\n"));
//	return 0;
//}

static int open_output_file(const char *filename)
{
	AVStream *out_stream;
	AVStream *in_stream;
	AVCodecContext *dec_ctx, *enc_ctx;
	AVCodec *encoder;
	int ret;
	unsigned int i;
	static AVFormatContext *ofmt_ctx;

	ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
	if (!ofmt_ctx) {
		//av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
		//return AVERROR_UNKNOWN;
		std::cout << "avformat_alloc_output_context2" << std::endl;
		return -1;
	}


	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		out_stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream) {
			std::cout << "avformat_new_stream" << std::endl;
			return -1;
		}

		in_stream = ifmt_ctx->streams[i];
		dec_ctx = stream_ctx[i].dec_ctx;

		if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
			|| dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
			/* in this example, we choose transcoding to same codec */
			encoder = avcodec_find_encoder(dec_ctx->codec_id);
			if (!encoder) {
				std::cout << "avcodec_find_encoder" << std::endl;
				return -1;
			}
			enc_ctx = avcodec_alloc_context3(encoder);
			if (!enc_ctx) {
				std::cout << "avcodec_alloc_context3" << std::endl;
				return -1;
			}

			/* In this example, we transcode to same properties (picture size,
			 * sample rate etc.). These properties can be changed for output
			 * streams easily using filters */
			if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
				enc_ctx->height = dec_ctx->height;
				enc_ctx->width = dec_ctx->width;
				enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
				/* take first format from list of supported formats */
				if (encoder->pix_fmts)
					enc_ctx->pix_fmt = encoder->pix_fmts[0];
				else
					enc_ctx->pix_fmt = dec_ctx->pix_fmt;
				/* video time_base can be set to whatever is handy and supported by encoder */
				enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
			}
			else {
				enc_ctx->sample_rate = dec_ctx->sample_rate;
				enc_ctx->channel_layout = dec_ctx->channel_layout;
				enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
				/* take first format from list of supported formats */
				enc_ctx->sample_fmt = encoder->sample_fmts[0];
				enc_ctx->time_base = { 1, enc_ctx->sample_rate };
			}

			if (ofmt_ctx->oformat->flags & 0x0040/*AVFMT_GLOBALHEADER*/)
				enc_ctx->flags |= (1 << 22)/*AV_CODEC_FLAG_GLOBAL_HEADER*/;

			/* Third parameter can be used to pass settings to encoder */
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
	av_dump_format(ofmt_ctx, 0, filename, 1);

	if (!(ofmt_ctx->oformat->flags & /*AVFMT_NOFILE*/0x0001)) {
		ret = avio_open(&ofmt_ctx->pb, filename, /*AVIO_FLAG_WRITE*/2);
		if (ret < 0) {
			error_pro(ret, "avio_open");
			return -1;
		}
	}

	/* init muxer, write output file header */
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		error_pro(ret, "avformat_write_header");
		return -1;
	}

	return 0;
}
