#include "FFSegmenter.h"

FFSegmenter::FFSegmenter(bool usingtrans) :m_mux(nullptr), m_trs(nullptr), Use_Trans(usingtrans), ISegmenter(0) {



}

FFSegmenter::~FFSegmenter() {


	m_mux = nullptr;
	m_trs = nullptr;

}

int FFSegmenter::Init() {

	return 0;
}

int FFSegmenter::Run() {
	return 0;
}

int FFSegmenter::Close() {
	return 0;
}

int FFSegmenter::Standby() {
	return 0;
}

int FFSegmenter::Stop() {
	return 0;
}
