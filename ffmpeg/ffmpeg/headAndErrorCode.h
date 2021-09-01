#define __STDC_CONSTANT_MACROS
#include "stdafx.h"
#include "ffmpeg.h"
#include "afxdialogex.h"
#include "string"
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

typedef enum enumPushCameraStreamCode
{
	success = 0,
	openLocalCameraFail,
	findCameraStreamFail,
	setDecoderFail,
	setEncoderFail,
	initPushFail,
	initSDLFail,
	encodePrepareFail

}PushCameraStreamCode;

struct structSetEncoder
{
	AVFormatContext *ofmt_ctx;
	AVCodecContext  *oCodecCtx;
	AVCodec *oCodec;
	AVDictionary *param;
};

#pragma once