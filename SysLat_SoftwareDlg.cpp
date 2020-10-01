// SysLat_SoftwareDlg.cpp : implementation file
//
// created by Unwinder
// modified by Skewjo
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RTSSSharedMemory.h"
#include "SysLat_Software.h"
#include "SysLat_SoftwareDlg.h"
#include "GroupedString.h"
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
// NOT AVIALABLE(Opacity) - Profile Setting - Change color of "background"(?) to black (0, 0, 0) with an opacity of 100
//		Better yet - if I could change the box to use a plain black and plain white box so any other text isn't fucked up, that would be better
// DONE(but code is in bad location) - Profile Setting - Set box size to what I want it to be?
// DONE - Change class/namespace name of RTSSSharedMemorySampleDlg to SysLatDlg
// DONE - Change class/namespace of RTSSSharedMemorySample to SysLat
// DONE - Add minimize button
// DONE(well... it half-ass works) - Make System Latency appear in OSD
// Add graph functionality
// Save results to a table
// DONE - Determine active window vs window that RTSS is operating in?
// Enumerate all 3D programs that RTSS can run in and display them in a menu
// Launch RTSS automatically in the background if it's not running
// Add hotkey to restart readings (F11?)
// Fix COM port change settings
// Seperate some initialization that happens in "Refresh" function into a different "Refresh" function??
// Re-org this file into 3-4 new classes - Dialog related functions, RTSS related, DrawingThread related, and USB related
///
/// 
/// 
#include <Windows.h>
#include <process.h>
/// 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Define static variables - these should probably be done as an inline... It's available in C++17 and above, but Visual Studio throws a fit when I try to inline these.
CString CSysLat_SoftwareDlg::m_strStatus = "";
CString CSysLat_SoftwareDlg::m_arduinoResultsComplete = "";
unsigned int CSysLat_SoftwareDlg::m_LoopCounterRefresh = 0;
unsigned int CSysLat_SoftwareDlg::m_loopSize = 0xFFFFFFFF;
bool CSysLat_SoftwareDlg::m_debugMode = true;
CString CSysLat_SoftwareDlg::m_PortSpecifier = "COM3";
int CSysLat_SoftwareDlg::m_systemLatencyTotal = 0;
double CSysLat_SoftwareDlg::m_systemLatencyAverage = 0;
int CSysLat_SoftwareDlg::m_loopCounterEVR = 0;
int CSysLat_SoftwareDlg::m_systemLatencyTotalEVR = 0;
double CSysLat_SoftwareDlg::m_systemLatencyAverageEVR = 0;
HANDLE CSysLat_SoftwareDlg::m_refreshMutex = NULL;
CString CSysLat_SoftwareDlg::m_strError = "";




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

	m_dwNumberOfProcessors		= 0;
	m_pNtQuerySystemInformation = NULL;
	m_strStatus					= "";
	m_strInstallPath			= "";

	m_bMultiLineOutput			= TRUE;
	m_bFormatTags				= TRUE;
	m_bFillGraphs				= FALSE;
	m_bConnected				= FALSE;

	m_dwHistoryPos				= 0;
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

	//init CPU usage calculation related variables	

	SYSTEM_INFO info;
	GetSystemInfo(&info);

	m_dwNumberOfProcessors = info.dwNumberOfProcessors;
	m_pNtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress(GetModuleHandle("NTDLL"), "NtQuerySystemInformation");

	for (DWORD dwCpu = 0; dwCpu < MAX_CPU; dwCpu++)
	{
		m_idleTime[dwCpu].QuadPart = 0;
		m_fltCpuUsage[dwCpu] = FLT_MAX;
		m_dwTickCount[dwCpu] = 0;

		for (DWORD dwPos = 0; dwPos < MAX_HISTORY; dwPos++)
			m_fltCpuUsageHistory[dwCpu][dwPos] = FLT_MAX;
	}

	//init RAM usage history

	for (DWORD dwPos = 0; dwPos < MAX_HISTORY; dwPos++)
		m_fltRamUsageHistory[dwPos] = FLT_MAX;


	//init timer
	m_nTimerID = SetTimer(0x1234, 1000, NULL);

	//init timer for Skewjo...
	time(&m_elapsedTimeStart);

	//Attempt to claim the first slot for SysLat(??) - this definitely feels like the wrong location
	UpdateOSD("", m_caSysLat);


	//init mutex for refresh operation
	m_refreshMutex = CreateMutex(NULL, FALSE, NULL);

	Refresh();

	unsigned int myCounter = 0;
	//HANDLE myhandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, 0, 0, 0);
	HANDLE myhandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, &myCounter, 0, 0);
	SetThreadPriority(myhandle, THREAD_PRIORITY_ABOVE_NORMAL);//31 is(apparently?) the highest possible thread priority - may be bad because it could cause deadlock using a loop? Need to read more here: https://docs.microsoft.com/en-us/windows/win32/procthread/scheduling-priorities
	//AfxBeginThread()

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

	ReleaseOSD(m_caSysLatStats);
	ReleaseOSD(m_caSysLat);
	CloseRefreshMutex();

	CDialog::OnDestroy();
}
DWORD CSysLat_SoftwareDlg::GetSharedMemoryVersion()
{
	DWORD dwResult = 0;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') &&
				(pMem->dwVersion >= 0x00020000))
				dwResult = pMem->dwVersion;

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return dwResult;
}


