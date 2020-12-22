#pragma once

#include <vector>

#include "StdAfx.h"
#include "resource.h"
#include "afxdialogex.h"
#include "SysLatData.h"


// TestCtrl dialog

class TestCtrl : public CDialogEx
{
	DECLARE_DYNAMIC(TestCtrl)

	std::vector<CSysLatData*>*	m_pPreviousSLD;

public:
	TestCtrl(std::vector<CSysLatData*>* p_previousSLD, CWnd* pParent = nullptr);   // standard constructor
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
