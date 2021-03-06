// TestCtrl.cpp : implementation file

#include "StdAfx.h"
#include "TestCtrl.h"

// TestCtrl dialog

IMPLEMENT_DYNAMIC(TestCtrl, CDialogEx)

TestCtrl::TestCtrl(vector<std::shared_ptr<CSysLatData>>* p_previousSLD, CWnd* pParent /*=nullptr*/)
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
		string targetApp = (*m_pPreviousSLD)[i]->m_targetApp;

		auto& testDuration = (*m_pPreviousSLD)[i]->GetTestDuration();
		string duration = format(" %R:%OS", testDuration);

		auto& data = (*m_pPreviousSLD)[i]->GetData();
		string sysLatAverage = to_string(data.m_statisticsEVR.average);
		string testCount = to_string(data.m_statisticsEVR.counter);

		bool exported = (*m_pPreviousSLD)[i]->m_bDataExported;
		bool uploaded = (*m_pPreviousSLD)[i]->m_bDataUploaded;

		nItem = m_TestListCtrl.InsertItem(i, "1");
		m_TestListCtrl.SetItemText(nItem, 1, targetApp.c_str());
		m_TestListCtrl.SetItemText(nItem, 2, duration.c_str());
		m_TestListCtrl.SetItemText(nItem, 3, sysLatAverage.c_str());
		m_TestListCtrl.SetItemText(nItem, 4, testCount.c_str());

		//NEED TO TURN THESE INTO BUTTONS - AND MAKE THEM MORE ACCURATE (BY ACCOUNTING FOR FAILURE TO UPLOAD SPECIFICALLY)
		if (data.m_statisticsEVR.counter == 0) {
			m_TestListCtrl.SetItemText(nItem, 5, "N/A");
		}
		else if (exported) {
			m_TestListCtrl.SetItemText(nItem, 5, "Yes");
		}
		else {
			m_TestListCtrl.SetItemText(nItem, 5, "No");
		}

		if (data.m_statisticsEVR.counter == 0) {
			m_TestListCtrl.SetItemText(nItem, 6, "N/A");
		}
		else if (uploaded) {
			m_TestListCtrl.SetItemText(nItem, 6, "Yes");
		}
		else {
			m_TestListCtrl.SetItemText(nItem, 6, "No");
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
