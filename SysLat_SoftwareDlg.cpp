// SysLat_SoftwareDlg.cpp : implementation file
//
// created by Unwinder
// modified by Skewjo
/////////////////////////////////////////////////////////////////////////////

#include "resource.h"

#include "stdafx.h"
#include <Windows.h>
#include <process.h>
#include <shlwapi.h>
#include <float.h>
#include <io.h>
#include <sstream>
#include <algorithm>
#include <uuids.h>
#include <Psapi.h>

/////////////////////////////////////////////////////////////////////////////
#include "AboutDlg.h"
#include "SysLat_Software.h"
#include "SysLat_SoftwareDlg.h"
#include "USBController.h"
#include "HTTP_Client_Async.h"
#include "HTTP_Client_Async_SSL.h"


//TODO:
// Transfer TODO to GitHub Issues...
// DONE - Organize TODO...
//		DONE - Core Functionality
//		DONE - Menu
//		DONE - Data Issues
//		DONE - Optimization
//		DONE - Organizational Issues
//		DONE - Anti-Fraud
//
//
//Issues completed before the TODO reorg:
//  NOT AVAILABLE(?) - Profile Setting - Set "Refresh Period" to 0 milliseconds  - doesn't appear to be an option available via shared memory
//  DONE - Profile Setting - Change default corner to bottom right
//  DONE (opacity) - Profile Setting - Change "text"(foreground) color to white (255, 255, 255) with an opacity of 100
//  DONE - Profile Setting - Change color of "background"(?) to black (0, 0, 0) with an opacity of 100
//		DONE - Better yet - if I could change the box to use a plain black and plain white box so any other text isn't screwed up, that would be better
//  DONE - Profile Setting - Set box size to what I want it to be?
//  DONE - Change class/namespace name of RTSSSharedMemorySampleDlg to SysLatDlg
//  DONE - Change class/namespace of RTSSSharedMemorySample to SysLat
//  DONE - Add minimize button
//  DONE(well... it half-ass works) - Make System Latency appear in OSD
//  DONE - Save results to a table - using an array
//  DONE - Determine active window vs window that RTSS is operating in?
//  DONE - Launch RTSS automatically in the background if it's not running
//  DONE - Add hotkey to restart readings (F11?)
//  DONE - Seperate some initialization that happens in "Refresh" function into a different "Refresh-like" function?? - partially done?
//  DONE - Re-org this file into 3-4 new classes - Dialog related functions, RTSS related, DrawingThread related, and USB related
//  DONE - BUT THERE ARE PROBLEMS(it just changed the priority of which client(syslat vs syslatStats) - Make the program statically linked so that it all packages together nicely in a single DLL
//  DONE(BUT NOT GREAT) - Dynamically build the "drawSquare" string and change the P tag to account for the current corner and all other OSD text?
//		DONE(Mostly...new issue created) - Then create an option to disable that setting - add keyboard arrow functionality to move it into place manually.
//
//
//
//
//Core Functionality:
//  DONE - Add HTTP post function for uploading logs to website - use boost.beast library?
//  Errors currently appear very briefly and are overwritten when the refresh function runs - Clean up the refresh function, then come up with new error scheme.
//		Either use error codes, or check all errors  again in the refresh function(that doesn't make sense though, right?)... or maybe do dialog error pop-ups when errors occur outside of "refresh"?
//  Move ExportData function out of SysLatData? Or just use it to retrieve a jsoncpp object & combine it with other jsoncpp objects
//  Make executable/window names mesh better together?  Need a map/lookup table or something? - JUST USE PID YA IDIOT
//
//
//Data Issues:
//  #1 - Put elapsed time in log file
//  Save fps and frametime and other stats as well?
//  Add graph functionality
//  Clear log files and put a configurable(?) cap on the number allowed
//  Keep track of total tests performed in a config file vs. looking for existing log files and picking up from there?
//		How many tests should we allow total? 100? 
//		Would it be fine if SysLat overwrote the tests every time it was restarted? ...I think it would
//
//
//Menu:
//  Create "Test Viewer" menu for viewing, exporting, and uploading tests
//  Enumerate all 3D programs that RTSS can run in and display them in a menu
//  Fix COM port change settings
//  Add lots more menu options - USB options, debug output, data upload, RTSS options(text color)
//  Option to close RTSS when you close SysLat
//  DONE - Box position manual override toggle
//
//
//Anti-Fraud:
//  Create new dynamic build/installation process in order to obscure some code
//  Think about hardware/software signatures for uploading data? This probably needs more consideration on the web side
//  Obscure most functionality(things that don't need to be optimized) into DLLs(requires a new build/installation process)
//  #1 - (Anti-Fraud, Optimization, and Data)Instead of recording certain variables on every measurement(such as RTSS XY position) record them once at the start and once at the end
//
//
//Optimization:
//  Move data update at the end of the CreateDrawingThread function into a different thread(or co-routine?)
//  Calculating the position of the box before we draw it adds unnecessary delay(?)
//
//
//Organizational Issues:
//  Clean up(or get rid of) static vars in SysLat_SoftwareDlg class
//  Clean up the refresh function a bit more by making some init functionality conditional
//
//

//////////////////////////////////////////////////////////////////////////////////////////////
//Major Bugs:
//  FIXED - The SysLat RTSSClient object cannot obtain the "0th" RTSS OSD - slot when restarting a test
//	FIXED - Issue when switching COM ports to an existing device that isn't SysLat and back
//  FIXED - Arrow key functionality has been optimized away somehow
//  
//
//Minor Bugs:
//  UNFIXABLE - SysLat may not play nicely with other applications that use RTSS such as MSI Afterburner, or with advanced RTSS setups
//
//////////////////////////////////////////////////////////////////////////////////////////////


