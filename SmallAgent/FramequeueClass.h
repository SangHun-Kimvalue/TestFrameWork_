#pragma once

#include <queue>
#include <mutex>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/dxva2.h>
#include <libavutil/hwcontext_dxva2.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_mediacodec.h>
}

using namespace std;

class FramequeueClass {

public:
	FramequeueClass() {
		VFrame = new queue<AVFrame*>;
		q_PushCount = 0;
		pop_end = false;
		q_PacketReadCycle = 0;
		q_ConvertCycle = 0;
		q_sleeptime = 0;
		VideoFPS = 0;
		Megabitrate = 0;
	}
	~FramequeueClass() {
		for (int i = 0; i < VFrame->size(); i++) {
			VFrame->pop();
		}
		delete VFrame;
		VFrame = 0;
	}
	bool PushFrame(AVFrame* pFrame, bool d_end, double FPS, float PacketReadCycle, float ConvertCycle);
	AVFrame* PoPFrame();

	bool PushPacket(std::vector<std::vector<uint8_t>>* pPacket, bool d_end, double FPS, float PacketReadCycle, float ConvertCycle);
	std::vector<std::vector<uint8_t>>* PoPPacket(AVFrame* pFrame, bool d_end, double FPS, float PacketReadCycle, float ConvertCycle);


	double CallFPS();
	void CallEnd(bool end);
	int CallBitrate();
	float q_sleeptime;

	queue<AVFrame*> *VFrame;
	int q_PushCount;

	double VideoFPS;
	bool pop_end;
	bool push_end;
	float q_PacketReadCycle; 
	float q_ConvertCycle;
	int Megabitrate;

private:

	//Debug_logClass *log;
};