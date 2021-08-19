
// ffmpegDlg.cpp : implementation file
//

#define __STDC_CONSTANT_MACROS
#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpegDlg.h"
#include "afxdialogex.h"
#include <string>
using namespace std;
#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "SDL/SDL.h"
//#include "SDL/SDL_main.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#ifdef __cplusplus
};
#endif
#endif
#pragma comment(lib ,"SDL2.lib")
//#pragma comment(lib ,"SDL2main.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//Output YUV420P 
#define OUTPUT_YUV420P 0
//'1' Use Dshow 
//'0' Use VFW
#define USE_DSHOW 0

#define USE_SDL 1

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

#define _CRT_SECURE_NO_DEPRECATE

int thread_exit = 0;
int thread_pause = 0;
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CffmpegDlg dialog



CffmpegDlg::CffmpegDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CffmpegDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CffmpegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CffmpegDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CffmpegDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CffmpegDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CffmpegDlg message handlers

BOOL CffmpegDlg::OnInitDialog()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CffmpegDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CffmpegDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CffmpegDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int sfp_refresh_thread(void *opaque)
{
	thread_exit = 0;
	while (!thread_exit) {
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	thread_exit = 0;
	thread_pause = 0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}

int OpenLocalCamera(AVFormatContext *pFormatCtx, bool isUseDshow, string cameraName)
{
	avdevice_register_all();
#ifdef _WIN32
	if (isUseDshow)
	{
		AVInputFormat *ifmt = av_find_input_format("dshow");
		//Set own video device's name
		if (avformat_open_input(&pFormatCtx, ("video=" + cameraName).c_str(), ifmt, NULL) != 0)
		{
			printf("Couldn't open input stream.���޷�����������\n");
			return -1;
		}
	}
	else
	{
		AVInputFormat *ifmt = av_find_input_format("vfwcap");
		if (avformat_open_input(&pFormatCtx, "0", ifmt, NULL) != 0)
		{
			printf("Couldn't open input stream.���޷�����������\n");
			return -1;
		}
	}
#endif
	//Linux
#ifdef linux
	AVInputFormat *ifmt = av_find_input_format("video4linux2");
	if (avformat_open_input(&pFormatCtx, "/dev/video0", ifmt, NULL) != 0) {
		printf("Couldn't open input stream.���޷�����������\n");
		return -1;
	}
#endif
	return 0;
}

static void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *pFrame, AVFrame *yuvFrame, struct SwsContext *imgCtx/*, FILE *outfile*/)
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


void CffmpegDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	av_register_all();
	avformat_network_init();
	AVOutputFormat *ofmt = NULL;
	AVFormatContext *ifmt_ctx = avformat_alloc_context();
	AVFormatContext *ofmt_ctx = avformat_alloc_context();
	AVCodecContext  *iCodecCtx = NULL;
	AVCodecContext  *oCodecCtx = NULL;
	AVStream		*video_st;
	ifmt_ctx->probesize = 10000 * 1024;
	ifmt_ctx->duration = 10 * AV_TIME_BASE;
	int	got_encpicture = -1;
	int frame_index = 0;
	int64_t start_time = 0;
	const char *out_filename = "rtsp://192.168.1.110:554/2420725677251317_video.sdp+123456";
	// �򿪱�������ͷ
	if (OpenLocalCamera(ifmt_ctx, true, "USB Camera") == -1)
	{
		printf("������ͷʧ��");
		return;
	}
	av_dump_format(ifmt_ctx, 0, NULL, 0);
	// Ѱ����Ƶ����Ϣ
	if (avformat_find_stream_info(ifmt_ctx, NULL) < 0)
	{
		printf("Ѱ����Ƶ����Ϣʧ��");
		return;
	}

	// ����Ƶ��ȡ��Ƶ����������ƵĬ������ֵ
	int videoindex = -1;
	for (int i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	// ���û���ҵ���Ƶ������˵��û����Ƶ��
	if (videoindex == -1)
	{
		printf("û���ҵ���Ƶ��");
		return;
	}
	// ���������������
	iCodecCtx = avcodec_alloc_context3(NULL);
	// ��ȡ��������������Ϣ
	if (avcodec_parameters_to_context(iCodecCtx, ifmt_ctx->streams[videoindex]->codecpar) < 0)//codecpar��stream�Ľ�������Ϣ����codecpar��Ϣ���ݸ�iCodeCtx
	{
		printf("��ȡ������ʧ��");
		return;
	}
	// ���ҽ�����
	AVCodec *iCodec = avcodec_find_decoder(iCodecCtx->codec_id);//����ͷ����������MJPEG�ķ�װ��Ѱ��MJPEG�Ľ�����
	if (iCodec == NULL)
	{
		printf("���ҽ�����ʧ��");
		return;
	}
	// �򿪽�����
	if (avcodec_open2(iCodecCtx, iCodec, NULL) < 0)
	{
		printf("�򿪽�����ʧ��");
		return;
	}
	// ��ͼ�ν��вü��Ա�����ʾ�ø���
	struct SwsContext *img_convert_ctx_SDL = sws_getContext(iCodecCtx->width, iCodecCtx->height, iCodecCtx->pix_fmt, iCodecCtx->width, iCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	if (NULL == img_convert_ctx_SDL)
	{
		printf("Get swscale context failed!");
		return;
	}
	//���׼����׼��������õĽ��б����H264
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "rtsp", out_filename);//rtmpʹ�õ���flv
	if (!ofmt_ctx)
	{
		printf("Could not create output context�����ܴ�����������ģ�\n");
		return;
	}
	//Ѱ��h264�ı�����
	AVCodec *oCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!oCodec)
	{
		printf("Can not find encoder! (û���ҵ����ʵı�������)\n");
		return;
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
	AVDictionary *param = 0; //AVDictionary��FFmpeg�ļ�ֵ�Դ洢���ߣ�FFmpeg����ʹ��AVDictionary����/��ȡ�ڲ�����
	av_dict_set(&param, "preset", "medium", 0);
	av_dict_set(&param, "rtsp_transport", "udp", 0);	//����ʱ�����������л�����ȥ������ʱ
	av_dict_set(&param, "tune", "zerolatency", 0);
	av_dict_set(&param, "profile", "baseline", 0);
	av_dict_set(&param, "muxdelay", "1", 0);	//�����ӳ����
	av_dict_set(&param, "stimeout", "5000000", 0);	//����5s��ʱ�Ͽ�����ʱ��
	av_dict_set(&param, "crf", "40", 0);
	av_dict_set(&param, "buffer_size", "1024000", 0);
	//�򿪱�����
	if (avcodec_open2(oCodecCtx, oCodec, &param) < 0) {
		printf("Failed to open encoder! (��������ʧ�ܣ�)\n");
		return;
	}
	//����µ����������avformat_write_header()֮ǰ��Ϻ�
	video_st = avformat_new_stream(ofmt_ctx, oCodec);
	if (video_st == NULL) {
		printf("video_st is NULL! (�������Ч��)\n");
		return;
	}
	video_st->time_base.num = 1;
	video_st->time_base.den = 25;
	avcodec_parameters_from_context(video_st->codecpar, oCodecCtx);//stream��ȡ����������Ϣ
	ofmt_ctx->audio_codec_id = ofmt_ctx->oformat->audio_codec;
	ofmt_ctx->video_codec_id = ofmt_ctx->oformat->video_codec;
	int ret = avformat_write_header(ofmt_ctx, &param);//дͷ�ļ�
	if (ret < 0)
	{
		return;
	}
	AVPacket enc_packet;//��װ�������õ�packet
	//av_new_packet(&enc_packet, 0);
	AVFrame *pFrame = av_frame_alloc();
	uint8_t *out_buffer;
	out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, oCodecCtx->width, oCodecCtx->height, 1));
	//SDL Init--------------------------------------------------------------
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		return;
	}
	//SDL 2.0 Support for multiple windows
	CRect CRect;//��ȡ��ǰ�ĶԻ����С
	GetWindowRect(&CRect);
	int screen_w = CRect.Width();
	int screen_h = CRect.Height();
	AVFrame *pFrameYUVSDL = av_frame_alloc();
	SDL_Window *screen = SDL_CreateWindowFrom(m_hWnd);//MFC���޷���ͬһ���߳�ʹ��SDL_CreateWindow����Ӧ�¼��������ͻ
	if (!screen)
	{
		return;
	}
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, iCodecCtx->width, iCodecCtx->height);
	SDL_Rect sdlRect;
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;
	AVPacket *dec_packet = (AVPacket *)av_malloc(sizeof(AVPacket)); //SDL�����õ�packet
	SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
	uint8_t *out_buffer_SDL = (uint8_t *)av_malloc((av_image_get_buffer_size(AV_PIX_FMT_YUV420P, iCodecCtx->width, iCodecCtx->height, 1)));
	av_image_fill_arrays(pFrameYUVSDL->data, pFrameYUVSDL->linesize, out_buffer_SDL, AV_PIX_FMT_YUV420P, iCodecCtx->width, iCodecCtx->height, 1);
	start_time = av_gettime();
	//uint8_t* cPacketData = new uint8_t((1024*1024)  * sizeof(uint8_t*));
	uint8_t* cExtradata = (uint8_t *)malloc((1024 * 1024) * sizeof(uint8_t));//oCodecCtx->extradata;
	AVRational time_base_in;
	AVRational time_base_conert;
	AVRational time_base1;
	int64_t calc_duration;
	AVRational time_base;
	AVRational r_framerate1;
	AVRational time_base_q;
	int64_t pts_time;
	int64_t now_time;
	SDL_Event event;//SDL�¼������ڿ���������SDL����
	FILE *file = fopen("C:\\Users\\admin\\Desktop\\123456.h264", "wb");
	//Event Loop
	for (;;)
	{
		//Wait
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT)
		{
			//------------------------------
			if (av_read_frame(ifmt_ctx, dec_packet) >= 0)
			{
				if (dec_packet->stream_index == videoindex && pFrame != NULL)
				{

					decode(iCodecCtx, dec_packet, pFrame, pFrameYUVSDL, img_convert_ctx_SDL);//�����YUV��SDL����
					//��ʼ����װ��
					enc_packet.data = NULL;
					enc_packet.size = 0;
					av_init_packet(&enc_packet);
					//����
					ret = avcodec_send_frame(oCodecCtx, pFrameYUVSDL);
					time_base_in = ifmt_ctx->streams[videoindex]->time_base;//{ 1, 1000000 };
					//time_base_in = iCodecCtx->time_base;//{ 1, 1000 };
					time_base_conert = { 1, AV_TIME_BASE };
					pFrameYUVSDL->pts = av_rescale_q(dec_packet->pts, time_base_in, time_base_conert);
					if (ret < 0)
					{
						av_frame_free(&pFrame);
						return;
					}
					got_encpicture = avcodec_receive_packet(oCodecCtx, &enc_packet);


					if (got_encpicture == 0)
					{
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
						if (pts_time > now_time)
							av_usleep(pts_time - now_time);

						//Print to Screen
						if (enc_packet.stream_index == videoindex) {
							if (enc_packet.flags &AV_PKT_FLAG_KEY)//�ҵ���I֡��AVPacket
							{

								if (enc_packet.data != NULL)
								{
									if (enc_packet.data[0] == 0 && enc_packet.data[1] == 0 && enc_packet.data[2] == 0 && enc_packet.data[3] == 1 && enc_packet.data[4] == 101)
									{
										//�ҵ�I֡������SPS��PPS
										memcpy(cExtradata, oCodecCtx->extradata, oCodecCtx->extradata_size);
										memcpy(cExtradata + oCodecCtx->extradata_size, enc_packet.data, enc_packet.size);//&enc_packet.data[0]
										enc_packet.size += oCodecCtx->extradata_size;
										memcpy(enc_packet.data, cExtradata, enc_packet.size);
									}
								}
							}
						}
						//д�����������
						ret = av_interleaved_write_frame(ofmt_ctx, &enc_packet);
					}
					av_packet_unref(&enc_packet);
					//SDL---------------------------
					SDL_UpdateTexture(sdlTexture, NULL, pFrameYUVSDL->data[0], pFrameYUVSDL->linesize[0]);
					SDL_RenderClear(sdlRenderer);
					SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
					SDL_RenderPresent(sdlRenderer);
					//����С��ʱsdl�޷��󶨣����������жϰ�
					if (sdlRenderer == NULL)
					{
						sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
						sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, iCodecCtx->width, iCodecCtx->height);
					}
				}
				av_packet_unref(dec_packet);
			}
			else
			{
				//Exit Thread
				thread_exit = 1;
			}
		}
		else if (event.type == SDL_QUIT)
		{
			thread_exit = 1;
		}
		else if (event.type == SFM_BREAK_EVENT)
		{
			break;
		}
	}
	sws_freeContext(img_convert_ctx_SDL);
	SDL_Quit();
	av_write_trailer(ofmt_ctx);
	av_free(out_buffer);
	av_free(out_buffer_SDL);
	if (video_st)
	{
		avcodec_close(oCodecCtx);
	}
	avformat_close_input(&ifmt_ctx);
	av_frame_free(&pFrameYUVSDL);
	av_frame_free(&pFrame);
	avcodec_close(iCodecCtx);
	avcodec_close(oCodecCtx);
	avformat_close_input(&ifmt_ctx);
	avformat_close_input(&ofmt_ctx);
	free(cExtradata);
	CDialogEx::OnOK();
}

	


void CffmpegDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