/// 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Define static variables - these should probably be done as inline... inlining is supposed to be available in C++17 and above, but Visual Studio throws a fit when I try to inline these.
CString CSysLat_SoftwareDlg::m_strStatus = "";
unsigned int CSysLat_SoftwareDlg::m_LoopCounterRefresh = 0;
unsigned int CSysLat_SoftwareDlg::m_loopSize = 0xFFFFFFFF;
CString	CSysLat_SoftwareDlg::m_updateString = "";
CString CSysLat_SoftwareDlg::m_PortSpecifier = "COM3";
CString CSysLat_SoftwareDlg::m_strError = "";
CSysLatData* CSysLat_SoftwareDlg::m_pOperatingSLD = new CSysLatData;

DWORD CSysLat_SoftwareDlg::m_positionX = 0;
DWORD CSysLat_SoftwareDlg::m_positionY = 0;
BOOL CSysLat_SoftwareDlg::m_bPositionManualOverride = false;
INT CSysLat_SoftwareDlg::m_internalX = 0;
INT CSysLat_SoftwareDlg::m_internalY = 0;

CString CSysLat_SoftwareDlg::m_strBlack = "<C=000000><B=10,10><C>";
CString CSysLat_SoftwareDlg::m_strWhite = "<C=FFFFFF><B=10,10><C>";
DWORD CSysLat_SoftwareDlg::m_sysLatOwnedSlot = 0;








#define DAYS_IN_WEEK 7

// Creates a tab control, sized to fit the specified parent window's client
//   area, and adds some tabs. 
// Returns the handle to the tab control. 
// hwndParent - parent window (the application's main window). 
// 
HWND DoCreateTabControl(HWND hwndParent)
{
	RECT rcClient;
	INITCOMMONCONTROLSEX icex;
	HWND hwndTab;
	TCITEM tie;
	int i;
	TCHAR achTemp[256];  // Temporary buffer for strings.

	// Initialize common controls.
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&icex);

	// Get the dimensions of the parent window's client area, and 
	// create a tab control child window of that size. Note that g_hInst
	// is the global instance handle.
	GetClientRect(hwndParent, &rcClient);
	
	
	HINSTANCE g_hInst = GetModuleHandle(NULL);
	hwndTab = CreateWindow(WC_TABCONTROL, "",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		0, 0, rcClient.right, rcClient.bottom,
		hwndParent, NULL, g_hInst, NULL);
	if (hwndTab == NULL)
	{
		return NULL;
	}

	// Add tabs for each day of the week. 
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;
	tie.pszText = achTemp;

	for (i = 0; i < DAYS_IN_WEEK; i++)
	{
		// Load the day string from the string resources. Note that
		// g_hInst is the global instance handle.
		UINT IDS_SUNDAY = 0;
		LoadString(g_hInst, IDS_SUNDAY + i,
			achTemp, sizeof(achTemp) / sizeof(achTemp[0]));
		if (TabCtrl_InsertItem(hwndTab, i, &tie) == -1)
		{
			DestroyWindow(hwndTab);
			return NULL;
		}
	}
	return hwndTab;
}



HWND DoCreateDisplayWindow(HWND hwndTab)
{
	HINSTANCE g_hInst = GetModuleHandle(NULL);
	HWND hwndStatic = CreateWindow(WC_STATIC, "",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		100, 100, 100, 100,        // Position and dimensions; example only.
		hwndTab, NULL, g_hInst,    // g_hInst is the global instance handle
		NULL);
	return hwndStatic;
}

// Handles the WM_SIZE message for the main window by resizing the 
//   tab control. 
// hwndTab - handle of the tab control.
// lParam - the lParam parameter of the WM_SIZE message.
//
HRESULT OnSize(HWND hwndTab, LPARAM lParam)
{
	RECT rc;

	if (hwndTab == NULL)
		return E_INVALIDARG;

	// Resize the tab control to fit the client are of main window.
	if (!SetWindowPos(hwndTab, HWND_TOP, 0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), SWP_SHOWWINDOW))
		return E_FAIL;

	return S_OK;
}

// Handles notifications from the tab control, as follows: 
//   TCN_SELCHANGING - always returns FALSE to allow the user to select a 
//     different tab.  
//   TCN_SELCHANGE - loads a string resource and displays it in a static 
//     control on the selected tab.
// hwndTab - handle of the tab control.
// hwndDisplay - handle of the static control. 
// lParam - the lParam parameter of the WM_NOTIFY message.
//
BOOL OnNotify(HWND hwndTab, HWND hwndDisplay, LPARAM lParam)
{
	TCHAR achTemp[256]; // temporary buffer for strings

	switch (((LPNMHDR)lParam)->code)
	{
	case TCN_SELCHANGING:
	{
		// Return FALSE to allow the selection to change.
		return FALSE;
	}

	case TCN_SELCHANGE:
	{
		int iPage = TabCtrl_GetCurSel(hwndTab);
		HINSTANCE g_hInst = GetModuleHandle(NULL);
		// Note that g_hInst is the global instance handle.
		LoadString(g_hInst, 0 + iPage, achTemp,
			sizeof(achTemp) / sizeof(achTemp[0]));
		LRESULT result = SendMessage(hwndDisplay, WM_SETTEXT, 0,
			(LPARAM)achTemp);
		break;
	}
	}
	return TRUE;
}





// Handles the WM_INITDIALOG message for a dialog box that contains 
//   a tab control used to select among three child dialog boxes.
// Returns a result code.
// hwndDlg - handle of the dialog box.
// 

