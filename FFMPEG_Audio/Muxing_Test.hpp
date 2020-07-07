#include <Windows.h>
#include <iostream>
#include "audiorw.hpp"

#define WC_ERR_INVALID_CHARS 0x00000080
class capture {

public:
	capture() {}
	~capture() {}

	AVCodecContext * codec_context = NULL;
	AVFormatContext * iformat_context = NULL;
	AVOutputFormat* ofmt;
	AVInputFormat *iformat;
	AVFormatContext * oformat_context = NULL;
	SwrContext * resample_context = NULL;
	AVFrame * frame = NULL;
	AVPacket packet = { 0 };

	AVDictionary* options = NULL;

	size_t errbuf_size = 200;
	char errbuf[200];

	int error = 0;
	int audioStream, videoStream;
	bool done = false;


	DWORD convert_unicode_to_utf8_string(__out std::string& utf8, __in const wchar_t* unicode, __in const size_t unicode_size) {
		DWORD error = 0; do {
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

	void grabFrames() {
		AVPacket pkt;
		int ret;
		while (av_read_frame(iformat_context, &pkt) >= 0) {
			AVStream *in_stream, *out_stream;
			in_stream = iformat_context->streams[pkt.stream_index];
			out_stream = oformat_context->streams[pkt.stream_index];
			/* copy packet */
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;
			int ret = av_interleaved_write_frame(oformat_context, &pkt);
			if (ret < 0) {
				//qDebug() << "Error muxing packet";
				//break;
			}
			av_packet_unref(&pkt);

			if (done) break;
		}
		av_write_trailer(oformat_context);

		avformat_close_input(&iformat_context);
		/* close output */
		if (oformat_context && !(ofmt->flags & AVFMT_NOFILE))
			avio_close(oformat_context->pb);
		avformat_free_context(oformat_context);
		if (ret < 0 && ret != AVERROR_EOF) {
			//return -1;
		   //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
		}
		return;
	}

	void init() {
		//AVInputFormat *fmt = av_find_input_format("dshow");
		//avdevice_register_all();
		av_dict_set(&options, "list_devices", "true", 0);
		iformat_context = avformat_alloc_context();
		iformat = av_find_input_format("dshow");
		if (iformat == NULL) {
			printf("iformat is NULL\n");
		}

	}

	int decode() {
		avdevice_register_all();
		AVInputFormat* iformat = NULL;
		AVFormatContext* iformat_context = NULL;

		iformat_context = avformat_alloc_context();
		iformat = av_find_input_format("dshow");
		if (iformat == NULL) {
			//CCommonInfo::GetInstance()->WriteLog(L"ERROR", L"Audio av_find_input_format(dshow)\n");
			//CCommonInfo::GetInstance()->KSNCOutputDebugString(L"Audio av_find_input_format(dshow)\n");
			return -1;
		}
		std::string conv;
		std::wstring wchar = L"audio=스테레오 믹스(Realtek High Definition Audio)";
		int size = wchar.size();
		convert_unicode_to_utf8_string(conv, wchar.c_str(), size);
		//std::string filename = "audio=마이크 배열(Realtek High Definition Audio)";

		//error = avformat_open_input(&iformat_context, conv.c_str(), iformat, NULL);
		error = avformat_open_input(&iformat_context, conv.c_str(), iformat, NULL);
		if (error < 0) {
			//av_strerror(error, errbuf, sizeof(errbuf));
			//std::cout << errbuf << std::endl;
			printf("Couldn't open input stream.\n");
			//return -1;
			av_strerror(error, errbuf, sizeof(errbuf));
			std::cout << conv.c_str() << std::endl;
			std::cout << "avformat_open_input Error : " << errbuf << std::endl;
		}


		int ret, i;
		const char* device_name = conv.c_str();
		//
		//if (avformat_open_input(&iformat_context, device_name, fmt, NULL) < 0) {
		//	fprintf(stderr, "Could not open input file '%s'", device_name);
		//	return -1;
		//}
		if ((ret = avformat_find_stream_info(iformat_context, 0)) < 0) {
			fprintf(stderr, "Failed to retrieve input stream information");
			return -1;
		}
		av_dump_format(iformat_context, 0, device_name, 0);
		avformat_alloc_output_context2(&oformat_context, NULL, NULL, "test.avi");
		//avformat_alloc_output_context2(&oformat_context, NULL, NULL, "test.avi");
		if (!oformat_context) {
			fprintf(stderr, "Could not create output context\n");
			ret = AVERROR_UNKNOWN;
			return -1;
		}

		ofmt = oformat_context->oformat;
		AVCodec *dec = avcodec_find_decoder(iformat_context->streams[0]->codecpar->codec_id);
		codec_context = avcodec_alloc_context3(dec);

		for (i = 0; i < iformat_context->nb_streams; i++) {
			AVStream *in_stream = iformat_context->streams[i];
			//avcodec_alloc_context3(in_stream->codec->codec);
			//AVStream *out_stream = avformat_new_stream(oformat_context, in_stream->codec->codec);
			AVStream *out_stream = avformat_new_stream(oformat_context, iformat_context->audio_codec);

			if (iformat_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				videoStream = i;
			}
			else if (iformat_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
				audioStream = i;
			}

			if (!out_stream) {
				fprintf(stderr, "Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				return -1;
			}
			ret = avcodec_parameters_from_context(in_stream->codecpar, codec_context);
			if (ret < 0) {
				fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
				return -1;
			}
			out_stream->codecpar->codec_tag = 0;
			if (oformat_context->oformat->flags & AVFMT_GLOBALHEADER)
				oformat_context->oformat->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}
		av_dump_format(oformat_context, 0, "test.avi", 1);
		if (!(ofmt->flags & AVFMT_NOFILE)) {
			ret = avio_open(&oformat_context->pb, "test.avi", AVIO_FLAG_WRITE);
			if (ret < 0) {
				fprintf(stderr, "Could not open output file '%s'", "test.avi");
				return -1;
			}
		}
		ret = avformat_write_header(oformat_context, NULL);
		if (ret < 0) {
			av_strerror(ret, errbuf, sizeof(errbuf));
			std::cout << errbuf << std::endl;
			std::cout << "avformat_open_input Error : " << errbuf << std::endl;
			fprintf(stderr, "Error occurred when opening output file\n");
			return -1;
		}
		//grabFrames;
		AVPacket pkt;
		//int ret;
		while (av_read_frame(iformat_context, &pkt) >= 0) {
			AVStream *in_stream, *out_stream;
			in_stream = iformat_context->streams[pkt.stream_index];
			out_stream = oformat_context->streams[pkt.stream_index];
			/* copy packet */
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;
			int ret = av_interleaved_write_frame(oformat_context, &pkt);
			if (ret < 0) {
				//qDebug() << "Error muxing packet";
				//break;
			}
			av_packet_unref(&pkt);

			if (done) break;
		}
		av_write_trailer(oformat_context);

		avformat_close_input(&iformat_context);
		/* close output */
		if (oformat_context && !(ofmt->flags & AVFMT_NOFILE))
			avio_close(oformat_context->pb);
		avformat_free_context(oformat_context);
		if (ret < 0 && ret != AVERROR_EOF) {
			return -1;
		   //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
		}
	}
};