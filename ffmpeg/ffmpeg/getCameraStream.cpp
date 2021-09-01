#include "stdafx.h"
#include "getCameraStream.h"


getCameraStream::getCameraStream()
{
}


getCameraStream::~getCameraStream()
{
}

int32_t getCameraStream::OpenLocalCamera(AVFormatContext *pFormatCtx, bool isUseDshow, string cameraName)
{
	avdevice_register_all();
#ifdef _WIN32
	if (isUseDshow)
	{
		AVInputFormat *ifmt = av_find_input_format("dshow");
		//Set own video device's name
		if (avformat_open_input(&pFormatCtx, ("video=" + cameraName).c_str(), ifmt, NULL) != 0)
		{
			printf("Couldn't open input stream.（无法打开输入流）\n");
			return openLocalCameraFail;
		}
	}
	else
	{
		AVInputFormat *ifmt = av_find_input_format("vfwcap");
		if (avformat_open_input(&pFormatCtx, "0", ifmt, NULL) != 0)
		{
			printf("Couldn't open input stream.（无法打开输入流）\n");
			return openLocalCameraFail;
		}
	}
#endif
	//Linux
#ifdef linux
	AVInputFormat *ifmt = av_find_input_format("video4linux2");
	if (avformat_open_input(&pFormatCtx, "/dev/video0", ifmt, NULL) != 0) {
		printf("Couldn't open input stream.（无法打开输入流）\n");
		return openLocalCameraFail;
	}
#endif
	return 0;
}

int32_t getCameraStream::FindStream(AVFormatContext *pFormatCtx, int &videoindex)
{
	av_dump_format(pFormatCtx, 0, NULL, 0);
	// 寻找视频流信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("寻找视频流信息失败");
		return findCameraStreamFail;
	}

	// 打开视频获取视频流，设置视频默认索引值

	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	// 如果没有找到视频的索引说明没有视频流
	if (videoindex == -1)
	{
		printf("没有找到视频流");
		return findCameraStreamFail;
	}

	return 0;
}