// Loads and locks a dialog box template resource. 
// Returns the address of the locked dialog box template resource. 
// lpszResName - name of the resource. 
//
DLGTEMPLATEEX* DoLockDlgRes(LPCTSTR lpszResName)
{
	HRSRC hrsrc = FindResourceExA(NULL, lpszResName, RT_DIALOG, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

	HINSTANCE g_hInst = GetModuleHandle(NULL);
	// Note that g_hInst is the global instance handle
	HGLOBAL hglb = LoadResource(g_hInst, hrsrc);
	return (DLGTEMPLATEEX*)LockResource(hglb);
}

// Processes the TCN_SELCHANGE notification. 
// hwndDlg - handle to the parent dialog box. 
//
VOID OnSelChanged(HWND hwndDlg)
{
	// Get the dialog header data.
	DLGHDR* pHdr = (DLGHDR*)GetWindowLongPtr(
		hwndDlg, GWLP_USERDATA);

	// Get the index of the selected tab.
	int iSel = TabCtrl_GetCurSel(pHdr->hwndTab);

	// Destroy the current child dialog box, if any. 
	if (pHdr->hwndDisplay != NULL)
		DestroyWindow(pHdr->hwndDisplay);

	HINSTANCE g_hInst = GetModuleHandle(NULL);
	// Create the new child dialog box. Note that g_hInst is the
	// global instance handle.
	//pHdr->hwndDisplay = CreateDialogIndirectA(g_hInst,
	//	(DLGTEMPLATE*)pHdr->apRes[iSel], hwndDlg, ChildDialogProc);

	return;
}


HRESULT OnTabbedDialogInit(HWND hwndDlg)
{
	INITCOMMONCONTROLSEX iccex;
	DWORD dwDlgBase = GetDialogBaseUnits();
	int cxMargin = LOWORD(dwDlgBase) / 4;
	int cyMargin = HIWORD(dwDlgBase) / 8;

	TCITEM tie;
	RECT rcTab;
	HWND hwndButton;
	RECT rcButton;
	int i;

	// Initialize common controls.
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&iccex);

	// Allocate memory for the DLGHDR structure. Remember to 
	// free this memory before the dialog box is destroyed.
	DLGHDR* pHdr = (DLGHDR*)LocalAlloc(LPTR, sizeof(DLGHDR));

	// Save a pointer to the DLGHDR structure in the window
	// data of the dialog box. 
	SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pHdr);

	// Create the tab control. Note that g_hInst is a global 
	// instance handle. 
	HINSTANCE g_hInst = GetModuleHandle(NULL);
	pHdr->hwndTab = CreateWindow(
		WC_TABCONTROL, "",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		0, 0, 100, 100,
		hwndDlg, NULL, g_hInst, NULL
	);
	if (pHdr->hwndTab == NULL)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Add a tab for each of the three child dialog boxes. 
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;
	tie.pszText = "First";
	TabCtrl_InsertItem(pHdr->hwndTab, 0, &tie);
	//tie.pszText = "Second";
	//TabCtrl_InsertItem(pHdr->hwndTab, 1, &tie);
	//tie.pszText = "Third";
	//TabCtrl_InsertItem(pHdr->hwndTab, 2, &tie);

	// Lock the resources for the three child dialog boxes. 
	
	pHdr->apRes[0] = DoLockDlgRes(MAKEINTRESOURCE(IDC_PLACEHOLDER));
	//pHdr->apRes[1] = DoLockDlgRes(MAKEINTRESOURCE(IDD_SECONDDLG));
	//pHdr->apRes[2] = DoLockDlgRes(MAKEINTRESOURCE(IDD_THIRDDLG));

	// Determine a bounding rectangle that is large enough to 
	// contain the largest child dialog box. 
	SetRectEmpty(&rcTab);
	for (i = 0; i < C_PAGES; i++)
	{
		if (pHdr->apRes[i]->cx > rcTab.right)
			rcTab.right = pHdr->apRes[i]->cx;
		if (pHdr->apRes[i]->cy > rcTab.bottom)
			rcTab.bottom = pHdr->apRes[i]->cy;
	}

	// Map the rectangle from dialog box units to pixels.
	MapDialogRect(hwndDlg, &rcTab);

	// Calculate how large to make the tab control, so 
	// the display area can accommodate all the child dialog boxes. 
	TabCtrl_AdjustRect(pHdr->hwndTab, TRUE, &rcTab);
	OffsetRect(&rcTab, cxMargin - rcTab.left, cyMargin - rcTab.top);

	// Calculate the display rectangle. 
	CopyRect(&pHdr->rcDisplay, &rcTab);
	TabCtrl_AdjustRect(pHdr->hwndTab, FALSE, &pHdr->rcDisplay);

	// Set the size and position of the tab control, buttons, 
	// and dialog box. 
	SetWindowPos(pHdr->hwndTab, NULL, rcTab.left, rcTab.top,
		rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
		SWP_NOZORDER);

	// Move the first button below the tab control. 
	//hwndButton = GetDlgItem(hwndDlg, IDB_CLOSE);
	//SetWindowPos(hwndButton, NULL,
	//	rcTab.left, rcTab.bottom + cyMargin, 0, 0,
	//	SWP_NOSIZE | SWP_NOZORDER);

	// Determine the size of the button. 
	GetWindowRect(hwndButton, &rcButton);
	rcButton.right -= rcButton.left;
	rcButton.bottom -= rcButton.top;

	// Move the second button to the right of the first. 
	//hwndButton = GetDlgItem(hwndDlg, IDB_TEST);
	//SetWindowPos(hwndButton, NULL,
	//	rcTab.left + rcButton.right + cxMargin,
	//	rcTab.bottom + cyMargin, 0, 0,
	//	SWP_NOSIZE | SWP_NOZORDER);

	// Size the dialog box. 
	SetWindowPos(hwndDlg, NULL, 0, 0,
		rcTab.right + cyMargin + (2 * GetSystemMetrics(SM_CXDLGFRAME)),
		rcTab.bottom + rcButton.bottom + (2 * cyMargin)
		+ (2 * GetSystemMetrics(SM_CYDLGFRAME))
		+ GetSystemMetrics(SM_CYCAPTION),
		SWP_NOMOVE | SWP_NOZORDER);

	// Simulate selection of the first item. 
	OnSelChanged(hwndDlg);

	return S_OK;
}




