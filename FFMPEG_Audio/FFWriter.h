#pragma once


extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};


class FileWriter
{
public:
	FileWriter();
	virtual ~FileWriter();

	int openFile(char *filename, int codec, int width, int height);
	int writeFile(enum AVMediaType type, char *buf, int len);
	void closeFile();

protected:
	AVStream* addVideoStream(enum AVCodecID codec_id, int width, int height);
	int writeVideo(char *buf, int len);
	int writeAudio(char *buf, int len);

protected:
	AVFormatContext *m_pFormatContext;
	AVStream  *m_pVideoStream;
	AVPacket  m_avPacket;

	enum AVCodecID m_nCodecID;

	__int64 m_nFrameCount;
};
