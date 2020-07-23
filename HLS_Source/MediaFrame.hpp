#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
#include <iostream>
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

public:
	//{ StreamId(streamid), Width(0), Height(0), Fps(0), Bitrate(0), Samplerate(0), Pkt(NULL), Frm(NULL), Pts(0) }
	MediaFrame(bool Pack, int streamid = 0) : Packing(Pack), Info({ streamid , 0, 0, 0, 0, 0, 0 }), Pkt(nullptr), Frm(nullptr) {
	
		//if (!Packing) {
		Pkt = new AVPacket();
		av_init_packet(Pkt);
		Frm = av_frame_alloc();
		//}
		//else {
		//	Pkt = new AVPacket();
		//	av_init_packet(Pkt);
		//	Frm = av_frame_alloc();
		//	//Frm = new AVFrame();
		//}
	}
	//MediaFrame(MediaFrame* a) : Packing(a->Packing), Info(a->Info), Pkt(nullptr), Frm(nullptr) {
	//
	//	//if (!Packing) {
	//		Pkt = new AVPacket(*a->Pkt);
	//		int error = av_packet_ref(Pkt, a->Pkt);
	//		PRef++;
	//		//int i = Pkt->size();
	//		//std::cout << error;
	//	//}
	//	//else {
	//	//	Pkt = new AVPacket();
	//	//	//Frm = av_frame_alloc();
	//		Frm = new AVFrame(*a->Frm);
	//		error = av_frame_ref(Frm, a->Frm);
	//	//	FRef++;
	//	//}
	//
	//}
	~MediaFrame() {

		if (Pkt->size != 0) {
			av_packet_unref(Pkt);
		}

		if(Frm != NULL){
			//av_freep(&Frm->data[0]);
			av_frame_unref(Frm);
			//av_frame_free(&Frm);
		}
	}

	int SetMediaFrame(FI info) {

		Info = info;

		return 0;
	}

	int SetMediaFrame(FI info, void* Pkt_or_Frame) {

		Info = info;
		if (Pkt_or_Frame == nullptr) return -1;
		if (!Packing) {
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

	int FRef = 0;
	int PRef = 0;

	AVPacket* Pkt;
	AVFrame* Frm;

};