// Positions the child dialog box to occupy the display area of the 
//   tab control. 
// hwndDlg - handle of the dialog box.
//
VOID WINAPI OnChildDialogInit(HWND hwndDlg)
{
	HWND hwndParent = GetParent(hwndDlg);
	DLGHDR* pHdr = (DLGHDR*)GetWindowLongPtr(
		hwndParent, GWLP_USERDATA);
	SetWindowPos(hwndDlg, NULL, pHdr->rcDisplay.left,
		pHdr->rcDisplay.top,//-2,
		(pHdr->rcDisplay.right - pHdr->rcDisplay.left),
		(pHdr->rcDisplay.bottom - pHdr->rcDisplay.top),
		SWP_SHOWWINDOW);

	return;
}























//Windows Dialog inherited function overrides
/////////////////////////////////////////////////////////////////////////////
// CSysLat_SoftwareDlg dialog
/////////////////////////////////////////////////////////////////////////////
CSysLat_SoftwareDlg::CSysLat_SoftwareDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSysLat_SoftwareDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSysLat_SoftwareDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	m_hIcon						= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_strStatus					= "";
	m_strInstallPath			= "";

	m_bMultiLineOutput			= TRUE;
	m_bFormatTags				= TRUE;
	m_bFillGraphs				= FALSE;
	m_bConnected				= FALSE;
}
void CSysLat_SoftwareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysLat_SoftwareDlg)
	//}}AFX_DATA_MAP
}
BEGIN_MESSAGE_MAP(CSysLat_SoftwareDlg, CDialog)
	//{{AFX_MSG_MAP(CSysLat_SoftwareDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_PORT_COM1, CSysLat_SoftwareDlg::SetPortCom1)
	ON_COMMAND(ID_PORT_COM2, CSysLat_SoftwareDlg::SetPortCom2)
	ON_COMMAND(ID_PORT_COM3, CSysLat_SoftwareDlg::SetPortCom3)
	ON_COMMAND(ID_PORT_COM4, CSysLat_SoftwareDlg::SetPortCom4)
	ON_COMMAND(ID_TOOLS_EXPORTDATA, CSysLat_SoftwareDlg::ExportData)
	ON_COMMAND(ID_TOOLS_UPLOADDATA, CSysLat_SoftwareDlg::UploadData)
	ON_COMMAND(ID_SETTINGS_DEBUGMODE, CSysLat_SoftwareDlg::DebugMode)
	ON_COMMAND(ID_SETTINGS_TESTUPLOADMODE, CSysLat_SoftwareDlg::TestUploadMode)
	ON_COMMAND(ID_SETTINGS_DISPLAYSYSLATINOSD, CSysLat_SoftwareDlg::DisplaySysLatInOSD)
	ON_COMMAND(ID_TOOLS_NEWTEST, CSysLat_SoftwareDlg::ReInitThread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CSysLat_SoftwareDlg message handlers
/////////////////////////////////////////////////////////////////////////////
BOOL CSysLat_SoftwareDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon

	//
	//HRESULT stuff = OnTabbedDialogInit(hwndParent);







	InitCommonControlsEx;
	CWnd* pTab = GetDlgItem(IDC_TAB1);
	if (pTab) {
		CRect rect;

		m_TabCtrl = (CTabCtrl*)pTab;
		m_TabCtrl->GetClientRect(&rect);
		
		
		m_TabCtrl->InsertItem(0, "Stats");
		m_TabCtrl->InsertItem(1, "Settings");
		m_TabCtrl->AdjustRect(true, &rect);

		BOOL getRect = m_TabCtrl->GetItemRect(0, &rect);
		//TCS_FIXEDWIDTH

		if (!m_richEditCtrl.Create(WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | WS_VSCROLL, rect, m_TabCtrl, 0))
			return FALSE;

		//if (!m_richEditCtrl2.Create(WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | WS_VSCROLL, rect, m_TabCtrl->GetWindow(1), 0))
			//return FALSE;

		m_font.CreateFont(-11, 0, 0, 0, FW_REGULAR, 0, 0, 0, BALTIC_CHARSET, 0, 0, 0, 0, "Courier New");
		m_richEditCtrl.SetFont(&m_font);
	}




	//init timers
	m_nTimerID = SetTimer(0x1234, 1000, NULL);	//Used by OnTimer function to refresh dialog box & OSD
	time(&m_elapsedTimeStart);					//Used to keep track of test length

	

	

	char userName[UNLEN + 1] = { 0 };
	DWORD userNameSize = UNLEN + 1;
	GetUserName(userName, &userNameSize);
	OutputDebugStringA("\n");
	OutputDebugStringA(userName);

	char computerName[UNLEN + 1] = { 0 };
	DWORD computerNameSize = UNLEN + 1;
	GetComputerName(computerName, &computerNameSize);
	OutputDebugStringA("\n");
	OutputDebugStringA(computerName);
	OutputDebugStringA("\n");




	

	

	Refresh();

	
	unsigned threadID;
	drawingThreadHandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, &myCounter, 0, &threadID);
	SetThreadPriority(drawingThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);//31 is(apparently?) the highest possible thread priority - may be bad because it could cause deadlock using a loop? Need to read more here: https://docs.microsoft.com/en-us/windows/win32/procthread/scheduling-priorities

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CSysLat_SoftwareDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
/////////////////////////////////////////////////////////////////////////////
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
/////////////////////////////////////////////////////////////////////////////
void CSysLat_SoftwareDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
HCURSOR CSysLat_SoftwareDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}
void CSysLat_SoftwareDlg::OnTimer(UINT nIDEvent)
{
	Refresh();
	CDialog::OnTimer(nIDEvent);
}
void CSysLat_SoftwareDlg::OnDestroy()
{
	if (m_nTimerID)
		KillTimer(m_nTimerID);

	m_nTimerID = NULL;

	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));

	TerminateThread(drawingThreadHandle, 0); //Does exit code need to be 0 for this?
	sysLatStatsClient.ReleaseOSD();
	m_pOperatingSLD->CloseSLDMutex();

	CDialog::OnDestroy();
}


