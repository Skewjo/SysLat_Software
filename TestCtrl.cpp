// TestCtrl.cpp : implementation file

#include "StdAfx.h"
#include "SysLat_Software.h"
#include "TestCtrl.h"
#include "afxdialogex.h"

// TestCtrl dialog

IMPLEMENT_DYNAMIC(TestCtrl, CDialogEx)

TestCtrl::TestCtrl(std::vector<CSysLatData*>* p_previousSLD, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTCTRL_DIALOG, pParent), m_pPreviousSLD(p_previousSLD)
{
}

BOOL TestCtrl::OnInitDialog() {
	CDialogEx::OnInitDialog();

	m_TestListCtrl.InsertColumn(0, "ID", LVCFMT_LEFT, 30);     
	m_TestListCtrl.InsertColumn(1, "Target", LVCFMT_LEFT, 60);
	m_TestListCtrl.InsertColumn(2, "Duration", LVCFMT_CENTER, 60);
	m_TestListCtrl.InsertColumn(3, "Avg. SysLat", LVCFMT_CENTER, 80);
	m_TestListCtrl.InsertColumn(4, "Cycles", LVCFMT_CENTER, 60);
	m_TestListCtrl.InsertColumn(5, "Exported", LVCFMT_CENTER, 60);
	m_TestListCtrl.InsertColumn(6, "Uploaded", LVCFMT_CENTER, 60);

	int nItem;

	for (unsigned int i = 0; i < m_pPreviousSLD->size(); i++) {
		int testCount = (*m_pPreviousSLD)[i]->GetCounterEVR();
		int sysLatAverage = (*m_pPreviousSLD)[i]->GetAverageEVR();
		bool exported = (*m_pPreviousSLD)[i]->dataExported;
		bool uploaded = (*m_pPreviousSLD)[i]->dataUploaded;
		//int stuff = *m_pPreviousSLD[i]->GetCounterEVR();
		char buffer[256];

		nItem = m_TestListCtrl.InsertItem(i, "1");
		m_TestListCtrl.SetItemText(nItem, 1, "PLACEHOLDER");
		m_TestListCtrl.SetItemText(nItem, 2, "PLACEHOLDER");
		m_TestListCtrl.SetItemText(nItem, 3, _itoa(sysLatAverage, buffer, 10));
		m_TestListCtrl.SetItemText(nItem, 4, _itoa(testCount, buffer, 10));

		//NEED TO TURN THESE INTO BUTTONS - AND MAKE THEM MORE ACCURATE (BY ACCOUNTING FOR FAILURE TO UPLOAD SPECIFICALLY)
		if (testCount == 0) {
			m_TestListCtrl.SetItemText(nItem, 5, "N/A");
		}
		else if (exported) {
			m_TestListCtrl.SetItemText(nItem, 5, "Yes");
		}
		else {
			m_TestListCtrl.SetItemText(nItem, 5, "X");
		}

		if (testCount == 0) {
			m_TestListCtrl.SetItemText(nItem, 6, "N/A");
		}
		else if (uploaded) {
			m_TestListCtrl.SetItemText(nItem, 6, "Yes");
		}
		else {
			m_TestListCtrl.SetItemText(nItem, 6, "X");
		}
	}
	return TRUE; // return TRUE unless you set the focus to a control
}




TestCtrl::~TestCtrl()
{
}

void TestCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TESTLISTCTRL, m_TestListCtrl);
}


BEGIN_MESSAGE_MAP(TestCtrl, CDialogEx)
END_MESSAGE_MAP()


// TestCtrl message handlers
