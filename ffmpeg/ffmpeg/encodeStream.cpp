#include "stdafx.h"
#include "encodeStream.h"


encodeStream::encodeStream()
{
	frame_index = 0;
}


encodeStream::~encodeStream()
{
}

structSetEncoder encodeStream::setEncoder(AVFormatContext *ofmt_ctx, AVFormatContext *ifmt_ctx, AVCodecContext  *oCodecCtx, AVCodec *oCodec, AVDictionary *param,
	const char* filePath, int videoindex)
{
	struct structSetEncoder handle;
	//���׼����׼��������õĽ��б����H264
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "rtsp", filePath);//rtmpʹ�õ���flv
	if (!ofmt_ctx)
	{
		printf("Could not create output context�����ܴ�����������ģ�\n");
		return handle;
	}
	//Ѱ��h264�ı�����
	oCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!oCodec)
	{
		printf("Can not find encoder! (û���ҵ����ʵı�������)\n");
		return handle;
	}
	//���б���������
	oCodecCtx = avcodec_alloc_context3(oCodec);
	oCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	oCodecCtx->width = ifmt_ctx->streams[videoindex]->codecpar->width;
	oCodecCtx->height = ifmt_ctx->streams[videoindex]->codecpar->height;
	oCodecCtx->time_base.num = 1;
	oCodecCtx->time_base.den = 25;
	oCodecCtx->bit_rate = 400000;
	oCodecCtx->gop_size = 50;
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		oCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	//H264 codec param  
	oCodecCtx->qmin = 10;
	oCodecCtx->qmax = 51;
	//Optional Param  
	oCodecCtx->max_b_frames = 3;
	param = 0; //AVDictionary��FFmpeg�ļ�ֵ�Դ洢���ߣ�FFmpeg����ʹ��AVDictionary����/��ȡ�ڲ�����
	av_dict_set(&param, "preset", "medium", 0);
	av_dict_set(&param, "rtsp_transport", "udp", 0);	//����ʱ�����������л�����ȥ������ʱ
	av_dict_set(&param, "tune", "zerolatency", 0);
	av_dict_set(&param, "profile", "baseline", 0);
	av_dict_set(&param, "muxdelay", "1", 0);	//�����ӳ����
	av_dict_set(&param, "stimeout", "5000000", 0);	//����5s��ʱ�Ͽ�����ʱ��
	av_dict_set(&param, "crf", "25", 0);
	av_dict_set(&param, "buffer_size", "1024000", 0);
	//�򿪱�����
	if (avcodec_open2(oCodecCtx, oCodec, &param) < 0) {
		printf("Failed to open encoder! (��������ʧ�ܣ�)\n");
		return handle;
	}
	
	handle.ofmt_ctx = ofmt_ctx;
	handle.oCodec = oCodec;
	handle.oCodecCtx = oCodecCtx;
	handle.param = param;

	return handle;
}

int32_t encodeStream::encode(AVCodecContext *oCodecCtx, AVFrame *pFrameYUVSDL, AVFormatContext *ifmt_ctx, int videoindex, AVPacket *dec_packet, int	got_encpicture)
{
	
	//��ʼ����װ��
	enc_packet.data = NULL;
	enc_packet.size = 0;
	av_init_packet(&enc_packet);
	//����
	int ret = avcodec_send_frame(oCodecCtx, pFrameYUVSDL);
	time_base_in = ifmt_ctx->streams[videoindex]->time_base;//{ 1, 1000000 };
	//time_base_in = iCodecCtx->time_base;//{ 1, 1000 };
	time_base_conert = { 1, AV_TIME_BASE };
	pFrameYUVSDL->pts = av_rescale_q(dec_packet->pts, time_base_in, time_base_conert);
	if (ret < 0)
	{
		av_frame_free(&pFrame);
		return -1;
	}
	
	got_encpicture = avcodec_receive_packet(oCodecCtx, &enc_packet);

	return got_encpicture;
}

