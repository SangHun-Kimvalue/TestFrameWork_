#pragma once

#include "IClient.h"
#include "../Libs/HLS_Source/include/Utility.hpp"
#include "DecoderFactory.hpp"
#include <thread>

class FFMPEG_Client : public IClient {

public:

	FFMPEG_Client(CLI info = {}, int qsize = 100);
	~FFMPEG_Client();

	int InitClient(CLI info);
	int Connect();
	int Play();
	void PrintClientInfo();
	void StopClient();
	int ReleaseClient();
	int IncreaseRef();
	int DecreaseRef();

	const std::string GetSDP() const;
	const CLI GetCLI() const;

private:

	static int DoWork(FFMPEG_Client*);
	int Decode(MediaFrame*, FFmpegDecoder* Dec, int StreamID);

	int SetFFOption();
	void SetCLI(AVCodecID VCo, AVCodecID ACo);
	std::string MakeSDP();

	int DecoderSet(FFmpegDecoder* Dec, DT DecoderType, int StreamID);
	int DecoderReset(FFmpegDecoder* Dec, DT DecoderType, int StreamID);

private:

	CLI m_info;
	const int Qsize;

	std::string Path;
	std::thread Workingthr;
	std::thread Keepalivethr;

	DT m_DT;
	FFmpegDecoder* VDec;
	FFmpegDecoder* ADec;
	AVFormatContext *pFormatCtx;
	FI F_Info;
	MediaFrame* MF;
	//AVCodecContext *pCodecCtx;
	//AVCodec *pCodec;
	//AVPacket *packet;

	bool Stopped;
	bool Use_Transcoding;
	bool IncludedAudio;

	TT Transport;
	int VSI;
	int ASI;

	std::string SDPString;
	std::string SPSPPS;
	std::string SDPconfig;
	std::string SDPprofile;

	int m_StreamFPS;
};

