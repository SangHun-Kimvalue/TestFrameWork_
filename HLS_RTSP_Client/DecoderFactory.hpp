#pragma once
#include "IDecoder.h"
#include "FFDecoder.h"

class DecoderFactory {
public:
	static IDecoder* CreateDecoder(DT Type) {

		switch (Type) {
		case SW_FFMPEG:
			return new FFmpegDecoder();
			break;
		case HW_FFMPEG:
			break;
		case HW_NVDEC:
			break;
		case HW_INTEL:
			break;
		case DECODE_NONE:
		default:
			return nullptr;
			break;
		}

		return nullptr;
	}
};