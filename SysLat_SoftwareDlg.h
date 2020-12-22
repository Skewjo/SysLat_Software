// SysLat_SoftwareDlg.h : header file
//
// created by Unwinder
// modified by Skewjo
/////////////////////////////////////////////////////////////////////////////

#ifndef _SYSLAT_SOFTWAREDLG_H_INCLUDED_
#define _SYSLAT_SOFTWAREDLG_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxdialogex.h"
#include <string>
#include <time.h>
#include <vector>
#include "RTSSSharedMemory.h"
#include "RTSSClient.h"
#include "SysLatData.h"

#include "HardwareID.h"
#include "MachineInfo.h"



class CSysLat_SoftwareDlg : public CDialogEx
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
	BOOL							PreTranslateMessage(MSG* pMsg);
	void							Refresh();
	void							R_GetRTSSConfigs();
	BOOL							R_SysLatStats();
	void							R_Position();
	void							R_ProcessNames();
	void							R_StrOSD();
	static void						AppendError(const CString& error); //this function is duplicated between this class and SysLatData - need to make this not used by the thread and then I can make it non-static like the other refresh functions
	static unsigned int __stdcall	CreateDrawingThread(void* data);
	

	static void						DrawSquare(CRTSSClient sysLatClient, CString& colorString);
	static std::string				GetProcessNameFromPID(DWORD processID);
	static std::string				GetActiveWindowTitle();
	static void						ProcessNameTrim(std::string&, std::string&);

	//Dialog menu related functions
	//Tools
	void							ReInitThread();//used by the "New Test" menu function
	void							ExportData();
	void							UploadData();

	//Settings
	void							SetPortCom1();
	void							SetPortCom2();
	void							SetPortCom3();
	void							SetPortCom4();
	CMenu*							ResetPortsMenuItems();
	void							DebugMode();
	void							TestUploadMode();
	void							DisplaySysLatInOSD();
	void							OpenPreferences();
	void							OpenTestCtrl();
	void							ExportData(Json::Value stuffToExport);

	//Members
	HANDLE						drawingThreadHandle;
	static CSysLatData*			m_pOperatingSLD; //Does this need to be a "const" pointer?  It changes if the thread is re-initialized, so I'm thinking no...
	std::vector<CSysLatData*>	m_previousSLD;
	CRTSSClient					sysLatStatsClient; //This RTSS client is "owned" by the dialog box and the "drawing thread" function "owns" the other
	static constexpr const char* m_caSysLatStats = "SysLatStats";
	static constexpr const char* m_caSysLat = "SysLat";
	static DWORD				m_sysLatOwnedSlot;//UGH - I'm specifcally making the sysLatClient object thread local... but then to get a value from it I need to make a static var in this class to track it.  Seems dumb.
	static CString				m_updateString;
	static CString				m_PortSpecifier;
	static CString				m_strBlack;
	static CString				m_strWhite;

	MachineInfo					m_machineInfo;
	HardwareID					m_hardwareID;

	time_t						m_elapsedTimeStart, m_elapsedTimeEnd;
	BOOL						m_bDebugMode = false; //save to config
	BOOL						m_bTestUploadMode = false; //change name?
	BOOL						m_bSysLatInOSD = true;
	
	//the names and uses of the following 3 vars is stupid... Need to fix it
	unsigned int				myCounter = 0; 
	static unsigned int			m_loopSize; //really need to change the name of this var to "threadContinue" or something more descriptive
	static unsigned int			m_LoopCounterRefresh;
	static CString				m_strError;

	//these 2 definitely belong somewhere else...
	static DWORD				m_positionX;
	static DWORD				m_positionY;
	static BOOL					m_bPositionManualOverride;
	static INT					m_internalX, m_internalY;

	//RTSS Configs
	DWORD						m_dwSharedMemoryVersion;
	DWORD						m_dwMaxTextSize;
	BOOL						m_bFormatTagsSupported;
	BOOL						m_bObjTagsSupported;
	BOOL						m_bRTSSInitConfig = false;
	
	//previously existing members
	BOOL						m_bMultiLineOutput;
	BOOL						m_bFormatTags;
	BOOL						m_bFillGraphs;
	BOOL						m_bConnected;

	HICON						m_hIcon;
	UINT						m_nTimerID;

	CFont						m_font;
	CRichEditCtrl				m_richEditCtrl;

	static CString				m_strStatus;
	CString						m_strInstallPath;


	//for dark mode
	COLORREF m_color;
	CBrush m_brush;
	CRect clientRect;

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
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
/////////////////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////////////////
