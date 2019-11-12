//
// Copyright 2019 Amy Wang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
// This class provides an example of how to use FFmpeg V4.1 to decode audio
// Some code was taken from the official FFmpeg example doc/examples/decode_audio.c 
// It also resamples the audio so that you can specify a desired sample rate
//

#pragma once

#include <vector>

class AVStream;
class AVCodecContext;
class AVFrame;
class AVPacket;
class SwrContext;

namespace FFmpegAudioDecoderCpp {

	/// A simple wrapper for the FFmpeg audio decoding APIs
	class AudioDecoderFfmpeg
	{

	public:
		AudioDecoderFfmpeg();

		int decode_audio_file(const char* file_path, int sample_rate, std::vector<float> &interleaved_audio_stereo);

	private:
		int decode_packet(AVCodecContext *decoder_context, AVPacket *packet, AVFrame *frame, SwrContext* swr,
			std::vector<float> &interleaved_audio_stereo);
		AVStream* find_audio_stream(const char* file_path);
	};
}
