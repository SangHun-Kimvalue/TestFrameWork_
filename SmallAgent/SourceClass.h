#pragma once

#include "systemclass.h"
#include "FramequeueClass.h"
#include "Decoder_f.h"
#include <io.h>

class SourceClass {
	//interface SourceClass {

public:
	SourceClass() : HA_Choice(CPU) {}
	SourceClass(FramequeueClass *Q_, DecoderType HAChoice, Debug_logClass *m_log, string filename)
		: m_Fqueue(Q_), HA_Choice(HAChoice), log(m_log), PushCount(0), isOpen(false){
	}
	~SourceClass() {}

	virtual const bool openFile(const string inputFile) = 0;
	virtual void CloseFile() = 0;
	virtual void Thread_Decode() = 0;
	virtual void Thread_Decode_Test();
	virtual bool Check_Open() = 0;
	virtual bool end_check() = 0;
	string Find_src_file();
	string open_source_name();


	FramequeueClass *m_Fqueue;
	int PushCount;
	bool isOpen;
	Debug_logClass *log;
	thread Thread_D_P;
	int Sleeptime;
	const DecoderType HA_Choice;
	int index_t;
	int Max_Frame_save;

private:

};
