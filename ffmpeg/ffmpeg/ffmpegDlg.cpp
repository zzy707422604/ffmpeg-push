#define __STDC_CONSTANT_MACROS
#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpegDlg.h"
#include "afxdialogex.h"
#include <string>
using namespace std;
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

void CffmpegDlg::Decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *pFrame, AVFrame *yuvFrame, struct SwsContext *imgCtx)
{
	handleDecode.decode(dec_ctx, pkt, pFrame, yuvFrame, imgCtx);
}

void CffmpegDlg::InitAll()
{
	av_register_all();
	avformat_network_init();
	ofmt = NULL;
	ifmt_ctx = avformat_alloc_context();
	ofmt_ctx = avformat_alloc_context();
	iCodecCtx = NULL;
	oCodecCtx = NULL;
	video_st = NULL;
	oCodec = NULL;
	param = NULL;
	ifmt_ctx->probesize = 10000 * 1024;
	ifmt_ctx->duration = 10 * AV_TIME_BASE;
	got_encpicture = -1;
	//frame_index = 0;
	//start_time = 0;
	out_filename = "rtsp://192.168.1.110:554/2420725677251317_video.sdp+123456";
	cameraName = "USB Camera";
	videoindex = -1;
	pFrame = av_frame_alloc();
}

int32_t CffmpegDlg::SetCamera(AVFormatContext *ifmt_ctx, string cameraName, int &videoindex)
{
	getCameraStream handleGetCameraStream;
	if (handleGetCameraStream.OpenLocalCamera(ifmt_ctx, true, cameraName) == -1)
	{
		printf("打开摄像头失败");
		return openLocalCameraFail;
	}
	
	if (handleGetCameraStream.FindStream(ifmt_ctx, videoindex) == findCameraStreamFail)
	{
		printf("获取摄像头流失败");
		return openLocalCameraFail;
	}

	return success;
}

int32_t CffmpegDlg::SetDecoder(AVCodecContext *inCodecCtx, AVFormatContext *inFmt_ctx, int iVideoindex)
{
	iCodecCtx = handleDecode.setDecoder(inCodecCtx, inFmt_ctx, iVideoindex);
	if (iCodecCtx == NULL)
		return setDecoderFail;
	else
		return success;
}

int32_t CffmpegDlg::SetEncoder(AVFormatContext *outFmt_ctx, AVFormatContext *inFmt_ctx, AVCodecContext  *outCodecCtx, AVCodec *outCodec, AVDictionary *pParam,
	const char* pFilePath, int iVideoindex)
{
	sSetEncoder = handleEncode.setEncoder(outFmt_ctx, inFmt_ctx, outCodecCtx, outCodec, pParam,
		pFilePath, iVideoindex);
	if (sSetEncoder.ofmt_ctx == NULL || sSetEncoder.oCodecCtx == NULL || sSetEncoder.oCodec == NULL || sSetEncoder.param == NULL)
	{
		return setEncoderFail;
	}
	else
	{
		oCodecCtx = sSetEncoder.oCodecCtx;
		oCodec = sSetEncoder.oCodec;
		param = sSetEncoder.param;
		ofmt_ctx = sSetEncoder.ofmt_ctx;
	}
	return success;
}

int32_t CffmpegDlg::InitPush(AVFormatContext *outFmt_ctx, AVCodec *outCodec, AVCodecContext  *outCodecCtx, AVDictionary *pParam, AVStream *pVideo_st)
{
	video_st = handlePush.initPush(outFmt_ctx, outCodec, outCodecCtx, pParam, pVideo_st);
	if (video_st == NULL)
	{
		return initPushFail;
	}
	start_time = av_gettime();
	return success;
}

