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
	void Refresh(CString externalString);

	DWORD						EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags);

	DWORD						GetClientsNum();
	DWORD						GetSharedMemoryVersion();
	BOOL						UpdateOSD(LPCSTR lpText);
	void						ReleaseOSD();
	void						IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement);
	void						SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty);
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	//Skewjo's stuff
	bool						WriteComPort(CString PortSpecifier, CString data);
	int							ReadByte(CString PortSpecifier);

	//I've got to find a way to make this COM port variable or something...
	//CString						PortSpecifier = CString(_T("COM3"));
	CString						PortSpecifier = CString(_T("COM4"));
	int							serialReadData = 0;

	//timer variables
	LARGE_INTEGER				begin, end, frequency;
	clock_t						regTime;
	double						duration;

	//Should be able to read total readings from the Arduino
	const static int totalReadings = 10;
	long arduinoClockBegin[totalReadings];
	long arduinoClockEnd[totalReadings];

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

	CString						m_strStatus;
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
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
/////////////////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////////////////
