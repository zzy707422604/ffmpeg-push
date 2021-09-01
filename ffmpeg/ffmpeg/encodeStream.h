#include "headAndErrorCode.h"
#include "pushStream.h"
#pragma once
class encodeStream
{
public:
	encodeStream();
	~encodeStream();
	structSetEncoder setEncoder(AVFormatContext *ofmt_ctx, AVFormatContext *ifmt_ctx, AVCodecContext  *oCodecCtx, AVCodec *oCodec, AVDictionary *param, const char* filePath, int videoindex);
	int32_t encode(AVCodecContext *oCodecCtx, AVFrame *pFrameYUVSDL, AVFormatContext *ifmt_ctx, int videoindex, AVPacket *dec_packet, int got_encpicture);
	void writeVideoControl(int videoindex, int64_t start_time, AVStream *video_st, AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx);
	void insertExtraData(int videoindex, AVCodecContext *oCodecCtx);
	void push(pushStream handle, AVFormatContext *ofmt_ctx);
public:
	AVPacket enc_packet;//封装后推流用的packet
	AVFrame *pFrame;
	AVRational time_base_in;
	AVRational time_base_conert;
	AVRational time_base1;
	int64_t calc_duration;
	AVRational time_base;
	AVRational r_framerate1;
	AVRational time_base_q;
	int64_t pts_time;
	int64_t now_time;
	int frame_index;
};

