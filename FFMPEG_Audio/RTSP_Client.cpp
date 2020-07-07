#include <Windows.h>
#include <chrono>
#include <thread>
#include <tchar.h>
#include <iostream>

extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavformat\avio.h>
#include <libswscale\swscale.h>
#include <libavutil\time.h>
#include <libavutil/opt.h>
}

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow) {
	FILE* cp = nullptr;
	AllocConsole();
	freopen_s(&cp, "CONOUT$", "wt", stdout);

	AVFormatContext* context = avformat_alloc_context();
	int video_stream_index;

	//av_register_all();
	//avcodec_register_all();
	avformat_network_init();

	//if(avformat_open_input(&context, "rtmp://192.168.0.40/vod/sample.mp4",NULL,NULL) != 0){      
	//if(avformat_open_input(&context, "d:\\windows\\b.mp4",NULL,NULL) != 0){

	//open rtsp
	if (avformat_open_input(&context, "rtsp://admin:1234@192.168.0.70:554/video1", NULL, NULL) != 0) {
		return EXIT_FAILURE;
	}

	if (avformat_find_stream_info(context, NULL) < 0) {
		return EXIT_FAILURE;
	}

	//search video stream
	int i;
	for (i = 0; i < context->nb_streams; i++) {
		if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			video_stream_index = i;
	}

	AVPacket packet;
	av_init_packet(&packet);

	//open output file
	AVOutputFormat* fmt = av_guess_format(NULL, "test2.avi", NULL);
	AVFormatContext* oc = avformat_alloc_context();
	oc->oformat = fmt;
	avio_open2(&oc->pb, "test2.avi", AVIO_FLAG_WRITE, NULL, NULL);

	AVStream* stream = NULL;
	int cnt = 0;
	//start reading packets from stream and write them to file

	av_read_play(context);//play RTSP
	while (av_read_frame(context, &packet) >= 0 && cnt < 100) {//read 100 frames
		if (packet.stream_index == video_stream_index) {//packet is video               
			if (stream == NULL) {//create stream in file
				stream = avformat_new_stream(oc, NULL);
				stream->sample_aspect_ratio = context->streams[video_stream_index]->codecpar->sample_aspect_ratio;
				avformat_write_header(oc, NULL);
			}
			packet.stream_index = stream->id;

			av_write_frame(oc, &packet);
			cnt++;
		}
		av_packet_unref(&packet);
		av_init_packet(&packet);
	}
	av_read_pause(context);
	av_write_trailer(oc);
	avio_close(oc->pb);
	avformat_free_context(oc);

	return (EXIT_SUCCESS);

}