//Skewjo's Dialog functions
std::string CSysLat_SoftwareDlg::GetProcessNameFromPID(DWORD processID) {
	std::string ret;
	HANDLE Handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		processID
	);
	if (Handle)
	{
		DWORD buffSize = 1024;
		CHAR Buffer[1024];
		if (QueryFullProcessImageNameA(Handle, 0, Buffer, &buffSize))
		{
			ret = strrchr(Buffer, '\\') + 1; // I can't believe this works
		}
		else
		{

			printf("Error GetModuleBaseNameA : %lu", GetLastError());
		}
		CloseHandle(Handle);
	}
	else
	{
		printf("Error OpenProcess : %lu", GetLastError());
	}
	return ret;

}
std::string CSysLat_SoftwareDlg::GetActiveWindowTitle()
{
	char wnd_title[256];
	CWnd* pWnd = GetForegroundWindow();
	::GetWindowText((HWND)*pWnd, wnd_title, 256); //Had to use scope resolution because this function is defined in both WinUser.h and afxwin.h
	return wnd_title;
}
void CSysLat_SoftwareDlg::ProcessNameTrim(std::string& processName, std::string& activeWindowTitle){
	size_t pos = processName.find(".exe");
	if (pos != std::string::npos) {
		processName.replace(pos, processName.size(), "");
	}
	while ((pos = processName.find(" ")) != std::string::npos) {
		processName.replace(pos, 1, "");
	}
	std::transform(processName.begin(), processName.end(), processName.begin(), [](unsigned char c) { return std::tolower(c); });
	while ((pos = activeWindowTitle.find(" ")) != std::string::npos) {
		activeWindowTitle.replace(pos, 1, "");
	}
	std::transform(activeWindowTitle.begin(), activeWindowTitle.end(), activeWindowTitle.begin(), [](unsigned char c) { return std::tolower(c); });
}

BOOL CSysLat_SoftwareDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_F11:
			ReInitThread();
			return TRUE;
		case ' ':
			if (!m_bConnected)
			{
				if (!CRTSSClient::m_strInstallPath.IsEmpty())
					ShellExecute(GetSafeHwnd(), "open", CRTSSClient::m_strInstallPath, NULL, NULL, SW_SHOWNORMAL);
			}
			return TRUE;
		//these next 4 keybinds are "optimized out" or something?? WTF???
		case VK_UP:
			if (m_internalY > 0) {
				m_internalY--;
			}
			//else appendError ??
			m_bPositionManualOverride = true;
			return TRUE;
		case VK_DOWN:
			if (m_internalY < 255) {
				m_internalY++;
			}
			m_bPositionManualOverride = true;
			return TRUE;
		case VK_LEFT:
			if (m_internalX > 0) {
				m_internalX--;
			}
			m_bPositionManualOverride = true;
			return TRUE;
		case VK_RIGHT:
			if (m_internalX < 255) {
				m_internalX++;
			}
			m_bPositionManualOverride = true;
			return TRUE;
		case 'R':
			CRTSSClient::SetProfileProperty("", "BaseColor", 0xFF0000);
			return TRUE;
		case 'G':
			CRTSSClient::SetProfileProperty("", "BaseColor", 0x00FF00);
			return TRUE;
		case 'B':
			CRTSSClient::SetProfileProperty("", "BaseColor", 0x0000FF);
			return TRUE;
		case 'F':
			if (m_bConnected)
			{
				m_bFormatTags = !m_bFormatTags;
				Refresh();
			}
			break;
		case 'I':
			if (m_bConnected)
			{
				m_bFillGraphs = !m_bFillGraphs;
				Refresh();
			}
			break;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
