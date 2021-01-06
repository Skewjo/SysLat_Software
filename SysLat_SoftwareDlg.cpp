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
//  The SysLat RTSSClient object cannot obtain the "0th" RTSS OSD - slot when restarting a test
//	Issue when switching COM ports to an existing device that isn't SysLat and back
//  Arrow key functionality has been optimized away somehow
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

/// 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Define static variables - these should probably be done as inline or something... inlining is supposed to be available in C++17 and above, but Visual Studio throws a fit when I try to inline these.
CString CSysLat_SoftwareDlg::m_strStatus = "";
unsigned int CSysLat_SoftwareDlg::m_LoopCounterRefresh = 0;
unsigned int CSysLat_SoftwareDlg::m_loopSize = 0xFFFFFFFF;
CString	CSysLat_SoftwareDlg::m_updateString = "";
CString CSysLat_SoftwareDlg::m_strError = "";
//need to turn the following into a unique_ptr or at least delete it in the (currently non-existent) dtor...
CSysLatData* CSysLat_SoftwareDlg::m_pOperatingSLD = new CSysLatData;
//The following 2 vars should definitely be constants...
CString CSysLat_SoftwareDlg::m_strBlack = "<C=000000><B=10,10><C>";
CString CSysLat_SoftwareDlg::m_strWhite = "<C=FFFFFF><B=10,10><C>";
DWORD CSysLat_SoftwareDlg::m_sysLatOwnedSlot = 0;
DWORD CSysLat_SoftwareDlg::m_AppArraySize = 0;


//As a non-static variable? Is this a good idea? Need to at least remove the "m_" from the var name... Does this make it local or global??
SysLatPreferences	m_sysLatPreferences;

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

	m_hIcon						= AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_strStatus					= "";
	m_strInstallPath			= "";

	m_bMultiLineOutput			= TRUE;
	m_bFormatTags				= TRUE;
	m_bFillGraphs				= FALSE;
	m_bConnected				= FALSE;
}
CSysLat_SoftwareDlg::~CSysLat_SoftwareDlg() {
	m_sysLatPreferences.WritePreferences();
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
	ON_COMMAND(ID_PORT_COM1, CSysLat_SoftwareDlg::SetPortCom1)
	ON_COMMAND(ID_PORT_COM2, CSysLat_SoftwareDlg::SetPortCom2)
	ON_COMMAND(ID_PORT_COM3, CSysLat_SoftwareDlg::SetPortCom3)
	ON_COMMAND(ID_PORT_COM4, CSysLat_SoftwareDlg::SetPortCom4)
	ON_COMMAND(ID_PORT_COM5, CSysLat_SoftwareDlg::SetPortCom5)
	ON_COMMAND(ID_PORT_COM6, CSysLat_SoftwareDlg::SetPortCom6)
	ON_COMMAND(ID_PORT_COM7, CSysLat_SoftwareDlg::SetPortCom7)
	ON_COMMAND(ID_PORT_COM8, CSysLat_SoftwareDlg::SetPortCom8)
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
	//ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CSysLat_SoftwareDlg::OnInitDialog()
{
	m_color = RGB(136, 217, 242);
	m_brush.CreateSolidBrush(m_color);

	CDialogEx::OnInitDialog();

	//CDialogEx::SetBackgroundColor(RGB(136, 217, 242), 1);

	CWnd* pMainDlg = GetDlgItem(IDD_SYSLAT_SOFTWARE_DIALOG);

	if (pMainDlg)
	{
		pMainDlg->GetClientRect(&clientRect);
	}
	
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

		//m_font.CreateFont(-11, 0, 0, 0, FW_REGULAR, 0, 0, 0, BALTIC_CHARSET, 0, 0, 0, 0, "Courier New");
		m_font.CreateFont(-11, 0, 0, 0, FW_REGULAR, 0, 0, 0, BALTIC_CHARSET, 0, 0, 0, 0, "Courier New");
		m_richEditCtrl.SetFont(&m_font);
		//m_richEditCtrl.SetBackgroundColor(FALSE, m_color);
	}

	
	//init timers
	m_nTimerID = SetTimer(0x1234, 1000, NULL);	//Used by OnTimer function to refresh dialog box & OSD
	time(&m_elapsedTimeStart);					//Used to keep track of test length


	//need to make these run again at the start of each test or something so that if the user changes hardware(??) while the program is running, I can update it(??) - seems dumb
	m_hardwareID.ExportData(m_sysLatPreferences.m_SysLatOptions.m_LogDir);
	m_machineInfo.ExportData(m_sysLatPreferences.m_SysLatOptions.m_LogDir);
	
	//////////////////////////////////////////////////////////////////////////////////
	//move this soon
	R_DynamicComPortMenu();
	//////////////////////////////////////////////////////////////////////////////////
	
	//There has GOT to be a better way for me to do this.
	CMenu* settingsMenu = GetMenu();
	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM5, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM6, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM7, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM8, MF_UNCHECKED);
	if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM1") {
		settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_CHECKED);
	}
	else if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM2") {
		settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_CHECKED);
	}
	else if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM3") {
		settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_CHECKED);
	}
	else if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM4") {
		settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_CHECKED);
	}
	else if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM5") {
		settingsMenu->CheckMenuItem(ID_PORT_COM5, MF_CHECKED);
	}
	else if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM6") {
		settingsMenu->CheckMenuItem(ID_PORT_COM6, MF_CHECKED);
	}
	else if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM7") {
		settingsMenu->CheckMenuItem(ID_PORT_COM7, MF_CHECKED);
	}
	else if (m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier == "COM8") {
		settingsMenu->CheckMenuItem(ID_PORT_COM8, MF_CHECKED);
	}





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
		CDialogEx::OnSysCommand(nID, lParam);
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

	TerminateThread(drawingThreadHandle, 0); //Does exit code need to be 0 for this?
	sysLatStatsClient.ReleaseOSD();
	m_pOperatingSLD->CloseSLDMutex();

	CDialogEx::OnDestroy();
}

