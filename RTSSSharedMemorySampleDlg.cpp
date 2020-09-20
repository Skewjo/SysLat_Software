// RTSSSharedMemorySampleDlg.cpp : implementation file
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "RTSSSharedMemory.h"
#include "RTSSSharedMemorySample.h"
#include "RTSSSharedMemorySampleDlg.h"
#include "GroupedString.h"
/////////////////////////////////////////////////////////////////////////////
#include <shlwapi.h>
#include <float.h>
#include <io.h>
#include <sstream>


///
#include <Windows.h>
#include <process.h>
/// 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Define static variables - these should probably be done as an inline...
CString CRTSSSharedMemorySampleDlg::m_strStatus = "";
CString	CRTSSSharedMemorySampleDlg::m_arduinoResults = "";
CString CRTSSSharedMemorySampleDlg::m_tLoopCounter = "";
CString	CRTSSSharedMemorySampleDlg::m_tInnerLoopTimer = "";
CString	CRTSSSharedMemorySampleDlg::m_tOuterLoopTimer = "";
CString	CRTSSSharedMemorySampleDlg::m_tDrawWhite = "";


CString CRTSSSharedMemorySampleDlg::m_arduinoResultsComplete = "";
CString CRTSSSharedMemorySampleDlg::m_arduinoResultsRefresh = "";
CString	CRTSSSharedMemorySampleDlg::m_tLoopCounterRefresh = "";
CString CRTSSSharedMemorySampleDlg::m_tInnerLoopTimerRefresh = "";
CString CRTSSSharedMemorySampleDlg::m_tOuterLoopTimerRefresh = "";
CString CRTSSSharedMemorySampleDlg::m_tDrawWhiteRefresh = "";


bool CRTSSSharedMemorySampleDlg::debugMode = true;
CString CRTSSSharedMemorySampleDlg::PortOption = "COM3";
CString CRTSSSharedMemorySampleDlg::PortSpecifier = CString(_T(PortOption));
int CRTSSSharedMemorySampleDlg::serialReadData = 0;
LARGE_INTEGER CRTSSSharedMemorySampleDlg::begin = { 0 };
LARGE_INTEGER CRTSSSharedMemorySampleDlg::end = { 0 };
LARGE_INTEGER CRTSSSharedMemorySampleDlg::frequency = { 0 };