DWORD CSysLat_SoftwareDlg::GetLastForegroundApp()
{
	DWORD dwResult = 0;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') &&
				(pMem->dwVersion >= 0x00020000))
				dwResult = pMem->dwLastForegroundApp;

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return dwResult;
}

DWORD CSysLat_SoftwareDlg::GetLastForegroundAppID()
{
	DWORD dwResult = 0;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') &&
				(pMem->dwVersion >= 0x00020000))
				dwResult = pMem->dwLastForegroundAppProcessID;

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return dwResult;
}

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

DWORD CSysLat_SoftwareDlg::EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags)
{
	DWORD dwResult = 0;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') &&
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwPass = 0; dwPass < 2; dwPass++)
					//1st pass : find previously captured OSD slot
					//2nd pass : otherwise find the first unused OSD slot and capture it
				{
					for (DWORD dwEntry = 1; dwEntry < pMem->dwOSDArrSize; dwEntry++)
						//allow primary OSD clients (i.e. EVGA Precision / MSI Afterburner) to use the first slot exclusively, so third party
						//applications start scanning the slots from the second one
					{
						RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

						if (dwPass)
						{
							if (!strlen(pEntry->szOSDOwner))
								strcpy_s(pEntry->szOSDOwner, sizeof(pEntry->szOSDOwner), "RTSSSharedMemorySample");
						}

						if (!strcmp(pEntry->szOSDOwner, "RTSSSharedMemorySample"))
						{
							if (pMem->dwVersion >= 0x0002000c)
								//embedded graphs are supported for v2.12 and higher shared memory
							{
								if (dwOffset + sizeof(RTSS_EMBEDDED_OBJECT_GRAPH) + dwBufferSize * sizeof(FLOAT) > sizeof(pEntry->buffer))
									//validate embedded object offset and size and ensure that we don't overrun the buffer
								{
									UnmapViewOfFile(pMapAddr);

									CloseHandle(hMapFile);

									return 0;
								}

								LPRTSS_EMBEDDED_OBJECT_GRAPH lpGraph = (LPRTSS_EMBEDDED_OBJECT_GRAPH)(pEntry->buffer + dwOffset);
								//get pointer to object in buffer

								lpGraph->header.dwSignature = RTSS_EMBEDDED_OBJECT_GRAPH_SIGNATURE;
								lpGraph->header.dwSize = sizeof(RTSS_EMBEDDED_OBJECT_GRAPH) + dwBufferSize * sizeof(FLOAT);
								lpGraph->header.dwWidth = dwWidth;
								lpGraph->header.dwHeight = dwHeight;
								lpGraph->header.dwMargin = dwMargin;
								lpGraph->dwFlags = dwFlags;
								lpGraph->fltMin = fltMin;
								lpGraph->fltMax = fltMax;
								lpGraph->dwDataCount = dwBufferSize;

								if (lpBuffer && dwBufferSize)
								{
									for (DWORD dwPos = 0; dwPos < dwBufferSize; dwPos++)
									{
										FLOAT fltData = lpBuffer[dwBufferPos];

										lpGraph->fltData[dwPos] = (fltData == FLT_MAX) ? 0 : fltData;

										dwBufferPos = (dwBufferPos + 1) & (dwBufferSize - 1);
									}
								}

								dwResult = lpGraph->header.dwSize;
							}

							break;
						}
					}

					if (dwResult)
						break;
				}
			}

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return dwResult;
}
BOOL CSysLat_SoftwareDlg::UpdateOSD(LPCSTR lpText, const char* OSDSlotOwner) {
	BOOL bResult = FALSE;

	//Doesn't it seem inefficient to open a handle to the shared memory every time?  Can I not just leave it open?
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') &&
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwPass = 0; dwPass < 2; dwPass++)
					//1st pass : find previously captured OSD slot
					//2nd pass : otherwise find the first unused OSD slot and capture it
				{
					//If the caller is "SysLat" allow it to take over the first OSD slot
					DWORD dwEntry = 0;
					if (!strcmp(m_caSysLat, OSDSlotOwner)) {
						dwEntry = 1;
					}
					else {
						dwEntry = 0;
					}
					for (dwEntry; dwEntry < pMem->dwOSDArrSize; dwEntry++)
						//allow primary OSD clients (e.g. EVGA Precision / MSI Afterburner) to use the first slot exclusively, so third party 
						//applications start scanning the slots from the second one - CHANGED THIS TO 0 SO I CAN BE PRIMARY BECAUSE I NEED THE CORNERS
					{
						RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

						if (dwPass)
						{
							if (!strlen(pEntry->szOSDOwner))
								strcpy_s(pEntry->szOSDOwner, sizeof(pEntry->szOSDOwner), OSDSlotOwner);
						}

						//remember that strcmp returns 0 if the strings match... so the following if statement basically says if the strings match 
						if (!strcmp(pEntry->szOSDOwner, OSDSlotOwner))
						{
							if (pMem->dwVersion >= 0x00020007)
								//use extended text slot for v2.7 and higher shared memory, it allows displaying 4096 symbols
								//instead of 256 for regular text slot
							{
								if (pMem->dwVersion >= 0x0002000e)
									//OSD locking is supported on v2.14 and higher shared memory
								{
									DWORD dwBusy = _interlockedbittestandset(&pMem->dwBusy, 0);
									//bit 0 of this variable will be set if OSD is locked by renderer and cannot be refreshed
									//at the moment

									if (!dwBusy)
									{
										strncpy_s(pEntry->szOSDEx, sizeof(pEntry->szOSDEx), lpText, sizeof(pEntry->szOSDEx) - 1);

										pMem->dwBusy = 0;
									}
								}
								else
									strncpy_s(pEntry->szOSDEx, sizeof(pEntry->szOSDEx), lpText, sizeof(pEntry->szOSDEx) - 1);

							}
							else
								strncpy_s(pEntry->szOSD, sizeof(pEntry->szOSD), lpText, sizeof(pEntry->szOSD) - 1);

							pMem->dwOSDFrame++;

							bResult = TRUE;

							break;
						}
					}

					if (bResult)
						break;
				}
			}

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return bResult;
}