void encodeStream::writeVideoControl(int videoindex, int64_t start_time, AVStream *video_st, AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx)
{
	//gop == 0

	frame_index++;
	enc_packet.stream_index = video_st->index;
	//FIX��No PTS (Example: Raw H.264)
	//Simple Write PTS
	if (enc_packet.pts == AV_NOPTS_VALUE)
	{
		//Write PTS
		time_base1 = ifmt_ctx->streams[videoindex]->time_base;
		//Duration between 2 frames (us)
		calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
		//Parameters
		enc_packet.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
		enc_packet.dts = enc_packet.pts;
		enc_packet.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
	}

	//Write PTS
	time_base = ofmt_ctx->streams[video_st->index]->time_base;//{ 1, 1000 };
	r_framerate1 = ifmt_ctx->streams[videoindex]->r_frame_rate;// { 50, 2 };
	time_base_q = { 1, AV_TIME_BASE };
	//Duration between 2 frames (us)
	calc_duration = (double)(AV_TIME_BASE)*(1 / av_q2d(r_framerate1));	//�ڲ�ʱ���
	//Parameters
	enc_packet.pts = av_rescale_q(frame_index*calc_duration, time_base_q, time_base);
	enc_packet.dts = enc_packet.pts;
	enc_packet.duration = av_rescale_q(calc_duration, time_base_q, time_base);
	enc_packet.pos = -1;
	//Delay
	pts_time = av_rescale_q(enc_packet.dts, time_base, time_base_q);
	now_time = av_gettime() - start_time;
	if (pts_time > now_time)	//����ʱ��֮���о�
		av_usleep(pts_time - now_time);
}

FILE *stream;
int k = 0;
void encodeStream::insertExtraData(int videoindex, AVCodecContext  *oCodecCtx)
{
	if (enc_packet.stream_index == videoindex)
	{
		if (enc_packet.flags &AV_PKT_FLAG_KEY)
		{
			//�ҵ���I֡��AVPacket
			if (enc_packet.data != NULL)
			{
				if (enc_packet.data[0] == 0 && enc_packet.data[1] == 0 && enc_packet.data[2] == 0 && enc_packet.data[3] == 1 && enc_packet.data[4] == 101)
				{
					//һ��I֡�ڿ�ͷ���ڿ�ͷ����SPS��PPS
					uint8_t* cExtradata = (uint8_t *)malloc(enc_packet.size + oCodecCtx->extradata_size);//oCodecCtx->extradata;
					memcpy(cExtradata, oCodecCtx->extradata, oCodecCtx->extradata_size);
					memcpy(cExtradata + oCodecCtx->extradata_size, enc_packet.data, enc_packet.size);//&enc_packet.data[0]
					enc_packet.size += oCodecCtx->extradata_size;
					memcpy(enc_packet.data, cExtradata, enc_packet.size);
					free(cExtradata);
				}
				else
				{
					//I֡���ڿ�ͷ����������һ֡I֡���ڿ�ͷ ��������memcpy���Է�������
					for (int i = 0; i < enc_packet.size; i++)
					{
						if (enc_packet.data[i] == 0)
						{
							if (enc_packet.data[i + 1] == 0 && enc_packet.data[i + 2] == 0 && enc_packet.data[i + 3] == 1 && enc_packet.data[i + 4] == 101)
							{
								//�ҵ�I֡������SPS��PPS
								uint8_t* cExtradata = (uint8_t *)malloc(enc_packet.size + oCodecCtx->extradata_size);//oCodecCtx->extradata;
								memset(cExtradata, 0, enc_packet.size + oCodecCtx->extradata_size);
								memcpy(cExtradata, enc_packet.data, i);
								memcpy(cExtradata + i, oCodecCtx->extradata, oCodecCtx->extradata_size);
								memcpy(cExtradata + i + oCodecCtx->extradata_size, enc_packet.data + i, enc_packet.size - i);//&enc_packet.data[0]
								enc_packet.size += oCodecCtx->extradata_size;
								memcpy(enc_packet.data, cExtradata, enc_packet.size);
								if (cExtradata != NULL)
								{
									free(cExtradata);
								}
								break;
							}
						}
					}
				}
			}
		}
	}

	if (k == 0)
	{
		stream = fopen("stream", "wb");
		k = 1;
	}
	fwrite(enc_packet.data, enc_packet.size, 1, stream);
}

void encodeStream::push(pushStream handle, AVFormatContext *ofmt_ctx)
{
	pushStream pushStreamHandle = handle;
	pushStreamHandle.push(ofmt_ctx,enc_packet);

}