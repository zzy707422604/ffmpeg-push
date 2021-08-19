
// ffmpegDlg.h : header file
//

#pragma once
//extern "C"
//{
//#include "libavcodec/avcodec.h"
//#include "libavformat/avformat.h"
//#include "libswscale/swscale.h"
//#include "libavdevice/avdevice.h"
//#include "libavutil/imgutils.h"
//#include "libavutil/time.h"
//#include "SDL/SDL.h"
//#include <SDL/SDL_main.h>
//};
//#pragma comment(lib ,"SDL2.lib")
//#pragma comment(lib ,"SDL2main.lib")
// CffmpegDlg dialog
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
};
