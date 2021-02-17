// SysLat_SoftwareDlg.cpp : implementation file
//
// created by Unwinder
// modified by Skewjo
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SysLat_Software.h"
#include "SysLat_SoftwareDlg.h"
#include "USBController.h"
#include "HTTP_Client_Async.h"
#include "HTTP_Client_Async_SSL.h"
#include "SysLatPreferences.h"
#include "AboutDlg.h"
#include "PreferencesDlg.h"
#include "TestCtrl.h" //this one should probably have a suffix of "dlg"...
#include "version.h"

//probably need to move the following 2 includes to stdafx.h
#include <memory> 
#include <shellapi.h>

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
//  DONE(mostly) - Launch RTSS automatically in the background if it's not running
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
//  Some errors currently appear very briefly and are overwritten when the refresh function runs - Clean up the refresh function, then come up with new error scheme.
//		Either use error codes, or check all errors  again in the refresh function(that doesn't make sense though, right?)... or maybe do dialog error pop-ups when errors occur outside of "refresh"?
//  Move ExportData function out of SysLatData? Or just use it to retrieve a jsoncpp object & combine it with other jsoncpp objects
//  DONE - Make executable/window names mesh better together?  Need a map/lookup table or something? - JUST USE PID YA IDIOT
//
//
//Data Issues:
//  Save fps and frametime and other stats as well?
//  Add graph functionality
//  Put elapsed time in log file
//  Clear log files and put a configurable(?) cap on the number allowed
//  Keep track of total tests performed in a config file vs. looking for existing log files and picking up from there?
//		How many tests should we allow total? 100? 
//		Would it be fine if SysLat overwrote the tests every time it was restarted? ...I think it would
//
//
//Menu:
//  Enumerate all 3D programs that RTSS can run in and display them in a menu
//  Fix COM port change settings
//  Add lots more menu options - USB options, debug output, data upload, RTSS options(text color)
//  Box position manual override toggle
//
//
//Anti-Fraud:
//  Create new dynamic build/installation process in order to obscure some code
//  Think about hardware/software signatures for uploading data? This probably needs more consideration on the web side
//  Obscure most functionality(things that don't need to be optimized) into DLLs(requires a new build/installation process)
//  (Anti-Fraud, Optimization, and Data)Instead of recording certain variables on every measurement(such as RTSS XY position) record them once at the start and once at the end
//
//
//Optimization:
//  Move data update at the end of the CreateDrawingThread function into a different thread(or co-routine?)
//  Calculating the position of the box before we draw it adds unnecessary delay(?)
//  Make flashing square resizeable
//
//
//Organizational Issues:
//  Clean up(or get rid of) static vars in SysLat_SoftwareDlg class
//  Clean up the refresh function a bit more by making some init functionality conditional
//  Attempt to get rid of most Windows type names like CString, BOOL, and INT(DWORD?)
//	Attempt to use a single style of string instead of "string", "char*", and "CString".
//	Look further into Windows style guides & straighten out all member var names with "m_" and the type, or do away with it completely
//  Look into file organization for .h and .cpp files because the repo is a mess(though it's fine in VS because of "filters")
//	Look into class naming schemes and organization - make sure dialog classes end in "dlg"(?)
//  Check whether or not my void "initialization" methods need to return ints or bools for success/failure or if I can just leave them as void

//////////////////////////////////////////////////////////////////////////////////////////////
//Major Bugs:
//  DONE - (hid the error)The SysLat RTSSClient object cannot obtain the "0th" RTSS OSD slot when restarting a test
//	Issue when switching COM ports to an existing device that isn't SysLat and back
//	DONE - Arrow key functionality has been optimized away somehow
//  
//
//Minor Bugs:
//  SysLat may not play nicely with other applications that use RTSS such as MSI Afterburner, or with advanced RTSS setups
//
//////////////////////////////////////////////////////////////////////////////////////////////

const int ID_COMPORT_START = 2000;
const int ID_COMPORT_END = 2099;
const int ID_RTSSAPP_START = 2100;
const int ID_RTSSAPP_END = 2199;
const int WM_STMESSAGE = WM_USER + 1;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//As a non-static non-member variable? Is this a good idea? Need to at least remove the "m_" from the var name... Does this make it global??
SysLatPreferences	SLPref;
#define SysLatOpt SLPref.m_SysLatOptions
#define PrivacyOpt SLPref.m_PrivacyOptions
#define DebugOpt SLPref.m_DebugOptions
#define RTSSOpt	SLPref.m_RTSSOptions

NOTIFYICONDATA nid;
int dotCounter = 0;

//Define static variables - these should probably be done as inline or something... inlining is supposed to be available in C++17 and above, but Visual Studio throws a fit when I try to inline these.
//also, I should probably move most of them to be global variables and NOT member variables
CString CSysLat_SoftwareDlg::m_strStatus = "";
unsigned int CSysLat_SoftwareDlg::m_LoopCounterRefresh = 0;
unsigned int CSysLat_SoftwareDlg::m_loopSize = 0xFFFFFFFF;
CString	CSysLat_SoftwareDlg::m_updateString = "";
CString CSysLat_SoftwareDlg::m_strError = "";
std::shared_ptr<CSysLatData> CSysLat_SoftwareDlg::m_pOperatingSLD = std::make_shared<CSysLatData>(); //does this need to be a unique_ptr?
CString CSysLat_SoftwareDlg::m_strBlack = "<C=000000><B=10,10><C>";
CString CSysLat_SoftwareDlg::m_strWhite = "<C=FFFFFF><B=10,10><C>";
DWORD CSysLat_SoftwareDlg::m_sysLatOwnedSlot = 0;
DWORD CSysLat_SoftwareDlg::m_AppArraySize = 0;

