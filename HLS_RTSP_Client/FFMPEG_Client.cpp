#pragma warning (disable:4996)

#include "FFMPEG_Client.h"
#include <regex>
#define TESTTRANS_

std::string GetVideoProfileLevel(std::string sdpstring);
std::string GetVideoSPSPPS(std::string sdpstring);
std::string GetVideoConfig(std::string sdpstring);
std::string GetVideoPacketization(std::string sdpstring);
const char* err_pro(int error, const char* msg);

const char* err_pro(int error, const char* msg) {

	char errstr[256] = "";

	av_strerror(error, errstr, 256);
	std::cout << "Error : " << errstr << " " << msg << std::endl;

	return errstr;
}

FFMPEG_Client::FFMPEG_Client(CLI info, int qsize) :m_info(info), Qsize(qsize), Stopped(true),
IncludedAudio(false), VSI(-1), ASI(-1), Use_Transcoding(false), VDec(nullptr), ADec(nullptr),
m_DT(DT_DECODE_NONE){

	m_info.Connected = false;
	//m_pRecAFrameQ = CreateRecievedFrameQueue(Qsize);
	m_pRecFrameQ = CreateRecievedFrameQueue(500);

	m_DT = DT_SW_FFMPEG;

}

FFMPEG_Client::~FFMPEG_Client() {

	if (pFormatCtx != nullptr || m_info.Connected == false || Stopped == false) {
		StopClient();
	}
}

int FFMPEG_Client::InitClient(CLI info) {

	m_info = info;
	avformat_network_init();

	Path = m_info.URL;
	
	if (SetFFOption() < 0) {
		return -1;
	}

	//if (Connect() < 0) {
	//	return -1;
	//}

	return 0;
}

int FFMPEG_Client::SetFFOption() {

	AVDictionary *avdic = NULL;
	pFormatCtx = avformat_alloc_context();

	int ret = av_dict_set(&avdic, "reorder_queue_size", "30", 0);
	if (ret < 0) {
		std::cout << ("RTSPClient av_dict_set (reorder_queue_size) error %d - %s\n", ret, m_info.URL.c_str()) << std::endl;
		return ret;
	}
	ret = av_dict_set(&avdic, "stimeout", "10000000", 0);
	if (ret < 0) {
		std::cout << ("RTSPClient av_dict_set (stimeout) error %d - %s\n", ret, m_info.URL.c_str()) << std::endl;
		return ret;
	}

	ret = av_dict_set(&avdic, "buffer_size", "64 * 1024 * 10", 0);
	if (ret < 0) {
		std::cout << ("RTSPClient av_dict_set (buffer_size) error %d - %s\n", ret, m_info.URL.c_str()) << std::endl;
		return ret;
	}
	
	ret = av_dict_set(&avdic, "threads", "auto", 0);
	if (ret < 0) {
		std::cout << ("RTSPClient av_dict_set (threads) error %d - %s\n", ret, m_info.URL.c_str()) << std::endl;
		return ret;
	}

	std::string option_value = "";
	if (m_info.TransportType == TT_UNKNOWN || m_info.TransportType == TT_TCP)
		option_value = "tcp";
	else if (m_info.TransportType == TT_UDP)
		option_value = "udp";
	else
		option_value = "tcp";

	ret = av_dict_set(&avdic, "rtsp_transport", option_value.c_str(), 0);
	if (ret < 0) {
		std::cout << ("RTSPClient av_dict_set (rtsp_transport) error %d - %s\n", ret, m_info.URL.c_str()) << std::endl;
		return ret;
	}

	ret = av_dict_set(&avdic, "max_delay", "50", 0);
	if (ret < 0) {
		std::cout << ("RTSPClient av_dict_set (max_delay) error %d - %s\n", ret, m_info.URL.c_str()) << std::endl;
		return ret;
	}

	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	ret = avformat_open_input(&pFormatCtx, Path.c_str(), NULL, &avdic);
	if (ret < 0) {
		err_pro(ret, "Can't connect the RTSP Server.");
		m_info.Connected = false;
		return ret;
	}

	ret = avformat_find_stream_info(pFormatCtx, NULL);
	if (ret < 0) {
		err_pro(ret, "Can't find stream infomation");
		m_info.Connected = false;
		return ret;
	}

	av_dump_format(pFormatCtx, 0, Path.c_str(), 0);

	return 0;
}

