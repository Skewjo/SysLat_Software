// PreferencesDlg.cpp : implementation file
//
#pragma once

#include "resource.h"
#include "StdAfx.h"
#include "SysLat_Software.h"
#include "PreferencesDlg.h"

// PreferencesDlg dialog

IMPLEMENT_DYNAMIC(PreferencesDlg, CDialogEx)

PreferencesDlg::PreferencesDlg(SysLatPreferences* p_preferences, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PREFERENCES_PROPPAGE, pParent), m_pPreferences(p_preferences)
{
}

PreferencesDlg::~PreferencesDlg()
{
}

void PreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BOOL PreferencesDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	CMFCEditBrowseCtrl* pEdit;
	CSliderCtrl* pSCtrl;
	CButton* pBtn;
	
	//SYSLAT OPTIONS
	//Log Dir
	SetDlgItemText(IDC_MFCEDITBROWSE, m_pPreferences->m_SysLatOptions.m_LogDir.c_str()); 
	ASSERT_KINDOF(CMFCEditBrowseCtrl, GetDlgItem(IDC_MFCEDITBROWSE));

	//Max Logs
	pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_MAX_LOGS);
	pSCtrl->SetRange(0, 101);
	pSCtrl->SetPos(m_pPreferences->m_SysLatOptions.m_maxLogs);
	pSCtrl->SetTicFreq(10);
	pSCtrl->SetLineSize(1);
	SetDlgItemText(IDC_STATIC_MAX_LOGS, to_string(m_pPreferences->m_SysLatOptions.m_maxLogs).c_str());
	
	//Max Test Duration
	pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_MAX_DURATION);
	pSCtrl->SetRange(0, 61);
	pSCtrl->SetPos(m_pPreferences->m_SysLatOptions.m_maxTestDuration);
	pSCtrl->SetTicFreq(5);
	//SetDlgItemText(IDC_STATIC_MAX_TEST, to_string(m_pPreferences->m_SysLatOptions.m_maxTestDuration).c_str());
	SetDlgItemText(IDC_STATIC_MAX_TEST, to_string(m_pPreferences->m_SysLatOptions.m_maxTestDuration).c_str());
	
	//Dark Mode
	if (m_pPreferences->m_SysLatOptions.m_bDarkMode == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_DARKMODE);
		pBtn->SetCheck(1);
	}


	//Privacy Options
	/* //The following have yet to be implemented.
	struct PrivacyOptions {
		bool						m_bAutoUploadMachineID = true;
		bool						m_bAutoUploadMachineInfo = true;
	};
	*/
	//Auto update program - not yet implemented
	if (m_pPreferences->m_PrivacyOptions.m_bAutoCheckUpdates == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPDATE);
		pBtn->SetCheck(1);
	}
	if (m_pPreferences->m_PrivacyOptions.m_bAutoExportLogs == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOEXPORT);
		pBtn->SetCheck(1);
	}
	if (m_pPreferences->m_PrivacyOptions.m_bAutoUploadLogs == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPLOAD);
		pBtn->SetCheck(1);
	}


	return TRUE;
}


BEGIN_MESSAGE_MAP(PreferencesDlg, CDialogEx)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_MFCEDITBROWSE, &PreferencesDlg::OnEnChangeMfceditbrowse)
	ON_BN_CLICKED(IDC_CHECK_DARKMODE, &PreferencesDlg::OnBnClickedCheckDarkmode)
	ON_BN_CLICKED(IDC_CHECK_AUTOUPDATE, &PreferencesDlg::OnBnClickedCheckAutoupdate)
	ON_BN_CLICKED(IDC_CHECK_AUTOEXPORT, &PreferencesDlg::OnBnClickedCheckAutoexport)
	ON_BN_CLICKED(IDC_CHECK_AUTOUPLOAD, &PreferencesDlg::OnBnClickedCheckAutoupload)
END_MESSAGE_MAP()

void PreferencesDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	

	// Check which slider sent the notification
	// Which is better here? C-Style cast(probably not) or the "reinterpret_cast"?
	CSliderCtrl* ACSliderCtrl = (CSliderCtrl*)pScrollBar;
	//CSliderCtrl* ACSliderCtrl = reinterpret_cast<CSliderCtrl*>(pScrollBar);
	int nID = ACSliderCtrl->GetDlgCtrlID();
	int NewPos = ((CSliderCtrl*)pScrollBar)->GetPos();
	CWnd* ACWnd = GetDlgItem(nID);

	switch (nID)
	{
		default:
			break;

		case IDC_SLIDER_MAX_LOGS:
			//m_edit1.Format("%d", NewPos);
			//UpdateData(FALSE);
			if (NewPos > 100) {
				m_pPreferences->m_SysLatOptions.m_maxLogs = 10000;
				SetDlgItemText(IDC_STATIC_MAX_LOGS, "Unlimited");
			}
			else {
				m_pPreferences->m_SysLatOptions.m_maxLogs = NewPos;
				SetDlgItemText(IDC_STATIC_MAX_LOGS, to_string(NewPos).c_str());
			}
			break;
		case IDC_SLIDER_MAX_DURATION:
			if (NewPos > 60) {
				m_pPreferences->m_SysLatOptions.m_maxTestDuration = 10000;
				SetDlgItemText(IDC_STATIC_MAX_TEST, "Unlimited");
			}
			else {
				m_pPreferences->m_SysLatOptions.m_maxTestDuration = NewPos;
				SetDlgItemText(IDC_STATIC_MAX_TEST, to_string(NewPos).c_str());
			}
			break;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void PreferencesDlg::OnEnChangeMfceditbrowse()
{
	CMFCEditBrowseCtrl* pEdit;
	pEdit = (CMFCEditBrowseCtrl*)GetDlgItem(IDC_MFCEDITBROWSE);
	CString strText;
	pEdit->GetWindowText(strText);

	m_pPreferences->m_SysLatOptions.m_LogDir = strText;
}


void PreferencesDlg::OnBnClickedCheckDarkmode()
{
	CButton* pBtn;
	
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_DARKMODE);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_pPreferences->m_SysLatOptions.m_bDarkMode = true;
		//CWnd* pWnd;
		pBtn = (CButton*)GetDlgItem(IDC_STATIC_GOTYA);
		pBtn->ShowWindow(true);
	}
	else {
		m_pPreferences->m_SysLatOptions.m_bDarkMode = false;
	}
}


void PreferencesDlg::OnBnClickedCheckAutoupdate()
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPDATE);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_pPreferences->m_PrivacyOptions.m_bAutoCheckUpdates = true;
	}
	else {
		m_pPreferences->m_PrivacyOptions.m_bAutoCheckUpdates = false;
	}
}


void PreferencesDlg::OnBnClickedCheckAutoexport()
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOEXPORT);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_pPreferences->m_PrivacyOptions.m_bAutoExportLogs = true;
	}
	else {
		m_pPreferences->m_PrivacyOptions.m_bAutoExportLogs = false;
	}
}


void PreferencesDlg::OnBnClickedCheckAutoupload()
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPLOAD);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_pPreferences->m_PrivacyOptions.m_bAutoUploadLogs = true;
	}
	else {
		m_pPreferences->m_PrivacyOptions.m_bAutoUploadLogs = false;
	}
}
