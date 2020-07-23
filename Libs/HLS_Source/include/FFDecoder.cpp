#include "FFDecoder.h"

const char* error_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;

	return errstr;
}

FFmpegDecoder::FFmpegDecoder()
{
	avformat_network_init();

	pFormatCtx = avformat_alloc_context();


	//AVDictionary *avdic = NULL;
	//char option_key[] = "rtsp_transport";

	//std::string option_value = "";
	//allowed_media_types
		/*	Set media types to accept from the server.

		The following flags are accepted :

	¡®video¡¯
		¡®audio¡¯
		¡®data¡¯
		By default it accepts all media types.*/

	//if (RTPTCPMode)
	//	option_value = "tcp";
	//else
	//	option_value = "udp";
	//
	//av_dict_set(&avdic, option_key, option_value.c_str(), 0);
	//char option_key2[] = "max_delay";
	//char option_value2[] = "100";
	//
	//av_dict_set(&avdic, option_key2, option_value2, 0);
	//
	//if (avformat_open_input(&pFormatCtx, path.c_str(), NULL, &avdic) != 0)
	//{
	//	std::cout << "can't connect the RTSP Server." << std::endl;
	//	bConnected = false;
	//	return;
	//}
	//
	//if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	//{
	//	std::cout << "can't find stream infomation" << std::endl;
	//	bConnected = false;
	//	return;
	//}
}

FFmpegDecoder::~FFmpegDecoder()
{
	//destroy();
}

int FFmpegDecoder::Init(int SI, void *FormatCtx)
{

	m_SI = SI;
	pFormatCtx = (AVFormatContext*)FormatCtx;
	pCodecID = pFormatCtx->streams[m_SI]->codecpar->codec_id;
	AVCodec* pCodec = avcodec_find_decoder(pCodecID);
	if (!pCodec) {
		std::cout << "RTSP_Client not avcodec_find_decoder" << std::endl;
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		std::cout << "RTSP_Client avcodec_alloc_context3" << std::endl;
		return -1;
	}

	int m_error = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[m_SI]->codecpar);
	if (m_error < 0) {
		error_pro(m_error, "RTSP_Client avcodec_parameters_to_context");
		return -1;
	}

	pCodecCtx->framerate = av_guess_frame_rate(pFormatCtx, pFormatCtx->streams[m_SI], NULL);
	pCodecCtx->bit_rate = pFormatCtx->streams[m_SI]->codecpar->bit_rate;
	pCodecCtx->sample_rate = pFormatCtx->streams[m_SI]->codecpar->sample_rate;
	pCodecCtx->time_base = pFormatCtx->streams[m_SI]->time_base;

	m_error = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (m_error < 0) {
		error_pro(m_error, "RTSP_Client dec avcodec_open2");
	
		//bConnected = false;
		return -1;
	}

	pCodecCtx->thread_count = 0;

	F_info.StreamId = m_SI;
	F_info.Width = pCodecCtx->width;
	F_info.Height = pCodecCtx->height;
	F_info.Fps = pCodecCtx->framerate.num;
	F_info.Bitrate = (int)pCodecCtx->bit_rate;
	F_info.Samplerate = pCodecCtx->sample_rate;

	return 0;
}

int FFmpegDecoder::Decode(MediaFrame* MF)
{
	static int count;
	if (count > 50000) {
		count = 0;
	}

	if (FailCount > DECODE_FAIL_COUNT) {
		return DECODE_FAIL_COUNT * -1;
	}

	int ret = 0;

	
	if (MF->Pkt->stream_index == m_SI)
	{
		ret = avcodec_send_packet(pCodecCtx, MF->Pkt);

		if (ret == AVERROR(EAGAIN)) {
			av_packet_unref(MF->Pkt);
			return ret;
		}
		else if (ret < 0) {
			error_pro(ret, "send_packet error");
			FailCount++;
			av_packet_unref(MF->Pkt);
			return -1;
		}
		
		ret = avcodec_receive_frame(pCodecCtx, MF->Frm);
		if (ret == AVERROR(EAGAIN)) {
			av_packet_unref(MF->Pkt);
			return ret;
		}
		else if (ret == AVERROR_EOF) {
			ret = 0;
			std::cout << "AVERROR_EOF - " << count++ << std::endl;
			av_packet_unref(MF->Pkt);
			av_freep(&MF->Frm->data[0]);
			av_frame_unref(MF->Frm);
			av_frame_free(&MF->Frm);
			return -10;
		}
		else if (ret < 0) {
			error_pro(ret, "avcodec_receive_frame error");
			FailCount++;
			av_freep(&MF->Frm->data[0]);
			av_frame_unref(MF->Frm);
			av_frame_free(&MF->Frm);
			return ret;
		}

		std::cout << "Decode Success - " << count++ << std::endl;

		FailCount = 0;
	}

	return 0;
}

int FFmpegDecoder::Release()
{
	avcodec_close(pCodecCtx);
	avcodec_free_context(&pCodecCtx);
	pCodecCtx = nullptr;

	return 0;
}