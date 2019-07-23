#include "FramequeueClass.h"
mutex m_mtx;

bool FramequeueClass::PushFrame(AVFrame* pFrame, bool d_end, double FPS, float PacketReadCycle, float ConvertCycle) {//여기서 렌더할때 쓰는 push카운터를 올리며 푸쉬의 끝 여부를 확인
	float timecheck = 0;		VideoFPS = FPS;
	static int count = 0;	
	q_PacketReadCycle = PacketReadCycle;	q_ConvertCycle = ConvertCycle;
	d_end = pop_end;
	if (push_end == true) {
		printf("\n\nPush End \n\n");
		return true;
	}
	m_mtx.lock();
	VFrame->push(pFrame);
	m_mtx.unlock();

	q_PushCount++;
	//printf("Queuesize : %d\t\t", VFrame->size());

	return false;
}

AVFrame* FramequeueClass::PoPFrame() {
	
	AVFrame* frame = av_frame_alloc();
	if (VFrame->empty() == false) {
		m_mtx.lock();
		*frame = *VFrame->front();
		VFrame->pop();
		m_mtx.unlock();

		return frame;
	}
	else
		return false;
	
}

void FramequeueClass::CallEnd(bool end) {
	pop_end = end;				//작업 중간에 종료 신호가 오게되면 디코딩과 푸쉬를 중지하도록 하는 신호를 줌.(쓰레드 종료)
	return;
}

double FramequeueClass::CallFPS() {

	return VideoFPS;
}

int FramequeueClass::CallBitrate() {

	return Megabitrate;
}