int FFMPEG_Client::Connect() {

	//for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
	//{
	//	if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
	//	{
	//		VSI = i;
	//	}
	//
	//	if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
	//	{
	//		ASI = i;
	//	}
	//}

	if (Stopped == false) {

		return 1;
	}

	//Video Check	---------------------------------------------------------
	VSI = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (VSI < 0)
	{
		std::cout << "Not included video stream" << std::endl;
		m_info.Connected = false;
		VSI = -1;
	}
	else {
		m_info.VCodecID = pFormatCtx->streams[VSI]->codecpar->codec_id;
		if (m_info.VCodecID == AV_CODEC_ID_H264) {
			Use_Transcoding = false;
		}
		else {
			Use_Transcoding = true;
			//int ret = DecoderSet(VDec, DT_SW_FFMPEG, VSI);
			
		}
		VDec = new FFmpegDecoder();
		int ret = VDec->Init(VSI, pFormatCtx);
	}
#ifdef TESTTRANS
	Use_Transcoding = true;
	VDec = new FFmpegDecoder();
	int tret = VDec->Init(VSI, pFormatCtx);
#endif

	//Audio Check	---------------------------------------------------------
	ASI = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (ASI < 0) {
		std::cout << "Not Use audio stream" << std::endl;
		ASI = -1;
		IncludedAudio = false;
	}
	else {
		m_info.ACodecID = pFormatCtx->streams[ASI]->codecpar->codec_id;
		if (m_info.ACodecID == AV_CODEC_ID_AAC) {
			Use_Transcoding = false;
		}
		else {
			//int ret = DecoderSet(ADec, DT_SW_FFMPEG, ASI);
			ADec = new FFmpegDecoder();
			int ret = ADec->Init(ASI, pFormatCtx);
			Use_Transcoding = true;
		}
		
	}

	//All Check	---------------------------------------------------------
	if (VSI == -1 && ASI == -1) {
		std::cout << "Not included A/V stream" << std::endl;
		m_info.Connected = false;
		return -1;
	}
	else {
		SetCLI(m_info.VCodecID, m_info.ACodecID);
	}
	
	//pCodec = avcodec_find_decoder(pFormatCtx->streams[VSI]->codecpar->codec_id);
	//if (!pCodec) {
	//	std::cout << "RTSP_Client avcodec_find_decoder" << std::endl;
	//	return -1;
	//}
	//
	//pCodecCtx = avcodec_alloc_context3(pCodec);
	//if (!pCodecCtx) {
	//	std::cout << "RTSP_Client avcodec_alloc_context3" << std::endl;
	//	return -1;
	//}
	//pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	//pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	//
	//int m_error = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[VSI]->codecpar);
	//if (m_error < 0) {
	//	err_pro(m_error, "RTSP_Client avcodec_parameters_to_context");
	//	return -1;
	//}
	//
	//pCodecCtx->framerate = av_guess_frame_rate(pFormatCtx, pFormatCtx->streams[VSI], NULL);
	//pCodecCtx->bit_rate = pFormatCtx->streams[VSI]->codecpar->bit_rate;
	//pCodecCtx->sample_rate = pFormatCtx->streams[VSI]->codecpar->sample_rate;
	//pCodecCtx->time_base = pFormatCtx->streams[VSI]->time_base;
	//
	//m_error = avcodec_open2(pCodecCtx, pCodec, NULL);
	//if (m_error < 0) {
	//	err_pro(m_error, "RTSP_Client dec avcodec_open2");
	//
	//	m_info.Connected = false;
	//	return -1;
	//}
	//
	//packet = (AVPacket *)malloc(sizeof(AVPacket));
	//av_new_packet(packet, pCodecCtx->width * pCodecCtx->height);
	//
	//pCodecCtx->thread_count = 0;
	
	if (MakeSDP() == "") {
		m_info.Connected = false;
		std::cout << "Error made SDP string" << std::endl;
		return -1;
	}
	
	m_info.Connected = true;

	//PrintClientInfo();

	return 0;
}

int FFMPEG_Client::Play() {

	
	if (Stopped) {
		Stopped = false;
		Workingthr = std::thread([&]() { DoWork(this); });
		return IncreaseRef();
	}
	else {
		return IncreaseRef();
	}

	return IncreaseRef();
}

