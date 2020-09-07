// RTSSSharedMemorySample.h : main header file for the RTSSSHAREDMEMORYSAMPLE application
//

#if !defined(AFX_RTSSSHAREDMEMORYSAMPLE_H__8215FD88_8420_491C_9CFB_8DC6FBB27DC7__INCLUDED_)
#define AFX_RTSSSHAREDMEMORYSAMPLE_H__8215FD88_8420_491C_9CFB_8DC6FBB27DC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRTSSSharedMemorySampleApp:
// See RTSSSharedMemorySample.cpp for the implementation of this class
//

class CRTSSSharedMemorySampleApp : public CWinApp
{
public:
	CRTSSSharedMemorySampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRTSSSharedMemorySampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRTSSSharedMemorySampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RTSSSHAREDMEMORYSAMPLE_H__8215FD88_8420_491C_9CFB_8DC6FBB27DC7__INCLUDED_)
