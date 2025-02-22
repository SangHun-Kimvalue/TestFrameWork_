#pragma once

#include "SourceClass.h"

class Local_fileClass : public SourceClass
{
public:

	Local_fileClass(FramequeueClass *Q_, const DecoderType HAChoice, int index, int Frame_size, int Sleep_t) : SourceClass() {
		m_Fqueue = Q_;
		PushCount = 0;
		isOpen = false;
		index_t = index;
		openFile(Find_src_file());
		Max_Frame_save = Frame_size;
		Sleeptime = Sleep_t;
	}

	Local_fileClass(FramequeueClass *Q_, const DecoderType HAChoice, const string filename) : SourceClass(Q_, HAChoice, filename){
		openFile(filename);
	}

	~Local_fileClass() {
		delete m_Fqueue;
	}

	const bool openFile(const string inputFile);
	void CloseFile();
	bool Check_Open();
	bool end_check();

private:

	void Thread_Decode();
	AVFrame* Frame;
	Decoder_f *FF_Decode;

};