//Skewjo's Dialog functions
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
			if (m_sysLatPreferences.m_RTSSOptions.m_internalY > 0) {
				m_sysLatPreferences.m_RTSSOptions.m_internalY--;
			}
			//else appendError ??
			m_sysLatPreferences.m_RTSSOptions.m_bPositionManualOverride = true;
			return TRUE;
		case VK_DOWN:
			if (m_sysLatPreferences.m_RTSSOptions.m_internalY < 255) {
				m_sysLatPreferences.m_RTSSOptions.m_internalY++;
			}
			m_sysLatPreferences.m_RTSSOptions.m_bPositionManualOverride = true;
			return TRUE;
		case VK_LEFT:
			if (m_sysLatPreferences.m_RTSSOptions.m_internalX > 0) {
				m_sysLatPreferences.m_RTSSOptions.m_internalX--;
			}
			m_sysLatPreferences.m_RTSSOptions.m_bPositionManualOverride = true;
			return TRUE;
		case VK_RIGHT:
			if (m_sysLatPreferences.m_RTSSOptions.m_internalX < 255) {
				m_sysLatPreferences.m_RTSSOptions.m_internalX++;
			}
			m_sysLatPreferences.m_RTSSOptions.m_bPositionManualOverride = true;
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
	// I'M NOT SURE WHAT THIS CODE IS FOR. I'M SURE I PUT IT HERE WHEN I WAS ATTEMPTING TO GET DARK MODE WORKING
	//CHARFORMAT cf;
	//cf.dwEffects &= ~CFE_AUTOCOLOR;
	//cf.cbSize = sizeof(CHARFORMAT);
	//m_richEditCtrl.GetSelectionCharFormat(cf);
	//cf.dwMask = CFM_COLOR | CFM_BOLD;
	//cf.dwEffects = CFE_BOLD;
	//cf.crTextColor = RGB(255, 0, 0);
	//m_richEditCtrl.SetSelectionCharFormat(cf);
	//m_richEditCtrl.Invalidate();

	//if (m_bConnected) {//m_bConnected is never set to true??
		DWORD AppArraySize = CRTSSClient::GetAppArray();
		if (m_AppArraySize != AppArraySize) {
			R_DynamicAppMenu();
			m_AppArraySize = AppArraySize;
			DEBUG_PRINT(("AppArraySize: " + to_string(AppArraySize)).c_str())
			
		}
	//}

	if (!(CRTSSClient::m_profileInterface.IsInitialized())) {
		CRTSSClient::InitRTSSInterface();
	}
	m_sysLatPreferences.m_RTSSOptions.m_positionX = CRTSSClient::GetProfileProperty("", "PositionX");
	m_sysLatPreferences.m_RTSSOptions.m_positionY = CRTSSClient::GetProfileProperty("", "PositionY");
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
	m_strStatus.AppendFormat("\nPositionX: %d", m_sysLatPreferences.m_RTSSOptions.m_positionX);
	m_strStatus.AppendFormat("\nPositionY: %d", m_sysLatPreferences.m_RTSSOptions.m_positionY);
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

