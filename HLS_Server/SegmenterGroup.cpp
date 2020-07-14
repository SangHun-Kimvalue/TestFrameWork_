#include "SegmenterGroup.h"


SegmenterGroup::SegmenterGroup() : SegType(ST_NOT_DEFINE), UseAudio(false), UseTranscoding(false), Interval(5), VCo(AV_CODEC_ID_NONE), ACo(AV_CODEC_ID_NONE)
{

}

SegmenterGroup::SegmenterGroup(ST SegType, std::string Filename, bool UseAudio, bool UseTranscoding, int Interval, QQ* DataQ, AVCodecID VCo, AVCodecID ACo)
	: SegType(SegType), Filename(Filename), UseAudio(UseAudio), UseTranscoding(UseTranscoding), Interval(Interval), DataQ(DataQ), VCo(VCo), ACo(ACo)
{
	CreateSeg();
}

SegmenterGroup::~SegmenterGroup() {

	//if (Ref > 0) {
	//	while (Ref != 0) {
	//		Stop();
	//	}
	//}
}

ISegmenter* SegmenterGroup::SetType(ST SegType, std::string Filename) {

	int i = 0;

	switch (SegType) {
	case ST_FFSW:
	default:
		//(std::string Filename, ST SegType, QQ* DataQ, bool UseAudio, int Interval,
			//AVCodecID VCo, AVCodecID ACo = AV_CODEC_ID_NONE);
		return new FFSegmenter(Filename, SegType, DataQ , UseAudio, Interval, UseTranscoding, VCo, ACo);
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

	int bitrate[SegmengerCount] = {1080, 720};

	for (int i = 0; i < SegmengerCount; i++) {

		if (Seg[i] == nullptr) {
			std::string tempFilename = std::string(std::to_string(bitrate[i])) + Filename.c_str();
			Seg[i] = SetType(SegType, tempFilename);
			
			if (Seg[i] == nullptr) {

				CCommonInfo::GetInstance()->WriteLog("ERROR", "Error in create segmenter URL+index - ", tempFilename.c_str());
				return false;
			}
			else {
				CCommonInfo::GetInstance()->WriteLog("INFO", "Success in create segmenter URL+index - ", tempFilename.c_str());
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


int SegmenterGroup::Run(std::string URL) {

	int Targetindex = ParseBitrateToIndex(URL);
	if (Targetindex < 0) {
		return -1;
	}

	int i = 0;
	while (i < SegmengerCount) {
		if (Seg[i]->Running == true) {
			Seg[i]->Ref++;
			continue;
		}
		Seg[i]->Run();
		Seg[i]->Ref++;	
		i++;
	}

	return Seg[Targetindex]->Ref;
}

int SegmenterGroup::Stop(std::string URL) {

	int Targetindex = ParseBitrateToIndex(URL);
	if (Targetindex < 0) {
		Targetindex = 0;
	}

	int i = 0;
	while (i < SegmengerCount) {
		Seg[i]->Ref--;
		if (Seg[i]->Ref > 0) {
			i++;
			continue;
		}
		Seg[i]->Stop();
		i++;
	}

	return Seg[Targetindex]->Ref;
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