#include "FFDecoder.h"
//
//  ffmpegdecoder.h
//  ffmpeg RTSP Client
//
//  Created by Yonghye Kwon on 9/14/18.
//  Copyright (c) Freeeeeeeeeeeeeee
//

using namespace std;

const char* error_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;

	return errstr;
}

FFmpegDecoder::FFmpegDecoder(std::string path, bool RTPTCPMode): 
	videoStream(-1), audioStream(-1), bConnected(false), Stopped(true)
{
	avformat_network_init();

	this->path = path;

	pFormatCtx = avformat_alloc_context();

	AVDictionary *avdic = NULL;
	char option_key[] = "rtsp_transport";
	std::string option_value = "";
	//allowed_media_types
		/*	Set media types to accept from the server.

		The following flags are accepted :

	¡®video¡¯
		¡®audio¡¯
		¡®data¡¯
		By default it accepts all media types.*/

	if (RTPTCPMode)
		option_value = "tcp";
	else
		option_value = "udp";

	av_dict_set(&avdic, option_key, option_value.c_str(), 0);
	char option_key2[] = "max_delay";
	char option_value2[] = "100";

	av_dict_set(&avdic, option_key2, option_value2, 0);

	if (avformat_open_input(&pFormatCtx, path.c_str(), NULL, &avdic) != 0)
	{
		std::cout << "can't connect the RTSP Server." << std::endl;
		bConnected = false;
		return;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		std::cout << "can't find stream infomation" << std::endl;
		bConnected = false;
		return;
	}
}

FFmpegDecoder::~FFmpegDecoder()
{
	destroy();
}

int FFmpegDecoder::connect()
{

	for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
		}
	}

	if (videoStream == -1)
	{
		std::cout << "Not Included video stream" << std::endl;
		bConnected = false;
		return -1;
	}

	pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
	if (!dec) {
		std::cout << "RTSP_Client avcodec_find_decoder" << std::endl;
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		std::cout << "RTSP_Client avcodec_alloc_context3" << std::endl;
		return -1;
	}
	//pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	//pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	int m_error = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
	if (m_error < 0) {
		error_pro(m_error, "RTSP_Client avcodec_parameters_to_context");
		return -1;
	}

	pCodecCtx->framerate = av_guess_frame_rate(pFormatCtx, pFormatCtx->streams[videoStream], NULL);
	pCodecCtx->bit_rate = pFormatCtx->streams[videoStream]->codecpar->bit_rate;
	pCodecCtx->sample_rate = pFormatCtx->streams[videoStream]->codecpar->sample_rate;
	pCodecCtx->time_base = pFormatCtx->streams[videoStream]->time_base;

	//pCodecCtx->bit_rate = 0;
	//pCodecCtx->time_base.num = 1;
	//pCodecCtx->time_base.den = 10;
	//pCodecCtx->frame_number = 1;

	m_error = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (m_error < 0) {
		error_pro(m_error, "RTSP_Client dec avcodec_open2");
	
		bConnected = false;
		return -1;
	}

	packet = (AVPacket *)malloc(sizeof(AVPacket));
	av_new_packet(packet, pCodecCtx->width * pCodecCtx->height);

	pCodecCtx->thread_count = 0;
	bConnected = true;

	return 0;
}

int FFmpegDecoder::decode()
{
	std::chrono::milliseconds duration(1);
	int count = 0;
	int ret = 0;
	int failcount = 0;
	Stopped = false;

	while (av_read_frame(pFormatCtx, packet) >= 0 && Stopped == false)
	{
		if (packet->stream_index == videoStream)
		{
			int got_picture = 0;
			ret = avcodec_send_packet(pCodecCtx, packet);

			if (ret == AVERROR(EAGAIN)) {
				av_packet_unref(packet);
				continue;
			}
			else if (ret < 0) {
				error_pro(ret, "send_packet error");
				failcount++;
			}
			AVFrame *pFrame = av_frame_alloc();

			ret = avcodec_receive_frame(pCodecCtx, pFrame);
			if (ret == AVERROR(EAGAIN)) {
				av_packet_unref(packet);
				continue;
			}
			else if (ret == AVERROR_EOF) {
				ret = 0;
				std::cout << "AVERROR_EOF - " << count++ << std::endl;
				return -10;
			}
			else if (ret < 0) {
				error_pro(ret, "avcodec_receive_packet error");
				failcount++;
			}

			std::cout << "Decode Success - " << count++ << std::endl;

			//if (got_picture)
			//{
				//sws_scale(imgConvertCtx,
				//	(uint8_t const * const *)pFrame->data,
				//	pFrame->linesize, 0, pCodecCtx->height, pFrameBGR->data,
				//	pFrameBGR->linesize);
				//
				//mtx.lock();
				//mtx.unlock();
			//}
			failcount = 0;
		}

		av_packet_unref(packet);
		std::this_thread::sleep_for(duration);

		if (failcount >= 60) {
			destroy();
			if (connect() != 0) {
				break;
			}
		}

	}

	destroy();
	return 0;
}

void FFmpegDecoder::destroy()
{

	if (bConnected) {
		Stopped = true;
		//av_free(outBuffer);
		//av_free(pFrameBGR);
		avcodec_close(pCodecCtx);
		avformat_close_input(&pFormatCtx);

		av_packet_unref(packet);
		bConnected = false;
	}
}