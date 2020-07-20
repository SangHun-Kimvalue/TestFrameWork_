#include "SegmenterGroup.h"


SegmenterGroup::SegmenterGroup() : SegType(ST_NOT_DEFINE), UseAudio(false), UseTranscoding(false), Interval(5), VCo(AV_CODEC_ID_NONE), ACo(AV_CODEC_ID_NONE)
{

}

SegmenterGroup::SegmenterGroup(ST SegType, std::string Filename, bool UseAudio, bool UseTranscoding, int Interval, QQ m_DataQ, AVCodecID VCo, AVCodecID ACo)
	: SegType(SegType), Filename(Filename), UseAudio(UseAudio), UseTranscoding(UseTranscoding), Interval(Interval), DataQ(m_DataQ), VCo(VCo), ACo(ACo),
	PathDir()
{
	*PathDir = nullptr;
}

SegmenterGroup::~SegmenterGroup() {


	if (TimeCheckthr.joinable()) {
		TimeCheckthr.join();
	}

	if (Nofitythr.joinable()) {
		Nofitythr.join();
	}
	//if (Ref > 0) {
	//	while (Ref != 0) {
	//		Stop();
	//	}
	//}
}

ISegmenter* SegmenterGroup::SetType(ST SegType, std::string Filename, int i) {


	switch (SegType) {
	case ST_FFSW:
	default:
		//(std::string Filename, ST SegType, QQ* DataQ, bool UseAudio, int Interval,
			//AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
		return new FFSegmenter(Filename, SegType, i, UseAudio, Interval, UseTranscoding, VCo, ACo);
		break;

	case ST_FFHW:
		break;
	case ST_CV:
		break;
	case ST_NV:
		break;
	}

	return nullptr;
}

bool SegmenterGroup::CreateSeg() {

	if (*PathDir == nullptr) {
		return false;
	}

	int bitrate[SegmengerCount] = {1080};

	for (int i = 0; i < SegmengerCount; i++) {

		if (Seg[i] == nullptr) {
			//std::string tempFilename = std::string(std::to_string(bitrate[i])) + Filename.c_str();
			//std::string root = "\\\\";
			std::string tempFilename = PathDir[i] + Filename;
			Seg[i] = SetType(SegType, tempFilename, i);
			
			if (Seg[i] == nullptr) {

				CCommonInfo::GetInstance()->WriteLog("ERROR", "Error in create segmenter URL - %s", tempFilename.c_str());
				return false;
			}
			else {
				CCommonInfo::GetInstance()->WriteLog("INFO", "Success create segmenter URL - %s", tempFilename.c_str());
				continue;
			}
		}
		else {
			continue;
		}
	}


	return true;
}
//
//bool SegmenterGroup::ChangeRunningSeg(int Bitrate) {
//
//	int Targetindex = -1;
//	int Runningindex = -1;
//
//	if (Bitrate == 1080) {
//		Targetindex = 0;
//	}
//	else if (Bitrate == 720) {
//		Targetindex = 1;
//	}
//	else {
//		Targetindex = -1;
//		return false;
//	}
//
//	if (RunningSegIndex == Targetindex) {
//		return true;
//	}
//	else {
//		for (int i = 0; i < SegmengerCount; i++) {
//			
//			if (Seg[i]->Index == Targetindex) {
//				continue;
//			}
//			else {
//				Seg[i]->Stop();
//			}
//		}
//		RunningSegIndex = Targetindex;
//		Seg[RunningSegIndex]->Run();
//		return true;
//	}
//
//	return false;
//
//}

void SegmenterGroup::Notify(std::shared_ptr<MediaFrame> Frame) {

	//parallel_for(0, SegmengerCount, [&](int i) {
	//	if(Seg[i]->Running == true)
	//		Seg[i]->Run(Frame);
	//});

	Seg[1]->Run(Frame);
	//Seg[0]->Run(Frame);

	return;
}

void SegmenterGroup::DoWork(SegmenterGroup* SG) {
	
	SG->Running = true;

	SG->TimeCheckthr = std::thread([&]() { SG->TimeCheck(this); });
	while (SG->Running) {
		if (!(SG->DataQ->empty())) {

			if (SG->DataQ->size()) {
				auto Frame = std::shared_ptr<MediaFrame>(SG->DataQ->pop());

				SG->Notify(Frame);
				SG->CheckTime = clock();

				Frame.reset();

			}
		}
	}

	return;
}

void SegmenterGroup::TimeCheck(SegmenterGroup* SG) {

	int base_time = 1000 * 20;
	int Dif = 0;
	while (Running) {
		SG->loopTime = clock();
		Dif = (SG->CheckTime - SG->loopTime);

		if (Dif < 0)
			Dif = Dif*-1;

		if (Dif > base_time) {
			Running = false;
			std::cout << "Time Checkout - " << std::endl;
			return ;
		}
		Sleep(1);
	}
}

int SegmenterGroup::Run(std::string URL) {

	int Targetindex = ParseBitrateToIndex(URL);
	if (Targetindex < 0) {
		return -1;
	}

	int i = 0;
	while (i < SegmengerCount) {
		if (Running == true) {
			Seg[0]->Ref++;
			i++;
			return Seg[0]->Ref;
			//continue;
		}
		//DoWork(this);
		Nofitythr = std::thread([&]() { DoWork(this); });
		Seg[0]->Ref++;
		i++;
	}

	return Seg[0]->Ref;
}

int SegmenterGroup::Stop() {

	int i = 0;
	while (i < SegmengerCount) {
		Seg[i]->Ref--;
		if (Seg[i]->Ref > 0) {
			i++;
			continue;
		}
		Running = false;

		if (Nofitythr.joinable()) {
			if (TimeCheckthr.joinable()) {
				TimeCheckthr.join();
				Nofitythr.join();
			}
		}

		Seg[i]->Stop();
		i++;
	}

	return Seg[0]->Ref;
}


bool SegmenterGroup::SetPath(char** Path) {

	int count = 0;
	for (int i = 0; i < SegCount; i++) {
		PathDir[i] = *(Path + i);
		count++;
		std::cout << PathDir[i] << std::endl;
	}
	
	if (count == SegCount) {
	return true;
	}
	else {
		return false;
	}
}

int SegmenterGroup::ParseBitrateToIndex(std::string URL) {

	int TargetIndex = -1;

	if (URL.find("720") != std::string::npos) {
		TargetIndex = 1;
	}
	else if (URL.find("1080") != std::string::npos) {
		TargetIndex = 0;
	}
	else {
		TargetIndex = 0;
	}


	return TargetIndex;
}