void CSysLat_SoftwareDlg::ReleaseOSD(const char* OSDSlotOwner)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') &&
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwEntry = 1; dwEntry < pMem->dwOSDArrSize; dwEntry++)
				{
					RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

					if (!strcmp(pEntry->szOSDOwner, OSDSlotOwner))
					{
						memset(pEntry, 0, pMem->dwOSDEntrySize);
						pMem->dwOSDFrame++;
					}
				}
			}

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}
}
DWORD CSysLat_SoftwareDlg::GetClientsNum() {
	DWORD dwClients = 0;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') &&
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwEntry = 0; dwEntry < pMem->dwOSDArrSize; dwEntry++)
				{
					RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

					if (strlen(pEntry->szOSDOwner))
						dwClients++;
				}
			}

			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return dwClients;
}
void CSysLat_SoftwareDlg::Refresh()
{
	//init RivaTuner Statistics Server installation path

	if (m_strInstallPath.IsEmpty())
	{
		HKEY hKey;

		if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Unwinder\\RTSS", &hKey))
		{
			char buf[MAX_PATH];

			DWORD dwSize = MAX_PATH;
			DWORD dwType;

			if (ERROR_SUCCESS == RegQueryValueEx(hKey, "InstallPath", 0, &dwType, (LPBYTE)buf, &dwSize))
			{
				if (dwType == REG_SZ)
					m_strInstallPath = buf;
			}

			RegCloseKey(hKey);
		}
	}

	//validate RivaTuner Statistics Server installation path

	if (_taccess(m_strInstallPath, 0))
		m_strInstallPath = "";

	//init profile interface 

	if (!m_strInstallPath.IsEmpty())
	{
		if (!m_profileInterface.IsInitialized())
			m_profileInterface.Init(m_strInstallPath);
	}

	//init some settings to global(?) profile - probably- scratch that, DEFINITELY need to move these
	//SetProfileProperty("", "BaseColor", 0xFFFFFF);
	//SetProfileProperty("", "BgndColor", 0x000000); //this value isn't actually modifiable in RTSS lol
	//SetProfileProperty("", "FillColor", 0x000000);
	//SetProfileProperty("", "ZoomRatio", 2);
	//SetProfileProperty("", "RefreshPeriod", 0); //found this property by looking at the plaintext of the RTSSHooks.dll.  Doesn't appear to change the value.  Also attempted to use the "Inc" function as well, but it also failed.
	//SetProfileProperty("", "RefreshPeriodMin", 0); //found this property by looking at the plaintext of the RTSSHooks.dll ... It didn't appear to change the value in RTSS... I hope I didn't break something lol
	//SetProfileProperty("", "CoordinateSpace", 1);
	//SetProfileProperty("", "CoordinateSpace", 0);
	m_strStatus = "";


	//init shared memory version

	DWORD dwSharedMemoryVersion = GetSharedMemoryVersion();
	//DWORD dwLastForegroundApp = GetLastForegroundApp();
	DWORD dwLastForegroundAppProcessID = GetLastForegroundAppID();

	//init max OSD text size, we'll use extended text slot for v2.7 and higher shared memory, 
	//it allows displaying 4096 symbols /instead of 256 for regular text slot

	DWORD dwMaxTextSize = (dwSharedMemoryVersion >= 0x00020007) ? sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSDEx) : sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSD);

	BOOL bFormatTagsSupported = (dwSharedMemoryVersion >= 0x0002000b);	//text format tags are supported for shared memory v2.11 and higher
	BOOL bObjTagsSupported = (dwSharedMemoryVersion >= 0x0002000c);		//embedded object tags are supporoted for shared memory v2.12 and higher


	CGroupedString strOSDBuilder(dwMaxTextSize - 1);
	
	GetOSDText(strOSDBuilder, bFormatTagsSupported, bObjTagsSupported);	// get OSD text

	BOOL bTruncated = FALSE;


	BOOL success = AcquireRefreshMutex();		// begin the sync access to fields
	if (!success)
		return;

	//Make my own fucking clock...
	time(&m_elapsedTimeEnd);

	double dif = difftime(m_elapsedTimeEnd, m_elapsedTimeStart);
	int minutes = static_cast<int>(dif) / 60;
	int seconds = static_cast<int>(dif) % 60;

	double measurementsPerSecond = m_LoopCounterRefresh / dif;

	m_strStatus.AppendFormat("Elapsed Time: %02d:%02d", minutes, seconds);
	m_strStatus.Append("\nLast RTSS Foreground App Name: ");
	std::string processName = GetProcessNameFromPID(dwLastForegroundAppProcessID);
	m_strStatus += processName.c_str();
	m_strStatus.Append("\nCurrently active window: ");
	std::string activeWindowTitle = GetActiveWindowTitle();
	m_strStatus += activeWindowTitle.c_str();

	size_t pos = processName.find(".exe");
	if (pos != std::string::npos) {
		processName.replace(pos, processName.size(), "");
	}
	while ((pos = processName.find(" ")) != std::string::npos) {
		processName.replace(pos, 1, "");
	}
	std::transform(processName.begin(), processName.end(), processName.begin(),[](unsigned char c) { return std::tolower(c); });

	
	while ((pos = activeWindowTitle.find(" ")) != std::string::npos) {
		activeWindowTitle.replace(pos, 1, "");
	}
	std::transform(activeWindowTitle.begin(), activeWindowTitle.end(), activeWindowTitle.begin(), [](unsigned char c) { return std::tolower(c); });
	
	m_strStatus.Append("\n");
	m_strStatus += processName.c_str();
	m_strStatus.Append("\n");
	m_strStatus += activeWindowTitle.c_str();


	m_strStatus.AppendFormat("\nSystem Latency: %s", m_arduinoResultsComplete);
	m_strStatus.AppendFormat("\nLoop Counter : %d", m_LoopCounterRefresh);
	m_strStatus.AppendFormat("\n\nMeasurements Per Second: %.2f", measurementsPerSecond);
	m_strStatus.AppendFormat("\nSystem Latency Average: %.2f", m_systemLatencyAverage);
	m_strStatus.AppendFormat("\nLoop Counter EVR(expected value range, 3-100): %d ", m_loopCounterEVR);
	m_strStatus.AppendFormat("\nSystem Latency Average(EVR): %.2f", m_systemLatencyAverageEVR);

	if (!m_strError.IsEmpty())
	{
		m_strStatus.Append(m_strError);
		m_strError = "";
	}
	ReleaseRefreshMutex();		// end the sync access to fields

	CString strOSD = strOSDBuilder.Get(bTruncated);
	strOSD += m_arduinoResultsComplete;
	if (!strOSD.IsEmpty())
	{
		BOOL bResult = UpdateOSD(strOSD, m_caSysLatStats);

		m_bConnected = bResult;

		if (bResult)
		{
			if (bTruncated)
				AppendError("Warning: The text is too long to be displayed in OSD, some info has been truncated!");
		}
		else
		{

			if (m_strInstallPath.IsEmpty())
				AppendError("Error: Failed to connect to RTSS shared memory!\nHints:\n-Install RivaTuner Statistics Server");
			else
				AppendError("Error: Failed to connect to RTSS shared memory!\nHints:\n-Press <Space> to start RivaTuner Statistics Server");
		}
	}



	m_richEditCtrl.SetWindowText(m_strStatus);

	
}