int32_t CffmpegDlg::InitSDL()
{
	//SDL Init
	dec_packet = (AVPacket *)av_malloc(sizeof(AVPacket)); //SDL播放用的packet
	pFrameYUVSDL = av_frame_alloc();	
	// 对图形进行裁剪以便于显示得更好
	img_convert_ctx_SDL = sws_getContext(iCodecCtx->width, iCodecCtx->height, iCodecCtx->pix_fmt, iCodecCtx->width, iCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	if (NULL == img_convert_ctx_SDL)
	{
		printf("Get swscale context failed!");
		return initSDLFail;
	}
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		return initSDLFail;
	}
	//SDL 2.0 Support for multiple windows


	screen = SDL_CreateWindowFrom(m_hWnd);//MFC中无法在同一条线程使用SDL_CreateWindow，响应事件会产生冲突
	if (!screen)
	{
		return initSDLFail;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, iCodecCtx->width, iCodecCtx->height);

	CRect CRect;//获取当前的对话框大小
	GetWindowRect(&CRect);
	int screen_w = CRect.Width();
	int screen_h = CRect.Height();
	SDL_Rect sdlRect;
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
	out_buffer_SDL = (uint8_t *)av_malloc((av_image_get_buffer_size(AV_PIX_FMT_YUV420P, iCodecCtx->width, iCodecCtx->height, 1)));
	av_image_fill_arrays(pFrameYUVSDL->data, pFrameYUVSDL->linesize, out_buffer_SDL, AV_PIX_FMT_YUV420P, iCodecCtx->width, iCodecCtx->height, 1);
}

int32_t CffmpegDlg::Encode(AVCodecContext *oCodecCtx, AVFrame *pFrameYUVSDL, AVFormatContext *ifmt_ctx, int videoindex, AVPacket *dec_packet, int got_encpicture)
{

	return handleEncode.encode(oCodecCtx, pFrameYUVSDL, ifmt_ctx, videoindex, dec_packet, got_encpicture);
}

void CffmpegDlg::WriteVideoExtraParam(int videoindex, AVStream *video_st, AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext  *oCodecCtx)
{
	handleEncode.writeVideoControl(videoindex, start_time, video_st, ifmt_ctx, ofmt_ctx);

	handleEncode.insertExtraData(videoindex, oCodecCtx);
}

void CffmpegDlg::PushStream(AVFormatContext *ofmt_ctx)
{
	handleEncode.push(handlePush,ofmt_ctx);
}

void CffmpegDlg::PlayStream()
{
	//SDL
	SDL_UpdateTexture(sdlTexture, NULL, pFrameYUVSDL->data[0], pFrameYUVSDL->linesize[0]);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
	//在最小化时sdl无法绑定，在这重新判断绑定
	if (sdlRenderer == NULL)
	{
		sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
		sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, iCodecCtx->width, iCodecCtx->height);
	}
	av_packet_unref(dec_packet);
}

void CffmpegDlg::RealseAll()
{
	sws_freeContext(img_convert_ctx_SDL);
	SDL_Quit();
	av_write_trailer(ofmt_ctx);
	//av_free(out_buffer);
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
}



void CffmpegDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	InitAll();

	// 打开并设置本地摄像头
	int ret = SetCamera(ifmt_ctx, cameraName, videoindex);

	//设置解码器
	ret = SetDecoder(iCodecCtx, ifmt_ctx, videoindex);

	//设置编码器
	ret = SetEncoder(ofmt_ctx, ifmt_ctx, oCodecCtx, oCodec, param,
		out_filename, videoindex);

	//推流前初始化
	ret = InitPush(ofmt_ctx, oCodec, oCodecCtx, param, video_st);

	//初始化SDL
	InitSDL();

	//Event Loop
	for (;;)
	{
		//Wait
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT)
		{
			if (av_read_frame(ifmt_ctx, dec_packet) >= 0)
			{
				if (dec_packet->stream_index == videoindex && pFrame != NULL)
				{
					//解码出YUV
					Decode(iCodecCtx, dec_packet, pFrame, pFrameYUVSDL, img_convert_ctx_SDL);

					//编码成H264
					ret = Encode(oCodecCtx, pFrameYUVSDL, ifmt_ctx, videoindex, dec_packet, got_encpicture);

					if (ret == 0)
					{
						//写ptd等参数以及添加sps pps头
						WriteVideoExtraParam(videoindex, video_st, ifmt_ctx, ofmt_ctx,oCodecCtx);

						//推流至服务器
						PushStream(ofmt_ctx);
					}

					//显示至屏幕
					PlayStream();
					//RealsePacket();
				}
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

	RealseAll();

	CDialogEx::OnOK();
}

	


void CffmpegDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	thread_exit = 1;

	CDialogEx::OnCancel();
}