int FFMPEG_Client::DoWork(FFMPEG_Client* fc) {

	std::chrono::milliseconds duration(1);
	int count = 0;
	int ret = 0;
	int failcount = 0;

	fc->F_Info.Fps = fc->m_StreamFPS = 24;

	while (!fc->Stopped) {
#ifdef TESTTRANS
		fc->Use_Transcoding = true;
#endif
		MediaFrame* MF = new MediaFrame(fc->Use_Transcoding);
		int ret = av_read_frame(fc->pFormatCtx, MF->Pkt);
		if (ret == AVERROR(EAGAIN)) {
			delete MF;
			continue;
		}
		else if (ret < 0)
		{
			if (ret == AVERROR_EOF) {
				err_pro(ret, "[ERROR] - ");
				std::cout << "Disconnected to server : " << fc->m_info.URL.c_str() << std::endl;
				delete MF;
				return ret;
			}

			err_pro(ret, "[ERROR] - ");
			std::cout << "Error in av_read_frame : " << fc->m_info.URL.c_str() << std::endl;
			delete MF;
			failcount++;
		}
		else {
			int Deret = -1;

			if (MF->Pkt->stream_index == fc->VSI) {
				Deret = fc->Decode(MF, fc->VDec, fc->VSI);
			}
			else if (MF->Pkt->stream_index == fc->ASI) {
				Deret = fc->Decode(MF, fc->ADec, fc->ASI);
			}

			if (Deret < 0) {
				failcount++;
				delete MF;
				continue;
			}
			else {
					
				fc->F_Info.StreamId = MF->Pkt->stream_index;
				MF->Frm->width = fc->F_Info.Width = fc->pFormatCtx->streams[MF->Pkt->stream_index]->codecpar->width;
				MF->Frm->height = fc->F_Info.Height = fc->pFormatCtx->streams[MF->Pkt->stream_index]->codecpar->height;
				fc->F_Info.Bitrate = (int)fc->pFormatCtx->streams[MF->Pkt->stream_index]->codecpar->bit_rate;
				fc->F_Info.Samplerate = fc->pFormatCtx->streams[MF->Pkt->stream_index]->codecpar->sample_rate;
			}

			//all success
			if (Deret >= 0) {
			
				FI info = { fc->F_Info.StreamId, fc->F_Info.Width, fc->F_Info.Height, fc->F_Info.Fps,
					fc->F_Info.Bitrate, fc->F_Info.Samplerate, MF->Pkt->pts };
				
				MF->SetMediaFrame(info);

				failcount = 0;
				int max_size = fc->m_pRecFrameQ->max_size();
				if (max_size < fc->m_pRecFrameQ->size()) {
					delete fc->m_pRecFrameQ->pop();
					fc->m_pRecFrameQ->push_back(MF);
				}
				else {
					fc->m_pRecFrameQ->push_back(MF);
				}
				count++;
#ifdef _DEBUG
				if (count % 50 == 0) {
					std::cout << "Pushing ... size : " << fc->m_pRecFrameQ->size() << std::endl;
					count = 0;
				}
#endif // _DEBUG
			}
			else {
				failcount++;
				delete MF;
			}
				
		}
		std::this_thread::sleep_for(duration);
	}

	return 0;
}

int FFMPEG_Client::Decode(MediaFrame* MF, FFmpegDecoder* Dec, int StreamID) {

	int ret = Dec->Decode(MF);
	if (ret < 0) {
		err_pro(ret, "Error Detected from Decoder");
		if (ret == (DECODE_FAIL_COUNT * -1)) {
			std::cout << "Reset Decoder -- ";
			ret = DecoderReset(Dec, m_DT, StreamID);
			if (ret < 0) {
				std::cout << "Failed" << std::endl;
				return ret;
			}
			std::cout << "Successed" << std::endl;
		}
	}

	return ret;
}

void FFMPEG_Client::PrintClientInfo() {
	unsigned char* str = nullptr;
	UuidToStringA(&m_info.uuid, &str);

	std::cout << "Connected - " << (m_info.Connected ? "True" : "False") << std::endl					//Connected 

		<< "UUID - " << str << std::endl						//index 쓸지 uuid 쓸지

		<< "Type - " << (m_info.Type == CT_RTSP_FF_CLIENT ? "RTSP_FF_CLIENT" 
			: m_info.Type == CT_RTSP_LIVE_CLIENT ? "RTSP_LIVE_CLIENT" : "UNKHOWN") << std::endl	

		<< "URL - " << m_info.URL.c_str() << std::endl					//

		<< "Interval - " << m_info.Interval << std::endl				//

		//<< "Connected - " << m_info.uuid << std::endl					//
		<< "VCodecID - " << ((m_info.VCodecID == AV_CODEC_ID_H264) ? "H.264" : "No 264 - Need Transcode")  << std::endl	

		<< "ACodecID - " << ((m_info.ACodecID == AV_CODEC_ID_AAC) ? "AAC"
			: (m_info.ACodecID == AV_CODEC_ID_NONE) ? "Not Use Audio" : "No AAC - Need Transcode") << std::endl

		//<< "ACodecID - " << ((m_info.ACodecID == AV_CODEC_ID_AAC) ? "AAC" : "No AAC") << std::endl	
		<< "Use_Transcoding - " << (m_info.Use_Transcoding ? "True" : "False") << std::endl	//

		<< "Ref - " << m_info.Ref << std::endl							//

		<< "--------------------------------------------------------" << std::endl

		<< "SDP - " << SDPString.c_str() << std::endl							//
		<< ""

		<< std::endl;

	return;

}

