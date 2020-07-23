#pragma once
#include "IDecoder.h"
#include "FFDecoder.h"

class DecoderFactory {
public:
	static IDecoder* CreateDecoder(DT Type) {

		switch (Type) {
		case DT_SW_FFMPEG:
			return new FFmpegDecoder();
			break;
		case DT_HW_FFMPEG:
			break;
		case DT_HW_NVDEC:
			break;
		case DT_HW_INTEL:
			break;
		case DT_DECODE_NONE:
		default:
			return nullptr;
			break;
		}

		return nullptr;
	}
};