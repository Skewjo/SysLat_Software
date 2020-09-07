// RTSSSharedMemorySample.cpp : Defines the class behaviors for the application.
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RTSSSharedMemorySample.h"
#include "RTSSSharedMemorySampleDlg.h"
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CRTSSSharedMemorySampleApp
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CRTSSSharedMemorySampleApp, CWinApp)
	//{{AFX_MSG_MAP(CRTSSSharedMemorySampleApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CRTSSSharedMemorySampleApp construction
/////////////////////////////////////////////////////////////////////////////
CRTSSSharedMemorySampleApp::CRTSSSharedMemorySampleApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}
/////////////////////////////////////////////////////////////////////////////
// The one and only CRTSSSharedMemorySampleApp object
/////////////////////////////////////////////////////////////////////////////
CRTSSSharedMemorySampleApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CRTSSSharedMemorySampleApp initialization

BOOL CRTSSSharedMemorySampleApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	CRTSSSharedMemorySampleDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