void FFMPEG_Client::StopClient() {
	//Pause 기능이 필요하다면 Release와 분리시켜야할듯
	Stopped = true;
	if (Workingthr.joinable())
		Workingthr.join();
	if(Keepalivethr.joinable())
		Keepalivethr.join();

	ReleaseClient();
}

int FFMPEG_Client::ReleaseClient() {

	if(VDec != nullptr)
		VDec->Release();

	if (IncludedAudio)
		if (ADec != nullptr)
			ADec->Release();

	if (pFormatCtx != nullptr) {
		avformat_close_input(&pFormatCtx);
		avformat_free_context(pFormatCtx);
	}
	pFormatCtx = nullptr;

	return 0;
}

int FFMPEG_Client::IncreaseRef() {
	std::string temp = m_info.URL.c_str() + std::to_string(m_info.Ref + 1);
	CCommonInfo::GetInstance()->WriteLog("INFO", "IncreaseRef - %s", temp.c_str());
	return ++m_info.Ref;
}

int FFMPEG_Client::DecreaseRef() {

	if (m_info.Ref <= 0) {
		CCommonInfo::GetInstance()->WriteLog("INFO", "DecreaseRef - %s : < 0", m_info.URL.c_str());
		return 0;
	}

	std::string temp = m_info.URL.c_str() + std::to_string(m_info.Ref - 1);
	CCommonInfo::GetInstance()->WriteLog("INFO", "DecreaseRef - %s", temp.c_str());
	return --m_info.Ref;
}

const std::string FFMPEG_Client::GetSDP()const {
	return SDPString;
}

const CLI FFMPEG_Client::GetCLI() const {
	return m_info;
}

void FFMPEG_Client::SetCLI(AVCodecID VCo, AVCodecID ACo) {

	//m_info.Connected;
	//m_info.Index;
	//m_info.Interval;
	//m_info.Ref;
	//m_info.Type;
	//m_info.TransportType;
	//m_info.uuid;

	m_info.VCodecID = VCo;
	m_info.ACodecID = ACo;
	m_info.Use_Transcoding = (m_info.ACodecID == AV_CODEC_ID_AAC && m_info.VCodecID == AV_CODEC_ID_H264) ? false : true;
	if (m_info.VCodecID == AV_CODEC_ID_H264)	
		m_info.Use_Transcoding = false;
	m_info.IncludedAudio = IncludedAudio;
	//m_info.URL;
#ifdef TESTTRANS
	m_info.Use_Transcoding = true;
#endif
	return;

}

