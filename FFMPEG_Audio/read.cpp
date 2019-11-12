//#include <dshow.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <ciso646>
#include <limits>
#include <locale>
#include <codecvt>

#include "audiorw.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
};

#define _WIN32_WINNT _WIN32_WINNT_XP
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <stringapiset.h>

#define WC_ERR_INVALID_CHARS 0x00000080

using namespace audiorw;

DWORD convert_unicode_to_utf8_string(__out std::string& utf8, __in const wchar_t* unicode, __in const size_t unicode_size) {
	DWORD error = 0; 
	do {
		if ((nullptr == unicode) || (0 == unicode_size)) { error = ERROR_INVALID_PARAMETER; break; } utf8.clear();
		// // getting required cch. // 
		int required_cch = ::WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, unicode, static_cast<int>(unicode_size), nullptr, 0, nullptr, nullptr ); 
		if (0 == required_cch) 
		{ 
			error = ::GetLastError(); 
			break; 
		} // // allocate. // 
		utf8.resize(required_cch); // // convert. // 
		if (0 == ::WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, unicode, static_cast<int>(unicode_size), 
			const_cast<char*>(utf8.c_str()), static_cast<int>(utf8.size()), nullptr, nullptr )) 
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
	            "Sample format %s not supported as output format\n",
	            av_get_sample_fmt_name(sample_fmt));
	    return AVERROR(EINVAL);
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

