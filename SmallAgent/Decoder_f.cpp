#include "Decoder_f.h"

static enum AVPixelFormat hw_pix_fmt;

enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts){
	const enum AVPixelFormat *p;

	for (p = pix_fmts; *p != -1; p++) {
		if (*p == hw_pix_fmt)
		    return *p;
	}

	std::printf("\n\n\n Failed to get HW surface format. Not support Codec\n\n\n");
	return AV_PIX_FMT_NONE;
}

//enum AVHWDeviceType Decoder_f::HA_Select() {
//
//	enum AVHWDeviceType temp;
//	switch (HA_Usage) {
//	case 1:
//		temp = AV_HWDEVICE_TYPE_DXVA2;
//		break;
//	case 2:
//		temp = AV_HWDEVICE_TYPE_D3D11VA;
//		break;
//	case 3:
//		temp = AV_HWDEVICE_TYPE_CUDA;
//		break;
//	//case 4:
//	//	temp = AV_HWDEVICE_TYPE_VAAPI;
//	//	break;
//	default:
//		return AV_HWDEVICE_TYPE_NONE;
//	}
//
//	return temp;
//}

const bool Decoder_f::Open_HA(const string inputFile) {

	int result;
	enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;

	switch (HA_Usage)
	{
	case DXVA2:
		type = av_hwdevice_find_type_by_name("d3d11va");
		break;
	case CUVID:
		type = av_hwdevice_find_type_by_name("cuda");
		break;
	}

	if (type == AV_HWDEVICE_TYPE_NONE) {
		std::printf("\n\n\n\nHA_Select Error(HA_Select)\n\n\n\n");
		return false;
	}

	if (avformat_open_input(&pFormatCtx, inputFile.c_str(), NULL, NULL) != 0)
		return false;

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) 
		return false;

	VSI = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pVideoCodec, 0);
	if (VSI < 0) 
		return false;

	for (auto i = 0;; i++) {
		const AVCodecHWConfig *config = avcodec_get_hw_config(pVideoCodec, i);
		if (!config) {
			fprintf(stderr, "Decoder %s does not support device type %s.\n",
				pVideoCodec->name, av_hwdevice_get_type_name(type));
			return -1;
		}
		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
			config->device_type == type) {
			hw_pix_fmt = config->pix_fmt;
			break;
		}
	}

	if (!(pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec)))
		return AVERROR(ENOMEM);

	if (avcodec_parameters_to_context(pVideoCodecCtx, pFormatCtx->streams[VSI]->codecpar) < 0)
		return false;

	pVideoCodecCtx->get_format = get_hw_format;
	//av_opt_set_int(pVideoCodecCtx, "refcounted_frames", 1, 0); 

	if ((result = av_hwdevice_ctx_create(&hw_device_ctx, type,
		NULL, NULL, 0)) < 0) {
		fprintf(stderr, "Failed to create specified HW device.\n");
		return result;
	}

	pVideoCodecCtx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
	//pVideoCodecCtx->thread_count = 0;
	if ((result = avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL)) < 0) 
		return false;
	
	realFPS = av_q2d(pFormatCtx->streams[VSI]->r_frame_rate);
	
	float Megabitrate_f = ((pFormatCtx->bit_rate) / 1000000);
	float Megabitrate_l = ((pFormatCtx->bit_rate) % 1000000) / 100000;
	int temp = Megabitrate_l / 5;
	Megabitrate = (int)Megabitrate_f;
	if (temp == 1) 
		Megabitrate++;
	//string temp;
	//int index = 0;
	//temp = to_string(index) + " Open ready";
	//to_string(index);
	//log.LogFile(temp);

	return true;
}

