// SysLat_SoftwareDlg.cpp : implementation file
//
// created by Unwinder
// modified by Skewjo
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SysLat_Software.h"
#include "SysLat_SoftwareDlg.h"
#include "USBController.h"
#include "psapi.h"

/////////////////////////////////////////////////////////////////////////////
#include <shlwapi.h>
#include <float.h>
#include <io.h>
#include <sstream>
#include <algorithm>

//TODO:
// 
// WON'T WORK(profiles are for individual programs) Profile Setting -  Create new "SysLat" profile in RTSS so as to not break other people's profiles 
// NOT AVAILABLE - Profile Setting - Set "Refresh Period" to 0 milliseconds  - doesn't appear to be an option available via shared memory
// NOT AVAILABLE - Profile Setting - Change default corner to bottom right
// NOT AVAILABLE(opacity) - Profile Setting - Change "text"(foreground) color to white (255, 255, 255) with an opacity of 100
// NOT AVIALABLE(opacity) - Profile Setting - Change color of "background"(?) to black (0, 0, 0) with an opacity of 100
//		Better yet - if I could change the box to use a plain black and plain white box so any other text isn't fucked up, that would be better
// DOESN'T WORK -(also code is in bad location) - Profile Setting - Set box size to what I want it to be?
// DONE - Change class/namespace name of RTSSSharedMemorySampleDlg to SysLatDlg
// DONE - Change class/namespace of RTSSSharedMemorySample to SysLat
// DONE - Add minimize button
// DONE(well... it half-ass works) - Make System Latency appear in OSD
// Add graph functionality
// Save results to a table(save fps and frametime as well?)
// DONE - Determine active window vs window that RTSS is operating in?
// Enumerate all 3D programs that RTSS can run in and display them in a menu
// DONE - Launch RTSS automatically in the background if it's not running
// DONE - Add hotkey to restart readings (F11?)
// Fix COM port change settings
// Seperate some initialization that happens in "Refresh" function into a different "Refresh-like" function?? - partially done?
// DONE - Re-org this file into 3-4 new classes - Dialog related functions, RTSS related, DrawingThread related, and USB related
// Errors currently appear very briefly and are overwritten when the refresh function runs - Clean up the refresh function, then come up with new error scheme.
//		Either use error codes, or check all errors  again in the refresh function(that doesn't make sense though, right?)... or maybe do dialog error pop-ups when errors occur outside of "refresh"?
// Keep track of total tests performed in a config file vs. looking for existing log files and picking up from there?
//		How many tests should we allow total? 100? 
//		Would it be fine if SysLat overwrote the tests every time it was restarted? ...I think it would
// Add lots more menu options - USB options, debug output, data upload?
// Put date/time in log file
// Clear log files and put a configurable(?) cap on the number allowed
// Move ExportData function out of SysLatData? Or just use it to retrieve a jsoncpp object & combine it with other jsoncpp objects
// Account for corners???
// Make executable/window names mesh better together?  Need a map/lookup table or something?

#include <Windows.h>
#include <process.h>
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
CString CSysLat_SoftwareDlg::m_PortSpecifier = "COM3";
CString CSysLat_SoftwareDlg::m_strError = "";
CSysLatData* CSysLat_SoftwareDlg::m_pOperatingSLD = new CSysLatData;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
/////////////////////////////////////////////////////////////////////////////
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
		//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//Windows Dialog function overrides(?)
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
	ON_COMMAND(ID_EXPORTDATA_EXPORTDATA, CSysLat_SoftwareDlg::ExportData)
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


	CWnd* pPlaceholder = GetDlgItem(IDC_PLACEHOLDER);

	if (pPlaceholder)
	{
		CRect rect;
		pPlaceholder->GetClientRect(&rect);

		if (!m_richEditCtrl.Create(WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | WS_VSCROLL, rect, this, 0))
			return FALSE;

		m_font.CreateFont(-11, 0, 0, 0, FW_REGULAR, 0, 0, 0, BALTIC_CHARSET, 0, 0, 0, 0, "Courier New");
		m_richEditCtrl.SetFont(&m_font);
	}

	//init timers
	m_nTimerID = SetTimer(0x1234, 1000, NULL);	//Used by OnTimer function to refresh dialog box & OSD
	time(&m_elapsedTimeStart);					//Used to keep track of test length

	//Attempt to claim the first slot for SysLat(??) - this definitely feels like the wrong location
	//UpdateOSD("", m_caSysLat);


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

		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