std::vector<std::vector<double>> audiorw::read(
	const std::string & filename,
	double & sample_rate,
	double start_seconds,
	double end_seconds) {

	system("chcp 65001"); // 코드페이지를 UTF-8로 변경.
	//printf("%c%c%c \n", 0xEC, 0x9C, 0x84);

		//////////////////////////input_cap//////////////////////////////////////////////

	// Get a buffer for writing errors to
	size_t errbuf_size = 200;
	char errbuf[200];

	avdevice_register_all();

	// Initialize variables
	AVCodecContext * codec_context = NULL;
	AVFormatContext * format_context = NULL;
	SwrContext * resample_context = NULL;
	AVDictionary* options = NULL;
	AVFrame * frame = NULL;
	AVPacket packet = { 0 };
	AVCodec* Codec;
	const char *dst_filename = "Test_A.mp4";
	int dst_bufsize = 0;
	int error;


	av_dict_set(&options, "list_devices", "true", 0);
	format_context = avformat_alloc_context();
	AVInputFormat *iformat = av_find_input_format("dshow");
	if (iformat == NULL) {
		printf("iformat is NULL\n");
	}

	//error = avformat_open_input(&format_context, "video=dummy:audio=마이크 배열(Realtek High Definition Audio)", iformat, &options);
	//av_strerror(error, errbuf, sizeof(errbuf));
	//std::cout << "avformat_open_input Error : " << errbuf << std::endl;

	std::string conv;
	std::wstring wchar = L"audio=마이크 배열(Realtek High Definition Audio)";
	int size = wchar.size();
	convert_unicode_to_utf8_string(conv, wchar.c_str(), size);
	//std::string filename = "audio=마이크 배열(Realtek High Definition Audio)";

	error = avformat_open_input(&format_context, conv.c_str(), iformat, NULL);
	av_strerror(error, errbuf, sizeof(errbuf));
	std::cout << conv.c_str() << std::endl;
	std::cout << "avformat_open_input Error : " << errbuf << std::endl;

	if (error != 0) {
		//av_strerror(error, errbuf, sizeof(errbuf));
		//std::cout << errbuf << std::endl;
		printf("Couldn't open input stream.\n");
		//return -1;
	}

	// Open the file and get format information
	// error = avformat_open_input(&format_context, filename.c_str(), iformat, 0);
	//if (error != 0) {
	//	av_strerror(error, errbuf, 200);
	//	std::cout << errbuf << std::endl;
	//	throw std::invalid_argument(
	//		"Could not open audio file: " + filename + "\n" +
	//		"Error: " + std::string(errbuf));
	//}

	// Get stream info
	if ((error = avformat_find_stream_info(format_context, NULL)) < 0) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		av_strerror(error, errbuf, errbuf_size);
		throw std::runtime_error(
			"Could not get information about the stream in file: " + filename + "\n" +
			"Error: " + std::string(errbuf));
	}

	// Find an audio stream and its decoder
	AVCodec * codec = NULL;
	int audio_stream_index = av_find_best_stream(
		format_context,
		AVMEDIA_TYPE_AUDIO,
		-1, -1, &codec, 0);
	if (audio_stream_index < 0) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		throw std::runtime_error(
			"Could not determine the best stream to use in the file: " + filename);
	}

	// Allocate context for decoding the codec
	codec_context = avcodec_alloc_context3(codec);
	if (!codec_context) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		throw std::runtime_error(
			"Could not allocate a decoding context for file: " + filename);
	}

	// Fill the codecContext with parameters of the codec
	//format_context->duration = (double)10000000;

	if ((error = avcodec_parameters_to_context(
		codec_context,
		format_context->streams[audio_stream_index]->codecpar
	)) != 0) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		throw std::runtime_error(
			"Could not set codec context parameters for file: " + filename);
	}
	codec_context->thread_count = 0;

	// Initialize the decoder
	if ((error = avcodec_open2(codec_context, codec, NULL)) != 0) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		av_strerror(error, errbuf, errbuf_size);
		throw std::runtime_error(
			"Could not initialize the decoder for file: " + filename + "\n" +
			"Error: " + std::string(errbuf));
	}

	// Make sure there is a channel layout
	if (codec_context->channel_layout == 0) {
		codec_context->channel_layout =
			av_get_default_channel_layout(codec_context->channels);
	}

	// Fetch the sample rate
	sample_rate = codec_context->sample_rate;
	if (sample_rate <= 0) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		throw std::runtime_error(
			"Sample rate is " + std::to_string(sample_rate));
	}
	
	// Initialize a resampler
	resample_context = swr_alloc_set_opts(
		NULL,
		// Output
		codec_context->channel_layout,
		AV_SAMPLE_FMT_DBL,
		//AV_SAMPLE_FMT_U8, 
		sample_rate,
		// Input
		codec_context->channel_layout,
		codec_context->sample_fmt,
		sample_rate,
		0, NULL);
	if (!resample_context) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		throw std::runtime_error(
			"Could not allocate resample context for file: " + filename);
	}

	// Open the resampler context with the specified parameters
	if ((error = swr_init(resample_context)) < 0) {
		cleanup(codec_context, format_context, resample_context, frame, packet);
		throw std::runtime_error(
			"Could not open resample context for file: " + filename);
	}
	double t = 0;

	// prepare a packet
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	format_context->duration = AV_TIME_BASE * 10;
	// Get start and end values in samples
	start_seconds = std::max(start_seconds, 0.);
	double duration = (format_context->duration) / (double)1000000;
	//double duration =(double)10;
	if (end_seconds < 0) {
		end_seconds = duration;
	}
	//else {
	//	end_seconds = std::min(end_seconds, duration);
	//}
	double start_sample = std::floor(start_seconds * sample_rate);
	double end_sample = std::floor(end_seconds   * sample_rate);
	
	// Allocate the output vector
	std::vector<std::vector<double>> audio(codec_context->channels);
	FILE *outfile;
	fopen_s(&outfile, dst_filename, "wb");
	if (!outfile) {
		av_free(format_context);
		exit(1);
	}

	////////////////////////////////////////////////////////////////////////////////

	// Read the file until either nothing is left
	// or we reach desired end of sample
	int sample = 0;
	while (sample < end_sample) {
		// Read from the frame
		error = av_read_frame(format_context, &packet);
		if (error == -1) {
			break;
		}
		else if (error < 0) {
			//cleanup(codec_context, format_context, resample_context, frame, packet);
			av_strerror(error, errbuf, errbuf_size);
			throw std::runtime_error(
				"Error reading from file: " + filename + "\n" +
				"Error: " + std::string(errbuf));
		}

		// Is this the correct stream?
		if (packet.stream_index != audio_stream_index) {
			// Otherwise move on
			continue;
		}

		int data_size = 0;

		// Send the packet to the decoder
		if ((error = avcodec_send_packet(codec_context, &packet)) < 0) {
			if (error == AVERROR(EAGAIN) || error == AVERROR_EOF)
				continue;
			else if (error < 0) {
				fprintf(stderr, "Error during decoding\n");
				av_strerror(error, errbuf, errbuf_size);
				std::cout << "avcodec_send_packet error : " << errbuf << std::endl;
				throw std::runtime_error(
					"Could not send packet to decoder for file: " + filename + "\n" +
					"Error: " + std::string(errbuf));
				exit(1);
			}
			else if (error == -11)
				continue;
			//cleanup(codec_context, format_context, resample_context, frame, packet);
		}
		//int src_nb_samples = 1024, dst_nb_samples, max_dst_nb_samples;
		//max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples, codec_context->sample_rate, codec_context->sample_rate, AV_ROUND_UP);
	 	//
	    // /* buffer is going to be directly written to a rawaudio file, no alignment */
		//int dst_linesize = 0;
	    // int dst_nb_channels = av_get_channel_layout_nb_channels(codec_context->channel_layout);
	    // error = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels,
	    //                                          dst_nb_samples, AV_SAMPLE_FMT_DBL, 0);
	    // if (error < 0) {
	    //     fprintf(stderr, "Could not allocate destination samples\n");
		//	 av_strerror(error, errbuf, errbuf_size);
	    // }

		// Receive a decoded frame from the decoder
		frame = av_frame_alloc();
		
		if ((error = avcodec_receive_frame(codec_context, frame)) == 0) {
		//while ((error = avcodec_receive_frame(codec_context, frame)) == 0) {
			
			data_size = av_get_bytes_per_sample(codec_context->sample_fmt);
			if (data_size < 0) {
				/* This should not occur, checking just for paranoia */
				fprintf(stderr, "Failed to calculate data size\n");
				exit(1);
			}
			for (int i = 0; i < frame->nb_samples; i++)
				for (int ch = 0; ch < codec_context->channels; ch++)
					fwrite(frame->data[ch] + data_size * i, 1, data_size, outfile);

			//do {
		    //    /* generate synthetic audio */
		    //    fill_samples((double *)frame->data[0], src_nb_samples, codec_context->channel_layout, codec_context->sample_rate, &t);
			//
		    //    /* compute destination number of samples */
		    //    dst_nb_samples = av_rescale_rnd(swr_get_delay(resample_context, codec_context->sample_rate) +
		    //                                    src_nb_samples, codec_context->sample_rate, codec_context->sample_rate, AV_ROUND_UP);
		    //    if (dst_nb_samples > max_dst_nb_samples) {
		    //        av_freep(&dst_data[0]);
		    //        error = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
		    //                               dst_nb_samples, AV_SAMPLE_FMT_U8, 1);
		    //        if (error < 0)
		    //            break;
		    //        max_dst_nb_samples = dst_nb_samples;
		    //    }
			//
		    //    /* convert to destination format */
		    //    error = swr_convert(resample_context, dst_data, dst_nb_samples, (const uint8_t **)frame->data, src_nb_samples);
		    //    if (error < 0) {
		    //        fprintf(stderr, "Error while converting\n");
			//		av_strerror(error, errbuf, errbuf_size);
		    //        //goto end;
		    //    }
		    //    dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, error, AV_SAMPLE_FMT_U8, 1);
		    //    if (dst_bufsize < 0) {
		    //        fprintf(stderr, "Could not get sample buffer size\n");
			//		//av_strerror(error, errbuf, errbuf_size);
		    //       // goto end;
		    //    }
		    //    printf("t:%f in:%d out:%d\n", t, src_nb_samples, error);
		    //    fwrite(dst_data[0], 1, dst_bufsize, dst_file);
		    //} while (t < 10);
			//const char *fmt;
			//if ((error = get_format_from_sample_fmt(&fmt, AV_SAMPLE_FMT_U8)) < 0) {
			//	av_strerror(error, errbuf, errbuf_size);
			//}
			//
			//// Increment the stamp
			//sample += frame->nb_samples;

			sample += frame->nb_samples;
		}
		if (error == AVERROR(EAGAIN) || error == AVERROR_EOF)
			continue;
		else if (error == -11) {

			av_frame_free(&frame);
			continue;
		}
		// Check if the decoder had any errors
		else if (error != 0 && error != -11) {
			av_strerror(error, errbuf, sizeof(errbuf));
			std::cout << errbuf << std::endl;
			cleanup(codec_context, format_context, resample_context, frame, packet);
			av_strerror(error, errbuf, errbuf_size);
			throw std::runtime_error(
				"Error receiving packet from decoder for file: " + filename + "\n" +
				"Error: " + std::string(errbuf));
		}
		//data_size = av_get_bytes_per_sample(codec_context->sample_fmt);
		//if (data_size < 0) {
		//	/* This should not occur, checking just for paranoia */
		//	fprintf(stderr, "Failed to calculate data size\n");
		//	exit(1);
		//}
		

	}
	// Cleanup
	cleanup(codec_context, format_context, resample_context, frame, packet);
	fclose(outfile);

	return audio;
	
}

void audiorw::cleanup(
	AVCodecContext * codec_context,
	AVFormatContext * format_context,
	SwrContext * resample_context,
	AVFrame * frame,
	AVPacket packet) {
	// Properly free any allocated space
	avcodec_close(codec_context);
	avcodec_free_context(&codec_context);
	avio_closep(&format_context->pb);
	avformat_free_context(format_context);
	swr_free(&resample_context);
	av_frame_free(&frame);
	av_packet_unref(&packet);
}


