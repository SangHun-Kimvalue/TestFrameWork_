#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ISegmenter.hpp"
#include "Muxer.h"
#include "Transcode.h"

class FFSegmenter : public ISegmenter
{

public:

	FFSegmenter(bool usingtrans);
	FFSegmenter(int index, bool usingtrans) : ISegmenter(index), Use_Trans(usingtrans) {}
	~FFSegmenter();

	int Init();
	int Run();
	int Close();
	int Standby();
	int Stop();

private:

	Muxer* m_mux;
	Transcoder* m_trs;
	const bool Use_Trans;

};

