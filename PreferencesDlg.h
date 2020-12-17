#pragma once

#include "StdAfx.h"
#include "resource.h"
#include "afxdialogex.h"
// PreferencesDlg dialog

class PreferencesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PreferencesDlg)

public:
	PreferencesDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~PreferencesDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PREFERENCES_PROPPAGE };
#endif

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
