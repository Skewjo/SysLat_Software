#pragma once

#include "StdAfx.h"
#include "resource.h"
#include "afxdialogex.h"
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

	DECLARE_MESSAGE_MAP()
public:
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckDarkmode();
	afx_msg void OnBnClickedCheckAutoupdate();
	afx_msg void OnBnClickedCheckAutoexport();
	afx_msg void OnBnClickedCheckAutoupload();
	afx_msg void OnEnChangeMfceditbrowse();
};
