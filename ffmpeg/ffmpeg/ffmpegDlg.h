#include "getCameraStream.h"
#include "headAndErrorCode.h"
#include "decodeStream.h"
#include "encodeStream.h"
#include "pushStream.h"
#include "playStream.h"
#include "ffmpeg.h"


#pragma once
class CffmpegDlg : public CDialogEx
{
// Construction
public:
	CffmpegDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FFMPEG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
public:
	void InitAll();
	int32_t SetCamera(AVFormatContext* ifmt_ctx, string cameraName, int &videoindex);
	int32_t SetDecoder(AVCodecContext *inCodecCtx, AVFormatContext *infmt_ctx, int ivideoindex);
	int32_t SetEncoder(AVFormatContext *ofmt_ctx, AVFormatContext *ifmt_ctx, AVCodecContext  *oCodecCtx, AVCodec *oCodec, AVDictionary *param,
		const char* filePath, int videoindex);
	int32_t InitPush(AVFormatContext *ofmt_ctx, AVCodec *oCodec, AVCodecContext  *oCodecCtx, AVDictionary *param, AVStream *video_st);
	int32_t InitSDL();
	void Decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *pFrame, AVFrame *yuvFrame, struct SwsContext *imgCtx);
	int32_t Encode(AVCodecContext *oCodecCtx, AVFrame *pFrameYUVSDL, AVFormatContext *ifmt_ctx, int videoindex, AVPacket *dec_packet, int got_encpicture);
	void WriteVideoExtraParam(int videoindex, AVStream *video_st, AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext  *oCodecCtx);
	void PushStream(AVFormatContext *ofmt_ctx);
	void PlayStream();
	void RealseAll();
public:
	decodeStream handleDecode;
	encodeStream handleEncode;
	structSetEncoder sSetEncoder;
	pushStream handlePush;
	playStream handlePlay;
public:
	AVOutputFormat *ofmt;
	AVFormatContext *ifmt_ctx;
	AVFormatContext *ofmt_ctx;
	AVCodecContext  *iCodecCtx;
	AVCodecContext  *oCodecCtx;
	AVStream *video_st;
	AVCodec *oCodec;
	AVDictionary *param;
	int	got_encpicture;
	//int64_t start_time;
	int videoindex;
	const char *out_filename;
	string cameraName;
	int64_t start_time;
public:
	AVPacket *dec_packet;
	AVFrame *pFrameYUVSDL;
	struct SwsContext *img_convert_ctx_SDL;
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	uint8_t *out_buffer_SDL;
	SDL_Event event;//SDL事件，用于控制推流和SDL播放
public:
	AVFrame *pFrame;
};
