#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>

extern "C"
{
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavdevice/avdevice.h>

}

typedef struct FrameInfo {

	//Video = 0, Audio = 1
	int StreamId;
	int Width;
	int Height;
	int Fps;
	int Bitrate;
	int Samplerate;
	int Pts;

}FI;

class MediaFrame {
	//{ StreamId(streamid), Width(0), Height(0), Fps(0), Bitrate(0), Samplerate(0), Pkt(NULL), Frm(NULL), Pts(0) }
public:
	MediaFrame(bool Pack, int streamid = 0) : Packing(Pack), Info({ streamid , 0, 0, 0, 0, 0, 0 }), Pkt(nullptr), Frm(nullptr) {
	
		if (!Packing) {
			Pkt = new AVPacket();
			av_init_packet(Pkt);
		}
		else {
			Pkt = new AVPacket();
			av_init_packet(Pkt);
			Frm = av_frame_alloc();
		}
	}
	~MediaFrame() { 
		//if (Pkt != nullptr) {
		//	av_packet_unref(Pkt);
		//	//if (data[0]) free(data[0]);
		//	delete Pkt;
		//	//if (packetData != nullptr) {
		//	//	packetData = 0;
		//	//	delete packetData;
		//	//}
		//}
		if (Pkt->size != 0) 
			av_packet_unref(Pkt);
		if (Packing && Frm->buf != nullptr)
			av_frame_unref(Frm);
	}

	int SetMediaFrame(FI info, void* Pkt_or_Frame){
		
		Info = info;
		if (Pkt_or_Frame == nullptr) return -1;
		if (Packing) {
			Pkt = (AVPacket*)Pkt_or_Frame;
		 }
		else {
			Frm = (AVFrame*)Pkt_or_Frame;
		}
		return 0;
	}

private:


public:

	FI Info;
	//false = Not Trans, true = Need Trans
	const bool Packing;

	AVPacket* Pkt;
	AVFrame* Frm;

};