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
#include "RTSSProfileInterface.h"
#include "GroupedString.h"
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
class CSysLat_SoftwareDlg : public CDialog
{
// Construction
public:
	CSysLat_SoftwareDlg(CWnd* pParent = NULL);	// standard constructor


	//////////////////

	/////////////////////

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
	void Refresh();
	//static void Refresh(CString externalString);

	DWORD						EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags);

	DWORD						GetClientsNum();
	DWORD						GetSharedMemoryVersion();
	DWORD						GetLastForegroundApp();
	DWORD						GetLastForegroundAppID();
	std::string					GetProcessNameFromPID(DWORD processID);
	std::string					GetActiveWindowTitle();
	static  BOOL			    UpdateOSD(LPCSTR lpText, const char* OSDSlotOwner);
	void						ReleaseOSD(const char* OSDSlotOwner);
	BOOL						PreTranslateMessage(MSG* pMsg);
	void						IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement);
	void						SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty);
	
	

	//////////////////////////////////////////////////////////////////////////////////////////////
	//Skewjo's stuff
	time_t						m_elapsedTimeStart, m_elapsedTimeEnd;
	
	static bool					m_debugMode;
	
	static CString				m_PortSpecifier;

	static int					m_systemLatencyTotal;
	static double				m_systemLatencyAverage;
	static int					m_loopCounterEVR;
	static int					m_systemLatencyTotalEVR; //EVR stands for expected value range
	static double				m_systemLatencyAverageEVR;
	

	static CString				m_arduinoResultsComplete;
	static unsigned int			m_loopSize;
	static unsigned int			m_LoopCounterRefresh;

	static CString				m_strError;

	static HANDLE				m_refreshMutex;

	unsigned int				myCounter = 0;
	HANDLE						drawingThreadHandle;
	void						ReInitThread();
	static unsigned int __stdcall		CreateDrawingThread(void* data);

	static HANDLE				OpenComPort(const CString& PortSpecifier);
	static void					CloseComPort(HANDLE hPort);
	static bool					IsComPortOpened(HANDLE hPort);
	static int					ReadByte(HANDLE port);

	static void					DrawBlack();
	static void					DrawWhite();
	void						SetPortCom1();
	void						SetPortCom2();
	void						SetPortCom3();
	void						SetPortCom4();
	CMenu*						ResetPortsMenuItems();
	void						GetOSDText(CGroupedString& osd, BOOL bFormatTagsSupported, BOOL bObjTagsSupported);
	static void					CheckRefreshMutex();
	static void					AppendError(const CString& error);
	static BOOL					AcquireRefreshMutex();
	static void					ReleaseRefreshMutex();
	static void					CloseRefreshMutex();

	static void SetArduinoResultsComplete(unsigned int loopCounter, const CString& arduinoResults);

	static constexpr const char* m_caSysLatStats = "SysLatStats";
	static constexpr const char* m_caSysLat = "SysLat";


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

	static CString				m_strStatus;
	CString						m_strInstallPath;

	CRTSSProfileInterface		m_profileInterface;

	
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