//Windows Dialog inherited function overrides
/////////////////////////////////////////////////////////////////////////////
// CSysLat_SoftwareDlg dialog
/////////////////////////////////////////////////////////////////////////////
CSysLat_SoftwareDlg::CSysLat_SoftwareDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSysLat_SoftwareDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSysLat_SoftwareDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strStatus = "";
	m_strInstallPath = "";

	m_bMultiLineOutput = TRUE;
	m_bFormatTags = TRUE;
	m_bFillGraphs = FALSE;
	m_bConnected = FALSE;
}
CSysLat_SoftwareDlg::~CSysLat_SoftwareDlg() {
	SLPref.WritePreferences();
}
void CSysLat_SoftwareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysLat_SoftwareDlg)
	//}}AFX_DATA_MAP
}
/////////////////////////////////////////////////////////////////////////////
// CSysLat_SoftwareDlg message handlers
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSysLat_SoftwareDlg, CDialogEx)
	//{{AFX_MSG_MAP(CSysLat_SoftwareDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_TOOLS_EXPORTDATA, CSysLat_SoftwareDlg::ExportData)
	ON_COMMAND(ID_TOOLS_UPLOADDATA, CSysLat_SoftwareDlg::UploadData)
	ON_COMMAND(ID_SETTINGS_DEBUGMODE, CSysLat_SoftwareDlg::DebugMode)
	ON_COMMAND(ID_SETTINGS_TESTUPLOADMODE, CSysLat_SoftwareDlg::TestUploadMode)
	ON_COMMAND(ID_SETTINGS_DISPLAYSYSLATINOSD, CSysLat_SoftwareDlg::DisplaySysLatInOSD)
	ON_COMMAND(ID_TOOLS_NEWTEST, CSysLat_SoftwareDlg::ReInitThread)
	ON_COMMAND(ID_SETTINGS_PREFERENCES, CSysLat_SoftwareDlg::OpenPreferences)
	ON_COMMAND(ID_TOOLS_TESTCONTROL, CSysLat_SoftwareDlg::OpenTestCtrl)
	ON_COMMAND_RANGE(ID_COMPORT_START, ID_COMPORT_END, CSysLat_SoftwareDlg::OnComPortChanged)
	ON_COMMAND_RANGE(ID_RTSSAPP_START, ID_RTSSAPP_END, CSysLat_SoftwareDlg::OnTargetWindowChanged)
	ON_MESSAGE(WM_STMESSAGE, CSysLat_SoftwareDlg::OnSTMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CSysLat_SoftwareDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_color = RGB(136, 217, 242);
	m_brush.CreateSolidBrush(m_color);

	GetModuleFileName(NULL, pathToSysLat, MAX_PATH);

	CWnd* pMainDlg = GetDlgItem(IDD_SYSLAT_SOFTWARE_DIALOG);

	if (pMainDlg)
	{
		pMainDlg->GetClientRect(&m_clientRect);
	}

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

	SetIcon(m_hIcon, TRUE);

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
	
	if (PrivacyOpt.m_bFirstRun) {
		::MessageBox(NULL, "This appears to be the first time you've run SysLat from this directory. Please set your privacy options.", "SysLat First Run", MB_OK);
		OpenPreferences();
		PrivacyOpt.m_bFirstRun = false;
	}

	
	if (PrivacyOpt.m_bRunOnStartup) {
		SetSURegValue(pathToSysLat);
	}
	else {
		SetSURegValue("");
	}
	
	//m_bTestUploadMode = true;
	if(PrivacyOpt.m_bAutoCheckUpdates){
		CheckUpdate();
	}
	

	m_nTimerID = SetTimer(0x1234, 1000, NULL);	//Used by OnTimer function to refresh dialog box & OSD
	//time(&m_elapsedTimeStart);					//Used to keep track of test length

	m_hardwareID.ExportData(SysLatOpt.m_LogDir);
	m_machineInfo.ExportData(SysLatOpt.m_LogDir);

	Refresh();

	unsigned threadID;
	m_drawingThreadHandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, &myCounter, 0, &threadID);
	SetThreadPriority(m_drawingThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);//31 is(apparently?) the highest possible thread priority - may be bad because it could cause deadlock using a loop? Need to read more here: https://docs.microsoft.com/en-us/windows/win32/procthread/scheduling-priorities
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CSysLat_SoftwareDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if ((nID & 0xFFF0) == SC_MINIMIZE)
	{
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = 100;
		nid.uVersion = NOTIFYICON_VERSION;
		nid.uCallbackMessage = WM_STMESSAGE;
		nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		strcpy_s(nid.szTip, "SysLat");
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		Shell_NotifyIcon(NIM_ADD, &nid);

		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		// Minimizing, post to main dialogue also.
		AfxGetMainWnd()->ShowWindow(SW_MINIMIZE);
	}
	//else if ((nID & 0xFFF0) == SC_CLOSE) {
	//	nid.cbSize = sizeof(NOTIFYICONDATA);
	//	nid.hWnd = m_hWnd;
	//	nid.uID = 100;
	//	nid.uVersion = NOTIFYICON_VERSION;
	//	nid.uCallbackMessage = WM_STMESSAGE;
	//	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//	strcpy_s(nid.szTip, "SysLat");
	//	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	//	Shell_NotifyIcon(NIM_ADD, &nid);

	//	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	//	// Minimizing, post to main dialogue also.
	//	AfxGetMainWnd()->ShowWindow(SW_MINIMIZE);
	//}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}