void CSysLat_SoftwareDlg::Refresh()
{
	//I believe this needs to be somewhere else...
	CRTSSClient::InitRTSSInterface();
	
	//init some settings to global(?) profile - probably- scratch that, DEFINITELY need to move these
	//SetProfileProperty("", "BaseColor", 0xFFFFFF);
	//SetProfileProperty("", "BgndColor", 0x000000); //this value isn't actually modifiable in RTSS lol
	//SetProfileProperty("", "FillColor", 0x000000);
	//SetProfileProperty("", "ZoomRatio", 2);
	//SetProfileProperty("", "RefreshPeriod", 0); //found this property by looking at the plaintext of the RTSSHooks.dll.  Doesn't appear to change the value.  Also attempted to use the "Inc" function as well, but it also failed.
	//SetProfileProperty("", "RefreshPeriodMin", 0); //found this property by looking at the plaintext of the RTSSHooks.dll ... It didn't appear to change the value in RTSS... I hope I didn't break something lol
	//SetProfileProperty("", "CoordinateSpace", 1); //IDK what these do, but I thought they would 
	//SetProfileProperty("", "CoordinateSpace", 0);
	m_strStatus = "";

	//I can definitely move the following function out and call it elsewhere... or at least conditionally
	GetRTSSConfigs();

	CGroupedString strOSDBuilder(dwMaxTextSize - 1); //I have no fucking clue wtf this CGroupedString class does, so I'm kind of scared to get rid of it.

	sysLatStatsClient.GetOSDText(strOSDBuilder, bFormatTagsSupported, bObjTagsSupported);	// get OSD text

	BOOL bTruncated = FALSE;

	BOOL success = m_pOperatingSLD->AcquireSLDMutex();		// begin the sync access to fields
	if (!success)
		return;

	//Make my own fucking clock... I think this should probably be done in the SysLatData class
	time(&m_elapsedTimeEnd);
	double dif = difftime(m_elapsedTimeEnd, m_elapsedTimeStart);
	int minutes = static_cast<int>(dif) / 60;
	int seconds = static_cast<int>(dif) % 60;
	double measurementsPerSecond = m_pOperatingSLD->GetCounter() / dif;


	// Need to clean up this giant block of "Appends", to make dialog box text more manageable, by creating multiple functions that append different things

	m_strStatus.AppendFormat("Elapsed Time: %02d:%02d", minutes, seconds);
	
	DWORD dwLastForegroundAppProcessID = CRTSSClient::GetLastForegroundAppID();
	m_strStatus.Append("\nLast RTSS Foreground App Name: ");
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
	

	m_strStatus.AppendFormat("\nSystem Latency: %s", m_pOperatingSLD->GetStringResult());
	m_strStatus.AppendFormat("\nLoop Counter : %d", m_pOperatingSLD->GetCounter());
	m_strStatus.AppendFormat("\n\nMeasurements Per Second: %.2f", measurementsPerSecond); //This value should probably be in the SLD...
	m_strStatus.AppendFormat("\nSystem Latency Average: %.2f", m_pOperatingSLD->GetAverage());
	m_strStatus.AppendFormat("\nLoop Counter EVR(expected value range, 3-100): %d ", m_pOperatingSLD->GetCounterEVR());
	m_strStatus.AppendFormat("\nSystem Latency Average(EVR): %.2f", m_pOperatingSLD->GetAverageEVR());

	if (!m_strError.IsEmpty())
	{
		m_strStatus.Append(m_strError);
		m_strError = "";
	}
	m_pOperatingSLD->ReleaseSLDMutex();		// end the sync access to fields

	CString strOSD = strOSDBuilder.Get(bTruncated);
	strOSD += m_pOperatingSLD->GetStringResult();
	if (!strOSD.IsEmpty())
	{
		BOOL bResult = sysLatStatsClient.UpdateOSD(strOSD);

		m_bConnected = bResult;

		if (bResult)
		{
			if (bTruncated)
				AppendError("Warning: The text is too long to be displayed in OSD, some info has been truncated!");
		}
		else
		{

			if (CRTSSClient::m_strInstallPath.IsEmpty())
				AppendError("Error: Failed to connect to RTSS shared memory!\nHints:\n-Install RivaTuner Statistics Server");
			else
				AppendError("Error: Failed to connect to RTSS shared memory!\nHints:\n-Press <Space> to start RivaTuner Statistics Server");
		}
	}

	m_richEditCtrl.SetWindowText(m_strStatus);
} 
void CSysLat_SoftwareDlg::AppendError(const CString& error)
{
	m_strError.Append("\n");
	m_strError.Append(error);
	m_strError.Append("\n");
}
void CSysLat_SoftwareDlg::GetRTSSConfigs() {
	dwSharedMemoryVersion = CRTSSClient::GetSharedMemoryVersion();
	dwMaxTextSize = (dwSharedMemoryVersion >= 0x00020007) ? sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSDEx) : sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSD);
	bFormatTagsSupported = (dwSharedMemoryVersion >= 0x0002000b);	//text format tags are supported for shared memory v2.11 and higher
	bObjTagsSupported = (dwSharedMemoryVersion >= 0x0002000c);		//embedded object tags are supporoted for shared memory v2.12 and higher
}