std::string FFMPEG_Client::MakeSDP() {

	char m_sdpString[2048] = "";
	SPSPPS = ""; 	SDPString = "";	SDPconfig = "";	SDPprofile = "";

	AVCodecID VCo = AV_CODEC_ID_NONE, ACo = AV_CODEC_ID_NONE;

	av_sdp_create(&pFormatCtx, 1, m_sdpString, 2048);

	m_StreamFPS = (int)av_q2d(pFormatCtx->streams[VSI]->avg_frame_rate);
	if (m_StreamFPS <= 0)
		m_StreamFPS = (int)av_q2d(pFormatCtx->streams[VSI]->r_frame_rate);

	std::string serverip = "192.168.0.70";//GetServerIP();
	SPSPPS = GetVideoSPSPPS(m_sdpString);
	SDPconfig = GetVideoConfig(m_sdpString).c_str();
	SDPprofile = GetVideoProfileLevel(m_sdpString).c_str();

	char payLoadTypeString[32] = { 0, };
	char fpsString[32] = { 0, };
	sprintf_s(fpsString, "%d", m_StreamFPS);
	switch (pFormatCtx->streams[VSI]->codecpar->codec_id)
	{
	case AV_CODEC_ID_H264:
		sprintf_s(payLoadTypeString, "%d", 96);
		break;
	case AV_CODEC_ID_HEVC:
		sprintf_s(payLoadTypeString, "%d", 98);
		break;
	case AV_CODEC_ID_MPEG4:
		sprintf_s(payLoadTypeString, "%d", 96);
		break;
	default:
		sprintf_s(payLoadTypeString, "%d", 96);
		break;
	}

	// note : Session Description
	SDPString += "v=0\r\n";
	SDPString += "o=- 0 0 IN IP4 ";
	SDPString += serverip;
	SDPString += "\r\n";
	SDPString += "s=KSNC DTrix.Relay \r\n";
	SDPString += "e=NONE\r\n";
	SDPString += "c=IN IP4 0.0.0.0\r\n";
	SDPString += "b=RR:0\r\n";

	// note : Time Description
	SDPString += "t=0 0\r\n";
	SDPString += "m=video 0 RTP/AVP " + std::string(payLoadTypeString) + "\r\n";
	//SDPString += "a=framerate:" + std::string(fpsString) + "\r\n";
	SDPString += "a=control:trackID=0\r\n";
	SDPString += "a=rtpmap:" + std::string(payLoadTypeString) + " ";

	switch (pFormatCtx->streams[VSI]->codecpar->codec_id)
	{
	case AV_CODEC_ID_H264:
		SDPString += "H264/90000\r\n";
		SDPString += "a=fmtp:" + std::string(payLoadTypeString) + " packetization-mode=1;";
		if (SPSPPS != "")
			SDPString += " sprop-parameter-sets=" + SPSPPS + ";";
		break;
	case AV_CODEC_ID_HEVC:
		SDPString += "H265/90000\r\n";
		SDPString += "a=fmtp:" + std::string(payLoadTypeString) + ";";
		break;
	case AV_CODEC_ID_MPEG4:
		SDPString += "MP4V-ES/90000\r\n";
		SDPString += "a=fmtp:" + std::string(payLoadTypeString) + "profile-level-id=" + SDPprofile + ";";
		SDPString += " config=" + SDPconfig + "\r\n";
		break;
	}

	return SDPString;
}

int FFMPEG_Client::DecoderSet(FFmpegDecoder* Dec, DT DecoderType, int StreamID) {

	if (Dec != nullptr) {
		Dec->Release();
		delete Dec;
	}
	//Dec = DecoderFactory::CreateDecoder(DecoderType);
	Dec = new FFmpegDecoder();
	if (Dec != nullptr)
		int ret = Dec->Init(StreamID, pFormatCtx);
	else
		return -1;

}

int FFMPEG_Client::DecoderReset(FFmpegDecoder* Dec, DT DecoderType, int StreamID) {
	
	if (Dec != nullptr) {
		Dec->Release();
		delete Dec;
	}

	//Dec = DecoderFactory::CreateDecoder(DecoderType);
	Dec = new FFmpegDecoder();
	if (Dec != nullptr)
		int ret = Dec->Init(StreamID, pFormatCtx);
	else
		return -1;

}

std::string GetVideoSPSPPS(std::string sdpstring)
{
	std::string expression = "sprop-parameter-sets=(.+);";
	std::tr1::regex reg;
	std::tr1::cmatch result;

	reg.assign(expression, std::tr1::regex_constants::icase);

	if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
	{
		return result[1];
	}

	return "";
}

std::string GetVideoProfileLevel(std::string sdpstring)
{
	std::string expression = "profile-level-id=(.+);";
	std::tr1::regex reg;
	std::tr1::cmatch result;

	reg.assign(expression, std::tr1::regex_constants::icase);

	if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
	{
		return result[1];
	}

	return "";
}

std::string GetVideoConfig(std::string sdpstring)
{
	std::string expression = "config=(.+)";
	std::tr1::regex reg;
	std::tr1::cmatch result;

	reg.assign(expression, std::tr1::regex_constants::icase);

	if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
	{
		return result[1];
	}

	return "";
}

std::string GetVideoPacketization(std::string sdpstring)
{
	std::string expression = "packetization-mode=(.+);";
	std::tr1::regex reg;
	std::tr1::cmatch result;

	reg.assign(expression, std::tr1::regex_constants::icase);

	if (std::tr1::regex_search(sdpstring.c_str(), result, reg))
	{
		return result[1];
	}

	return "";
}