void CSysLat_SoftwareDlg::Refresh()
{
	if (!(CRTSSClient::m_profileInterface.IsInitialized())) {
		CRTSSClient::InitRTSSInterface();
	}
	m_positionX = CRTSSClient::GetProfileProperty("", "PositionX");
	m_positionY = CRTSSClient::GetProfileProperty("", "PositionY");
	if (m_bConnected && !m_bRTSSInitConfig) {
		R_GetRTSSConfigs();
	}
	else {
		m_bRTSSInitConfig = false;
	}
	
	m_strStatus = "";
	if (!R_SysLatStats()) return;
	if (m_bDebugMode) {
		R_Position();
		R_ProcessNames();
	}
	if (m_bSysLatInOSD) {
		R_StrOSD();
	}
	else if (!m_bSysLatInOSD) { //need to add another condition to make this only happen once so that it will clear whatever exists in the buffer... or maybe use the releaseOSD function properly? IDK
		sysLatStatsClient.UpdateOSD("");
	}

	//Need to make a new function & boolean for displaying controls/hints
	if (CRTSSClient::m_profileInterface.IsInitialized())
	{
		m_strStatus += "\n\n-Press <Up>,<Down>,<Left>,<Right> to change OSD position in global profile";
		m_strStatus += "\n-Press <R>,<G>,<B> to change OSD color in global profile";
	}
	
	if (!m_strError.IsEmpty())
	{
		m_strStatus += "\n\nErrors:";
		m_strStatus.Append(m_strError);
		m_strError = "";
	}

	m_richEditCtrl.SetWindowText(m_strStatus);
}
void CSysLat_SoftwareDlg::R_GetRTSSConfigs() {
	m_dwSharedMemoryVersion = CRTSSClient::GetSharedMemoryVersion();
	m_dwMaxTextSize = (m_dwSharedMemoryVersion >= 0x00020007) ? sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSDEx) : sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSD);
	m_bFormatTagsSupported = (m_dwSharedMemoryVersion >= 0x0002000b);	//text format tags are supported for shared memory v2.11 and higher
	m_bObjTagsSupported = (m_dwSharedMemoryVersion >= 0x0002000c);		//embedded object tags are supporoted for shared memory v2.12 and higher
	m_bRTSSInitConfig = true;
}
BOOL CSysLat_SoftwareDlg::R_SysLatStats() {

	//this timer stuff definitely needs to be in the SLD
	time(&m_elapsedTimeEnd);
	double dif = difftime(m_elapsedTimeEnd, m_elapsedTimeStart);
	int minutes = static_cast<int>(dif) / 60;
	int seconds = static_cast<int>(dif) % 60;
	double measurementsPerSecond = m_pOperatingSLD->GetCounter() / dif;

	BOOL success = m_pOperatingSLD->AcquireSLDMutex();		// begin the sync access to fields
	if (!success)
		return success;

	m_strStatus.AppendFormat("Elapsed Time: %02d:%02d", minutes, seconds);
	m_strStatus.AppendFormat("\nSystem Latency: %s", m_pOperatingSLD->GetStringResult());
	m_strStatus.AppendFormat("\nLoop Counter : %d", m_pOperatingSLD->GetCounter());
	m_strStatus.AppendFormat("\nMeasurements Per Second: %.2f", measurementsPerSecond); //This value should probably be in the SLD...
	m_strStatus.AppendFormat("\nSystem Latency Average: %.2f", m_pOperatingSLD->GetAverage());
	m_strStatus.AppendFormat("\nLoop Counter EVR(expected value range, 3-100): %d ", m_pOperatingSLD->GetCounterEVR());
	m_strStatus.AppendFormat("\nSystem Latency Average(EVR): %.2f", m_pOperatingSLD->GetAverageEVR());

	m_pOperatingSLD->ReleaseSLDMutex();		// end the sync access to fields

	return success;
}
void CSysLat_SoftwareDlg::R_Position() {
	m_strStatus.AppendFormat("\n\nClients num: %d", CRTSSClient::clientsNum);
	m_strStatus.AppendFormat("\nSysLat Owned Slot: %d", m_sysLatOwnedSlot);
	m_strStatus.AppendFormat("\nPositionX: %d", m_positionX);
	m_strStatus.AppendFormat("\nPositionY: %d", m_positionY);
}
void CSysLat_SoftwareDlg::R_ProcessNames() {
	DWORD dwLastForegroundAppProcessID = CRTSSClient::GetLastForegroundAppID();
	m_strStatus.Append("\n\nLast RTSS Foreground App Name: ");
	std::string processName = GetProcessNameFromPID(dwLastForegroundAppProcessID);
	m_strStatus += processName.c_str();
	m_strStatus.Append("\nCurrently active window: ");
	std::string activeWindowTitle = GetActiveWindowTitle();
	m_strStatus += activeWindowTitle.c_str();
	ProcessNameTrim(processName, activeWindowTitle);
	m_strStatus.Append("\nTrimmed:");
	m_strStatus += processName.c_str();
	m_strStatus.Append("\nTrimmed:");
	m_strStatus += activeWindowTitle.c_str();
}
void CSysLat_SoftwareDlg::R_StrOSD() {
	//BOOL bTruncated = FALSE;
	CString strOSD;// = strOSDBuilder.Get(bTruncated);
	strOSD += m_pOperatingSLD->GetStringResult();
	if (!strOSD.IsEmpty())
	{
		BOOL bResult = sysLatStatsClient.UpdateOSD(strOSD);

		m_bConnected = bResult;

		if (bResult)
		{
			m_strStatus += "\n\nThe following text is being forwarded to OSD:\nFrom SysLat client: " + m_updateString + "\nFrom SysLatStats client: " + strOSD;

			if (m_bFormatTagsSupported)
				m_strStatus += "\n-Press <F> to toggle OSD text formatting tags";

			if (m_bFormatTagsSupported)
				m_strStatus += "\n-Press <I> to toggle graphs fill mode";
			//if (bTruncated)
			//	AppendError("Warning: The text is too long to be displayed in OSD, some info has been truncated!");
		}
		else
		{
			if (CRTSSClient::m_strInstallPath.IsEmpty())
				AppendError("Error: Failed to connect to RTSS shared memory!\nHints:\n-Install RivaTuner Statistics Server");
			else
				AppendError("Error: Failed to connect to RTSS shared memory!\nHints:\n-Press <Space> to start RivaTuner Statistics Server");
		}

		if (m_sysLatOwnedSlot != 0) {
			AppendError("The SysLat client is unable to occupy RTSS client slot 0.\nThis may cause issues with the blinking square appearing in the corner.\nTo resolve this error try one of the following:\n\t1. Close other applications that use RTSS(such as MSI Afterburner)\n\t2. Restart RTSS\n\t3. Restart the testing phase(by pressing <F11>).");
		}
	}
}
void CSysLat_SoftwareDlg::AppendError(const CString& error)
{
	m_strError.Append("\n");
	m_strError.Append(error);
	m_strError.Append("\n");
}



