/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#define _WIN32_WINNT _WIN32_WINNT_XP
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <stringapiset.h>

#define WC_ERR_INVALID_CHARS 0x00000080

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
}
#undef exit
//
////* 5 seconds stream duration */
//#define STREAM_DURATION   5.0
//#define STREAM_FRAME_RATE 25 /* 25 images/s */
//#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
//#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P /* default pix_fmt */
//
//static int sws_flags = SWS_BICUBIC;
//
////**************************************************************/
////* audio output */
//
//float t, tincr, tincr2;
//int16_t *samples;
//uint8_t *audio_outbuf;
//int audio_outbuf_size;
//int audio_input_frame_size;
//AVCodecContext *c;
//
////*
// * add an audio output stream
// */
//static AVStream *add_audio_stream(AVFormatContext *oc, enum AVCodecID codec_id)
//{
//    //AVCodecContext *c;
//
//	AVStream * st = avformat_new_stream(oc, NULL);
//	st->id = codec_id;
//
//    if (!st) {
//        fprintf(stderr, "Could not alloc stream\n");
//        exit(1);
//    }
//
//    c = st->codec;
//    c->codec_id = codec_id;
//    c->codec_type = AVMEDIA_TYPE_AUDIO;
//
//    /* put sample parameters */
//    c->sample_fmt = AV_SAMPLE_FMT_S16;
//    c->bit_rate = 64000;
//    c->sample_rate = 44100;
//    c->channels = 2;
//
//    // some formats want stream headers to be separate
//    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
//        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//
//    return st;
//}
//
//static void open_audio(AVFormatContext *oc, AVStream *st)
//{
//    //AVCodecContext *c;
//    AVCodec *codec;
//
//    c = st->codec;
//
//    /* find the audio encoder */
//    codec = avcodec_find_encoder(c->codec_id);
//    if (!codec) {
//        fprintf(stderr, "codec not found\n");
//        exit(1);
//    }
//
//    /* open it */
//    if (avcodec_open2(c, codec, NULL) < 0) {
//        fprintf(stderr, "could not open codec\n");
//        exit(1);
//    }
//
//    /* init signal generator */
//    t = 0;
//    tincr = 2 * M_PI * 110.0 / c->sample_rate;
//    /* increment frequency by 110 Hz per second */
//    tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;
//
//    audio_outbuf_size = 10000;
//    audio_outbuf = (uint8_t*)av_malloc(audio_outbuf_size);
//
//    /* ugly hack for PCM codecs (will be removed ASAP with new PCM
//       support to compute the input frame size in samples */
//    if (c->frame_size <= 1) {
//        audio_input_frame_size = audio_outbuf_size / c->channels;
//        switch(st->codec->codec_id) {
//        case AV_CODEC_ID_PCM_S16LE:
//        case AV_CODEC_ID_PCM_S16BE:
//        case AV_CODEC_ID_PCM_U16LE:
//        case AV_CODEC_ID_PCM_U16BE:
//            audio_input_frame_size >>= 1;
//            break;
//        default:
//            break;
//        }
//    } else {
//        audio_input_frame_size = c->frame_size;
//    }
//    samples = (int16_t*)av_malloc(audio_input_frame_size * 2 * c->channels);
//}
//
////* prepare a 16 bit dummy audio frame of 'frame_size' samples and
//   'nb_channels' channels */
//static void get_audio_frame(int16_t *samples, int frame_size, int nb_channels)
//{
//    int j, i, v;
//    int16_t *q;
//
//    q = samples;
//    for(j=0;j<frame_size;j++) {
//        v = (int)(sin(t) * 10000);
//        for(i = 0; i < nb_channels; i++)
//            *q++ = v;
//        t += tincr;
//        tincr += tincr2;
//    }
//}
//
//static void write_audio_frame(AVFormatContext *oc, AVStream *st)
//{
//    //AVCodecContext *c;
//    AVPacket pkt;
//    av_init_packet(&pkt);
//	int error = 0;
//    c = st->codec;
//	
//    get_audio_frame(samples, audio_input_frame_size, c->channels);
//	AVFrame* m_pAFrame = av_frame_alloc();
//	char errstr[256] = { 0, };
//	
//
//    //pkt.size= avcodec_encode_audio2(c, audio_outbuf, audio_outbuf_size, samples);
//    //if (c->coded_frame && c->coded_frame->pts != AV_NOPTS_VALUE)
//    //    pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
//    //pkt.flags |= AV_PKT_FLAG_KEY;
//    //pkt.stream_index= st->index;
//    //pkt.data= audio_outbuf;
//
//	error = avcodec_receive_packet(c, &pkt);
//	if (error != 0) {
//
//		av_strerror(error, errstr, 256);
//	}
//
//	error = avcodec_send_frame(c, m_pAFrame);
//	if (error != 0) {
//
//		av_strerror(error, errstr, 256);
//	}
//
//    /* write the compressed frame in the media file */
//    if (av_interleaved_write_frame(oc, &pkt) != 0) {
//        fprintf(stderr, "Error while writing audio frame\n");
//        exit(1);
//    }
//}
//
//static void close_audio(AVFormatContext *oc, AVStream *st)
//{
//    avcodec_close(st->codec);
//
//    av_free(samples);
//    av_free(audio_outbuf);
//}
//
////**************************************************************/
////* video output */
//
//AVFrame *picture, *tmp_picture;
//uint8_t *video_outbuf;
//int frame_count, video_outbuf_size;
//
////* add a video output stream */
//static AVStream *add_video_stream(AVFormatContext *oc, enum AVCodecID codec_id)
//{
//
//
//	AVStream * st = avformat_new_stream(oc, NULL);
//	st->id = codec_id;
//    if (!st) {
//        fprintf(stderr, "Could not alloc stream\n");
//        exit(1);
//    }
//
//    c = st->codec;
//    c->codec_id = codec_id;
//    c->codec_type = AVMEDIA_TYPE_VIDEO;
//
//    /* put sample parameters */
//    c->bit_rate = 400000;
//    /* resolution must be a multiple of two */
//    c->width = 352;
//    c->height = 288;
//    /* time base: this is the fundamental unit of time (in seconds) in terms
//       of which frame timestamps are represented. for fixed-fps content,
//       timebase should be 1/framerate and timestamp increments should be
//       identically 1. */
//    c->time_base.den = STREAM_FRAME_RATE;
//    c->time_base.num = 1;
//    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
//    c->pix_fmt = STREAM_PIX_FMT;
//    if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
//        /* just for testing, we also add B frames */
//        c->max_b_frames = 2;
//    }
//    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO){
//        /* Needed to avoid using macroblocks in which some coeffs overflow.
//           This does not happen with normal video, it just happens here as
//           the motion of the chroma plane does not match the luma plane. */
//        c->mb_decision=2;
//    }
//    // some formats want stream headers to be separate
//    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
//        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//
//    return st;
//}
//
//static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
//{
//    AVFrame *picture;
//    uint8_t *picture_buf;
//    int size;
//	int error = 0;
//
//    picture = av_frame_alloc();
//    if (!picture)
//        return NULL;
//	size = av_image_get_buffer_size(c->pix_fmt, width, height, *c->slice_offset);
//    //size = avpicture_get_size(pix_fmt, width, height);
//    picture_buf = (uint8_t*)av_malloc(size);
//    if (!picture_buf) {
//        av_free(picture);
//        return NULL;
//    }
//	error = av_image_fill_arrays(picture->data, picture->linesize, picture_buf,
//		c->pix_fmt, width, height, *picture->linesize);
//	if (error) {
//
//		return NULL;
//	}
//    return picture;
//}
//
//static void open_video(AVFormatContext *oc, AVStream *st)
//{
//    AVCodec *codec;
//    //AVCodecContext *c;
//
//    c = st->codec;
//
//    /* find the video encoder */
//    codec = avcodec_find_encoder(c->codec_id);
//    if (!codec) {
//        fprintf(stderr, "codec not found\n");
//        exit(1);
//    }
//
//    /* open the codec */
//    if (avcodec_open2(c, codec, NULL) < 0) {
//        fprintf(stderr, "could not open codec\n");
//        exit(1);
//    }
//
//    video_outbuf = NULL;
//    if (!(oc->oformat->flags & AVFMT_GLOBALHEADER)) {
//        /* allocate output buffer */
//        /* XXX: API change will be done */
//        /* buffers passed into lav* can be allocated any way you prefer,
//           as long as they're aligned enough for the architecture, and
//           they're freed appropriately (such as using av_free for buffers
//           allocated with av_malloc) */
//        video_outbuf_size = 200000;
//        video_outbuf = (uint8_t*)av_malloc(video_outbuf_size);
//    }
//
//    /* allocate the encoded raw picture */
//    picture = alloc_picture(c->pix_fmt, c->width, c->height);
//    if (!picture) {
//        fprintf(stderr, "Could not allocate picture\n");
//        exit(1);
//    }
//
//    /* if the output format is not YUV420P, then a temporary YUV420P
//       picture is needed too. It is then converted to the required
//       output format */
//    tmp_picture = NULL;
//    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
//        tmp_picture = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
//        if (!tmp_picture) {
//            fprintf(stderr, "Could not allocate temporary picture\n");
//            exit(1);
//        }
//    }
//}
//
////* prepare a dummy image */
//static void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
//{
//    int x, y, i;
//
//    i = frame_index;
//
//    /* Y */
//    for(y=0;y<height;y++) {
//        for(x=0;x<width;x++) {
//            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;
//        }
//    }
//
//    /* Cb and Cr */
//    for(y=0;y<height/2;y++) {
//        for(x=0;x<width/2;x++) {
//            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
//            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
//        }
//    }
//}
//
//static void write_video_frame(AVFormatContext *oc, AVStream *st)
//{
//    int out_size, ret;
//    //AVCodecContext *c;
//    static struct SwsContext *img_convert_ctx;
//	int error = 0;
//	char errstr[256];
//    c = st->codec;
//
//    if (frame_count >= STREAM_NB_FRAMES) {
//        /* no more frame to compress. The codec has a latency of a few
//           frames if using B frames, so we get the last frames by
//           passing the same picture again */
//    } else {
//        if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
//            /* as we only generate a YUV420P picture, we must convert it
//               to the codec pixel format if needed */
//            if (img_convert_ctx == NULL) {
//                img_convert_ctx = sws_getContext(c->width, c->height,
//					AV_PIX_FMT_YUV420P,
//                                                 c->width, c->height,
//                                                 c->pix_fmt,
//                                                 sws_flags, NULL, NULL, NULL);
//                if (img_convert_ctx == NULL) {
//                    fprintf(stderr, "Cannot initialize the conversion context\n");
//                    exit(1);
//                }
//            }
//            fill_yuv_image(tmp_picture, frame_count, c->width, c->height);
//            sws_scale(img_convert_ctx, tmp_picture->data, tmp_picture->linesize,
//                      0, c->height, picture->data, picture->linesize);
//        } else {
//            fill_yuv_image(picture, frame_count, c->width, c->height);
//        }
//    }
//
//	AVFMT_GLOBALHEADER;
//    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
//        /* raw video case. The API will change slightly in the near
//           futur for that */
//        AVPacket pkt;
//        av_init_packet(&pkt);
//
//        pkt.flags |= AV_PKT_FLAG_KEY;
//        pkt.stream_index= st->index;
//        pkt.data= (uint8_t *)picture;
//        pkt.size= sizeof(AVPicture);
//
//        ret = av_interleaved_write_frame(oc, &pkt);
//    } else {
//        /* encode the image */
//        //out_size = avcodec_encode_video2(c, video_outbuf, video_outbuf_size, picture);
//		AVPacket pkt;
//		av_init_packet(&pkt);
//		error = avcodec_receive_packet(c, &pkt);
//		if (error != 0) {
//
//			av_strerror(error, errstr, 256);
//		}
//
//		error = avcodec_send_frame(c, m_pAFrame);
//		if (error != 0) {
//
//			av_strerror(error, errstr, 256);
//		}
//        /* if zero size, it means the image was buffered */
//        if (out_size > 0) {
//            AVPacket pkt;
//            av_init_packet(&pkt);
//
//            if (c->coded_frame->pts != AV_NOPTS_VALUE)
//                pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
//            if(c->coded_frame->key_frame)
//                pkt.flags |= AV_PKT_FLAG_KEY;
//            pkt.stream_index= st->index;
//            pkt.data= video_outbuf;
//            pkt.size= out_size;
//
//            /* write the compressed frame in the media file */
//            ret = av_interleaved_write_frame(oc, &pkt);
//        } else {
//            ret = 0;
//        }
//    }
//    if (ret != 0) {
//        fprintf(stderr, "Error while writing video frame\n");
//        exit(1);
//    }
//    frame_count++;
//}
//
//static void close_video(AVFormatContext *oc, AVStream *st)
//{
//    avcodec_close(st->codec);
//    av_free(picture->data[0]);
//    av_free(picture);
//    if (tmp_picture) {
//        av_free(tmp_picture->data[0]);
//        av_free(tmp_picture);
//    }
//    av_free(video_outbuf);
//}
//
////**************************************************************/
////* media file output */
//
//int main(int argc, char **argv)
//{
//    const char *filename;
//    AVOutputFormat *fmt;
//    AVFormatContext *oc;
//    AVStream *audio_st, *video_st;
//    double audio_pts, video_pts;
//    int i;
//
//    /* initialize libavcodec, and register all codecs and formats */
//    av_register_all();
//
//    if (argc != 2) {
//        printf("usage: %s output_file\n"
//               "API example program to output a media file with libavformat.\n"
//               "The output format is automatically guessed according to the file extension.\n"
//               "Raw images can also be output by using '%%d' in the filename\n"
//               "\n", argv[0]);
//        exit(1);
//    }
//
//    filename = argv[1];
//
//    /* allocate the output media context */
//    avformat_alloc_output_context2(&oc, NULL, NULL, filename);
//    if (!oc) {
//        printf("Could not deduce output format from file extension: using MPEG.\n");
//        avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
//    }
//    if (!oc) {
//        exit(1);
//    }
//    fmt= oc->oformat;
//
//    /* add the audio and video streams using the default format codecs
//       and initialize the codecs */
//    video_st = NULL;
//    audio_st = NULL;
//    if (fmt->video_codec != AV_CODEC_ID_NONE) {
//        video_st = add_video_stream(oc, fmt->video_codec);
//    }
//    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
//        audio_st = add_audio_stream(oc, fmt->audio_codec);
//    }
//
//    av_dump_format(oc, 0, filename, 1);
//
//    /* now that all the parameters are set, we can open the audio and
//       video codecs and allocate the necessary encode buffers */
//    if (video_st)
//        open_video(oc, video_st);
//    if (audio_st)
//        open_audio(oc, audio_st);
//
//    /* open the output file, if needed */
//    if (!(fmt->flags & AVFMT_NOFILE)) {
//        if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
//            fprintf(stderr, "Could not open '%s'\n", filename);
//            exit(1);
//        }
//    }
//
//    /* write the stream header, if any */
//    avformat_write_header(oc, NULL);
//
//    for(;;) {
//        /* compute current audio and video time */
//		if (audio_st)
//			audio_pts = (double)audio_st->time_base.;
//		//pts.val * audio_st->time_base.num / audio_st->time_base.den;
//        else
//            audio_pts = 0.0;
//
//        if (video_st)
//            video_pts = (double)video_st->pts.val * video_st->time_base.num / video_st->time_base.den;
//        else
//            video_pts = 0.0;
//
//        if ((!audio_st || audio_pts >= STREAM_DURATION) &&
//            (!video_st || video_pts >= STREAM_DURATION))
//            break;
//
//        /* write interleaved audio and video frames */
//        if (!video_st || (video_st && audio_st && audio_pts < video_pts)) {
//            write_audio_frame(oc, audio_st);
//        } else {
//            write_video_frame(oc, video_st);
//        }
//    }
//
//    /* write the trailer, if any.  the trailer must be written
//     * before you close the CodecContexts open when you wrote the
//     * header; otherwise write_trailer may try to use memory that
//     * was freed on av_codec_close() */
//    av_write_trailer(oc);
//
//    /* close each codec */
//    if (video_st)
//        close_video(oc, video_st);
//    if (audio_st)
//        close_audio(oc, audio_st);
//
//    /* free the streams */
//    for(i = 0; i < oc->nb_streams; i++) {
//        av_freep(&oc->streams[i]->codec);
//        av_freep(&oc->streams[i]);
//    }
//
//    if (!(fmt->flags & AVFMT_NOFILE)) {
//        /* close the output file */
//        avio_close(oc->pb);
//    }
//
//    /* free the stream */
//    av_free(oc);
//
//    return 0;
//}

