
// ffmpeg.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CffmpegApp:
// See ffmpeg.cpp for the implementation of this class
//

class CffmpegApp : public CWinApp
{
public:
	CffmpegApp();

// Overrides
public:
	virtual BOOL InitInstance();

	void show_vfw_device();
	void show_dshow_device();
	void show_dshow_device_option();
	void show_avfoundation_device();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CffmpegApp theApp;