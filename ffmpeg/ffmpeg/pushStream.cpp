#include "stdafx.h"
#include "pushStream.h"


pushStream::pushStream()
{
}


pushStream::~pushStream()
{
}

AVStream *pushStream::initPush(AVFormatContext *ofmt_ctx, AVCodec *oCodec, AVCodecContext  *oCodecCtx, AVDictionary *param, AVStream *video_st)
{
	//����µ����������avformat_write_header()֮ǰ��Ϻ�
	video_st = avformat_new_stream(ofmt_ctx, oCodec);
	if (video_st == NULL) {
		printf("video_st is NULL! (�������Ч��)\n");
		return NULL;
	}
	video_st->time_base.num = 1;
	video_st->time_base.den = 25;
	avcodec_parameters_from_context(video_st->codecpar, oCodecCtx);//stream��ȡ����������Ϣ
	ofmt_ctx->audio_codec_id = ofmt_ctx->oformat->audio_codec;
	ofmt_ctx->video_codec_id = ofmt_ctx->oformat->video_codec;
	avformat_write_header(ofmt_ctx, &param);//дͷ�ļ�
	return video_st;
}

void pushStream::push(AVFormatContext *ofmt_ctx, AVPacket enc_packet)
{
	av_interleaved_write_frame(ofmt_ctx, &enc_packet);
	av_packet_unref(&enc_packet);
}