//SysLat thread functions
void CSysLat_SoftwareDlg::ReInitThread() { 
	//since implementing the the RTSS and active window string arrays in the SysLatData class, this function now hangs/freezes (sometimes?) - most likely because it's trying to create a new object with a struct that contains 3 arrays that are  3600 ints, and 2 that are 3600 strings...
	//Set loop size to 0, wait for thread to finish so that it closes the COM port, then reset loop size before you kick off a new thread - THIS PROBLEM GOES AWAY IF I PUT *ANY* BREAKPOINTS IN THIS FUNCTION???
	m_loopSize = 0;
	//DWORD waitForThread;
	//do {
	//waitForThread = 
		WaitForSingleObjectEx(drawingThreadHandle, INFINITE, false); // since this is the thread created by the one and only "beginThreadEx" function... does cleanup of this thread automatically occur when the function ends?
	//} while (waitForThread != WAIT_OBJECT_0);
	//CloseHandle(drawingThreadHandle);
	
	m_pOperatingSLD->SetEndTime();
	m_loopSize = 0xFFFFFFFF;

	//resets the timer 
	time(&m_elapsedTimeStart); //probably need to add a second timer for total run time
	myCounter = 0;

	//"save" the data from the test that just completed
	m_previousSLD.push_back(m_pOperatingSLD);
	m_pOperatingSLD = new CSysLatData;
	
	//restart the thread
	drawingThreadHandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, &myCounter, 0, 0);
	SetThreadPriority(drawingThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);

	//convert the previous data to JSON
	m_previousSLD.back()->CreateJSONSLD();
	//then export it
	//then upload it
}
unsigned int __stdcall CSysLat_SoftwareDlg::CreateDrawingThread(void* data) //this is probably dangerous, right?
{
	int TIMEOUT = 5; //this should probably be a defined constant
	int serialReadData = 0;
	CString	sysLatResults;
	CRTSSClient sysLatClient("SysLat", 0);
	m_sysLatOwnedSlot = sysLatClient.ownedSlot;

	CUSBController usbController;
	HANDLE hPort = usbController.OpenComPort(m_PortSpecifier);

	while (!usbController.IsComPortOpened(hPort) && m_loopSize > 0)
	{
		hPort = usbController.OpenComPort(m_PortSpecifier);
		AppendError("Failed to open COM port: " + m_PortSpecifier);
		//poll the device once per second
		Sleep(1000);
	}

	DrawSquare(sysLatClient, m_strBlack);

	for (unsigned int loopCounter = 0; loopCounter < m_loopSize; loopCounter++)
	{
		time_t start = time(NULL);
		while (serialReadData != 65 && time(NULL) - start < TIMEOUT) {
			serialReadData = usbController.ReadByte(hPort);
		}
		DrawSquare(sysLatClient, m_strWhite);
		while (serialReadData != 66 && time(NULL) - start < TIMEOUT) {
			serialReadData = usbController.ReadByte(hPort);
		}
		DrawSquare(sysLatClient, m_strBlack);
		sysLatResults = "";
		while (serialReadData != 67 && time(NULL) - start < TIMEOUT) {
			serialReadData = usbController.ReadByte(hPort);
			if (serialReadData != 67 && serialReadData != 65 && serialReadData != 66) {
				sysLatResults += (char)serialReadData;
			}
		}

		//I think everything below should be happening in a different thread so that the serial reads can continue uninterrupted - could the following be a coroutine?
		std::string processName = GetProcessNameFromPID(CRTSSClient::GetLastForegroundAppID());
		std::string activeWindowTitle;
		if (loopCounter < m_loopSize) { //this was for a really strange issue
			activeWindowTitle = GetActiveWindowTitle();
		}
		else {
			activeWindowTitle = "";
		}
		ProcessNameTrim(processName, activeWindowTitle);

		m_pOperatingSLD->UpdateSLD(loopCounter, sysLatResults, processName, activeWindowTitle);
	}

	usbController.CloseComPort(hPort);
	sysLatClient.ReleaseOSD();

	return 0;
}
void CSysLat_SoftwareDlg::DrawSquare(CRTSSClient sysLatClient, CString& colorString)
{
	m_updateString = "";
	//The following conditional is FAR from perfect... In order for it to work properly I may need to count the number of rows and columns(in zoomed pixel units?) and use that value. 
	if (sysLatClient.ownedSlot == 0 && !m_bPositionManualOverride) {
		if ((int)m_positionX < 0) {
			m_internalX = 0;
		}
		if ((int)m_positionY < 0) {
			//y = CRTSSClient::clientsNum * 20;
			m_internalY = 20;
		}
		m_updateString.AppendFormat("<P=%d,%d>", m_internalX, m_internalY);
		m_updateString += colorString;
		m_updateString += "<P=0,0>";
	}
	else if (m_bPositionManualOverride) {
		m_updateString.AppendFormat("<P=%d,%d>", m_internalX, m_internalY);
		m_updateString += colorString;
		m_updateString += "<P=0,0>";
	}
	else {
		m_updateString += colorString;
	}
	
	sysLatClient.UpdateOSD(m_updateString);
}