//#include "Stopwatch.hpp"
//#include "CommonInfo.h"
//#include "ExtraDataTools.h"
//
//#include <regex>
//#include "ClientManager.h"
//
//class HLS_RTSP_Client
//{
//};
//
//
//class RTSPClient : public HLS_Source
//{
//public:
//	RTSPClient(ClientInfo info) :
//		m_pFmtCtx(NULL)
//	{
//		m_strBindingAddress = info.bindAddr;
//		m_strUrl = info.url;
//		m_wstrUrl.assign(m_strUrl.begin(), m_strUrl.end());
//
//		m_VideoStreamInx = -1;
//		m_VideoStreamCodec = AV_CODEC_ID_NONE;
//
//		m_AudioStreamInx = -1;
//		m_AudioStreamCodec = AV_CODEC_ID_NONE;
//
//		m_extraInfo.videoExtraDataSize = 0;
//		m_extraInfo.audioExtraDataSize = 0;
//		m_extraInfo.extExtraDataSize = 0;
//		CCommonInfo::GetInstance()->KSNCOutputDebugString(L"RTSPClient Initialize - %s\n", m_wstrUrl.c_str());
//		m_ConnectionMode = info.ConnectionMode;
//	}
//
//	~RTSPClient()
//	{
//		CCommonInfo::GetInstance()->KSNCOutputDebugString(L"RTSPClient Destoryed - %s\n", m_wstrUrl.c_str());
//
//		m_strBindingAddress.clear();
//		m_strUrl.clear();
//		m_wstrUrl.clear();
//	};
//
//	int Start()
//	{
//		m_bMustClose = false;
//
//		if (m_pFmtCtx != NULL)
//			return 0;
//
//		if (OpenStream(m_strUrl) < 0)
//		{
//			m_DoneStart = false;
//			return -1;
//		}
//
//		if (m_pFmtCtx == NULL)
//		{
//			m_DoneStart = false;
//			return -1;
//		}
//
//		//초기 연결완료 확인
//		m_DoneStart = true;
//		return 0;
//	}
//
//	int Stop()
//	{
//		//OpenStream이 완료 되기 전까지 기다린다.
//		while (!m_DoneStart)
//		{
//			Sleep(1);
//		}
//
//		if (pkt.data) {
//			av_packet_unref(&pkt);
//		}
//		if (pktFiltered.data) {
//			av_packet_unref(&pktFiltered);
//		}
//
//		m_bMustClose = true;
//		m_bCanRead = false;
//
//		m_VideoStreamInx = -1;
//		m_VideoStreamCodec = AV_CODEC_ID_NONE;
//
//		m_AudioStreamInx = -1;
//		m_AudioStreamCodec = AV_CODEC_ID_NONE;
//
//		if (m_pFmtCtx)
//		{
//			try {
//				avformat_close_input(&m_pFmtCtx);
//			}
//			catch (...) {
//				CCommonInfo::GetInstance()->WriteLog(L"ERROR", L"Exeption when avformat_close_input\n");
//				CCommonInfo::GetInstance()->KSNCOutputDebugString(L"Exeption when avformat_close_input\n");
//				m_pFmtCtx = NULL;
//			}
//			//avformat_close_input(&m_pFmtCtx);
//			m_pFmtCtx = NULL;
//		}
//
//		return 0;
//	}
//
//	void ChangeURl(std::string url)
//	{
//		CCommonInfo::GetInstance()->KSNCOutputDebugString(L"ChangeURL %s to %s in RtspClient", m_wstrUrl.c_str(), StringTools::utf8_to_utf16(url).c_str());
//		CCommonInfo::GetInstance()->WriteLog(L"INFO", L"ChangeURL %s to %s in RtspClient", m_wstrUrl.c_str(), StringTools::utf8_to_utf16(url).c_str());
//		m_strUrl = url;
//	}
//
//	int Read(AVPacket* avpkt)
//	{
//		if (m_bCanRead == false) return -1;
//
//		if (av_read_frame(m_pFmtCtx, avpkt) < 0)
//		{
//			if (av_seek_frame(m_pFmtCtx, -1, 0, AVSEEK_FLAG_FRAME) < 0)
//			{
//				return -1;
//			}
//		}
//
//		if (avpkt->size <= 0)
//		{
//			av_packet_unref(avpkt);
//			return -1;
//		}
//
//		m_swOpenCheck.mark();
//
//		/*int e = 0;
//		while ((e = av_read_frame(m_pFmtCtx, avpkt)) >= 0 && avpkt->stream_index != m_VideoStreamInx)
//		{
//			av_packet_unref(avpkt);
//		}
//		if (e < 0) {
//			return -1;
//		}
//
//		m_swOpenCheck.mark();*/
//
//		return 0;
//	}
//
//	bool Read(uint8_t **ppVideo, int *pnVideoBytes)
//	{
//		*pnVideoBytes = 0;
//
//		if (pkt.data) {
//			av_packet_unref(&pkt);
//		}
//
//		int e = 0;
//		while ((e = av_read_frame(m_pFmtCtx, &pkt)) >= 0 && pkt.stream_index != m_VideoStreamInx) {
//			av_packet_unref(&pkt);
//		}
//		if (e < 0) {
//			return false;
//		}
//
//		if (bMp4H264) {
//			if (pktFiltered.data) {
//				av_packet_unref(&pktFiltered);
//			}
//			av_bsf_send_packet(bsfc, &pkt);
//			av_bsf_receive_packet(bsfc, &pktFiltered);
//			*ppVideo = pktFiltered.data;
//			*pnVideoBytes = pktFiltered.size;
//		}
//		else
//		{
//			*ppVideo = pkt.data;
//			*pnVideoBytes = pkt.size;
//		}
//
//		return true;
//	}
//
//	int Read(AVMediaFrame* avframe)
//	{
//		if (m_bCanRead == false) return -1;
//
//		AVPacket avPacket;
//		av_init_packet(&avPacket);
//
//		avPacket.data = NULL;
//		avPacket.size = 0;
//
//		if (av_read_frame(m_pFmtCtx, &avPacket) < 0)
//		{
//			if (av_seek_frame(m_pFmtCtx, -1, 0, AVSEEK_FLAG_FRAME) < 0)
//			{
//				CCommonInfo::GetInstance()->KSNCOutputDebugString(L"RTSPClient Read Fail, av_read_frame Fail");
//				return -1;
//			}
//		}
//
//		/*int e = 0;
//		while ((e = av_read_frame(m_pFmtCtx, &avPacket)) >= 0 && avPacket.stream_index != m_streamIndex) {
//			av_packet_unref(&avPacket);
//		}
//		if (e < 0) {
//			return -1;
//		}*/
//
//		if (avPacket.size <= 0)
//		{
//			av_packet_unref(&avPacket);
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(L"RTSPClient Read Fail, avPacket.size is 0");
//			return -1;
//		}
//
//		if (!CopyToMediaFrame(&avPacket, avframe))
//		{
//			av_packet_unref(&avPacket);
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(L"RTSPClient Read Fail, CopyToMediaFrame Fail");
//			return -1;
//		}
//
//		av_packet_unref(&avPacket);
//
//		// 읽기 검사 시작 업데이트
//		m_swOpenCheck.mark();
//
//		return 0;
//	}
//
//	AVMediaFrame* ReadFrame()
//	{
//		return nullptr;
//	}
//
//	MediaFrame::MediaFrame* Read()
//	{
//		return nullptr;
//	}
//
//	int GetSleepTime()
//	{
//		return 0;
//	}
//
//	int	GetVideoWidth()
//	{
//		if (m_pFmtCtx == NULL || m_VideoStreamInx == -1)
//			return -1;
//
//		return m_StreamWidth;// m_pFmtCtx->streams[m_streamIndex]->codecpar->width;
//	}
//	int	GetVideoHeight()
//	{
//		if (m_pFmtCtx == NULL || m_VideoStreamInx == -1)
//			return -1;
//
//		return m_StreamHeight;// m_pFmtCtx->streams[m_streamIndex]->codecpar->height;
//	}
//	double GetTargetFPS()
//	{
//		if (m_pFmtCtx == NULL || m_VideoStreamInx == -1)
//			return -1;
//
//		return m_StreamFPS;
//	}
//
//private:
//
//	bool CopyToMediaFrame(AVPacket* avPacket, AVMediaFrame* mediaFrame)
//	{
//		if (m_VideoStreamCodec == AV_CODEC_ID_H264)
//		{
//			//int			payloadLength = 0;
//			//unsigned char	*data = avPacket->data;
//
//			//memcpy(&payloadLength, data, sizeof(payloadLength));
//			//payloadLength = ntohl(payloadLength);
//
//			///* SPS */
//			//if ((data[4] & 0x1F) == 7) {
//			//	H264Tools::Set4ByteHeader((char *)data);
//			//	data = data + payloadLength + 4;
//			//	memcpy(&payloadLength, data, sizeof(payloadLength));
//			//	payloadLength = ntohl(payloadLength);
//			//}
//
//			///* PPS */
//			//if ((data[4] & 0x1F) == 8) {
//			//	H264Tools::Set4ByteHeader((char *)data);
//			//	data = data + payloadLength + 4;
//			//	memcpy(&payloadLength, data, sizeof(payloadLength));
//			//	payloadLength = ntohl(payloadLength);
//			//}
//
//			///* IFrame or PFrame */
//			//if (((data[4] & 0x1F) == 5) || ((data[4] & 0x1F) == 1)) {
//			//	H264Tools::Set4ByteHeader((char *)data);
//			//}
//
//			if ((H264Tools::GetKeyChar((char*)avPacket->data, avPacket->size) & 0x1F) != 6)
//			{
//				mediaFrame->data[0] = (char*)malloc(avPacket->size);
//				memcpy(mediaFrame->data[0], avPacket->data, avPacket->size);
//				mediaFrame->dataSize[0] = avPacket->size;
//				mediaFrame->mediaFormatType = H264FRAME;
//				mediaFrame->trackID = avPacket->stream_index;
//				mediaFrame->timeStamp = (unsigned int)avPacket->pts;// (unsigned int)ComputeTools::GetTimeStampNow();// avPacket->pts;
//
//				//CCommonInfo::GetInstance()->KSNCOutputDebugString(L"PTS : %d,	DTS : %d,	Duration : %d\n", avPacket->pts, avPacket->dts, avPacket->duration);
//				return true;
//			}
//			else
//				return false;
//		}
//		else if (m_VideoStreamCodec == AV_CODEC_ID_HEVC)
//		{
//			//if ((H265Tools::GetKeyChar((char*)avPacket->data, avPacket->size) & 0x1F) != 6)
//			//{
//			mediaFrame->data[0] = (char*)malloc(avPacket->size);
//			memcpy(mediaFrame->data[0], avPacket->data, avPacket->size);
//			mediaFrame->dataSize[0] = avPacket->size;
//			mediaFrame->mediaFormatType = H265FRAME;
//			mediaFrame->trackID = avPacket->stream_index;
//			mediaFrame->timeStamp = (unsigned int)avPacket->pts;
//
//			return true;
//			/*}
//			else
//				return false;*/
//		}
//		else if (m_VideoStreamCodec == AV_CODEC_ID_MPEG4)
//		{
//			//if ((H265Tools::GetKeyChar((char*)avPacket->data, avPacket->size) & 0x1F) != 6)
//			{
//				mediaFrame->data[0] = (char*)malloc(avPacket->size);
//				memcpy(mediaFrame->data[0], avPacket->data, avPacket->size);
//				mediaFrame->dataSize[0] = avPacket->size;
//				mediaFrame->mediaFormatType = MPEG4FRAME;
//				mediaFrame->trackID = avPacket->stream_index;
//				mediaFrame->timeStamp = (unsigned int)avPacket->pts;
//
//				return true;
//			}
//			/*else
//			return false;*/
//		}
//		else
//			return false;
//	}
//
//	// FFMPEG에서 스트림 Open시 주기적으로 호출하는 콜백
//	static int OpenStreamInterruptCallback(void *p)
//	{
//		auto fc = reinterpret_cast<RTSPClient*>(p);
//
//		// 스트림을 닫아야하면 읽기 중지
//		if (fc->m_bMustClose == true)
//		{
//			//return AVERROR_EXIT;
//			return 1;
//		}
//
//		// 스트림을 1초 안에 못잦으면 읽기 중지
//		if (fc->m_swOpenCheck.past_seconds() > 10.0)
//		{
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(
//				L"RTSPClient OpenStreamInterruptCallback timout - %s\n", fc->m_wstrUrl.c_str());
//			CCommonInfo::GetInstance()->WriteLog(L"ERROR",
//				L"RTSPClient OpenStreamInterruptCallback timout - %s\n", fc->m_wstrUrl.c_str());
//
//			//return AVERROR_EXIT;
//			return 1;
//		}
//
//		return 0;
//	}
//
//	int OpenStream(std::string path)
//	{
//		auto timecheck = Stopwatch::mark_now();
//		int ret = 0;
//
//		// 스트림 읽기 콜백 (잘 읽는지 확인하기 위해)
//		//m_pFmtCtx = avformat_alloc_context();
//		//rtsp에서는 interrupt_callback 으로 Timeout을 체크하지 않는다.
//		//m_pFmtCtx->interrupt_callback.callback = OpenStreamInterruptCallback;
//		//m_pFmtCtx->interrupt_callback.opaque = this;// &m_pFmtCtx;
//
//
//		// FFMPEG에서 스트림 Open시 옵션들 설정하는 속성설정
//		AVDictionary *options = NULL;
//		ret = av_dict_set(&options, "reorder_queue_size", "30", 0);
//		if (ret < 0)
//		{
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(
//				L"RTSPClient av_dict_set (reorder_queue_size) error %d - %s\n", ret, m_wstrUrl.c_str());
//			return ret;
//		}
//		ret = av_dict_set(&options, "stimeout", "10000000", 0);
//		if (ret < 0)
//		{
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(
//				L"RTSPClient av_dict_set (stimeout) error %d - %s\n", ret, m_wstrUrl.c_str());
//			return ret;
//		}
//
//		ret = av_dict_set(&options, "buffer_size", "64 * 1024 * 10", 0);
//		if (ret < 0)
//		{
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(
//				L"RTSPClient av_dict_set (stimeout) error %d - %s\n", ret, m_wstrUrl.c_str());
//			return ret;
//		}
//
//		// 접속 모드가 TCP인지 UdP 인지를 확인한다.
//		if (m_ConnectionMode)
//		{
//			ret = av_dict_set(&options, "rtsp_transport", "tcp", 0);
//			if (ret < 0)
//			{
//				CCommonInfo::GetInstance()->KSNCOutputDebugString(
//					L"RTSPClient av_dict_set (stimeout) error %d - %s\n", ret, m_wstrUrl.c_str());
//				return ret;
//			}
//		}
//
//		// 읽기 검사 시작 업데이트
//		m_swOpenCheck.mark();
//
//		// Open media file.
//		ret = avformat_open_input(&m_pFmtCtx, path.c_str(), NULL, &options);
//		av_dict_free(&options);
//
//		if (ret < 0)
//		{
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(
//				L"RTSPClient avformat_open_input error %d - %s\n", ret, m_wstrUrl.c_str());
//			CCommonInfo::GetInstance()->WriteLog(L"ERROR",
//				L"RTSPClient avformat_open_input error %d - %s\n", ret, m_wstrUrl.c_str());
//
//			avformat_close_input(&m_pFmtCtx);
//			m_pFmtCtx = NULL;
//			return ret;
//		}
//
//		//*
//		ret = avformat_find_stream_info(m_pFmtCtx, NULL);
//
//		if (ret < 0)
//		{
//			CCommonInfo::GetInstance()->KSNCOutputDebugString(
//				L"RTSPClient avformat_find_stream_info error %d - %s\n", ret, m_wstrUrl.c_str());
//			CCommonInfo::GetInstance()->WriteLog(L"ERROR",
//				L"RTSPClient avformat_find_stream_info error %d - %s\n", ret, m_wstrUrl.c_str());
//
//			avformat_close_input(&m_pFmtCtx);
//			m_pFmtCtx = NULL;
//			return ret;
//		}
//		//*/
//
//		av_dump_format(m_pFmtCtx, 0, path.c_str(), 0);
//		m_VideoStreamInx = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
//
//		int fps = 0;
//		//for (unsigned int i = 0; i < m_pFmtCtx->nb_streams; i++)
//		//{
//		//	if (m_streamIndex < 0 && m_pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
//		//	{
//		//		m_streamIndex = i;
//		//		m_streamCodec = m_pFmtCtx->streams[i]->codecpar->codec_id;
//		//		m_StreamFPS = (int)av_q2d(m_pFmtCtx->streams[i]->r_frame_rate);
//		//		//m_needsPrefix = m_pFmtCtx->streams[i]->codecpar->extradata[0];
//		//		m_StreamWidth = m_pFmtCtx->streams[m_streamIndex]->codecpar->width;
//		//		m_StreamHeight = m_pFmtCtx->streams[m_streamIndex]->codecpar->height;
//		//		break;
//		//	}
//		//}
//
//		if (m_VideoStreamInx < 0)
//		{
//			avformat_close_input(&m_pFmtCtx);
//			m_pFmtCtx = NULL;
//			return -1;
//		}
//		else
//		{
//			m_VideoStreamCodec = m_pFmtCtx->streams[m_VideoStreamInx]->codecpar->codec_id;
//			m_StreamFPS = (int)av_q2d(m_pFmtCtx->streams[m_VideoStreamInx]->avg_frame_rate);
//			if (m_StreamFPS <= 0)
//				m_StreamFPS = (int)av_q2d(m_pFmtCtx->streams[m_VideoStreamInx]->r_frame_rate);
//			//m_needsPrefix = m_pFmtCtx->streams[i]->codecpar->extradata[0];
//			m_StreamWidth = m_pFmtCtx->streams[m_VideoStreamInx]->codecpar->width;
//			m_StreamHeight = m_pFmtCtx->streams[m_VideoStreamInx]->codecpar->height;
//		}
//
//		m_AudioStreamInx = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
//		if (m_AudioStreamInx > 0)
//		{
//			m_AudioStreamCodec = m_pFmtCtx->streams[m_AudioStreamInx]->codecpar->codec_id;
//		}
//
//		//eVideoCodec = m_pFmtCtx->streams[m_VideoStreamInx]->codecpar->codec_id;
//		nBitDepth = 8;
//
//		bMp4H264 = m_VideoStreamCodec == AV_CODEC_ID_H264 && (
//			!strcmp(m_pFmtCtx->iformat->long_name, "QuickTime / MOV")
//			|| !strcmp(m_pFmtCtx->iformat->long_name, "FLV (Flash Video)")
//			|| !strcmp(m_pFmtCtx->iformat->long_name, "Matroska / WebM")
//			);
//
//		av_init_packet(&pkt);
//		pkt.data = NULL;
//		pkt.size = 0;
//		av_init_packet(&pktFiltered);
//		pktFiltered.data = NULL;
//		pktFiltered.size = 0;
//
//		if (bMp4H264) {
//			const AVBitStreamFilter *bsf = av_bsf_get_by_name("h264_mp4toannexb");
//			if (!bsf) {
//				//LOG(ERROR) << "FFmpeg error: " << __FILE__ << " " << __LINE__ << " " << "av_bsf_get_by_name() failed";
//				return -1;
//			}
//
//			av_bsf_alloc(bsf, &bsfc);
//			bsfc->par_in = m_pFmtCtx->streams[m_VideoStreamInx]->codecpar;
//			av_bsf_init(bsfc);
//		}
//
//		m_bCanRead = true;
//
//		memset(m_sdpString, 0, 2048);
//		sdpString = "";
//		m_extraInfo.extExtraDataSize = 0;
//		av_sdp_create(&m_pFmtCtx, 1, m_sdpString, 2048);
//		MakeSDPString(m_StreamFPS);
//
//		CCommonInfo::GetInstance()->KSNCOutputDebugString(
//			L"RTSPClient OpenStream (success) time : %f msec - %s\n", timecheck.past_milliseconds(), m_wstrUrl.c_str());
//
//		return ret;
//	}
//
//	AVFormatContext* GetFormatContext() { return m_pFmtCtx; }
//
//	int GetVideoStreamIndex()
//	{
//		return m_VideoStreamInx;
//	}
//
//	AVCodecID GetVideoStreamCodec(int index)
//	{
//		return m_VideoStreamCodec;
//	}
//
//	int GetAudioStreamIndex()
//	{
//		return m_AudioStreamInx;
//	}
//
//	AVCodecID GetAudioStreamCodec(int index)
//	{
//		return m_AudioStreamCodec;
//	}
//
//	std::string GetSDPString()
//	{
//		return sdpString;
//	}
//
//	std::string MakeSDPString(int fps)
//	{
//		std::string serverip = CCommonInfo::GetInstance()->GetServerIP();
//		m_spspps = GetVideoSPSPPS(m_sdpString);
//		std::string config = GetVideoConfig(m_sdpString).c_str();
//		std::string profile = GetVideoProfileLevel(m_sdpString).c_str();
//
//		char payLoadTypeString[32] = { 0, };
//		char fpsString[32] = { 0, };
//		sprintf_s(fpsString, "%d", fps);
//		switch (m_VideoStreamInx)
//		{
//		case AV_CODEC_ID_H264:
//			sprintf_s(payLoadTypeString, "%d", 96);
//			break;
//		case AV_CODEC_ID_HEVC:
//			sprintf_s(payLoadTypeString, "%d", 98);
//			break;
//		case AV_CODEC_ID_MPEG4:
//			sprintf_s(payLoadTypeString, "%d", 96);
//			break;
//		default:
//			sprintf_s(payLoadTypeString, "%d", 96);
//			break;
//		}
//
//		// note : Session Description
//		sdpString += "v=0\r\n";
//		sdpString += "o=- 0 0 IN IP4 ";
//		sdpString += serverip;
//		sdpString += "\r\n";
//		sdpString += "s=KSNC MediaCenter\r\n";
//		sdpString += "e=NONE\r\n";
//		sdpString += "c=IN IP4 0.0.0.0\r\n";
//		sdpString += "b=RR:0\r\n";
//
//		// note : Time Description
//		sdpString += "t=0 0\r\n";
//		sdpString += "m=video 0 RTP/AVP " + std::string(payLoadTypeString) + "\r\n";
//		//sdpString += "a=framerate:" + std::string(fpsString) + "\r\n";
//		sdpString += "a=control:trackID=0\r\n";
//		sdpString += "a=rtpmap:" + std::string(payLoadTypeString) + " ";
//
//		switch (m_VideoStreamCodec)
//		{
//		case AV_CODEC_ID_H264:
//			sdpString += "H264/90000\r\n";
//			sdpString += "a=fmtp:" + std::string(payLoadTypeString) + " packetization-mode=1;";
//			if (m_spspps != "")
//				sdpString += " sprop-parameter-sets=" + m_spspps + ";";
//			break;
//		case AV_CODEC_ID_HEVC:
//			sdpString += "H265/90000\r\n";
//			sdpString += "a=fmtp:" + std::string(payLoadTypeString) + ";";
//			break;
//		case AV_CODEC_ID_MPEG4:
//			sdpString += "MP4V-ES/90000\r\n";
//			sdpString += "a=fmtp:" + std::string(payLoadTypeString) + "profile-level-id=" + profile + ";";
//			sdpString += " config=" + config + "\r\n";
//			break;
//		}
//
//		//sdpString += " profile-level-id=" + GetVideoProfileLevel(m_sdpString);
//
//		return sdpString;
//	}
//
//	std::string GetVideoSPSPPS(std::string sdpstring)
//	{
//		std::string expression = "sprop-parameter-sets=(.+);";
//		std::tr1::regex reg;
//		std::tr1::cmatch result;
//
//		reg.assign(expression, std::tr1::regex_constants::icase);
//
//		if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
//		{
//			return result[1];
//		}
//
//		return "";
//	}
//
//	std::string GetVideoProfileLevel(std::string sdpstring)
//	{
//		std::string expression = "profile-level-id=(.+);";
//		std::tr1::regex reg;
//		std::tr1::cmatch result;
//
//		reg.assign(expression, std::tr1::regex_constants::icase);
//
//		if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
//		{
//			return result[1];
//		}
//
//		return "";
//	}
//
//	std::string GetVideoConfig(std::string sdpstring)
//	{
//		std::string expression = "config=(.+)";
//		std::tr1::regex reg;
//		std::tr1::cmatch result;
//
//		reg.assign(expression, std::tr1::regex_constants::icase);
//
//		if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
//		{
//			return result[1];
//		}
//
//		return "";
//	}
//
//	std::string GetVideoPacketization(std::string sdpstring)
//	{
//		std::string expression = "packetization-mode=(.+);";
//		std::tr1::regex reg;
//		std::tr1::cmatch result;
//
//		reg.assign(expression, std::tr1::regex_constants::icase);
//
//		if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
//		{
//			return result[1];
//		}
//
//		return "";
//	}
//
//	unsigned char NeedsFrefix() { return m_needsPrefix; }
//
//private:
//	AVFormatContext*	m_pFmtCtx;
//	std::string			m_strBindingAddress;
//	std::string			m_strUrl;
//	std::wstring		m_wstrUrl;
//
//	// FFMPEG에서 스트림 읽기시 타임아웃 관련 속성들
//	bool				m_bCanRead = false;
//	bool				m_bMustClose = false;
//	Stopwatch			m_swOpenCheck;
//
//	bool				m_DoneStart = false;
//	int					m_VideoStreamInx;
//	AVCodecID			m_VideoStreamCodec;
//
//	int					m_AudioStreamInx;
//	AVCodecID			m_AudioStreamCodec;
//
//	ExtraInfo			m_extraInfo;
//	unsigned char		m_needsPrefix;
//	char				m_sdpString[2048];
//	std::string			sdpString;
//	std::string			m_spspps;
//
//	bool				m_ConnectionMode;
//	int					m_StreamWidth;
//	int					m_StreamHeight;
//	int					m_StreamFPS;
//
//	AVBSFContext *bsfc = NULL;
//	AVPacket pkt, pktFiltered;
//	bool bMp4H264;
//	int nBitDepth;
//	//AVCodecID eVideoCodec;
//};
