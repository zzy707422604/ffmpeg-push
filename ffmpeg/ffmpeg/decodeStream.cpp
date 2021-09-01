#include "stdafx.h"
#include "decodeStream.h"


decodeStream::decodeStream()
{
}


decodeStream::~decodeStream()
{
}

AVCodecContext *decodeStream::setDecoder(AVCodecContext *iCodecCtx, AVFormatContext *ifmt_ctx, int videoindex)
{
	// ���������������
	iCodecCtx = avcodec_alloc_context3(NULL);
	// ��ȡ��������������Ϣ
	if (avcodec_parameters_to_context(iCodecCtx, ifmt_ctx->streams[videoindex]->codecpar) < 0)//codecpar��stream�Ľ�������Ϣ����codecpar��Ϣ���ݸ�iCodeCtx
	{
		printf("��ȡ������ʧ��");
		return NULL;
	}
	// ���ҽ�����
	AVCodec *iCodec = avcodec_find_decoder(iCodecCtx->codec_id);//����ͷ����������MJPEG�ķ�װ��Ѱ��MJPEG�Ľ�����
	if (iCodec == NULL)
	{
		printf("���ҽ�����ʧ��");
		return NULL;
	}
	// �򿪽�����
	if (avcodec_open2(iCodecCtx, iCodec, NULL) < 0)
	{
		printf("�򿪽�����ʧ��");
		return NULL;
	}
	return iCodecCtx;
}

void decodeStream::decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *pFrame, AVFrame *yuvFrame, struct SwsContext *imgCtx/*, FILE *outfile*/)
{
	int ret;
	/* send the packet with the compressed data to the decoder */
	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0)
	{
		fprintf(stderr, "Error submitting the packet to the decoder\n");
		exit(1);
	}
	/* read all the output frames (in general there may be any number of them */
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(dec_ctx, pFrame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0)
		{
			fprintf(stderr, "Error during decoding\n");
			exit(1);
		}
		sws_scale(imgCtx, pFrame->data, pFrame->linesize, 0, dec_ctx->height, yuvFrame->data, yuvFrame->linesize);
	}
}