const bool Decoder_f::Open(const string inputFile) {

	CloseFile();
	int result = 0;
	
	if (result = avformat_open_input(&pFormatCtx, inputFile.c_str(), NULL, NULL) != 0) {
		CloseFile();
		return false;
	}
	if (result = avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		CloseFile();
		return false;
	}

	//av_frame_get_best_effort_timestamp
	VSI = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (VSI < 0) {
		std::printf("Cannot find a video stream in the input file\n");
		return false;
	}

	//av_dump_format(pFormatCtx, 0, inputFile.c_str(), 0);
	pVideoCodec = avcodec_find_decoder(pFormatCtx->streams[VSI]->codecpar->codec_id);
	if (pVideoCodec == NULL) {
		av_log(NULL, AV_LOG_ERROR, "No Decoder\n");
		exit(-1);
	}

	pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
	if (avcodec_parameters_to_context(pVideoCodecCtx, pFormatCtx->streams[VSI]->codecpar) < 0) {
		std::printf("avcodec_parameters_to_context Error\n");
		exit(-1);
	}

	//pVideoCodecCtx->bit_rate = pFormatCtx->bit_rate;
	//pVideoCodecCtx->flags = AV_CODEC_FLAG_QSCALE;
	pVideoCodecCtx->thread_count = 0;
	if (avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL) < 0){
		av_log(NULL, AV_LOG_ERROR, "Fail to Initialize Decoder\n");
		exit(-1);
	}

	av_init_packet(&packet);
	realFPS = av_q2d(pFormatCtx->streams[VSI]->r_frame_rate);

	float Megabitrate_f = ((pFormatCtx->bit_rate) / 1000000);
	float Megabitrate_l = ((pFormatCtx->bit_rate) % 1000000) / 100000;
	int temp = Megabitrate_l / 5;
	Megabitrate = (int)Megabitrate_f;
	if (temp == 1)
		Megabitrate++;

	return true;
}

bool Decoder_f::CloseFile(){

	packet.data = NULL;
	packet.size = 0;

	if (hw_device_ctx) {
		av_buffer_unref(&hw_device_ctx);
		hw_device_ctx = NULL;
	}
	if (pVideoCodecCtx) {
		av_buffer_unref(&hw_device_ctx);
		avcodec_close(pVideoCodecCtx);
		avcodec_free_context(&pVideoCodecCtx);
		pVideoCodecCtx = NULL;

	}
	if (pVideoCodec) {
		pVideoCodec = NULL;
	}
	if (pFormatCtx) {
		avformat_close_input(&pFormatCtx);
		pFormatCtx = NULL;
	}

	return true;
}

AVFrame* Decoder_f::ReadPacket() {

	int  res = 0;				static int VSI_ERROR;
	int timecheck = 0;

	av_init_packet(&packet);

	if (d_end == true)
		return false;
	else if (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (VSI == packet.stream_index) {
			res = avcodec_send_packet(pVideoCodecCtx, &packet);
			if (res < 0) {
				if (res == AVERROR_EOF) 
					d_end = true;

				av_packet_unref(&packet);
				return false;
			}
		}
		else {
			VSI_ERROR++;
			av_packet_unref(&packet);
			return false;
		}
		if (HA_Usage != 0) {	//HA Decode		
			res = HA_Decode();
			if (res == true)		
				return pVFrame;
		}
		else {				//CPU Decode
			if (Decode() == true)
				return pVFrame;
		}
		av_packet_unref(&packet);
		return false;
	}
	else {					//EOF
		d_end = true;
		std::printf("\n\nDecode End\n\n");
	}
}

bool Decoder_f::HA_Decode() {

	int res = 0;		
	static int count;					
	static int VSI_ERROR;		
	AVFrame *frame = NULL;				
	
	count++;
	frame = av_frame_alloc();

	res = avcodec_receive_frame(pVideoCodecCtx, frame);				//리턴값 잘 들어오는걸로  봐서는 내가 뺴먹은게 있고.
	//receive_frame_end = clock();
	//std::printf("\n                  receive_frame time : %d\n", receive_frame_end - receive_frame_start);
	//if (res == AVERROR(EAGAIN)) {							//패킷을 모아 프레임을 만드는데 프레임을 만들수있는 패킷의 수가 부족함.
	//	std::printf("\nagain_count = %d\n", againcount++);
	//			return false;								//return -11 만 들어옴
	//}
	//else if (res == AVERROR_INVALIDDATA) {
	//	std::printf("receive_frame = VERROR_INVALIDDATA \n");
	//	return false;
	//}
	//if (res == AVERROR_EOF) {
	//	av_frame_free(&frame);
	//	av_frame_free(&pVFrame);
	//	return false;
	//}
	if (res < 0) {
		//std::printf("Error while receive_frame\n");
		av_frame_free(&frame);
		av_packet_unref(&packet);
		return false;
	}
	//if (pVideoCodecCtx->hwaccel == NULL) {
	//	d_end = true;
	//	std::printf("\n\ncodec not support.\n\n");
	//	av_frame_free(&frame);
	//	av_packet_unref(&packet);
	//	return false;
	//}
	if (frame->format == hw_pix_fmt) {
		pVFrame = av_frame_alloc();
		if ((res = av_hwframe_transfer_data(pVFrame, frame, 0)) >= 0) {
			av_frame_free(&frame);
			av_packet_unref(&packet);

			return true;
		}
		std::printf("Error transferring the data to system memory\n");

		av_frame_free(&frame);
		av_frame_free(&pVFrame);
		av_packet_unref(&packet);
	}

	return false;
}

