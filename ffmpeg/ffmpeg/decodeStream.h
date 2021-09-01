#include "headAndErrorCode.h"
#pragma once
class decodeStream
{
public:
	decodeStream();
	~decodeStream();
	AVCodecContext *setDecoder(AVCodecContext  *iCodecCtx, AVFormatContext *ifmt_ctx, int videoindex);
	void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *pFrame, AVFrame *yuvFrame, struct SwsContext *imgCtx);
};

