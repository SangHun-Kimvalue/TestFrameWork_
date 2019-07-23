#include "FramequeueClass.h"
mutex m_mtx;

bool FramequeueClass::PushFrame(AVFrame* pFrame, bool d_end, double FPS, float PacketReadCycle, float ConvertCycle) {//���⼭ �����Ҷ� ���� pushī���͸� �ø��� Ǫ���� �� ���θ� Ȯ��
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
	pop_end = end;				//�۾� �߰��� ���� ��ȣ�� ���ԵǸ� ���ڵ��� Ǫ���� �����ϵ��� �ϴ� ��ȣ�� ��.(������ ����)
	return;
}

double FramequeueClass::CallFPS() {

	return VideoFPS;
}

int FramequeueClass::CallBitrate() {

	return Megabitrate;
}

