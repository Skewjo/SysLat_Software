#pragma once

#include "StdAfx.h"
#include "resource.h"
#include "SysLatData.h"


// TestCtrl dialog

class TestCtrl : public CDialogEx
{
	DECLARE_DYNAMIC(TestCtrl)

	vector<CSysLatData*>*	m_pPreviousSLD;

public:
	TestCtrl(vector<CSysLatData*>* p_previousSLD, CWnd* pParent = nullptr);   // standard constructor
	virtual ~TestCtrl();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTCTRL_DIALOG };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_TestListCtrl;
};