int Decoder_f::Decode() {
	int res = 0;
	static int count;	

	pVFrame = av_frame_alloc();
	count++;
	if (res = avcodec_receive_frame(pVideoCodecCtx, pVFrame) != 0) {
		if (res == AVERROR(EAGAIN) || res == AVERROR(EINVAL) || res == AVERROR(ENOMEM) || res == AVERROR(EPERM)) {
			av_frame_free(&pVFrame);
			return -1;
		}
		else if (res == AVERROR_INVALIDDATA || res == AVERROR_BSF_NOT_FOUND) {
			std::printf("av_codec_send_packet = VERROR_INVALIDDATA \n");
			return false;
		}
		//printf("\n\nreceive frame errer\n\n");
		return false;
	}
	if (pVFrame->data[0] == NULL && pVFrame->data[3] == NULL)
		return -1;
	else {
		//pVFrame = Con_yuv_RGB(pVFrame);
		//RGB_frame = Con_yuv_YUV420P(RGB_frame);
		//pVFrame = Con_yuv_YUV420P(pVFrame);
		av_packet_unref(&packet);
		return 1;
	}
	return -1;
}

AVFrame* Decoder_f::Con_yuv_RGB(AVFrame* YUV_frame)
{
	AVFrame* RGBFrame = av_frame_alloc();

	RGBFrame->width = YUV_frame->width;		RGBFrame->format = AV_PIX_FMT_RGBA;
	RGBFrame->height = YUV_frame->height;

	int ret = av_image_alloc(RGBFrame->data, RGBFrame->linesize, RGBFrame->width, RGBFrame->height, AV_PIX_FMT_RGBA, YUV_frame->pict_type);
	if (ret < 0)
		return false;
	enum AVPixelFormat tempformat;
	//tempformat = (enum AVPixelFormat)pVideoCodecCtx->pix_fmt;
	tempformat = (enum AVPixelFormat)YUV_frame->format;// YUV_frame->format;
	ret = av_frame_get_buffer(RGBFrame, 32);
	SwsContext* sws_Context = NULL;
	sws_Context = sws_getCachedContext(sws_Context, YUV_frame->width, YUV_frame->height, tempformat,// pVideoCodecCtx->pix_fmt,//AV_PIX_FMT_YUV420P,
		YUV_frame->width, YUV_frame->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
	if (sws_Context == NULL)
		return false;
	//clock_t sws_scals_start = clock();
	int result = sws_scale(sws_Context, YUV_frame->data, YUV_frame->linesize, 0, (int)YUV_frame->height, RGBFrame->data, RGBFrame->linesize);
	if (result < 0)
		return false;
	//clock_t sws_scals_end = clock();
	//std::printf("\t\t sws_scale time : %d\n", sws_scals_end - sws_scals_start);
	//CHAR file[100];
	//sprintf_s(file, "test1.bmp");
	//SaveBMP2(file, RGBFrame->data[0], RGBFrame->width, RGBFrame->height);
	if (RGBFrame == NULL) {
		av_frame_unref(RGBFrame);
		return false;
	}

	sws_freeContext(sws_Context);

	return RGBFrame;

}

AVFrame* Decoder_f::Con_yuv_YUV420P(AVFrame* YUV_frame)
{
	AVFrame* YUV420Frame = av_frame_alloc();
	
	YUV420Frame->width = YUV_frame->width;		YUV420Frame->format = AV_PIX_FMT_YUV420P;
	YUV420Frame->height = YUV_frame->height;	
	
	int ret = av_image_alloc(YUV420Frame->data, YUV420Frame->linesize, YUV420Frame->width, YUV420Frame->height, AV_PIX_FMT_YUV420P, YUV_frame->pict_type);
	if (ret < 0)
		return false;

	SwsContext* sws_Context = NULL;
	enum AVPixelFormat tempformat;
	//tempformat = (enum AVPixelFormat)pVideoCodecCtx->pix_fmt;// YUV_frame->format;
	tempformat = (enum AVPixelFormat)YUV_frame->format;// YUV_frame->format;
	ret = av_frame_get_buffer(YUV420Frame, 12);

	sws_Context = sws_getCachedContext(sws_Context, YUV_frame->width, YUV_frame->height, tempformat,
			YUV_frame->width, YUV_frame->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
	if (sws_Context == NULL)
		return false;
	
	clock_t sws_scals_start = clock();
	int result = sws_scale(sws_Context, YUV_frame->data, YUV_frame->linesize, 0, (int)YUV_frame->height, YUV420Frame->data, YUV420Frame->linesize);
	if (result < 0)		
		return false;
	clock_t sws_scals_end = clock();
	std::printf("\t\t sws_scale time : %d\n", sws_scals_end - sws_scals_start);

	if (YUV420Frame == NULL) {
		av_frame_unref(YUV420Frame);
		return false;
	}

	//if (YUV_frame->format == 23) {
	//	YUV420Frame->linesize[0] = YUV_frame->linesize[0];
	//	YUV420Frame->linesize[1] = YUV_frame->linesize[0] / 2;
	//	YUV420Frame->linesize[2] = YUV_frame->linesize[0] / 2;
	//}

	//av_freep(&YUV_frame->data[0]);
	sws_freeContext(sws_Context);
	av_frame_unref(YUV_frame);

	return YUV420Frame;

}

bool Decoder_f::SaveBMP2(char* filename, byte* pImage, int width, int height)
{
	// DIB의 형식을 정의한다.
	BITMAPINFO dib_define;
	dib_define.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib_define.bmiHeader.biWidth = width;
	dib_define.bmiHeader.biHeight = height;
	dib_define.bmiHeader.biPlanes = 1;
	dib_define.bmiHeader.biBitCount = 32;
	dib_define.bmiHeader.biCompression = BI_RGB;
	dib_define.bmiHeader.biSizeImage = width * height * 4;
	dib_define.bmiHeader.biXPelsPerMeter = 0x0ec4;
	dib_define.bmiHeader.biYPelsPerMeter = 0x0ec4;
	dib_define.bmiHeader.biClrImportant = 0;
	dib_define.bmiHeader.biClrUsed = 0;

	// DIB 파일의 헤더 내용을 구성한다.
	BITMAPFILEHEADER dib_format_layout;
	ZeroMemory(&dib_format_layout, sizeof(BITMAPFILEHEADER));
	dib_format_layout.bfType = *(WORD*)"BM";
	dib_format_layout.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dib_define.bmiHeader.biSizeImage;
	dib_format_layout.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// 원본 파일을 뒤집는다.
	std::unique_ptr<BYTE> pBuf(new BYTE[dib_define.bmiHeader.biSizeImage]);
	UINT lBmpRowPitch = width * 4;

	BYTE* sptr = reinterpret_cast<BYTE*>(pImage);
	BYTE* dptr = pBuf.get() + dib_define.bmiHeader.biSizeImage - lBmpRowPitch;

	UINT lRowPitch = lBmpRowPitch;

	for (size_t h = 0; h < height; ++h)
	{
		memcpy_s(dptr, lBmpRowPitch, sptr, lRowPitch);				//메모리 카피하는건 OK. 근데 어디서 BGR로 넣는거야?
		sptr += lRowPitch;
		dptr -= lBmpRowPitch;
	}

	// 파일 생성.
	FILE* fp = nullptr;
	fopen_s(&fp, filename, "wb");
	if (nullptr == fp){
		return false;
	}

	// 생성 후 헤더 및 데이터 쓰기.
	fwrite(&dib_format_layout, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&dib_define, sizeof(BITMAPINFOHEADER), 1, fp);
	fwrite(pBuf.get(), dib_define.bmiHeader.biSizeImage, 1, fp);
	fclose(fp);

	return true;
}