int CRTSSSharedMemorySampleDlg::systemLatencyTotal = 0;
double CRTSSSharedMemorySampleDlg::systemLatencyAverage = 0;
int CRTSSSharedMemorySampleDlg::loopCounterEVR = 0;
int CRTSSSharedMemorySampleDlg::systemLatencyTotalEVR = 0;
double CRTSSSharedMemorySampleDlg::systemLatencyAverageEVR = 0;




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
// CRTSSSharedMemorySampleDlg dialog
/////////////////////////////////////////////////////////////////////////////
CRTSSSharedMemorySampleDlg::CRTSSSharedMemorySampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRTSSSharedMemorySampleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRTSSSharedMemorySampleDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	m_hIcon						= AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_dwNumberOfProcessors		= 0;
	m_pNtQuerySystemInformation	= NULL;
	m_strStatus					= "";
	m_strInstallPath			= "";

	m_bMultiLineOutput			= TRUE;
	m_bFormatTags				= TRUE;
	m_bFillGraphs				= FALSE;
	m_bConnected				= FALSE;

	m_dwHistoryPos				= 0;
}
void CRTSSSharedMemorySampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRTSSSharedMemorySampleDlg)
	//}}AFX_DATA_MAP
}
BEGIN_MESSAGE_MAP(CRTSSSharedMemorySampleDlg, CDialog)
	//{{AFX_MSG_MAP(CRTSSSharedMemorySampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_PORT_COM1, CRTSSSharedMemorySampleDlg::SetPortCom1)
	ON_COMMAND(ID_PORT_COM2, CRTSSSharedMemorySampleDlg::SetPortCom2)
	ON_COMMAND(ID_PORT_COM3, CRTSSSharedMemorySampleDlg::SetPortCom3)
	ON_COMMAND(ID_PORT_COM4, CRTSSSharedMemorySampleDlg::SetPortCom4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRTSSSharedMemorySampleDlg message handlers
/////////////////////////////////////////////////////////////////////////////
BOOL CRTSSSharedMemorySampleDlg::OnInitDialog()
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

		if (!m_richEditCtrl.Create(WS_VISIBLE|ES_READONLY|ES_MULTILINE|ES_AUTOHSCROLL|WS_HSCROLL|ES_AUTOVSCROLL|WS_VSCROLL, rect, this, 0))
			return FALSE;

		m_font.CreateFont(-11, 0, 0, 0, FW_REGULAR, 0, 0, 0, BALTIC_CHARSET, 0, 0, 0, 0, "Courier New");
		m_richEditCtrl.SetFont(&m_font);
	}

	//init CPU usage calculation related variables	

	SYSTEM_INFO info;
	GetSystemInfo(&info); 

	m_dwNumberOfProcessors		= info.dwNumberOfProcessors;
	m_pNtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress(GetModuleHandle("NTDLL"), "NtQuerySystemInformation");

	for (DWORD dwCpu=0; dwCpu<MAX_CPU; dwCpu++)
	{
		m_idleTime[dwCpu].QuadPart			= 0;
		m_fltCpuUsage[dwCpu]				= FLT_MAX;
		m_dwTickCount[dwCpu]				= 0;

		for (DWORD dwPos=0; dwPos<MAX_HISTORY; dwPos++)
			m_fltCpuUsageHistory[dwCpu][dwPos] = FLT_MAX;
	}

	//init RAM usage history

	for (DWORD dwPos=0; dwPos<MAX_HISTORY; dwPos++)
		m_fltRamUsageHistory[dwPos] = FLT_MAX;


	//init timer

	m_nTimerID = SetTimer(0x1234, 1000, NULL);
	
	//init timer for Skewjo...
	time(&elapsedTimeStart);

	Refresh();

	unsigned int myCounter = 0;
	//HANDLE myhandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, 0, 0, 0);
	HANDLE myhandle = (HANDLE)_beginthreadex(0, 0, CreateDrawingThread, &myCounter, 0, 0);
	SetThreadPriority(myhandle, 31);// highest possible thread priority? - may be bad because it could cause deadlock using a loop? Need to read more here: https://docs.microsoft.com/en-us/windows/win32/procthread/scheduling-priorities
	//AfxBeginThread()

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CRTSSSharedMemorySampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CRTSSSharedMemorySampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
HCURSOR CRTSSSharedMemorySampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
void CRTSSSharedMemorySampleDlg::OnTimer(UINT nIDEvent) 
{
	Refresh();
	CDialog::OnTimer(nIDEvent);
}
void CRTSSSharedMemorySampleDlg::OnDestroy() 
{
	if (m_nTimerID)
		KillTimer(m_nTimerID);

	m_nTimerID = NULL;

	MSG msg; 
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));

	ReleaseOSD();

	CDialog::OnDestroy();
}
DWORD CRTSSSharedMemorySampleDlg::GetSharedMemoryVersion()
{
	DWORD dwResult	= 0;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr				= MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem	= (LPRTSS_SHARED_MEMORY)pMapAddr;

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
DWORD CRTSSSharedMemorySampleDlg::EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags)
{
	DWORD dwResult	= 0;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr				= MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem	= (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') && 
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwPass=0; dwPass<2; dwPass++)
					//1st pass : find previously captured OSD slot
					//2nd pass : otherwise find the first unused OSD slot and capture it
				{
					for (DWORD dwEntry=1; dwEntry<pMem->dwOSDArrSize; dwEntry++)
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

								lpGraph->header.dwSignature	= RTSS_EMBEDDED_OBJECT_GRAPH_SIGNATURE;
								lpGraph->header.dwSize		= sizeof(RTSS_EMBEDDED_OBJECT_GRAPH) + dwBufferSize * sizeof(FLOAT);
								lpGraph->header.dwWidth		= dwWidth;
								lpGraph->header.dwHeight	= dwHeight;
								lpGraph->header.dwMargin	= dwMargin;
								lpGraph->dwFlags			= dwFlags;
								lpGraph->fltMin				= fltMin;
								lpGraph->fltMax				= fltMax;
								lpGraph->dwDataCount		= dwBufferSize;

								if (lpBuffer && dwBufferSize)
								{
									for (DWORD dwPos=0; dwPos<dwBufferSize; dwPos++)
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
BOOL CRTSSSharedMemorySampleDlg::UpdateOSD(LPCSTR lpText){
	BOOL bResult	= FALSE;

	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr				= MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem	= (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') && 
				(pMem->dwVersion >= 0x00020000))
			{
				for (DWORD dwPass=0; dwPass<2; dwPass++)
					//1st pass : find previously captured OSD slot
					//2nd pass : otherwise find the first unused OSD slot and capture it
				{
					for (DWORD dwEntry=1; dwEntry<pMem->dwOSDArrSize; dwEntry++)
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
void CRTSSSharedMemorySampleDlg::ReleaseOSD()
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
				for (DWORD dwEntry=1; dwEntry<pMem->dwOSDArrSize; dwEntry++)
				{
					RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

					if (!strcmp(pEntry->szOSDOwner, "RTSSSharedMemorySample"))
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
DWORD CRTSSSharedMemorySampleDlg::GetClientsNum(){
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
				for (DWORD dwEntry=0; dwEntry<pMem->dwOSDArrSize; dwEntry++)
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
void CRTSSSharedMemorySampleDlg::Refresh()
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

	//init shared memory version

	DWORD dwSharedMemoryVersion = GetSharedMemoryVersion();

	//init max OSD text size, we'll use extended text slot for v2.7 and higher shared memory, 
	//it allows displaying 4096 symbols /instead of 256 for regular text slot

	DWORD dwMaxTextSize = (dwSharedMemoryVersion >= 0x00020007) ? sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSDEx) : sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSD);

	CGroupedString groupedString(dwMaxTextSize - 1);
	// RivaTuner based products use similar CGroupedString object for convenient OSD text formatting and length control
	// You may use it to format your OSD similar to RivaTuner's one or just use your own routines to format OSD text

	BOOL bFormatTagsSupported = (dwSharedMemoryVersion >= 0x0002000b);
	//text format tags are supported for shared memory v2.11 and higher
	BOOL bObjTagsSupported = (dwSharedMemoryVersion >= 0x0002000c);
	//embedded object tags are supporoted for shared memory v2.12 and higher

	CString strOSD;

	if (bFormatTagsSupported && m_bFormatTags)
	{
		if (GetClientsNum() == 1)
			strOSD += "<P=0,10>";
		//move to position 0,10 (in zoomed pixel units)

		//Note: take a note that position is specified in absolute coordinates so use this tag with caution because your text may
		//overlap with text slots displayed by other applications, so in this demo we explicitly disable this tag usage if more than
		//one client is currently rendering something in OSD

		strOSD = "";

	}
	else
		strOSD = "";

	BOOL bTruncated = FALSE;

	//Make my own fucking clock...
	time(&elapsedTimeEnd);
	double dif = difftime(elapsedTimeEnd, elapsedTimeStart);
	int minutes = (int)dif / 60;
	int seconds = (int)dif % 60;
	char aMinutes[2];
	char aSeconds[2];
	std::string elapsedMinutes = itoa(minutes, aMinutes, 10);
	std::string elapsedSeconds = itoa(seconds, aSeconds, 10);

	m_strStatus = "";

	m_strStatus += "Elapsed Time: ";
	if (minutes < 10) m_strStatus += "0";
	m_strStatus += elapsedMinutes.c_str();
	m_strStatus += ":";
	if (seconds < 10) m_strStatus += "0";
	m_strStatus += elapsedSeconds.c_str();
	m_strStatus += "\n\nSystem Latency: ";
	m_strStatus += m_arduinoResultsComplete;
	m_strStatus += "\nLoop Counter: ";
	m_strStatus += m_tLoopCounterRefresh;
	
	m_strStatus += "\nMeasurements Per Second: ";
	double measurementsPerSecond = StrToInt(m_tLoopCounterRefresh) / dif;
	char buffer3[4];
	gcvt(measurementsPerSecond, 4, buffer3);
	m_strStatus.Append(buffer3);

	m_strStatus += "\nSystem Latency Average: ";
	char buffer[4];
	gcvt(systemLatencyAverage, 4, buffer);
	m_strStatus.Append(buffer);

	m_strStatus.Append("\n\nExpected value range 3-100\nLoop Counter(EVR): ");
	//m_strStatus += loopCounterEVR;
	m_strStatus.Append("\nSystem Latency Average(EVR): ");
	char buffer2[4];
	gcvt(systemLatencyAverageEVR, 4, buffer2);
	m_strStatus.Append(buffer2);
	//m_strStatus.Append("\0");

	
	
	//m_strStatus += systemLatencyAverage;
	//m_strStatus += m_tInnerLoopTimerRefresh;
	//m_strStatus += m_tOuterLoopTimerRefresh;
	//m_strStatus += m_tDrawWhiteRefresh;



	if (!strOSD.IsEmpty())
	//if (!m_strStatus.IsEmpty())
	{
		BOOL bResult = UpdateOSD(strOSD);

		m_bConnected = bResult;

		if (bResult)
		{
			if (bTruncated)
				m_strStatus += "\n\nWarning!\nThe text is too long to be displayed in OSD, some info has been truncated!";
		}
		else
		{

			if (m_strInstallPath.IsEmpty())
				m_strStatus = "Failed to connect to RTSS shared memory!\n\nHints:\n-Install RivaTuner Statistics Server";
			else
				m_strStatus = "Failed to connect to RTSS shared memory!\n\nHints:\n-Press <Space> to start RivaTuner Statistics Server";
		}


		m_richEditCtrl.SetWindowText(m_strStatus);
	}
	//This else was added by me, but feels wrong...
	else {
		m_richEditCtrl.SetWindowText(m_strStatus);
	}
}
//the following is an overload of the Refresh method I modified and used before I discovered the "UpdateOSD" method
/*
void CRTSSSharedMemorySampleDlg::Refresh(CString externalString)
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

	//init shared memory version

	DWORD dwSharedMemoryVersion = GetSharedMemoryVersion();

	//init max OSD text size, we'll use extended text slot for v2.7 and higher shared memory, 
	//it allows displaying 4096 symbols /instead of 256 for regular text slot

	DWORD dwMaxTextSize = (dwSharedMemoryVersion >= 0x00020007) ? sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSDEx) : sizeof(RTSS_SHARED_MEMORY::RTSS_SHARED_MEMORY_OSD_ENTRY().szOSD);

	CGroupedString groupedString(dwMaxTextSize - 1);
	// RivaTuner based products use similar CGroupedString object for convenient OSD text formatting and length control
	// You may use it to format your OSD similar to RivaTuner's one or just use your own routines to format OSD text

	BOOL bFormatTagsSupported = (dwSharedMemoryVersion >= 0x0002000b);
	//text format tags are supported for shared memory v2.11 and higher
	BOOL bObjTagsSupported = (dwSharedMemoryVersion >= 0x0002000c);
	//embedded object tags are supporoted for shared memory v2.12 and higher

	CString strOSD;

	if (bFormatTagsSupported && m_bFormatTags)
	{
		if (GetClientsNum() == 1)
			strOSD += "<P=0,10>";
		//move to position 0,10 (in zoomed pixel units)

		//Note: take a note that position is specified in absolute coordinates so use this tag with caution because your text may
		//overlap with text slots displayed by other applications, so in this demo we explicitly disable this tag usage if more than
		//one client is currently rendering something in OSD

		strOSD = externalString;

	}
	else
		strOSD = "";

	BOOL bTruncated = FALSE;


	if (!strOSD.IsEmpty())
	{
		BOOL bResult = UpdateOSD(strOSD);

		m_bConnected = bResult;

		if (bResult)
		{
			if (bTruncated)
				m_strStatus += "\n\nWarning!\nThe text is too long to be displayed in OSD, some info has been truncated!";
		}
		else
		{

			if (m_strInstallPath.IsEmpty())
				m_strStatus = "Failed to connect to RTSS shared memory!\n\nHints:\n-Install RivaTuner Statistics Server";
			else
				m_strStatus = "Failed to connect to RTSS shared memory!\n\nHints:\n-Press <Space> to start RivaTuner Statistics Server";
		}


		m_richEditCtrl.SetWindowText(m_strStatus);
	}
}
*/
void CRTSSSharedMemorySampleDlg::IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement)
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
void CRTSSSharedMemorySampleDlg::SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty)
{
	if (m_profileInterface.IsInitialized())
	{
		m_profileInterface.LoadProfile(lpProfile);
		m_profileInterface.SetProfileProperty(lpProfileProperty, (LPBYTE)&dwProperty, sizeof(dwProperty));
		m_profileInterface.SaveProfile(lpProfile);
		m_profileInterface.UpdateProfiles();
	}
}


unsigned int __stdcall CRTSSSharedMemorySampleDlg::CreateDrawingThread(void* data) {
	unsigned int& loopIncrementer = *((unsigned int*)data); //??
	
	std::stringstream sst;

	LARGE_INTEGER begin1 = { 0 };
	LARGE_INTEGER end1 = { 0 };
	double PCFreq;
	
	int loopCounter = 0;
	loopCounterEVR = 0;

	double totalReadTime = 0;
	double avgReadTime = 0;
	int totalReadLoops = 0;
	float avgReadLoops = 0;


	if (debugMode) {
		QueryPerformanceFrequency(&frequency);
		PCFreq = double(frequency.QuadPart) / 1000.0;
	}
	
	//Need to DrawBlack box once before loop starts
	DrawBlack();

	while (loopIncrementer < 0xFFFFFFFF) {
		loopCounter++;
		
		m_tLoopCounter = "";
		m_arduinoResults = "";
		//m_tInnerLoopTimer = "";
		//m_tOuterLoopTimer = "";
		//m_tDrawWhite = "";
		if (debugMode) {
			sst << loopCounter;
			m_tLoopCounter += (sst.str().c_str());
			sst.str(std::string()); //most efficient way to empty the stingstream?
			QueryPerformanceCounter(&begin);
		}
		m_tLoopCounterRefresh = m_tLoopCounter;

		while (serialReadData != 65) {
			serialReadData = ReadByte(PortSpecifier);
		}
		DrawWhite();
		Sleep(100); //Can't remember why I had this sleep here, but it was necessary 2 years ago...

		while (serialReadData != 66) {
			serialReadData = ReadByte(PortSpecifier);
		}
		DrawBlack();
		Sleep(100); // Ok, these sleeps definitely coincide with synching the microcontroller and the PC.  Even just 10 ms each seems to help for some stupid reason. 

		
		while (serialReadData != 67) {
			serialReadData = ReadByte(PortSpecifier);
			if (serialReadData != 67 && serialReadData != 65 && serialReadData != 66) {
				m_arduinoResults += (char)serialReadData;
			}
		}
		m_arduinoResultsComplete = m_arduinoResults;

		
		int systemLatency = 0;
		if (!m_arduinoResultsComplete.IsEmpty()) {
			systemLatency = StrToInt(m_arduinoResultsComplete);
			systemLatencyTotal += systemLatency;
			systemLatencyAverage = systemLatencyTotal / loopCounter; //when I try to make one of these a double, it appears to get the program out of sync and shoots the displayed syslat up quite a bit...

			if (systemLatency > 3 && systemLatency < 100) {
				loopCounterEVR++;
				systemLatencyTotalEVR += systemLatency;
				systemLatencyAverageEVR = systemLatencyTotalEVR / loopCounterEVR; 
			}
		}
		

		/*if (debugMode) {
	QueryPerformanceCounter(&end);
	//ugh... do I have to use a stringstream for this?
	m_tOuterLoopTimer += "Total old read loop time: \t";
	sst << double(end.QuadPart - begin.QuadPart) / PCFreq << " ms" << std::endl;
	m_tOuterLoopTimer += (sst.str().c_str());
	sst.str(std::string()); //most efficient way to empty the stingstream?
}*/
		++loopIncrementer;
		
	}

	return 0;
}

/*
bool CRTSSSharedMemorySampleDlg::WriteComPort(CString PortSpecifier, CString data)
{
	//DCB dcb;
	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(DCB);

	DWORD byteswritten;
	HANDLE hPort = CreateFile(
		PortSpecifier,
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	//if (!GetCommState(hPort, &dcb))
	//	return false;
	if (!::GetCommState(hPort, &dcb))
	{
		TRACE("CSerialCommHelper : Failed to Get Comm State Reason:% d",GetLastError());
			return E_FAIL;
	}

	dcb.BaudRate = CBR_9600; //9600 Baud
	dcb.ByteSize = 8; //8 data bits
	dcb.Parity = NOPARITY; //no parity
	dcb.StopBits = ONESTOPBIT; //1 stop
	
	//if (!SetCommState(hPort, &dcb))
	//	return false;
	if (!::SetCommState(hPort, &dcb))
	{
		ASSERT(0);
		TRACE("CSerialCommHelper : Failed to Set Comm State Reason:% d",GetLastError());
			return E_FAIL;
	}

	bool retVal = WriteFile(hPort, data, 1, &byteswritten, NULL);
	CloseHandle(hPort); //close the handle

	return retVal;
}
*/

int CRTSSSharedMemorySampleDlg::ReadByte(CString PortSpecifier)
{
	DCB dcb;
	int retVal;
	BYTE Byte;
	DWORD dwBytesTransferred;
	DWORD dwCommModemStatus;
	HANDLE hPort = CreateFile(
		PortSpecifier,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (!GetCommState(hPort, &dcb))
		return 0x100;
	dcb.BaudRate = CBR_9600; //9600 Baud
	dcb.ByteSize = 8; //8 data bits
	dcb.Parity = NOPARITY; //no parity
	dcb.StopBits = ONESTOPBIT; //1 stop
	if (!SetCommState(hPort, &dcb))
		return 0x100;
	//need to check out EV_TXTEMPTY flag
	SetCommMask(hPort, EV_RXCHAR | EV_ERR); //receive character event
	WaitCommEvent(hPort, &dwCommModemStatus, 0); //wait for character
	if (dwCommModemStatus & EV_RXCHAR)
		ReadFile(hPort, &Byte, 1, &dwBytesTransferred, 0); //read 1
	else if (dwCommModemStatus & EV_ERR)
		retVal = 0x101;
	retVal = Byte;
	PurgeComm(hPort, PURGE_RXCLEAR);
	CloseHandle(hPort);
	return retVal;
}



void CRTSSSharedMemorySampleDlg::DrawBlack() {
	CString formatString;
	formatString = "<P=0,0><L0><C=80000000><B=0,0>\b<C><C=000000><I=-2,0,384,384,128,128><C>";
	UpdateOSD(formatString);
}
void CRTSSSharedMemorySampleDlg::DrawWhite() {
	CString formatString;
	formatString = "<P=0,0><L0><C=80FFFFFF><B=0,0>\b<C><C=FFFFFF><I=-2,0,384,384,128,128><C>";
	UpdateOSD(formatString);
}
//There's a lot of repeated code in these 4 functions...
void CRTSSSharedMemorySampleDlg::SetPortCom1() {
	CMenu* settingsMenu = GetMenu();

	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_CHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_UNCHECKED);

	PortOption = "COM1";
	PortSpecifier = CString(_T(PortOption));
}
void CRTSSSharedMemorySampleDlg::SetPortCom2() {
	CMenu* settingsMenu = GetMenu();
	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_CHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_UNCHECKED);

	PortOption = "COM2";
	PortSpecifier = CString(_T(PortOption));
}
void CRTSSSharedMemorySampleDlg::SetPortCom3() {
	CMenu* settingsMenu = GetMenu();
	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_CHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_UNCHECKED);

	PortOption = "COM3";
	PortSpecifier = CString(_T(PortOption));
}
void CRTSSSharedMemorySampleDlg::SetPortCom4() {
	CMenu* settingsMenu = GetMenu();
	settingsMenu->CheckMenuItem(ID_PORT_COM1, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM2, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM3, MF_UNCHECKED);
	settingsMenu->CheckMenuItem(ID_PORT_COM4, MF_CHECKED);

	PortOption = "COM4";
	PortSpecifier = CString(_T(PortOption));
}