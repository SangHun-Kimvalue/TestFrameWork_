#include "pch.h"
#include "DXResourceClass.h"

bool DXResourceClass::end_check() {

	return FF_Decode->d_end;
}

bool DXResourceClass::Check_Open() {

	if (isOpen == true)
		return true;
	else
		return false;
}

void DXResourceClass::Thread_Decode() {

	m_Fqueue->Megabitrate = FF_Decode->Megabitrate;

	while (FF_Decode->d_end == false) {
		Frame = FF_Decode->ReadPacket();
		if (Frame != false) {
			if (m_Fqueue->PushFrame(Frame, FF_Decode->d_end, FF_Decode->realFPS,
				FF_Decode->PacketReadCycle, FF_Decode->ConvertCycle) != true)
				PushCount++;

			if (m_Fqueue->VFrame->size() > 100)
				Sleep(m_Fqueue->q_sleeptime);
			//FF_Decode->d_end = true;					CPU 렌더링 사용량 측정
		}
		m_Fqueue->push_end = FF_Decode->d_end;
		if (HA_Choice == 0) {
			if (m_Fqueue->q_sleeptime > 0)
				Sleep(m_Fqueue->q_sleeptime / 4);
			else
				Sleep(1);
		}
		//else
		//	Sleep(1);
	}
	//maintain	//stabilize

	m_Fqueue->push_end = FF_Decode->d_end;
	avcodec_free_context(&FF_Decode->pVideoCodecCtx);
	avformat_close_input(&FF_Decode->pFormatCtx);
	av_buffer_unref(&FF_Decode->hw_device_ctx);

}

const bool DXResourceClass::openFile(const string inputFile) {

	bool result = false;

	FF_Decode = new Decoder_f(HA_Choice);			//매개변수로 HWACCel 선택
													// AV_HWDEVICE_TYPE_DXVA2 // AV_HWDEVICE_TYPE_D3D11VA  //AV_HWDEVICE_TYPE_CUDA   // AV_HWDEVICE_TYPE_VAAPI	//AV_HWDEVICE_TYPE_MEDIACODEC
													// 1						2							3							4, 5조사 필요

	if (FF_Decode->HA_Usage == CPU) {
		result = FF_Decode->Open(inputFile);
	}
	else
		result = FF_Decode->Open_HA(inputFile);

	if (result) {
		Thread_D_P = thread(&DXResourceClass::Thread_Decode, this);
		//Thread_Decode();
		return true;
	}
	else {
		//system("cls");
		printf("\n\n\n\-------------------------------------Open Error-------------------------------------\n\n\n\n");
		return false;
	}

	return false;
}

void DXResourceClass::CloseFile() {

	if (FF_Decode == NULL)
		return;

	FF_Decode->d_end = true;

	if (Thread_D_P.joinable() == true)
		Thread_D_P.join();

	FF_Decode->CloseFile();

	delete FF_Decode;

	return;
}