void CSysLat_SoftwareDlg::OnPaint()
{
	if (IsIconic())
	{
		//THIS CODE FOR THE MINIMIZE BUTTON IS NO LONGER(NEVER WAS??) NEEDED??
		//CPaintDC dc(this); // device context for painting

		//SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		//// Center icon in client rectangle
		//int cxIcon = GetSystemMetrics(SM_CXICON);
		//int cyIcon = GetSystemMetrics(SM_CYICON);
		//CRect rect;
		//GetClientRect(&rect);
		//int x = (rect.Width() - cxIcon + 1) / 2;
		//int y = (rect.Height() - cyIcon + 1) / 2;

		//dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CSysLat_SoftwareDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}
void CSysLat_SoftwareDlg::OnTimer(UINT nIDEvent)
{
	Refresh();
	CDialogEx::OnTimer(nIDEvent);
}
void CSysLat_SoftwareDlg::OnDestroy()
{
	if (m_nTimerID)
		KillTimer(m_nTimerID);

	m_nTimerID = NULL;

	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));

	TerminateThread(m_drawingThreadHandle, 0); //Does exit code need to be 0 for this?
	m_SysLatStatsClient.ReleaseOSD();
	//m_pOperatingSLD->CloseSLDMutex();

	CDialogEx::OnDestroy();
}
LRESULT CSysLat_SoftwareDlg::OnSTMessage(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	if (lParam == WM_LBUTTONDBLCLK){
		if (IsIconic()) {
			ModifyStyleEx(WS_EX_TOOLWINDOW, WS_EX_APPWINDOW);
			ShowWindow(SW_SHOWNORMAL);
			Shell_NotifyIcon(NIM_DELETE, &nid);
		}
	}
	return 0;
}

