#include "headAndErrorCode.h"
#pragma once
class pushStream
{
public:
	pushStream();
	~pushStream();
	AVStream *initPush(AVFormatContext *ofmt_ctx, AVCodec *oCodec, AVCodecContext  *oCodecCtx, AVDictionary *param, AVStream *video_st);

	void push(AVFormatContext *ofmt_ctx, AVPacket enc_packet);
};

