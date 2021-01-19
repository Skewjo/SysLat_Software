#pragma once

#include "StdAfx.h"
#include "resource.h"
#include "SysLatPreferences.h"
// PreferencesDlg dialog

class PreferencesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PreferencesDlg)

	SysLatPreferences* m_pPreferences;
	
public:
	PreferencesDlg(SysLatPreferences* p_preferences, CWnd* pParent = nullptr);   // standard constructor
	virtual BOOL OnInitDialog();
	virtual ~PreferencesDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PREFERENCES_PROPPAGE };
#endif

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	bool m_bRunOnStartup = false;
	bool m_bAutoDownloadUpdates = false;
	bool m_bAutoExportLogs = false;
	bool m_bAutoUploadLogs = false;
	bool m_bDarkMode = false;

	string m_logDirectory;
	int m_MaxLogs;
	int m_MaxTestDuration;


	DECLARE_MESSAGE_MAP()
public:
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckDarkmode();
	afx_msg void OnBnClickedCheckAutoupdate();
	afx_msg void OnBnClickedCheckAutoexport();
	afx_msg void OnBnClickedCheckAutoupload();
	afx_msg void OnEnChangeMfceditbrowse();
	afx_msg void OnBnClickedCheckRunStartup();
	afx_msg void OnBnClickedButtonPrefOk();
};