void CSysLat_SoftwareDlg::GetOSDText(CGroupedString& osd, BOOL bFormatTagsSupported, BOOL bObjTagsSupported)
{
	if (bFormatTagsSupported && bObjTagsSupported)
	{
		
		//if (GetClientsNum() == 1)
			//osd.Add("<P=0,10>", "Skewjo's stuff", "|");
			//osd.Add("<P=0,10>", "", "|");
		//move to position 0,10 (in zoomed pixel units)
		//Note: take a note that position is specified in absolute coordinates so use this tag with caution because your text may
		//overlap with text slots displayed by other applications, so in this demo we explicitly disable this tag usage if more than
		//one client is currently rendering something in OSD
	}
}

void CSysLat_SoftwareDlg::CheckRefreshMutex()
{
	if (m_refreshMutex == NULL)
	{
		AppendError("Error: Failed to create mutex");
	}
}
void CSysLat_SoftwareDlg::AppendError(const CString& error)
{
	AcquireRefreshMutex();

	if (!m_strError.IsEmpty())
		m_strError.Append("\n");
	m_strError.Append(error);

	ReleaseRefreshMutex();
}
BOOL CSysLat_SoftwareDlg::AcquireRefreshMutex()
{
	if (m_refreshMutex != NULL)
	{
		return WAIT_ABANDONED != WaitForSingleObject(m_refreshMutex, INFINITE);
	}

	return TRUE;
}
void CSysLat_SoftwareDlg::ReleaseRefreshMutex()
{
	if (m_refreshMutex != NULL)
	{
		ReleaseMutex(m_refreshMutex);
	}
}
void CSysLat_SoftwareDlg::CloseRefreshMutex()
{
	if (m_refreshMutex != NULL)
	{
		CloseHandle(m_refreshMutex);
		m_refreshMutex = NULL;
	}
}