//Dialog functions
string CSysLat_SoftwareDlg::GetProcessNameFromPID(DWORD processID) {
	string ret;
	HANDLE Handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		processID
	);
	if (Handle)
	{
		DWORD buffSize = 1024;
		CHAR Buffer[1024];
		if (QueryFullProcessImageName(Handle, 0, Buffer, &buffSize))
		{
			ret = strrchr(Buffer, '\\') + 1; // I can't believe this works
		}
		else
		{
			printf("Error GetModuleBaseName : %lu", GetLastError());
		}
		CloseHandle(Handle);
	}
	else
	{
		printf("Error OpenProcess : %lu", GetLastError());
	}
	return ret;
}
string CSysLat_SoftwareDlg::GetActiveWindowTitle()
{
	char wnd_title[256];
	CWnd* pWnd = GetForegroundWindow();
	::GetWindowText((HWND)*pWnd, wnd_title, 256); //Had to use scope resolution because this function is defined in both WinUser.h and afxwin.h
	return wnd_title;
}
void CSysLat_SoftwareDlg::ProcessNameTrim(string& processName, string& activeWindowTitle){
	size_t pos = processName.find(".exe");
	if (pos != string::npos) {
		processName.replace(pos, processName.size(), "");
	}
	while ((pos = processName.find(" ")) != string::npos) {
		processName.replace(pos, 1, "");
	}
	std::transform(processName.begin(), processName.end(), processName.begin(), [](unsigned char c) { return std::tolower(c); });
	while ((pos = activeWindowTitle.find(" ")) != string::npos) {
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
		case VK_UP:
			if (RTSSOpt.m_internalY > 0) {
				RTSSOpt.m_internalY--;
			}
			//else appendError ??
			RTSSOpt.m_bPositionManualOverride = true;
			return TRUE;
		case VK_DOWN:
			if (RTSSOpt.m_internalY < 255) {
				RTSSOpt.m_internalY++;
			}
			RTSSOpt.m_bPositionManualOverride = true;
			return TRUE;
		case VK_LEFT:
			if (RTSSOpt.m_internalX > 0) {
				RTSSOpt.m_internalX--;
			}
			RTSSOpt.m_bPositionManualOverride = true;
			return TRUE;
		case VK_RIGHT:
			if (RTSSOpt.m_internalX < 255) {
				RTSSOpt.m_internalX++;
			}
			RTSSOpt.m_bPositionManualOverride = true;
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

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CSysLat_SoftwareDlg::Refresh()
{
	//Had to add this section because it was initializing fine in debug mode, but then initializing COMPortCount too fast in release mode??? Probably some stupid issue with my debug macros...
	CMenu* MainMenu = GetMenu();
	CMenu* SettingsMenu = MainMenu->GetSubMenu(1);
	CMenu* ComPortMenu = SettingsMenu->GetSubMenu(0);
	char menuCString[256];
	MainMenu->GetMenuString(ID_USBPORT_PLACEHOLDER, (LPSTR)menuCString, 256, (UINT)MF_BYCOMMAND);
	
	if (strcmp(menuCString, "Placeholder") == 0) {
		m_COMPortCount = 0;
	}

	CUSBController usbController;
	usbController.EnumSerialPorts(m_COMPortInfo, FALSE);
	if (m_COMPortInfo.GetSize() != m_COMPortCount) {
		R_DynamicComPortMenu();
	}

	if (m_bConnected) {//m_bConnected is never set to true??
		DWORD AppArraySize = CRTSSClient::GetAppArray();
		if (m_AppArraySize != AppArraySize) {
			R_DynamicAppMenu();
			m_AppArraySize = AppArraySize;
			DEBUG_PRINT("AppArraySize: " + to_string(AppArraySize))
		}
	}

	if (!(CRTSSClient::m_profileInterface.IsInitialized())) {
		CRTSSClient::InitRTSSInterface();
	}
	RTSSOpt.m_positionX = CRTSSClient::GetProfileProperty("", "PositionX");
	RTSSOpt.m_positionY = CRTSSClient::GetProfileProperty("", "PositionY");
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
	
	R_StrOSD();
	
	if (!m_bSysLatInOSD) { //need to add another condition to make this only happen once so that it will clear whatever exists in the buffer... or maybe use the releaseOSD function properly? IDK
		m_SysLatStatsClient.UpdateOSD("");
	}

	//Need to make a new function & boolean for displaying controls/hints
	if (CRTSSClient::m_profileInterface.IsInitialized())
	{
		m_strStatus += "\n\n-Press <Up>,<Down>,<Left>,<Right> to change OSD position in global profile";
		if (DebugOpt.m_bSysLatInOSD) {
			m_strStatus += "\n-Press <R>,<G>,<B> to change OSD color in global profile";
		}
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
	//Introduced a new bug here (but only in debug mode) where it looks like I'm getting some kind of under/overflow value.
	//Instead of duration starting at 00:00::00, it will start a value that looks something like "238609:17:-40"
	auto& CurrentTestDuration = m_pOperatingSLD->GetCurrentDuration();
	if (duration_cast<std::chrono::minutes>(CurrentTestDuration).count() >= SysLatOpt.m_maxTestDuration) {
		ReInitThread();
	}
	auto& data = m_pOperatingSLD->GetData();
	double measurementsPerSecond = static_cast<double>(data.m_statistics.counter) / static_cast<double>(duration_cast<std::chrono::seconds>(CurrentTestDuration).count());
	string CurrentTestDurationString = format("%R:%OS", CurrentTestDuration);
	m_strStatus.Append(("Elapsed Time: " + CurrentTestDurationString).c_str());
	
	if (m_pOperatingSLD->GetSystemLatency() < 500) { //using "c_str()" here because "stoi(m_pOperatingSLD->GetStringResult())" was getting immediate exceptions for some reason...
		m_strStatus.AppendFormat("\nSystem Latency: %i", m_pOperatingSLD->GetSystemLatency());
	}
	else {
		m_strStatus.AppendFormat("\nSystem Latency: Waiting");
		if (dotCounter == 1) {
			m_strStatus.AppendFormat(".");
		}
		else if (dotCounter == 2) {
			m_strStatus.AppendFormat("..");
		}
		else if (dotCounter == 3) {
			m_strStatus.AppendFormat("...");
		}
	}

	m_strStatus.AppendFormat("\nLoop Counter : %d", data.m_statistics.counter);

	if (isnan(measurementsPerSecond)) {
		m_strStatus.AppendFormat("\nMeasurements Per Second: 0.00");
	}
	else {
		m_strStatus.AppendFormat("\nMeasurements Per Second: %.2f", measurementsPerSecond); //This value should probably be in the SLD...
	}

	if (data.m_statistics.average < 500) {
		m_strStatus.AppendFormat("\nSystem Latency Average: %.2f", data.m_statistics.average);
	}
	else {
		m_strStatus.AppendFormat("\nSystem Latency Average: Waiting");
		if (dotCounter == 1) {
			m_strStatus.AppendFormat(".");
		}
		else if (dotCounter == 2) {
			m_strStatus.AppendFormat("..");
		}
		else if (dotCounter == 3) {
			m_strStatus.AppendFormat("...");
		}
	}
	dotCounter++;
	if (dotCounter >= 4) {
		dotCounter = 0;
	}

	m_strStatus.AppendFormat("\nLoop Counter EVR(expected value range, 3-100): %d ", data.m_statisticsEVR.counter);
	m_strStatus.AppendFormat("\nSystem Latency Average(EVR): %.2f", data.m_statisticsEVR.average);

	return true; //this return value needs to change or be removed
}
void CSysLat_SoftwareDlg::R_Position() {
	m_strStatus.AppendFormat("\n\nClients num: %d", CRTSSClient::clientsNum);
	m_strStatus.AppendFormat("\nSysLat Owned Slot: %d", m_sysLatOwnedSlot);
	m_strStatus.AppendFormat("\nPositionX: %d", RTSSOpt.m_positionX);
	m_strStatus.AppendFormat("\nPositionY: %d", RTSSOpt.m_positionY);
}
void CSysLat_SoftwareDlg::R_ProcessNames() {
	DWORD dwLastForegroundAppProcessID = CRTSSClient::GetLastForegroundAppID();
	m_strStatus.Append("\n\nLast RTSS Foreground App Name: ");
	string processName = GetProcessNameFromPID(dwLastForegroundAppProcessID);
	m_strStatus += processName.c_str();
	m_strStatus.Append("\nCurrently active window: ");
	string activeWindowTitle = GetActiveWindowTitle();
	m_strStatus += activeWindowTitle.c_str();
	ProcessNameTrim(processName, activeWindowTitle);
	m_strStatus.Append("\nTrimmed:");
	m_strStatus += processName.c_str();
	m_strStatus.Append("\nTrimmed:");
	m_strStatus += activeWindowTitle.c_str();
}
void CSysLat_SoftwareDlg::R_StrOSD() {
	//BOOL bTruncated = FALSE;
	string strOSD;// = strOSDBuilder.Get(bTruncated);
	strOSD += m_pOperatingSLD->GetSystemLatency();
	if (!(strOSD.size() == 0))
	{
		bool bResult = m_SysLatStatsClient.UpdateOSD(strOSD.c_str());

		m_bConnected = bResult;

		if (bResult)
		{
			m_strStatus += "\nTarget Window: ";
			m_strStatus += (SysLatOpt.m_targetApp).c_str();
			/*
			m_strStatus += "\n\nThe following text is being forwarded to OSD:\nFrom SysLat client: " + m_updateString + "\nFrom SysLatStats client: " + strOSD;

			
			if (m_bFormatTagsSupported)
				m_strStatus += "\n-Press <F> to toggle OSD text formatting tags";

			if (m_bFormatTagsSupported)
				m_strStatus += "\n-Press <I> to toggle graphs fill mode";
			*/
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
		//TODO: 1-6-21 - I THOUGHT I FREAKING FIXED THIS??
		if (m_sysLatOwnedSlot != 0) { 
			//AppendError("The SysLat client is unable to occupy RTSS client slot 0.\nThis may cause issues with the blinking square appearing in the corner.\nTo resolve this error try one of the following:\n\t1. Close other applications that use RTSS(such as MSI Afterburner)\n\t2. Restart RTSS\n\t3. Restart the testing phase(by pressing <F11>).");
		}
	}
}
void CSysLat_SoftwareDlg::R_DynamicComPortMenu()
{
	CMenu* MainMenu = GetMenu();
	CMenu* SettingsMenu = MainMenu->GetSubMenu(1);
	CMenu* ComPortMenu = SettingsMenu->GetSubMenu(0);

	if (ComPortMenu)
	{
		BOOL appended = false;
		BOOL deleted = false;
		m_COMPortCount = 0;
		
		for(auto i = 0; i < m_COMPortInfo.GetSize(); i++) {
			ComPortMenu->DeleteMenu(ID_COMPORT_START + m_COMPortCount, MF_BYCOMMAND);
			if (m_COMPortCount < ID_COMPORT_END - ID_COMPORT_START) {
				string usb_info = m_COMPortInfo[i].strFriendlyName;
				DEBUG_PRINT("Friendly Name: " + usb_info)

				appended = ComPortMenu->AppendMenu(MF_STRING, ID_COMPORT_START + m_COMPortCount, m_COMPortInfo[i].strFriendlyName);
				string menuString = m_COMPortInfo[i].strFriendlyName;
				size_t pos = menuString.rfind("(");
				menuString.replace(0, pos + 1, "");
				pos = menuString.rfind(")");
				menuString.replace(pos, menuString.size(), "");

				if (strcmp(menuString.c_str(), SysLatOpt.m_PortSpecifier.c_str()) == 0) {
					MainMenu->CheckMenuItem(ID_COMPORT_START + m_COMPortCount, MF_CHECKED);
				}

				m_COMPortCount++;
			}
			else { //catch or throw errors here maybe?
				break;
			}
		}
		deleted = ComPortMenu->DeleteMenu(ID_USBPORT_PLACEHOLDER, MF_BYCOMMAND);

		DEBUG_PRINT(("String appended: " + to_string(appended)).c_str())
		DEBUG_PRINT(("Placeholder deleted: " + to_string(deleted)).c_str())

	}
	DrawMenuBar();

}
void CSysLat_SoftwareDlg::R_DynamicAppMenu()
{
	CMenu* MainMenu = GetMenu();
	CMenu* SettingsMenu = MainMenu->GetSubMenu(1);
	CMenu* TargetAppMenu = SettingsMenu->GetSubMenu(1);
	
	if (TargetAppMenu)
	{
		BOOL appended = false;
		BOOL deleted = false;
		int count = 0;
		for (auto const& [pid, pName] : CRTSSClient::m_vszAppArr) {
			SettingsMenu->DeleteMenu(ID_RTSSAPP_START + count, MF_BYCOMMAND); 
			if (count < ID_RTSSAPP_END - ID_RTSSAPP_START) {
				if (pName != "SysLat_Software") {
					string id_name =  pName + " (" + to_string(pid) + ")";
					appended = TargetAppMenu->AppendMenu(MF_STRING, ID_RTSSAPP_START + count, id_name.c_str());

					if (strcmp(pName.c_str(), SysLatOpt.m_targetApp.c_str()) == 0) {
						MainMenu->CheckMenuItem(ID_RTSSAPP_START + count, MF_CHECKED);
					}
					count++;
				}
			}
			else { //error here?
				break;
			}
		}
		deleted = TargetAppMenu->DeleteMenu(ID_TARGETWINDOW_PLACEHOLDER, MF_BYCOMMAND);

		DEBUG_PRINT(("String appended: " + to_string(appended)).c_str())
		DEBUG_PRINT(("Placeholder deleted: " + to_string(deleted)).c_str())
	}
	DrawMenuBar();
}
void CSysLat_SoftwareDlg::AppendError(const CString& error)
{
	m_strError.Append("\n");
	m_strError.Append(error);
	m_strError.Append("\n");
}

//SysLat thread functions
void CSysLat_SoftwareDlg::ReInitThread() {
	m_loopSize = 0;
	WaitForSingleObjectEx(m_drawingThreadHandle, INFINITE, false);
	m_pOperatingSLD->m_targetApp = SysLatOpt.m_targetApp;
	m_pOperatingSLD->SetEnd();
	m_loopSize = 0xFFFFFFFF;

	myCounter = 0;

	//Save the data from the test that just completed in a vector of "SysLatData"s
	m_vpPreviousSLD.push_back(m_pOperatingSLD);
	m_pOperatingSLD = std::make_shared<CSysLatData>();

	//Restart the thread
	m_drawingThreadHandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, &myCounter, 0, 0);
	SetThreadPriority(m_drawingThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);

	//Convert the previous data to JSON
	m_vpPreviousSLD.back()->CreateJSONSLD();

	//Export and upload the data if enabled
	if (PrivacyOpt.m_bAutoExportLogs && SysLatOpt.m_maxLogs > 0) {
		ExportData();
	}
	if (PrivacyOpt.m_bAutoUploadLogs) {
		UploadData();
	}
}
unsigned int __stdcall CSysLat_SoftwareDlg::CreateDrawingThread(void* data) //this is probably dangerous, right?
{
	int TIMEOUT = 5; //this should probably be a defined constant
	int serialReadData = 0;
	string	sysLatResults;
	CRTSSClient sysLatClient("SysLat", 0);
	m_sysLatOwnedSlot = sysLatClient.ownedSlot;
	//the following should not be here because if RTSS isn't running when this is hit, the version is "0"
	m_pOperatingSLD->m_RTSSVersion = "v" + to_string(sysLatClient.sharedMemoryVersion);
	

	CUSBController usbController;
	HANDLE hPort = usbController.OpenComPort(SysLatOpt.m_PortSpecifier.c_str());

	while (!usbController.IsComPortOpened(hPort) && m_loopSize > 0)
	{
		hPort = usbController.OpenComPort(SysLatOpt.m_PortSpecifier.c_str());
		AppendError("Failed to open COM port: ");
		AppendError(SysLatOpt.m_PortSpecifier.c_str());
		Sleep(1000);
	}

	DrawSquare(sysLatClient, m_strBlack);

	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicrosecondsDraw, ElapsedMicrosecondsExtra, Frequency;
	QueryPerformanceFrequency(&Frequency);
	vector<long long> timeVectorDraw, timeVectorExtra;

	for (unsigned int loopCounter = 0; loopCounter < m_loopSize; loopCounter++)
	{
		QueryPerformanceCounter(&StartingTime);


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

		QueryPerformanceCounter(&EndingTime);
		ElapsedMicrosecondsDraw.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicrosecondsDraw.QuadPart *= 1000000;
		ElapsedMicrosecondsDraw.QuadPart /= Frequency.QuadPart;

		
		timeVectorDraw.push_back(ElapsedMicrosecondsDraw.QuadPart);



		//I think everything below(ESPECIALLY the "UpdateSLD" method) should be happening in a different thread so that the serial reads can continue uninterrupted - could the following be a coroutine?
		// 1-3-2021 thinking on this more, I need the following work to be "queued" up for the main thread... Not sure what the best way to accomplish that is.
		// 1-13-2021 - After some extensive testing, these functions are taking anywhere from 100-500 microseconds(half of a milllisecond) to complete, and should not be affecting the test accuracy by very much... still needs to be fixed though
		//push_back() lots of this stuff to a vector and then have the Refresh(?) function handle it?
		QueryPerformanceCounter(&StartingTime);

		string processName = GetProcessNameFromPID(CRTSSClient::GetLastForegroundAppID());
		string activeWindowTitle;
		if (loopCounter < m_loopSize) { //this was for a really strange issue when trying to end the thread.
			activeWindowTitle = GetActiveWindowTitle();
		}
		else {
			activeWindowTitle = "";
		}
		ProcessNameTrim(processName, activeWindowTitle);

		//This does the same as the block above, but uses PID instead of a bunch of unnecessary string editing.
		//Both of the following work? I bet there's another(probably better way) to use the class name instead of the macro-definition(?) "_WINUSER_".
		//HWND hWnd = ::GetForegroundWindow();
		HWND hWnd = _WINUSER_::GetForegroundWindow();
		DWORD PID;
		GetWindowThreadProcessId(hWnd, &PID);
		
		DWORD RTSS_Pid = CRTSSClient::GetLastForegroundAppID();
		
		m_pOperatingSLD->UpdateSLD(loopCounter, sysLatResults, processName, activeWindowTitle, PID, RTSS_Pid);

		QueryPerformanceCounter(&EndingTime);
		ElapsedMicrosecondsExtra.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
		ElapsedMicrosecondsExtra.QuadPart *= 1000000;
		ElapsedMicrosecondsExtra.QuadPart /= Frequency.QuadPart;

		timeVectorExtra.push_back(ElapsedMicrosecondsExtra.QuadPart);

		//DEBUG_PRINT("Draw: \t" + to_string(ElapsedMicrosecondsDraw.QuadPart) + "\tExtra: \t" + to_string(ElapsedMicrosecondsExtra.QuadPart))
	}

	long long totalMicroseconds = 0;
	long long averageMicroseconds = 0;
	for (size_t i = 0; i < timeVectorDraw.size(); i++) {
		totalMicroseconds += timeVectorDraw[i];
	}
	double averageMilliseconds;
	if (timeVectorDraw.size() != 0) {
		averageMicroseconds = totalMicroseconds / timeVectorDraw.size();
		averageMilliseconds = static_cast<double>(averageMicroseconds) / 1000;
		//DEBUG_PRINT("Draw Total microseconds: " + to_string(totalMicroseconds))
		//DEBUG_PRINT("Draw Average microseconds: " + to_string(averageMicroseconds))
		//DEBUG_PRINT("Draw Average milliseconds: " + to_string(averageMilliseconds))
	}

	totalMicroseconds = 0;
	averageMicroseconds = 0;

	if (timeVectorExtra.size() != 0) {
		for (size_t i = 0; i < timeVectorExtra.size(); i++) {
			totalMicroseconds += timeVectorExtra[i];
		}

		averageMicroseconds = totalMicroseconds / timeVectorExtra.size();
		averageMilliseconds = static_cast<double>(averageMicroseconds) / 1000;
	}
	//DEBUG_PRINT("Extra Total microseconds: " + to_string(totalMicroseconds))
	//DEBUG_PRINT("Extra Average microseconds: " + to_string(averageMicroseconds))
	//DEBUG_PRINT("Extra Average milliseconds: " + to_string(averageMilliseconds))

	usbController.CloseComPort(hPort);
	sysLatClient.ReleaseOSD();

	return 0;
}
void CSysLat_SoftwareDlg::DrawSquare(CRTSSClient sysLatClient, CString& colorString)
{
	m_updateString = "";
	//The following conditional is FAR from perfect... In order for it to work properly I may need to count the number of rows and columns(in zoomed pixel units?) and use that value. 
	if (sysLatClient.ownedSlot == 0 && !RTSSOpt.m_bPositionManualOverride) {
		if ((int)RTSSOpt.m_positionX < 0) {
			RTSSOpt.m_internalX = 0;
		}
		if ((int)RTSSOpt.m_positionY < 0) {
			//y = CRTSSClient::clientsNum * 20;
			RTSSOpt.m_internalY = 20;
		}
		m_updateString.AppendFormat("<P=%d,%d>", RTSSOpt.m_internalX, RTSSOpt.m_internalY);
		m_updateString += colorString;
		m_updateString += "<P=0,0>";
	}
	else if (RTSSOpt.m_bPositionManualOverride) {
		m_updateString.AppendFormat("<P=%d,%d>", RTSSOpt.m_internalX, RTSSOpt.m_internalY);
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
//The version without a parameter uses the other classes "ExportData" functions
void CSysLat_SoftwareDlg::ExportData()
{
	if (m_vpPreviousSLD.size() > 0) {
		for (unsigned int i = 0; i < m_vpPreviousSLD.size(); i++) {
			//The following code is for testing file export changes - maybe I should make it run only in debugMode?
			//Json::Value newJSON;
			//const Json::Value* const sources[] = {
			//	&m_previousSLD[i]->m_JSONsld,
			//	&m_hardwareID.HardwareIDJSON,
			//	&m_machineInfo.MachineInfoJSON
			//};
			//for (const Json::Value* src : sources)
			//	for (auto srcIt = src->begin(); srcIt != src->end(); ++srcIt)
			//		newJSON[srcIt.name()] = *srcIt;
			//ExportData(newJSON);

			if (!m_vpPreviousSLD[i]->m_bDataExported) {
				m_vpPreviousSLD[i]->ExportData(i, SysLatOpt.m_LogDir, SysLatOpt.m_maxLogs);
			}

			//else {
			//	string error = "Data from test " + to_string(i) + " already exported."; //this error message is garbage in every way
			//	AppendError(error.c_str());
			//}

		}
	}
	else {
		//this is one of the errors that only appears for a few seconds and then dissapears... open an error dialog instead maybe?
		AppendError("No tests have completed yet. \nPress F11 to begin a new test(ending the current test), or wait for the current test to finish. \nYou can change the test size in the menu."); // (Not yet you can't lol)
	}
}
//This overload with a "Json::Value" as a parameter does the export here using streams
void CSysLat_SoftwareDlg::ExportData(Json::Value stuffToExport) {
	std::ofstream exportData;
	exportData.open("./logs/exportSLD.json");

	if (exportData.is_open()) {
		exportData << stuffToExport;
	}
	else {
		DEBUG_PRINT("\nError exporting JSON SLD file.\n")
	}

	exportData.close();
}
void CSysLat_SoftwareDlg::UploadData()
{
	//I DIDN'T WANT TO SET THE API TARGET LIKE THIS - HAD TO DO IT THIS WAY BECAUSE FUNCTIONS THAT ARE USED BY DIALOG MENU BUTTONS CAN'T HAVE PARAMETERS <.<
	const char* APItarget = "/api/benchmarkData";
	
	if (m_vpPreviousSLD.size() > 0) {
		for (unsigned int i = 0; i < m_vpPreviousSLD.size(); i++) {
			http::response<http::string_body> uploadStatus;
			if (!m_vpPreviousSLD[i]->m_bDataUploaded) {

				Json::Value newJSON;
				const Json::Value* const sources[] = {
					&m_vpPreviousSLD[i]->GetJSONData(),
					&m_hardwareID.HardwareIDJSON,
					&m_machineInfo.MachineInfoJSON
				};
				for (const Json::Value* src : sources)
					for (auto srcIt = src->begin(); srcIt != src->end(); ++srcIt)
						newJSON[srcIt.name()] = *srcIt;

				if (m_bTestUploadMode) {
					uploadStatus = upload_data(newJSON, APItarget);
				}
				else {
					uploadStatus = upload_data_secure(newJSON, APItarget);
				}
				m_vpPreviousSLD[i]->m_bDataUploaded = true; //need to make uploadStatus return a bool or something and use it to set this var
			}

			/*else {
				string error = "Data from test " + to_string(i) + " already uploaded.";
				AppendError(error.c_str());
			}*/

		}
	}
	else {
		//this is one of the errors that only appears for a few seconds and then dissapears... open an error dialog instead maybe?
		AppendError("No tests have completed yet. \nPress F11 to begin a new test(ending the current test), or wait for the current test to finish. \nYou can change the test size in the menu."); // (Not yet you can't lol)
	}
}

void CSysLat_SoftwareDlg::OnComPortChanged(UINT nID)
{
	int nButton = nID - ID_COMPORT_START;
	ASSERT(nButton >= 0 && nButton < 100);

	CMenu* MainMenu = GetMenu();

	int count = 0;
	for (auto i = 0; i < m_COMPortInfo.GetSize(); i++) {
		if (count == nButton) {
			MainMenu->CheckMenuItem(nID, MF_CHECKED);

			char menuCString[256];
			MainMenu->GetMenuString(nID, (LPSTR)menuCString, 256, (UINT)MF_BYCOMMAND);
			string menuString = menuCString;
			size_t pos = menuString.rfind("(");
			menuString.replace(0, pos + 1, "");
			pos = menuString.rfind(")");
			menuString.replace(pos, menuString.size(), "");

			SysLatOpt.m_PortSpecifier = menuString;
		}
		else {
			MainMenu->CheckMenuItem(count + ID_COMPORT_START, MF_UNCHECKED);
		}
		count++;
	}

	//1-6-21: getting an error on my machine when going from COM1 to COM4(they are currently 2 different devices) & it's clearly being caused by recreating the thread(and therefore the USB connection)
	ReInitThread();
}
void CSysLat_SoftwareDlg::OnTargetWindowChanged(UINT nID)
{
	int nButton = nID - ID_RTSSAPP_START;
	ASSERT(nButton >= 0 && nButton < 100);

	CMenu* MainMenu = GetMenu();

	int count = 0;
	for (auto const& [pid, pName] : CRTSSClient::m_vszAppArr) {
		if (count == nButton) {
			MainMenu->CheckMenuItem(nID, MF_CHECKED);

			char menuCString[256];
			MainMenu->GetMenuString(nID, (LPSTR)menuCString, 256, (UINT)MF_BYCOMMAND);
			string menuString = menuCString;
			size_t pos = menuString.rfind(" ");
			menuString.replace(pos, menuString.size(), "");

			SysLatOpt.m_targetApp = menuString;
		}
		else {
			MainMenu->CheckMenuItem(count + 1100, MF_UNCHECKED);
		}
		count++;
	}
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
void CSysLat_SoftwareDlg::OpenPreferences() {
	PreferencesDlg preferencesDlg(&SLPref);
	preferencesDlg.DoModal();

	//this should probably be set somewhere else...
	if (PrivacyOpt.m_bRunOnStartup) {
		SetSURegValue(pathToSysLat);
	}
	else {
		SetSURegValue("");
	}
}
void CSysLat_SoftwareDlg::OpenTestCtrl() {
	TestCtrl testCtrl(&m_vpPreviousSLD);
	testCtrl.DoModal();
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

	//m_SysLatStatsClient.GetOSDText(strOSDBuilder, bFormatTagsSupported, bObjTagsSupported);	// get OSD text
*/
							
HBRUSH CSysLat_SoftwareDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	pDC->SetTextColor(RGB(255, 0, 0));
	return m_brush;
}

//This is a duplicate function that was only used for testing, but I think it needs to be moved here so I left it for now...




void CSysLat_SoftwareDlg::CheckUpdate() {
	const char* APItarget = "/api/updateSysLat";
	Json::Value versionNumber;
	versionNumber["version"] = VER_PRODUCT_VERSION_STR;
	versionNumber["versionMajor"] = VERSION_MAJOR;
	versionNumber["versionMinor"] = VERSION_MINOR;

	DEBUG_PRINT(VER_PRODUCT_VERSION_STR)

	boost::beast::http::response<boost::beast::http::string_body> uploadStatus;
	if (m_bTestUploadMode) {
		uploadStatus = upload_data(versionNumber, APItarget);
	}
	else {
		uploadStatus = upload_data_secure(versionNumber, APItarget);
	}

	DEBUG_PRINT("uploadStatus.body(): " + uploadStatus.body())
	int userUpdateChoice = 0;

	if (uploadStatus.result_int() == 302) {
		userUpdateChoice = ::MessageBox(NULL, "Click ok to download the newest version of SysLat or cancel to continue", "Update Available", MB_OKCANCEL);
	}

	if (userUpdateChoice == 1) {
		string newFilePath = pathToSysLat;
		SL::RemoveFileNameFromPath(newFilePath);
		newFilePath += "\\SysLat.exe";
		URLDownloadToFile(NULL, uploadStatus.body().c_str(), newFilePath.c_str(), 0, NULL);
		//if download completed properly...
		::MessageBox(NULL, ("Download complete. Please close this window and start the new version of SysLat at: " + newFilePath).c_str(), "Update Complete", MB_OK);
		//else {
			//download failed
		//}
	}	
}

// if option is no, check to see if it exists and delete it if it does(probably can't use "RegOpenKeyEx" for this)
// if option is yes open and/or create the key
	//if it already exists, delete its value? or delete the entire thing itself?
	//then (re)create it

void CSysLat_SoftwareDlg::SetSURegValue(string regValue) {

	string regSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\");//SysLat
	string regValueName = "SysLat";
	DEBUG_PRINT(regValue)
		
	try
	{
		size_t bufferSize = 0xFFF; // If too small, will be resized down below.
		auto cbData = static_cast<DWORD>(regValue.size() * sizeof(char) + sizeof(char));//leaving off "bufferSize * sizeof(char)" caused Windows defender to think SysLat was a trojan... Maybe? IDK, the problem just went away all of a sudden.
		HKEY hKey;
		DWORD position;
		
		auto rc = RegCreateKeyEx(HKEY_CURRENT_USER, regSubKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &position);
		if ((position == REG_OPENED_EXISTING_KEY || position == REG_CREATED_NEW_KEY) && rc == ERROR_SUCCESS) {
			if (position == REG_OPENED_EXISTING_KEY) {
				DEBUG_PRINT("Key already exists & has been opened.")
			}
			else if (position == REG_CREATED_NEW_KEY) {
				DEBUG_PRINT("Created new key.")
			}

			auto rc = RegSetValueEx(hKey, regValueName.c_str(), 0, REG_SZ, (BYTE*)regValue.data(), cbData);
			if (rc != ERROR_SUCCESS){
				throw std::runtime_error("Windows system error code: " + to_string(rc));
			}
		}
		else if(rc != ERROR_SUCCESS){
			DEBUG_PRINT("Error setting key.\n")
		}
		else {
			DEBUG_PRINT("UNKNOWN ERROR: Key does not exist, and a new key was not created.")
		}
	}
	catch (std::exception& e)
	{
		string errorString = e.what();
		DEBUG_PRINT(errorString)
	}
}


//DEBUG_PRINT("BYTE*: " + to_string(sizeof(BYTE*)));
//DEBUG_PRINT("BYTE: " + to_string(sizeof(BYTE)));
//DEBUG_PRINT("char*: " + to_string(sizeof(char*)));
//DEBUG_PRINT("char: " + to_string(sizeof(char)));
//ASSERT(sizeof(BYTE*) == sizeof(char));

//I never used this function, but it seemed like it could be really nice in the future...
//inline std::wstring convert(const std::string& as)
//{
//	wchar_t* buf = new wchar_t[as.size() * 2 + 2];
//	swprintf(buf, L"%S", as.c_str());
//	std::wstring rval = buf;
//	delete[] buf;
//	return rval;
//}