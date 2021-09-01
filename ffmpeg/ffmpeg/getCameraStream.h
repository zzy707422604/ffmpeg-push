#include "headAndErrorCode.h"
#pragma once

class getCameraStream
{
public:
	getCameraStream();
	~getCameraStream();
	int32_t OpenLocalCamera(AVFormatContext *pFormatCtx, bool isUseDshow, string cameraName);
	int32_t FindStream(AVFormatContext *pFormatCtx, int &videoindex);
};