void CSysLat_SoftwareDlg::IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement)
{
	if (m_profileInterface.IsInitialized())
	{
		m_profileInterface.LoadProfile(lpProfile);

		LONG dwProperty = 0;

		if (m_profileInterface.GetProfileProperty(lpProfileProperty, (LPBYTE)&dwProperty, sizeof(dwProperty)))
		{
			dwProperty += dwIncrement;

			m_profileInterface.SetProfileProperty(lpProfileProperty, (LPBYTE)&dwProperty, sizeof(dwProperty));
			m_profileInterface.SaveProfile(lpProfile);
			m_profileInterface.UpdateProfiles();
		}
	}
}
void CSysLat_SoftwareDlg::SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty)
{
	if (m_profileInterface.IsInitialized())
	{
		m_profileInterface.LoadProfile(lpProfile);
		m_profileInterface.SetProfileProperty(lpProfileProperty, (LPBYTE)&dwProperty, sizeof(dwProperty));
		m_profileInterface.SaveProfile(lpProfile);
		m_profileInterface.UpdateProfiles();
	}
}


unsigned int __stdcall CSysLat_SoftwareDlg::CreateDrawingThread(void* data)
{
	int TIMEOUT = 5;

	HANDLE hPort = OpenComPort(m_PortSpecifier);
	CString	m_localPortSpecifier = m_PortSpecifier;

	if (!IsComPortOpened(hPort))
	{
		AppendError("Failed to open the COM port");
		return 0;
	}

	int serialReadData = 0;

	CString	arduinoResults;

	DrawBlack();

	for (unsigned int loopCounter = 1; loopCounter < m_loopSize; loopCounter++)
	{
		//std::ostringstream ostream1;
		//ostream1 << sizeof(loopCounter);
		
		//OutputDebugStringA(ostream1.str().c_str());

		//This is not yet working properly if you attempt to use a port that's not active, unless you switch to a working port very quickly.
		if (m_localPortSpecifier != m_PortSpecifier) {
			CloseComPort(hPort);
			hPort = OpenComPort(m_PortSpecifier);
			m_localPortSpecifier = m_PortSpecifier;
			if (!IsComPortOpened(hPort))
			{
				AppendError("Failed to open the COM port");
				//return 0;
			}
		}


		time_t start = time(NULL);
		while (serialReadData != 65 && time(NULL) - start < TIMEOUT) {
			serialReadData = ReadByte(hPort);
		}
		DrawWhite();
		//Sleep(100); //Can't remember why I had this sleep here, but it was necessary 2 years ago...

		while (serialReadData != 66 && time(NULL) - start < TIMEOUT) {
			serialReadData = ReadByte(hPort);
		}
		DrawBlack();
		//Sleep(100); // Ok, these sleeps definitely coincide with synching the microcontroller and the PC.  Even just 10 ms each seems to help for some stupid reason. 

		arduinoResults = "";

		while (serialReadData != 67 && time(NULL) - start < TIMEOUT) {
			serialReadData = ReadByte(hPort);
			if (serialReadData != 67 && serialReadData != 65 && serialReadData != 66) {
				arduinoResults += (char)serialReadData;
			}
		}

		//I think this should be happening in a different thread so that the serial reads can continue uninterrupted
		SetArduinoResultsComplete(loopCounter, arduinoResults);


	}

	CloseComPort(hPort);

	return 0;
}
void CSysLat_SoftwareDlg::SetArduinoResultsComplete(unsigned int loopCounter, const CString& arduinoResults)
{
	BOOL success = AcquireRefreshMutex();		// begin the sync access to fields
	if (!success)
		return;

	m_LoopCounterRefresh = loopCounter;

	m_arduinoResultsComplete = arduinoResults;

	int systemLatency = 0;
	if (!m_arduinoResultsComplete.IsEmpty()) {
		systemLatency = StrToInt(m_arduinoResultsComplete);
		m_systemLatencyTotal += systemLatency;
		m_systemLatencyAverage = static_cast<double>(m_systemLatencyTotal) / loopCounter; //when I try to cast one of these to a double, it appears to get the program out of sync and shoots the displayed syslat up quite a bit...

		if (systemLatency > 3 && systemLatency < 100) {
			m_loopCounterEVR++;
			m_systemLatencyTotalEVR += systemLatency;
			m_systemLatencyAverageEVR = static_cast<double>(m_systemLatencyTotalEVR) / m_loopCounterEVR;
		}
	}


	//moving average

	int a_movingAverage[10];

	ReleaseRefreshMutex();		// end the sync access to fields
}

