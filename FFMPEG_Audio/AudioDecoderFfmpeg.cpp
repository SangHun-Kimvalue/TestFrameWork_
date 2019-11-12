//
// Copyright 2019 Amy Wang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "AudioDecoderFfmpeg.h"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
}

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace {
	const int OUTPUT_CHANNEL_COUNT = 2;

	const size_t AUDIO_INBUF_SIZE = 20480;
	const size_t AUDIO_REFILL_THRESH = 4096;
}

using namespace FFmpegAudioDecoderCpp;

AudioDecoderFfmpeg::AudioDecoderFfmpeg()
{

}

int AudioDecoderFfmpeg::decode_packet(AVCodecContext *decoder_context, AVPacket *packet, AVFrame *frame, SwrContext* swr,
	std::vector<float> &interleaved_audio_stereo)
{
	// Send the packet with the compressed data to the decoder
	int succeed = avcodec_send_packet(decoder_context, packet);
	if (succeed < 0) {
		std::cout << "Error submitting the packet to the decoder\n";
		return -1;
	}

	// Read all the output frames
	while (succeed >= 0) {
		succeed = avcodec_receive_frame(decoder_context, frame);
		if (succeed == AVERROR(EAGAIN) || succeed == AVERROR_EOF)
			return 0;
		else if (succeed < 0) {
			std::cout << "Decoder error, fail to receive frames." << std::endl;
			break;
		}

		// Resample frames
		float* buffer;
		av_samples_alloc((uint8_t**)&buffer, nullptr, OUTPUT_CHANNEL_COUNT, frame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
		int frame_count = swr_convert(swr, (uint8_t**)&buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
		if (frame_count > 0) {
			// Append resampled frames to data
			std::copy(buffer, buffer + frame_count * OUTPUT_CHANNEL_COUNT, std::back_inserter(interleaved_audio_stereo));
		}
	}

	return -1;
}

AVStream* AudioDecoderFfmpeg::find_audio_stream(const char* file_path)
{
	// Get format from audio file
	AVFormatContext* format = avformat_alloc_context();
	if (avformat_open_input(&format, file_path, nullptr, nullptr) != 0) {
		std::stringstream ss;
		ss << "Could not open file " << file_path;
		std::cout << ss.str() << std::endl;
		return nullptr;
	}
	if (avformat_find_stream_info(format, nullptr) < 0) {
		std::cout << "Could not retrieve stream info from input file." << std::endl;
		return nullptr;
	}
	// Find the index of the first audio stream
	int stream_index = -1;
	for (int i = 0; i < format->nb_streams; i++) {
		if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			stream_index = i;
			break;
		}
	}
	if (stream_index >= 0 && stream_index < static_cast<int>(format->nb_streams)) {
		AVStream* stream = format->streams[stream_index];
		return stream;
	}

	std::cout << "Could not retrieve audio stream from input file." << std::endl;
	return nullptr;
}

// Based on the example decode_audio.c from FFmpeg v4.1 docs
int AudioDecoderFfmpeg::decode_audio_file(const char* file_path, int sample_rate, std::vector<float> &interleaved_audio_stereo)
{
	AVStream * audio_stream = find_audio_stream(file_path);
	if (!audio_stream) {
		std::cout << "Audio stream not found." << std::endl;
		return -1;
	}

	AVCodecParameters* codec_par = audio_stream->codecpar;
	AVCodec * codec = avcodec_find_decoder(codec_par->codec_id);
	if (!codec) {
		std::cout << "Codec not found." << std::endl;
		return -1;
	}

	AVCodecContext *codec_context = avcodec_alloc_context3(codec);
	if (!codec_context) {
		std::cout << "Could not allocate audio codec context." << std::endl;
		return -1;
	}

	if (avcodec_open2(codec_context, codec, NULL) < 0) {
		std::cout << "Could not open codec!" << std::endl;
		return -1;
	}

	FILE *in_file;
	fopen_s(&in_file, file_path, "rb");
	if (!in_file) {
		std::cout << "Could not open audio file!" << std::endl;
		return -1;
	}

	AVCodecParserContext *parser = av_parser_init(codec->id);
	if (!parser) {
		std::cout << "Codec parser not found." << std::endl;
		return -1;
	}

	// Prepare resampler
	SwrContext* swr = swr_alloc();
	av_opt_set_int(swr, "in_channel_count", codec_par->channels, 0);
	av_opt_set_int(swr, "out_channel_count", OUTPUT_CHANNEL_COUNT, 0);
	av_opt_set_int(swr, "in_channel_layout", static_cast<int>(codec_par->channel_layout), 0);
	av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(swr, "in_sample_rate", codec_par->sample_rate, 0);
	av_opt_set_int(swr, "out_sample_rate", sample_rate, 0);
	av_opt_set_sample_fmt(swr, "in_sample_fmt", codec_context->sample_fmt, 0);
	av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
	swr_init(swr);
	if (!swr_is_initialized(swr)) {
		std::cout << "Resampler has not been properly initialized." << std::endl;
		return -1;
	}

	// Decode until eof
	uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	uint8_t *data = inbuf;
	size_t data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, in_file);
	AVPacket *packet = av_packet_alloc();
	AVFrame *decoded_frame = nullptr;
	size_t length_read = 0;
	int in_bytes_used = 0;
	while (data_size > 0) {
		if (!decoded_frame) {
			if (!(decoded_frame = av_frame_alloc())) {
				std::cout << "Could not allocate audio frame" << std::endl;
				break;
			}
		}

		in_bytes_used = av_parser_parse2(parser, codec_context, &packet->data, &packet->size,
			data, static_cast<int>(data_size),
			static_cast<int64_t>(AV_NOPTS_VALUE), static_cast<int64_t>(AV_NOPTS_VALUE), 0);
		if (in_bytes_used < 0) {
			std::cout << "Decoder error while parsing" << std::endl;
			break;
		}
		data += static_cast<unsigned long>(in_bytes_used);
		data_size -= static_cast<unsigned long>(in_bytes_used);

		if (packet->size)
			decode_packet(codec_context, packet, decoded_frame, swr, interleaved_audio_stereo);

		if (data_size < AUDIO_REFILL_THRESH) {
			memmove(inbuf, data, data_size);
			data = inbuf;
			length_read = fread(data + data_size, 1,
				AUDIO_INBUF_SIZE - data_size, in_file);
			if (length_read > 0)
				data_size += length_read;
		}
	}

	// Indicate end of stream by sending an empty packet
	packet->data = NULL;
	packet->size = 0;
	decode_packet(codec_context, packet, decoded_frame, swr, interleaved_audio_stereo);

	fclose(in_file);

	avcodec_free_context(&codec_context);
	av_parser_close(parser);
	av_frame_free(&decoded_frame);
	av_packet_free(&packet);
	swr_free(&swr);

	return 0;
}