//AVDictionary* options = NULL;
//AVFormatContext * format_context = NULL;
//AVPacket pkt, outpkt;
//AVOutputFormat *outfmt = NULL;
//AVFormatContext* m_outformat = NULL;
//AVStream* m_out_aud_strm = NULL;
//int ASI = 0;
//AVStream* m_in_aud_strm = NULL;
//double m_in_end_time;
//int ret = 0;
//char errbuf[64];
//int error = 0;
//AVCodecContext* codec_context = NULL;
//
//
//DWORD convert_unicode_to_utf8_string(__out std::string& utf8, __in const wchar_t* unicode, __in const size_t unicode_size) {
//	DWORD error = 0;
//	do {
//		if ((nullptr == unicode) || (0 == unicode_size)) { error = ERROR_INVALID_PARAMETER; break; } utf8.clear();
//		// // getting required cch. // 
//		int required_cch = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, unicode, static_cast<int>(unicode_size), nullptr, 0, nullptr, nullptr);
//		if (0 == required_cch)
//		{
//			error = ::GetLastError();
//			break;
//		} // // allocate. // 
//		utf8.resize(required_cch); // // convert. // 
//		if (0 == ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, unicode, static_cast<int>(unicode_size),
//			const_cast<char*>(utf8.c_str()), static_cast<int>(utf8.size()), nullptr, nullptr))
//		{
//			error = ::GetLastError();
//			break;
//		}
//	} while (false);
//	return error;
//}
//
//int Init(const char* filename)
//{
//	avdevice_register_all();
//	av_dict_set(&options, "list_devices", "true", 0);
//	format_context = avformat_alloc_context();
//	AVInputFormat *iformat = av_find_input_format("dshow");
//	if (iformat == NULL) {
//		printf("iformat is NULL\n");
//	}
//
//	//error = avformat_open_input(&format_context, "video=dummy:audio=마이크 배열(Realtek High Definition Audio)", iformat, &options);
//	//av_strerror(error, errbuf, sizeof(errbuf));
//	//std::cout << "avformat_open_input Error : " << errbuf << std::endl;
//
//	std::string conv;
//	std::wstring wchar = L"audio=마이크 배열(Realtek High Definition Audio)";
//	int size = wchar.size();
//	convert_unicode_to_utf8_string(conv, wchar.c_str(), size);
//	//std::string filename = "audio=마이크 배열(Realtek High Definition Audio)";
//
//	error = avformat_open_input(&format_context, conv.c_str(), iformat, NULL);
//	av_strerror(error, errbuf, sizeof(errbuf));
//	std::cout << conv.c_str() << std::endl;
//	std::cout << "avformat_open_input Error : " << errbuf << std::endl;
//
//	if (error != 0) {
//		//av_strerror(error, errbuf, sizeof(errbuf));
//		//std::cout << errbuf << std::endl;
//		printf("Couldn't open input stream.\n");
//		//return -1;
//	}
//
//	if ((error = avformat_find_stream_info(format_context, 0)) < 0)
//	{
//
//		av_strerror(ret, errbuf, sizeof(errbuf));
//		//PRINT_VAL("Not Able to find stream info:: ", errbuf)
//		//	ret = -1;
//		return ret;
//	}
//
//	
//	for (unsigned int i = 0; i < format_context->nb_streams; i++)
//	{
//		if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
//		{
//			return -1;
//			//PRINT_MSG("Found Video Stream ")
//			//	m_in_vid_strm_idx = i;
//			//m_in_vid_strm = m_informat->streams[i];
//		}
//
//		if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
//		{
//			//PRINT_MSG("Found Audio Stream ")
//			ASI = i;
//			m_in_aud_strm = format_context->streams[i];
//			m_in_aud_strm->duration = format_context->duration = 10 * (AV_TIME_BASE);
//			//format_context->duration = 10 * (AV_TIME_BASE);
//		}
//	}
//
//	if (ASI == -1)
//	{
//		ret = -1;
//	}
//
//	if (format_context->duration == AV_NOPTS_VALUE)
//	{
//		//if (m_in_vid_strm_idx != -1 && format_context->streams[m_in_vid_strm_idx])
//		//{
//		//	if (m_informat->streams[m_in_vid_strm_idx]->duration != AV_NOPTS_VALUE)
//		//	{
//		//		//m_in_end_time = (m_informat->streams[m_in_vid_strm_idx]->duration)/(AV_TIME_BASE);
//		//		m_in_end_time = (m_informat->streams[m_in_vid_strm_idx]->duration) / (m_informat->streams[m_in_vid_strm_idx]->time_base.den / m_informat->streams[m_in_vid_strm_idx]->time_base.num);
//		//
//		//	}
//		//
//		//}
//		if (ASI != -1 && format_context->streams[ASI])
//		{
//			if (format_context->streams[ASI]->duration != AV_NOPTS_VALUE)
//			{
//				m_in_end_time = (format_context->streams[ASI]->duration) / (AV_TIME_BASE);
//			}
//		}
//	}
//	else
//	{
//		m_in_end_time = (format_context->duration) / (AV_TIME_BASE);
//		//m_in_end_time = 10;
//	}
//
//	//if (m_in_vid_strm_idx != -1 && m_informat->streams[m_in_vid_strm_idx])
//	//{
//	//	if (m_informat->streams[m_in_vid_strm_idx]->r_frame_rate.num != AV_NOPTS_VALUE && m_informat->streams[m_in_vid_strm_idx]->r_frame_rate.den != 0)
//	//	{
//	//		m_fps = (m_informat->streams[m_in_vid_strm_idx]->r_frame_rate.num) / (m_informat->streams[m_in_vid_strm_idx]->r_frame_rate.den);
//	//	}
//	//}
//	//else
//	//{
//	//	m_fps = 25;
//	//}
//	
//	std::string outfile = std::string(filename) + "clip_out.mp4";
//	outfmt = av_guess_format(NULL, outfile.c_str(), NULL);
//
//	if (outfmt == NULL)
//	{
//		ret = -1;
//		return ret;
//	}
//	else
//	{
//		m_outformat = avformat_alloc_context();
//		if (m_outformat)
//		{
//			m_outformat->oformat = outfmt;
//			m_outformat->audio_codec_id = outfmt->audio_codec;
//			m_outformat->url = const_cast<char*>(outfile.c_str());
//			//_snprintf_s(,);
//			//_snprintf_s(m_outformat->url, sizeof(m_outformat->url), 1024, const_cast<char*>(outfile.c_str()));
//		}
//		else
//		{
//			ret = -1;
//			return ret;
//		}
//	}
//
//	AVCodec /**out_vid_codec; , */*out_aud_codec;
//	/*out_vid_codec = */out_aud_codec = NULL;
//
//	//if (outfmt->video_codec != AV_CODEC_ID_NONE && m_in_vid_strm != NULL)
//	//{
//	//	out_vid_codec = avcodec_find_encoder(outfmt->video_codec);
//	//	if (NULL == out_vid_codec)
//	//	{
//	//		PRINT_MSG("Could Not Find Vid Encoder")
//	//			ret = -1;
//	//		return ret;
//	//	}
//	//	else
//	//	{
//	//		PRINT_MSG("Found Out Vid Encoder ")
//	//			m_out_vid_strm = avformat_new_stream(m_outformat, out_vid_codec);
//	//		if (NULL == m_out_vid_strm)
//	//		{
//	//			PRINT_MSG("Failed to Allocate Output Vid Strm ")
//	//				ret = -1;
//	//			return ret;
//	//		}
//	//		else
//	//		{
//	//			PRINT_MSG("Allocated Video Stream ")
//	//				if (avcodec_copy_context(m_out_vid_strm->codec, m_informat->streams[m_in_vid_strm_idx]->codec) != 0)
//	//				{
//	//					PRINT_MSG("Failed to Copy Context ")
//	//						ret = -1;
//	//					return ret;
//	//				}
//	//				else
//	//				{
//	//					m_out_vid_strm->sample_aspect_ratio.den = m_out_vid_strm->codec->sample_aspect_ratio.den;
//	//					m_out_vid_strm->sample_aspect_ratio.num = m_in_vid_strm->codec->sample_aspect_ratio.num;
//	//					PRINT_MSG("Copied Context ")
//	//						m_out_vid_strm->codec->codec_id = m_in_vid_strm->codec->codec_id;
//	//					m_out_vid_strm->codec->time_base.num = 1;
//	//					m_out_vid_strm->codec->time_base.den = m_fps * (m_in_vid_strm->codec->ticks_per_frame);
//	//					m_out_vid_strm->time_base.num = 1;
//	//					m_out_vid_strm->time_base.den = 1000;
//	//					m_out_vid_strm->r_frame_rate.num = m_fps;
//	//					m_out_vid_strm->r_frame_rate.den = 1;
//	//					m_out_vid_strm->avg_frame_rate.den = 1;
//	//					m_out_vid_strm->avg_frame_rate.num = m_fps;
//	//					m_out_vid_strm->duration = (m_out_end_time - m_out_start_time) * 1000;
//	//				}
//	//		}
//	//	}
//	//}
//
//	if (outfmt->audio_codec != AV_CODEC_ID_NONE && m_in_aud_strm != NULL)
//	{
//		out_aud_codec = avcodec_find_encoder(outfmt->audio_codec);
//		if (NULL == out_aud_codec)
//		{
//			//PRINT_MSG("Could Not Find Out Aud Encoder ")
//			//	ret = -1;
//			return ret;
//		}
//		else
//		{
//			//PRINT_MSG("Found Out Aud Encoder ")
//				m_out_aud_strm = avformat_new_stream(m_outformat, out_aud_codec);
//			if (NULL == m_out_aud_strm)
//			{
//				//PRINT_MSG("Failed to Allocate Out Vid Strm ")
//				//	ret = -1;
//				return ret;
//			}
//			else
//			{
//				codec_context = avcodec_alloc_context3(out_aud_codec);
//				if (codec_context == NULL) {
//
//					return -1;
//				}
//
//
//				//if (avcodec_copy_context(m_out_aud_strm->codec, format_context->streams[ASI]->codec) != 0)
//				if (ret = avcodec_parameters_from_context(m_out_aud_strm->codecpar, codec_context) != 0)
//				{
//					//PRINT_MSG("Failed to Copy Context ")
//					//	ret = -1;
//					return ret;
//				}
//				else
//				{
//					//PRINT_MSG("Copied Context ")
//						m_out_aud_strm->codecpar->codec_id = m_in_aud_strm->codecpar->codec_id;
//					m_out_aud_strm->codecpar->codec_tag = 0;
//					pkt.pts;
//					//m_out_aud_strm->pts = m_in_aud_strm->pts;
//					m_out_aud_strm->duration = m_in_aud_strm->duration;
//					m_out_aud_strm->time_base.num = m_in_aud_strm->time_base.num;
//					m_out_aud_strm->time_base.den = m_in_aud_strm->time_base.den;
//
//				}
//			}
//		}
//	}
//
//	if (!(outfmt->flags & AVFMT_NOFILE))
//	{
//		if (avio_open2(&m_outformat->pb, outfile.c_str(), AVIO_FLAG_WRITE, NULL, NULL) < 0)
//		{
//			//PRINT_VAL("Could Not Open File ", outfile)
//			//	ret = -1;
//			return ret;
//		}
//	}
//	m_outformat->flags = AVFMT_GLOBALHEADER;
//
//	/* Write the stream header, if any. */
//	error = avformat_write_header(m_outformat, NULL);
//	if (error < 0)
//	{
//		av_strerror(error, errbuf, sizeof(errbuf));
//		//PRINT_VAL("Error Occurred While Writing Header ", outfile)
//			ret = -1;
//		return ret;
//	}
//	//else
//	//{
//		//PRINT_MSG("Written Output header ")
//		//	m_init_done = true;
//	//}
//
//	return ret;
//}
//
//int GenerateClip(void)
//{
//	
//	int aud_pts = 0, vid_pts = 0, aud_dts = 0, vid_dts = 0;
//	int last_vid_pts = 0;
//	if (1)
//	{
//		av_seek_frame(format_context, ASI, m_out_aud_strm->time_base.num, NULL);
//		while (error = av_read_frame(format_context, &pkt) >= 0 )
//		{
//			if (pkt.stream_index == m_out_aud_strm->index)
//			{
//				//PRINT_VAL("ACTUAL VID Pkt PTS ", av_rescale_q(pkt.pts, m_in_vid_strm->time_base, m_in_vid_strm->codec->time_base))
//				//	PRINT_VAL("ACTUAL VID Pkt DTS ", av_rescale_q(pkt.dts, m_in_vid_strm->time_base, m_in_vid_strm->codec->time_base))
//					av_init_packet(&outpkt);
//				if (pkt.pts != AV_NOPTS_VALUE)
//				{
//					if (last_vid_pts == vid_pts)
//					{
//						vid_pts++;
//						last_vid_pts = vid_pts;
//					}
//					outpkt.pts = vid_pts;
//					//PRINT_VAL("ReScaled VID Pts ", outpkt.pts)
//				}
//				else
//				{
//					outpkt.pts = AV_NOPTS_VALUE;
//				}
//
//				if (pkt.dts == AV_NOPTS_VALUE)
//				{
//					outpkt.dts = AV_NOPTS_VALUE;
//				}
//				else
//				{
//					outpkt.dts = vid_pts;
//					//PRINT_VAL("ReScaled VID Dts ", outpkt.dts)
//					//	PRINT_MSG("=======================================")
//				}
//
//				outpkt.data = pkt.data;
//				outpkt.size = pkt.size;
//				outpkt.stream_index = pkt.stream_index;
//				outpkt.flags |= AV_PKT_FLAG_KEY;
//				last_vid_pts = vid_pts;
//				if (error = av_interleaved_write_frame(m_outformat, &outpkt) < 0)
//				{
//					av_strerror(error, errbuf, sizeof(errbuf));
//					error;
//				}
//				//else
//				//{
//					//m_out_aud_strm->codecpar->;
//					//m_out_aud_strm->codec->frame_number++;
//				//}
//				av_packet_unref(&outpkt);
//				av_packet_unref(&pkt);
//			}
//			else if (pkt.stream_index == m_in_aud_strm->index)
//			{
//				//PRINT_VAL("ACTUAL AUD Pkt PTS ", av_rescale_q(pkt.pts, m_in_aud_strm->time_base, m_in_aud_strm->codec->time_base))
//				//	PRINT_VAL("ACTUAL AUD Pkt DTS ", av_rescale_q(pkt.dts, m_in_aud_strm->time_base, m_in_aud_strm->codec->time_base))
//					//num_aud_pkt++;
//					av_init_packet(&outpkt);
//				if (pkt.pts != AV_NOPTS_VALUE)
//				{
//					outpkt.pts = aud_pts;
//					//PRINT_VAL("ReScaled AUD PTS ", outpkt.pts)
//				}
//				else
//				{
//					outpkt.pts = AV_NOPTS_VALUE;
//				}
//
//				if (pkt.dts == AV_NOPTS_VALUE)
//				{
//					outpkt.dts = AV_NOPTS_VALUE;
//				}
//				else
//				{
//					outpkt.dts = aud_pts;
//					//PRINT_VAL("ReScaled AUD DTS ", outpkt.dts)
//						//PRINT_MSG("====================================")
//						if (outpkt.pts >= outpkt.dts)
//						{
//							outpkt.dts = outpkt.pts;
//						}
//					if (outpkt.dts == aud_dts)
//					{
//						outpkt.dts++;
//					}
//					if (outpkt.pts < outpkt.dts)
//					{
//						outpkt.pts = outpkt.dts;
//						aud_pts = outpkt.pts;
//					}
//				}
//
//				outpkt.data = pkt.data;
//				outpkt.size = pkt.size;
//				outpkt.stream_index = pkt.stream_index;
//				outpkt.flags |= AV_PKT_FLAG_KEY;
//				vid_pts = aud_pts;
//				aud_pts++;
//				if (av_interleaved_write_frame(m_outformat, &outpkt) < 0)
//				{
//					//PRINT_MSG("Faile Audio Write ")
//				}
//				//else
//				//{
//				//	m_out_aud_strm->codec->frame_number++;
//				//}
//				av_packet_unref(&outpkt);
//				av_packet_unref(&pkt);
//			}
//			else
//			{
//				//PRINT_MSG("Got Unknown Pkt ")
//					//num_unkwn_pkt++;
//			}
//			//num_total_pkt++;
//		}
//
//		av_write_trailer(m_outformat);
//		av_packet_unref(&outpkt);
//		av_packet_unref(&pkt);
//		return 0;
//	}
//	return -1;
//}
//
//int APIENTRY wWinMain(HINSTANCE hInstance,
//	HINSTANCE hPrevInstance,
//	LPTSTR    lpCmdLine,
//	int       nCmdShow)
//{
//	FILE* cp = nullptr;
//	AllocConsole();
//	freopen_s(&cp, "CONOUT$", "wt", stdout);
//
//	Init("test.mp4");
//	GenerateClip();
//
//	fclose(cp);
//	return 0;
//}