HANDLE CSysLat_SoftwareDlg::OpenComPort(const CString& PortSpecifier)
{
	HANDLE hPort = CreateFile(PortSpecifier, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hPort == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;
	PurgeComm(hPort, PURGE_RXCLEAR);
	DCB dcb = { 0 };
	if (!GetCommState(hPort, &dcb))
	{
		CloseHandle(hPort);
		return INVALID_HANDLE_VALUE;
	}
	dcb.BaudRate = CBR_9600; //9600 Baud
	dcb.ByteSize = 8; //8 data bits
	dcb.Parity = NOPARITY; //no parity
	dcb.StopBits = ONESTOPBIT; //1 stop
	if (!SetCommState(hPort, &dcb))
	{
		CloseHandle(hPort);
		return INVALID_HANDLE_VALUE;
	}

	SetCommMask(hPort, EV_RXCHAR | EV_ERR); //receive character event

	// Read this carefully because timeouts are important
	// https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-commtimeouts
	COMMTIMEOUTS timeouts = { 0 };

	return hPort;
}
void CSysLat_SoftwareDlg::CloseComPort(HANDLE hPort)
{
	PurgeComm(hPort, PURGE_RXCLEAR);    // it is not clear whether the purge is needed on each read of the byte, or only when we need to close the port
	CloseHandle(hPort);
}
bool CSysLat_SoftwareDlg::IsComPortOpened(HANDLE hPort)
{
	return hPort != INVALID_HANDLE_VALUE;
}
int CSysLat_SoftwareDlg::ReadByte(HANDLE hPort)
{
	int retVal;

	BYTE Byte;
	DWORD dwBytesTransferred;
	if (FALSE == ReadFile(hPort, &Byte, 1, &dwBytesTransferred, 0)) //read 1
		retVal = 0x101;
	retVal = Byte;

	return retVal;
}

void CSysLat_SoftwareDlg::DrawBlack()
{
	//UpdateOSD("<P=0,0><L0><C=80000000><B=0,0>\b<C><E=-1,-1,8><C=000000><I=-2,0,384,384,128,128><C>", m_caSysLat);
	UpdateOSD("<C=80000000><B=0,0>\b<C><E=-1,-1,8><C=000000><I=-2,0,384,384,128,128><C>", m_caSysLat);
}
void CSysLat_SoftwareDlg::DrawWhite()
{
	//UpdateOSD("<P=0,0><L0><C=80FFFFFF><B=0,0>\b<C><E=-1,-1,8><C=FFFFFF><I=-2,0,384,384,128,128><C>", m_caSysLat);
	UpdateOSD("<C=80FFFFFF><B=0,0>\b<C><E=-1,-1,8><C=FFFFFF><I=-2,0,384,384,128,128><C>", m_caSysLat);
}
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
	return settingsMenu;
}