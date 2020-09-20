// RTSSSharedMemorySampleDlg.h : header file
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#ifndef _RTSSSHAREDMEMORYSAMPLEDLG_H_INCLUDED_
#define _RTSSSHAREDMEMORYSAMPLEDLG_H_INCLUDED_
/////////////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////
#include "RTSSSharedMemory.h"
#include "RTSSProfileInterface.h"
#include "SerialCommHelper.h"
#include <string>
#include <time.h>

/////////////////////////////////////////////////////////////////////////////
#define MAX_CPU									8
#define MAX_HISTORY								512
/////////////////////////////////////////////////////////////////////////////
// define constants / structures and function prototype for NTDLL.dll
// NtQuerySystemInformation function which will be used for CPU usage 
// calculation
/////////////////////////////////////////////////////////////////////////////
#define SystemProcessorPerformanceInformation	8
/////////////////////////////////////////////////////////////////////////////
typedef HRESULT (WINAPI *NTQUERYSYSTEMINFORMATION)(UINT, PVOID, ULONG, PULONG);
/////////////////////////////////////////////////////////////////////////////
typedef struct SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION 
{
    LARGE_INTEGER	IdleTime;
    LARGE_INTEGER	KernelTime;
    LARGE_INTEGER	UserTime;
    LARGE_INTEGER	Reserved1[2];
    ULONG			Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;
/////////////////////////////////////////////////////////////////////////////
class CRTSSSharedMemorySampleDlg : public CDialog
{
// Construction
public:
	CRTSSSharedMemorySampleDlg(CWnd* pParent = NULL);	// standard constructor


	//////////////////

	/////////////////////

// Dialog Data
	//{{AFX_DATA(CRTSSSharedMemorySampleDlg)
	enum { IDD = IDD_RTSSSHAREDMEMORYSAMPLE_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRTSSSharedMemorySampleDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Refresh();
	//static void Refresh(CString externalString);

	DWORD						EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags);

	DWORD						GetClientsNum();
	DWORD						GetSharedMemoryVersion();
	static  BOOL						UpdateOSD(LPCSTR lpText);
	void						ReleaseOSD();
	void						IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement);
	void						SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty);
	
	

	//////////////////////////////////////////////////////////////////////////////////////////////
	//Skewjo's stuff
	time_t elapsedTimeStart, elapsedTimeEnd;
	
	static bool					debugMode;
	
	//bool						WriteComPort(CString PortSpecifier, CString data);
	static int					ReadByte(CString PortSpecifier);

	static unsigned int __stdcall		CreateDrawingThread(void* data);

	static CString				PortOption;
	static CString				PortSpecifier;
	static int					serialReadData;

	static LARGE_INTEGER		begin, end, frequency;
	clock_t						regTime;
	double						duration;
	
	static CString				m_tLoopCounter;
	static int					systemLatencyTotal;
	static double				systemLatencyAverage;
	static int					loopCounterEVR;
	static int					systemLatencyTotalEVR; //EVR stands for expected value range
	static double				systemLatencyAverageEVR;
	

	static CString				m_arduinoResults;
	
	static CString				m_tInnerLoopTimer;
	static CString				m_tOuterLoopTimer;
	static CString				m_tDrawWhite;
	
	static CString				m_arduinoResultsComplete;
	static CString				m_arduinoResultsRefresh;
	static CString				m_tLoopCounterRefresh;
	static CString				m_tInnerLoopTimerRefresh;
	static CString				m_tOuterLoopTimerRefresh;
	static CString				m_tDrawWhiteRefresh;

	static void					DrawBlack();
	static void					DrawWhite();
	void						SetPortCom1();
	void						SetPortCom2();
	void						SetPortCom3();
	void						SetPortCom4();
	//End Skewjo's stuff
	////////////////////////////////////////////////////////////////////////////////////////////////

	BOOL						m_bMultiLineOutput;
	BOOL						m_bFormatTags;
	BOOL						m_bFillGraphs;
	BOOL						m_bConnected;

	HICON						m_hIcon;
	UINT						m_nTimerID;

	DWORD						m_dwNumberOfProcessors;
	NTQUERYSYSTEMINFORMATION	m_pNtQuerySystemInformation;

	DWORD						m_dwTickCount[MAX_CPU];
	LARGE_INTEGER				m_idleTime[MAX_CPU];
	FLOAT						m_fltCpuUsage[MAX_CPU];

	FLOAT						m_fltCpuUsageHistory[MAX_CPU][MAX_HISTORY];
	FLOAT						m_fltRamUsageHistory[MAX_HISTORY];
	DWORD						m_dwHistoryPos;

	CFont						m_font;
	CRichEditCtrl				m_richEditCtrl;

	static CString						m_strStatus;
	CString						m_strInstallPath;

	CRTSSProfileInterface		m_profileInterface;

	
	// Generated message map functions
	//{{AFX_MSG(CRTSSSharedMemorySampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//CString CRTSSSharedMemorySampleDlg::PortOption = "COM3";
//CString CRTSSSharedMemorySampleDlg::PortSpecifier = CString(_T(PortOption));
//int CRTSSSharedMemorySampleDlg::serialReadData = 0;

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
/////////////////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////////////////
