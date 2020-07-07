#pragma once

#include "MediaFrame.hpp"
#include "Utility.hpp"

typedef SharedQueue<MediaFrame*> QQ;
class IClient;

#define CT_TYPE_NUMBER ((int)(CT_MAX_COUNT) -1)

typedef enum ClientType		//CT_NOT_DEFINED, CT_RTSP_FF_CLIENT, CT_RTSP_LIVE_CLIENT, CT_LOCAL_FILE_CLIENT, CT_HLS_CLIENT, CT_RTMP_CLIENT, CT_MAX_COUNT
{
	CT_NOT_DEFINED, CT_RTSP_FF_CLIENT, CT_RTSP_LIVE_CLIENT
	, CT_LOCAL_FILE_CLIENT, CT_HLS_CLIENT, CT_RTMP_CLIENT
	, CT_MAX_COUNT
}CT;

static const char* ParseCT(CT Type) {

	const char* CTST = "CT_NOT_DEFINED";

	switch (Type) {
	case CT_NOT_DEFINED:default:	CTST = "CT_NOT_DEFINED";			break;
	case CT_RTSP_FF_CLIENT:			CTST = "CT_RTSP_FF_CLIENT";			break;
	case CT_RTSP_LIVE_CLIENT:		CTST = "CT_RTSP_LIVE_CLIENT";		break;
	case CT_LOCAL_FILE_CLIENT:		CTST = "CT_LOCAL_FILE_CLIENT";		break; 
	case CT_HLS_CLIENT:				CTST = "CT_HLS_CLIENT";				break; 
	case CT_RTMP_CLIENT:			CTST = "CT_RTMP_CLIENT";			break; 
	}
	return CTST;
}

//TT_UNKNOWN, TT_TCP, TT_UDP, TT_HTTP, TT_TS
typedef enum TransportType
{
	TT_UNKNOWN, TT_TCP, TT_UDP, TT_HTTP, TT_TS
}TT;

typedef struct LLIST {
	const CT Type;
	LinkedList<IClient*>* List;
	LLIST(CT type, LinkedList<IClient*>* list) :Type(type), List(list) {}
}TL;

typedef struct ClientInfomation {

	CT Type = CT_NOT_DEFINED;
	TT TransportType = TT_TCP;
	UUID uuid = { 0,0,0,0 };
	std::string URL;

	AVCodecID VCodecID;
	AVCodecID ACodecID;

	int Ref = 0;
	int Interval = 0;
	int Index = 0;

	bool Connected = false;
	bool IncludedAudio = false;
	
	bool Use_Transcoding = false;		//false = Not Trans, true = Need Trans

	ClientInfomation() {}
	ClientInfomation(CT m_CT, TT m_TT, UUID m_uuid, std::string m_URL,
		AVCodecID m_VCodecID = AV_CODEC_ID_NONE, AVCodecID m_ACodecID = AV_CODEC_ID_NONE,
		int m_Ref = 0, int m_Interval = 5, int m_Index = 0,
		bool m_Connected = false, bool m_IncludedAudio = false, bool m_Use_Transcoding = false)
		: Type(m_CT), TransportType(m_TT), uuid(m_uuid), URL(m_URL),
		VCodecID(m_VCodecID), ACodecID(m_ACodecID), Ref(m_Ref), Interval(m_Interval), Index(m_Index),
		Connected(m_Connected), IncludedAudio(m_IncludedAudio), Use_Transcoding(m_Use_Transcoding)
	{}

}CLI;

#ifndef GCVS
#define GCVS

typedef struct GetClientValue {	//CT Type, UUID uuid, std::string URL
	CT Type = CT_NOT_DEFINED;
	UUID uuid = {};
	std::string URL = "";

	GetClientValue(CT mtype = CT_NOT_DEFINED, UUID muuid = {}, std::string mURL = "") :
		Type(mtype), uuid(muuid), URL(mURL) {}
}GCV;
#endif