//SysLat thread functions
void CSysLat_SoftwareDlg::ReInitThread() { //since implementing the the target and active window string arrays in the SysLatData class, this function now hangs/freezes (sometimes?) - most likely because it's trying to create a new object with a struct that contains 3 arrays that are  3600 ints, and 2 that are 3600 strings...
	//Set loop size to 0, wait for thread to finish so that it closes the COM port, then reset loop size before you kick off a new thread - THIS PROBLEM GOES AWAY IF I PUT *ANY* BREAKPOINTS IN THIS FUNCTION???
	m_loopSize = 0;
	WaitForSingleObject(drawingThreadHandle, INFINITE); // since this is the thread created by the one and only "beginThreadEx" function... does cleanup of this thread automatically occur when the function ends?
	//WaitForMultipleObjects(2, drawingThreadHandle, INFINITE);
	m_loopSize = 0xFFFFFFFF;
	time(&m_elapsedTimeStart);
	myCounter = 0;

	m_pOperatingSLD->SetEndTime();
	m_previousSLD.push_back(m_pOperatingSLD);
	m_pOperatingSLD = new CSysLatData;

	drawingThreadHandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, &myCounter, 0, 0);
	SetThreadPriority(drawingThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
}
unsigned int __stdcall CSysLat_SoftwareDlg::CreateDrawingThread(void* data)
{
	int TIMEOUT = 5; //this should probably be a defined constant
	int serialReadData = 0;
	CString	sysLatResults;
	CRTSSClient sysLatClient("SysLat", 0);

	CUSBController usbController;
	HANDLE hPort = usbController.OpenComPort(m_PortSpecifier);
	CString	m_localPortSpecifier = m_PortSpecifier;

	if (!usbController.IsComPortOpened(hPort))
	{
		AppendError("Failed to open COM port: " + m_PortSpecifier);
		return 0;
	}

	DrawBlack(sysLatClient);

	for (unsigned int loopCounter = 0; loopCounter < m_loopSize; loopCounter++)
	{
		time_t start = time(NULL);
		while (serialReadData != 65 && time(NULL) - start < TIMEOUT) {
			serialReadData = usbController.ReadByte(hPort);
		}
		DrawWhite(sysLatClient);
		while (serialReadData != 66 && time(NULL) - start < TIMEOUT) {
			serialReadData = usbController.ReadByte(hPort);
		}
		DrawBlack(sysLatClient);
		sysLatResults = "";
		while (serialReadData != 67 && time(NULL) - start < TIMEOUT) {
			serialReadData = usbController.ReadByte(hPort);
			if (serialReadData != 67 && serialReadData != 65 && serialReadData != 66) {
				sysLatResults += (char)serialReadData;
			}
		}

		//I think everything below should be happening in a different thread so that the serial reads can continue uninterrupted
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

	return 0;
}

//The following 2 functions should probably be combined into a "DrawSquare" function that takes different input strings for black and white
void CSysLat_SoftwareDlg::DrawBlack(CRTSSClient sysLatClient)
{
	//UpdateOSD("<P=0,0><L0><C=80000000><B=0,0>\b<C><E=-1,-1,8><C=000000><I=-2,0,384,384,128,128><C>", m_caSysLat);
	sysLatClient.UpdateOSD("<C=80000000><B=0,0>\b<C><E=-1,-1,8><C=000000><I=-2,0,384,384,128,128><C>");
}
void CSysLat_SoftwareDlg::DrawWhite(CRTSSClient sysLatClient)
{
	//UpdateOSD("<P=0,0><L0><C=80FFFFFF><B=0,0>\b<C><E=-1,-1,8><C=FFFFFF><I=-2,0,384,384,128,128><C>", m_caSysLat);
	sysLatClient.UpdateOSD("<C=80FFFFFF><B=0,0>\b<C><E=-1,-1,8><C=FFFFFF><I=-2,0,384,384,128,128><C>");
}

//Dialog menu functions
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
//This function is definitely broken right now - specifically breaks when I go from the real SysLat port to another occupied port & back
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
void CSysLat_SoftwareDlg::ExportData()
{
	if (m_previousSLD.size() > 0) {
		for (int i = 0; i < m_previousSLD.size(); i++) {
			if (!m_previousSLD[i]->dataExported) {
				m_previousSLD[i]->ExportData(i);
			}
			else {
				std::string error = "Data from test " + std::to_string(i) + " already exported.";
				AppendError(error.c_str());
			}
		}
	}
	else {
		//this is one of the errors that only appears for a few seconds and then dissapears... open an error dialog instead maybe?
		AppendError("No tests have completed yet. Press F11 to begin a new test(ending the current test), or wait for the current test to finish.  You can change the test size in the menu.");
	}
}
/*
void CSysLat_SoftwareDlg::UploadData() {

}
*/