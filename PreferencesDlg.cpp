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

	//CMFCEditBrowseCtrl* pEdit;
	CSliderCtrl* pSCtrl;
	CButton* pBtn;

	//Privacy Options
	/* //The following have yet to be implemented.
	struct PrivacyOptions {
		bool						m_bAutoUploadMachineID = true;
		bool						m_bAutoUploadMachineInfo = true;
	};
	*/
	if (m_pPreferences->m_PrivacyOptions.m_bRunOnStartup == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_RUN_STARTUP);
		pBtn->SetCheck(1);
		m_bRunOnStartup = true;
	}
	if (m_pPreferences->m_PrivacyOptions.m_bAutoCheckUpdates == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPDATE);
		pBtn->SetCheck(1);
		m_bAutoDownloadUpdates = true;
	}
	if (m_pPreferences->m_PrivacyOptions.m_bAutoExportLogs == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOEXPORT);
		pBtn->SetCheck(1);
		m_bAutoExportLogs = true;
	}
	if (m_pPreferences->m_PrivacyOptions.m_bAutoUploadLogs == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPLOAD);
		pBtn->SetCheck(1);
		m_bAutoUploadLogs = true;
	}
	if (m_pPreferences->m_SysLatOptions.m_bDarkMode == true) {
		pBtn = (CButton*)GetDlgItem(IDC_CHECK_DARKMODE);
		pBtn->SetCheck(1);
		m_bDarkMode = true;
	}
	

	//SYSLAT OPTIONS
	//Log Dir
	m_logDirectory = m_pPreferences->m_SysLatOptions.m_LogDir;
	SetDlgItemText(IDC_MFCEDITBROWSE, m_pPreferences->m_SysLatOptions.m_LogDir.c_str()); 
	ASSERT_KINDOF(CMFCEditBrowseCtrl, GetDlgItem(IDC_MFCEDITBROWSE));

	//Max Logs
	m_MaxLogs = m_pPreferences->m_SysLatOptions.m_maxLogs;
	pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_MAX_LOGS);
	pSCtrl->SetRange(0, 101);
	pSCtrl->SetPos(m_pPreferences->m_SysLatOptions.m_maxLogs);
	pSCtrl->SetTicFreq(10);
	pSCtrl->SetLineSize(1);
	SetDlgItemText(IDC_STATIC_MAX_LOGS, to_string(m_pPreferences->m_SysLatOptions.m_maxLogs).c_str());
	
	//Max Test Duration
	m_MaxTestDuration = m_pPreferences->m_SysLatOptions.m_maxTestDuration;
	pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_MAX_DURATION);
	pSCtrl->SetRange(0, 61);
	pSCtrl->SetPos(m_pPreferences->m_SysLatOptions.m_maxTestDuration);
	pSCtrl->SetTicFreq(5);
	SetDlgItemText(IDC_STATIC_MAX_TEST, to_string(m_pPreferences->m_SysLatOptions.m_maxTestDuration).c_str());

	return TRUE;
}


BEGIN_MESSAGE_MAP(PreferencesDlg, CDialogEx)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_MFCEDITBROWSE, &PreferencesDlg::OnEnChangeMfceditbrowse)
	ON_BN_CLICKED(IDC_CHECK_DARKMODE, &PreferencesDlg::OnBnClickedCheckDarkmode)
	ON_BN_CLICKED(IDC_CHECK_AUTOUPDATE, &PreferencesDlg::OnBnClickedCheckAutoupdate)
	ON_BN_CLICKED(IDC_CHECK_AUTOEXPORT, &PreferencesDlg::OnBnClickedCheckAutoexport)
	ON_BN_CLICKED(IDC_CHECK_AUTOUPLOAD, &PreferencesDlg::OnBnClickedCheckAutoupload)
	ON_BN_CLICKED(IDC_CHECK_RUN_STARTUP, &PreferencesDlg::OnBnClickedCheckRunStartup)
	ON_BN_CLICKED(IDC_BUTTON_PREFOK, &PreferencesDlg::OnBnClickedButtonPrefOk)
END_MESSAGE_MAP()

void PreferencesDlg::OnBnClickedCheckRunStartup()
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_RUN_STARTUP);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_bRunOnStartup = true;
	}
	else {
		m_bRunOnStartup = false;
	}
}

void PreferencesDlg::OnBnClickedCheckAutoupdate()
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPDATE);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_bAutoDownloadUpdates = true;
	}
	else {
		m_bAutoDownloadUpdates = false;
	}
}


void PreferencesDlg::OnBnClickedCheckAutoexport()
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOEXPORT);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_bAutoExportLogs = true;
	}
	else {
		m_bAutoExportLogs = false;
	}
}


void PreferencesDlg::OnBnClickedCheckAutoupload()
{
	CButton* pBtn;
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOUPLOAD);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_bAutoUploadLogs = true;
	}
	else {
		m_bAutoUploadLogs = false;
	}
}

void PreferencesDlg::OnBnClickedCheckDarkmode()
{
	CButton* pBtn;

	pBtn = (CButton*)GetDlgItem(IDC_CHECK_DARKMODE);
	bool checked = pBtn->GetCheck();
	if (checked) {
		m_bDarkMode = true;
		//CWnd* pWnd;
		pBtn = (CButton*)GetDlgItem(IDC_STATIC_GOTYA);
		pBtn->ShowWindow(true);
	}
	else {
		m_bDarkMode = false;
	}
}

void PreferencesDlg::OnEnChangeMfceditbrowse()
{
	CMFCEditBrowseCtrl* pEdit;
	pEdit = (CMFCEditBrowseCtrl*)GetDlgItem(IDC_MFCEDITBROWSE);
	CString strText;
	pEdit->GetWindowText(strText);

	m_logDirectory = strText;
}

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
			m_MaxLogs = 10000;
			SetDlgItemText(IDC_STATIC_MAX_LOGS, "Unlimited");
		}
		else {
			m_MaxLogs = NewPos;
			SetDlgItemText(IDC_STATIC_MAX_LOGS, to_string(NewPos).c_str());
		}
		break;
	case IDC_SLIDER_MAX_DURATION:
		if (NewPos > 60) {
			m_MaxTestDuration = 10000;
			SetDlgItemText(IDC_STATIC_MAX_TEST, "Unlimited");
		}
		else {
			m_MaxTestDuration = NewPos;
			SetDlgItemText(IDC_STATIC_MAX_TEST, to_string(NewPos).c_str());
		}
		break;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void PreferencesDlg::OnBnClickedButtonPrefOk()
{
	m_pPreferences->m_PrivacyOptions.m_bRunOnStartup = m_bRunOnStartup;
	m_pPreferences->m_PrivacyOptions.m_bAutoCheckUpdates = m_bAutoDownloadUpdates;
	m_pPreferences->m_PrivacyOptions.m_bAutoExportLogs = m_bAutoExportLogs;
	m_pPreferences->m_PrivacyOptions.m_bAutoUploadLogs = m_bAutoUploadLogs;
	m_pPreferences->m_SysLatOptions.m_bDarkMode = m_bDarkMode;

	m_pPreferences->m_SysLatOptions.m_LogDir = m_logDirectory;
	m_pPreferences->m_SysLatOptions.m_maxLogs = m_MaxLogs;
	m_pPreferences->m_SysLatOptions.m_maxTestDuration = m_MaxTestDuration;

	EndDialog(IDOK);
}