void CSysLat_SoftwareDlg::R_DynamicComPortMenu()
{
	ULONG portNumbers[100];
	ULONG portsFound;
	ULONG CommPorts = _WINBASE_::GetCommPorts(portNumbers, 100, &portsFound);



	if (CommPorts == ERROR_SUCCESS) {
		DEBUG_PRINT("COM Ports retrieved successfully.")
		
		DEBUG_PRINT(("Found " + to_string(portsFound) + " ports.").c_str())
		
		for (auto i = 0; i < portsFound; i++) {
			DEBUG_PRINT(("\t" + to_string(portNumbers[i])).c_str())
			
		}
	}
	else if (CommPorts == ERROR_MORE_DATA) {
		DEBUG_PRINT("Array to small to retrieve all port numbers.")
		
	}
	else if (CommPorts == ERROR_FILE_NOT_FOUND) {
		DEBUG_PRINT("No COM ports found.")
		
	}
	else {
		DEBUG_PRINT("CommPorts function not equal to any of the standard error codes...")
		
	}

}

void CSysLat_SoftwareDlg::R_DynamicAppMenu()
{
	CMenu* MainMenu = GetMenu();
	CMenu* SettingsMenu = MainMenu->GetSubMenu(1);
	CMenu* TargetAppMenu = SettingsMenu->GetSubMenu(5);
	
	if (TargetAppMenu)
	{
		BOOL appended = false;
		BOOL deleted = false;
		int count = 0;
		for (auto const& [pid, pName] : CRTSSClient::m_mapRTSSApps) {
			//Adding any of the 3 statements below to the conditional below made it not work at all???
			//&& pid > 0
			//&& pid != 0 //IF THIS MAKES IT FAIL EVERY TIME, PID IS ALWAYS EVALUATING TO 0???? BUT IT SHOWS THE PROCESS ID CORRECTLY ALL THE TIME???
			//&& static_cast<unsigned int>(pid) > 0
			//&& to_string(pid) != "0" //OMG THIS DOESN'T WORK EITHER???
			//I ended up having to create a seperate if statement for it for some reason????
			if (count < ID_RTSSAPP_END - ID_RTSSAPP_START) {
				if (pName != "SysLat_Software") { //pid != 0 &&  - this was a part of this conditional, but it changes all of the time?? At one point DotA showed PID 0??
					string id_name =  pName + " (" + to_string(pid) + ")";
					appended = TargetAppMenu->AppendMenu(MF_STRING, ID_RTSSAPP_START + count, id_name.c_str());
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
	if (m_sysLatPreferences.m_PrivacyOptions.m_bAutoExportLogs && m_sysLatPreferences.m_SysLatOptions.m_maxLogs > 0) {
		ExportData();
	}
	if (m_sysLatPreferences.m_PrivacyOptions.m_bAutoUploadLogs) {
		UploadData();
	}
}
unsigned int __stdcall CSysLat_SoftwareDlg::CreateDrawingThread(void* data) //this is probably dangerous, right?
{
	int TIMEOUT = 5; //this should probably be a defined constant
	int serialReadData = 0;
	CString	sysLatResults;
	CRTSSClient sysLatClient("SysLat", 0);
	m_sysLatOwnedSlot = sysLatClient.ownedSlot;

	CUSBController usbController;
	HANDLE hPort = usbController.OpenComPort(m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier.c_str());

	while (!usbController.IsComPortOpened(hPort) && m_loopSize > 0)
	{
		hPort = usbController.OpenComPort(m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier.c_str());
		AppendError("Failed to open COM port: ");
		AppendError(m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier.c_str());
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

		//I think everything below(ESPECIALLY the "UpdateSLD" method) should be happening in a different thread so that the serial reads can continue uninterrupted - could the following be a coroutine?
		// 1-3-2021 thinking on this more, I need the following work to be "queued" up for the main thread... Not sure what the best way to accomplish that is.
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
		/*DEBUG_PRINT(("Current foregound PID: " + to_string(PID)).c_str())
		*/
		DWORD RTSS_Pid = CRTSSClient::GetLastForegroundAppID();
		//DEBUG_PRINT(("RTSS foregound PID: " + to_string(RTSS_Pid)).c_str())
		//
		//	if (PID == RTSS_Pid) {
		//		DEBUG_PRINT("SUCCESS")
		//	}
		//	else {
		//		DEBUG_PRINT("FAIL")
		//	}
		//

		m_pOperatingSLD->UpdateSLD(loopCounter, sysLatResults, processName, activeWindowTitle, PID, RTSS_Pid);



	}

	usbController.CloseComPort(hPort);
	sysLatClient.ReleaseOSD();

	return 0;
}
void CSysLat_SoftwareDlg::DrawSquare(CRTSSClient sysLatClient, CString& colorString)
{
	m_updateString = "";
	//The following conditional is FAR from perfect... In order for it to work properly I may need to count the number of rows and columns(in zoomed pixel units?) and use that value. 
	if (sysLatClient.ownedSlot == 0 && !m_sysLatPreferences.m_RTSSOptions.m_bPositionManualOverride) {
		if ((int)m_sysLatPreferences.m_RTSSOptions.m_positionX < 0) {
			m_sysLatPreferences.m_RTSSOptions.m_internalX = 0;
		}
		if ((int)m_sysLatPreferences.m_RTSSOptions.m_positionY < 0) {
			//y = CRTSSClient::clientsNum * 20;
			m_sysLatPreferences.m_RTSSOptions.m_internalY = 20;
		}
		m_updateString.AppendFormat("<P=%d,%d>", m_sysLatPreferences.m_RTSSOptions.m_internalX, m_sysLatPreferences.m_RTSSOptions.m_internalY);
		m_updateString += colorString;
		m_updateString += "<P=0,0>";
	}
	else if (m_sysLatPreferences.m_RTSSOptions.m_bPositionManualOverride) {
		m_updateString.AppendFormat("<P=%d,%d>", m_sysLatPreferences.m_RTSSOptions.m_internalX, m_sysLatPreferences.m_RTSSOptions.m_internalY);
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
			//The following code is for testing file export changes - maybe I should make it run only in debugMode?
			//Json::Value newJSON;
			//const Json::Value* const sources[] = {
			//	&m_previousSLD[i]->jsonSLD,
			//	&m_hardwareID.HardwareIDJSON,
			//	&m_machineInfo.MachineInfoJSON
			//};
			//for (const Json::Value* src : sources)
			//	for (auto srcIt = src->begin(); srcIt != src->end(); ++srcIt)
			//		newJSON[srcIt.name()] = *srcIt;
			//ExportData(newJSON);

			if (!m_previousSLD[i]->dataExported) {
				m_previousSLD[i]->ExportData(i, m_sysLatPreferences.m_SysLatOptions.m_LogDir, m_sysLatPreferences.m_SysLatOptions.m_maxLogs);
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
void CSysLat_SoftwareDlg::UploadData()
{
	//I DIDN'T WANT TO SET THE API TARGET LIKE THIS - HAD TO DO IT THIS WAY BECAUSE FUNCTIONS THAT ARE USED BY DIALOG MENU BUTTONS CAN'T HAVE PARAMETERS <.<
	const char* APItarget = "/api/benchmarkData";
	if (m_previousSLD.size() > 0) {
		for (unsigned int i = 0; i < m_previousSLD.size(); i++) {
			if (!m_previousSLD[i]->dataUploaded) {

				Json::Value newJSON;
				const Json::Value* const sources[] = {
					&m_previousSLD[i]->jsonSLD,
					&m_hardwareID.HardwareIDJSON,
					&m_machineInfo.MachineInfoJSON
				};
				for (const Json::Value* src : sources)
					for (auto srcIt = src->begin(); srcIt != src->end(); ++srcIt)
						newJSON[srcIt.name()] = *srcIt;

				if (m_bTestUploadMode) {
					int uploadStatus = upload_data(newJSON, APItarget);
				}
				else {
					int uploadStatus = upload_data_secure(newJSON, APItarget);
				}
				m_previousSLD[i]->dataUploaded = true; //need to make uploadStatus return a bool or something and use it to set this var
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

//Settings - need to look into GetCommPorts function to enumerate COM ports https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcommports
void CSysLat_SoftwareDlg::SetPortCom1()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM1";
}
void CSysLat_SoftwareDlg::SetPortCom2()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM2";
}
void CSysLat_SoftwareDlg::SetPortCom3()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM3";
}
void CSysLat_SoftwareDlg::SetPortCom4()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM4";
}
void CSysLat_SoftwareDlg::SetPortCom5()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM5, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM5";
}
void CSysLat_SoftwareDlg::SetPortCom6()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM6, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM6";
}
void CSysLat_SoftwareDlg::SetPortCom7()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM7, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM7";
}
void CSysLat_SoftwareDlg::SetPortCom8()
{
	CMenu* settingsMenu = ResetPortsMenuItems();
	settingsMenu->CheckMenuItem(ID_PORT_COM8, MF_CHECKED);
	m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM8";
}
CMenu* CSysLat_SoftwareDlg::ResetPortsMenuItems()
{
	CMenu* settingsMenu = GetMenu();
	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM5, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM6, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM7, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM8, MF_UNCHECKED);
	ReInitThread();
	return settingsMenu;
}

//not yet in use
void CSysLat_SoftwareDlg::OnComPortChanged(UINT nID)
{
	int nButton = nID - ID_RTSSAPP_START;
	ASSERT(nButton >= 0 && nButton < 100);

	CMenu* MainMenu = GetMenu();

	int count = 0;
	for (auto const& [pid, pName] : CRTSSClient::m_mapRTSSApps) {
		if (count == nButton) {
			MainMenu->CheckMenuItem(nID, MF_CHECKED);
			//set USB port from preferences & other vars here...
			m_sysLatPreferences.m_SysLatOptions.m_PortSpecifier = "COM" + to_string(nButton+1);
		}
		else {
			MainMenu->CheckMenuItem(count + 1000, MF_UNCHECKED);
		}
		count++;
	}

	ReInitThread();
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
	PreferencesDlg preferencesDlg(&m_sysLatPreferences);
	preferencesDlg.DoModal();
}
void CSysLat_SoftwareDlg::OpenTestCtrl() {
	TestCtrl testCtrl(&m_previousSLD);
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

	//sysLatStatsClient.GetOSDText(strOSDBuilder, bFormatTagsSupported, bObjTagsSupported);	// get OSD text
*/
							
HBRUSH CSysLat_SoftwareDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	pDC->SetTextColor(RGB(255, 0, 0));
	return m_brush;
}

//This is a duplicate function that was only used for testing, but I think it needs to be moved here so I left it for now...
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

void CSysLat_SoftwareDlg::OnTargetWindowChanged(UINT nID)
{
	int nButton = nID - ID_RTSSAPP_START;
	ASSERT(nButton >= 0 && nButton < 100);

	CMenu* MainMenu = GetMenu();

	int count = 0;
	for (auto const& [pid, pName] : CRTSSClient::m_mapRTSSApps) {
		if (count == nButton) {
			MainMenu->CheckMenuItem(nID, MF_CHECKED);
			//set jsonSLD something to pid & something else to pName HERE
		}
		else {
			MainMenu->CheckMenuItem(count + 1100, MF_UNCHECKED);
		}
		count++;
	}
}