//Dialog menu functions
//Tools
void CSysLat_SoftwareDlg::ExportData()
{
	if (m_previousSLD.size() > 0) {
		for (unsigned int i = 0; i < m_previousSLD.size(); i++) {
			if (!m_previousSLD[i]->dataExported) {
				m_previousSLD[i]->ExportData(i);
			}
			else {
				std::string error = "Data from test " + std::to_string(i) + " already exported."; //this error message is garbage in every way
				AppendError(error.c_str());
			}
		}
	}
	else {
		//this is one of the errors that only appears for a few seconds and then dissapears... open an error dialog instead maybe?
		AppendError("No tests have completed yet. \nPress F11 to begin a new test(ending the current test), or wait for the current test to finish. \nYou can change the test size in the menu."); // (Not yet you can't lol)
	}
}
void CSysLat_SoftwareDlg::UploadData()
{
	const char* APItarget = "/api/benchmarkData/benchmarkData";
	if (m_previousSLD.size() > 0) {
		for (unsigned int i = 0; i < m_previousSLD.size(); i++) {
			if (!m_previousSLD[i]->dataUploaded) {
				if (m_bTestUploadMode) {
					int uploadStatus = upload_data(m_previousSLD[i]->jsonSLD, APItarget);
				}
				else {
					int uploadStatus = upload_data_secure(m_previousSLD[i]->jsonSLD, APItarget);
				}
				m_previousSLD[i]->dataUploaded = true; //need to make uploadStatus return a bool or something and use it to set this var
			}
			else {
				std::string error = "Data from test " + std::to_string(i) + " already uploaded.";
				AppendError(error.c_str());
			}
		}
	}
	else {
		//this is one of the errors that only appears for a few seconds and then dissapears... open an error dialog instead maybe?
		AppendError("No tests have completed yet. \nPress F11 to begin a new test(ending the current test), or wait for the current test to finish. \nYou can change the test size in the menu."); // (Not yet you can't lol)
	}
}

//Settings - need to look into GetCommPorts function to enumerate COM ports https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcommports
void CSysLat_SoftwareDlg::SetPortCom1()
{
	CMenu* settingsMenu = ResetPortsMenuItems();

	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_CHECKED);

	m_PortSpecifier = "COM1";
}
void CSysLat_SoftwareDlg::SetPortCom2()
{
	CMenu* settingsMenu = ResetPortsMenuItems();

	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_CHECKED);

	m_PortSpecifier = "COM2";
}
void CSysLat_SoftwareDlg::SetPortCom3()
{
	CMenu* settingsMenu = ResetPortsMenuItems();

	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_CHECKED);

	m_PortSpecifier = "COM3";
}
void CSysLat_SoftwareDlg::SetPortCom4()
{
	CMenu* settingsMenu = ResetPortsMenuItems();

	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_CHECKED);

	m_PortSpecifier = "COM4";
}
CMenu* CSysLat_SoftwareDlg::ResetPortsMenuItems()
{
	CMenu* settingsMenu = GetMenu();
	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_UNCHECKED);
	ReInitThread();
	return settingsMenu;
}

void CSysLat_SoftwareDlg::DebugMode() {
	CMenu* settingsMenu = GetMenu();
	if (m_bDebugMode) {
		settingsMenu->CheckMenuItem(ID_SETTINGS_DEBUGMODE, MF_UNCHECKED);
		m_bDebugMode = false;
	}
	else {
		settingsMenu->CheckMenuItem(ID_SETTINGS_DEBUGMODE, MF_CHECKED);
		m_bDebugMode = true;
	}
}
void CSysLat_SoftwareDlg::TestUploadMode() {
	CMenu* settingsMenu = GetMenu();
	if (m_bTestUploadMode) {
		settingsMenu->CheckMenuItem(ID_SETTINGS_TESTUPLOADMODE, MF_UNCHECKED);
		m_bTestUploadMode = false;
	}
	else {
		settingsMenu->CheckMenuItem(ID_SETTINGS_TESTUPLOADMODE, MF_CHECKED);
		m_bTestUploadMode = true;
	}
}
void CSysLat_SoftwareDlg::DisplaySysLatInOSD() {
	//this needs to default to "on" if you have a version with no LCD on "off" if your SysLat does have an LCD
	CMenu* settingsMenu = GetMenu();
	if (m_bSysLatInOSD) {
		settingsMenu->CheckMenuItem(ID_SETTINGS_DISPLAYSYSLATINOSD, MF_UNCHECKED);
		m_bSysLatInOSD = false;
	}
	else {
		settingsMenu->CheckMenuItem(ID_SETTINGS_DISPLAYSYSLATINOSD, MF_CHECKED);
		m_bSysLatInOSD = true;
	}
}

/*
	//init some settings to global(?) profile - probably- scratch that, DEFINITELY need to move these
	//SetProfileProperty("", "BaseColor", 0xFFFFFF);
	//SetProfileProperty("", "BgndColor", 0x000000); //this value isn't actually modifiable in RTSS lol
	//SetProfileProperty("", "FillColor", 0x000000);
	//SetProfileProperty("", "ZoomRatio", 2);
	//SetProfileProperty("", "RefreshPeriod", 0); //found this property by looking at the plaintext of the RTSSHooks.dll.  Doesn't appear to change the value.  Also attempted to use the "Inc" function as well, but it also failed.
	//SetProfileProperty("", "RefreshPeriodMin", 0); //found this property by looking at the plaintext of the RTSSHooks.dll ... It didn't appear to change the value in RTSS... I hope I didn't break something lol
	//SetProfileProperty("", "CoordinateSpace", 1); //IDK what these do, but I thought they would
	//SetProfileProperty("", "CoordinateSpace", 0);
	//DWORD coordinateSpace = CRTSSClient::GetProfileProperty("", "CoordinateSpace");
	//CGroupedString strOSDBuilder(dwMaxTextSize - 1); //I have no freaking clue what this CGroupedString class does, so I'm kind of scared to get rid of it.

	//sysLatStatsClient.GetOSDText(strOSDBuilder, bFormatTagsSupported, bObjTagsSupported);	// get OSD text
*/




