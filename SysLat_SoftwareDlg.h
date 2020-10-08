// SysLat_SoftwareDlg.h : header file
//
// created by Unwinder
// modified by Skewjo
/////////////////////////////////////////////////////////////////////////////
#ifndef _SYSLAT_SOFTWAREDLG_H_INCLUDED_
#define _SYSLAT_SOFTWAREDLG_H_INCLUDED_
/////////////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////
#include "RTSSSharedMemory.h"
#include "RTSSClient.h"
#include "SysLatData.h"
#include <string>
#include <time.h>
#include <vector>


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
//Do I want to keep the following struct?  It's used in the "CalcCpuUsage" function I got rid of, but would I ever want that info?
typedef struct SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION 
{
    LARGE_INTEGER	IdleTime;
    LARGE_INTEGER	KernelTime;
    LARGE_INTEGER	UserTime;
    LARGE_INTEGER	Reserved1[2];
    ULONG			Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;


class CSysLat_SoftwareDlg : public CDialog
{
// Construction
public:
	CSysLat_SoftwareDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSysLat_SoftwareDlg)
	enum { IDD = IDD_SYSLAT_SOFTWARE_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysLat_SoftwareDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL						PreTranslateMessage(MSG* pMsg);
	void						Refresh();
	static void					AppendError(const CString& error);
	
	static unsigned int __stdcall		CreateDrawingThread(void* data);
	void						ReInitThread();


	static void					DrawBlack(CRTSSClient sysLatClient);
	static void					DrawWhite(CRTSSClient sysLatClient);
	std::string					GetProcessNameFromPID(DWORD processID);
	std::string					GetActiveWindowTitle();

	//Dialog menu related functions
	void						SetPortCom1();
	void						SetPortCom2();
	void						SetPortCom3();
	void						SetPortCom4();
	CMenu*						ResetPortsMenuItems();


	
	HANDLE						drawingThreadHandle;
	static CSysLatData*			m_pOperatingSLD; // = new CSysLatData; //I think this is what I want...a static pointer(meaning I can use it both for "the thread" and for "refresh") to a data object so I can hold the old "SysLatData" objects for later
	std::vector<CSysLatData*>	m_previousSLD;
	CRTSSClient					sysLatStatsClient; //This RTSS client is "owned" by the dialog box and the "drawing thread" function "owns" the other
	static constexpr const char* m_caSysLatStats = "SysLatStats";
	static constexpr const char* m_caSysLat = "SysLat";
	static CString				m_PortSpecifier;

	time_t						m_elapsedTimeStart, m_elapsedTimeEnd;

	static bool					m_debugMode; // I don't think this is even used any more... but maybe I should re-implement it?
	
	unsigned int				myCounter = 0;
	static unsigned int			m_loopSize;
	static unsigned int			m_LoopCounterRefresh;
	static CString				m_strError;



	//previously existing members
	//need to go through these and figure out which ones I'm still using?
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

	static CString				m_strStatus;
	CString						m_strInstallPath;

	
	// Generated message map functions
	//{{AFX_MSG(CSysLat_SoftwareDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
/////////////////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////////////////
