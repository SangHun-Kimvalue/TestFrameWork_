#pragma once

#include <queue>
#include <mutex>
//#include "Debug_logClass.h"
using namespace std;

struct Queuestr {
	AVFrame* data;
};

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
	double CallFPS();
	AVFrame* PoPFrame();
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