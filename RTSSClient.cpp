#include "stdafx.h"
#include "RTSSClient.h"
#include "RTSSSharedMemory.h"

//static member initializations
DWORD CRTSSClient::sharedMemoryVersion = 0;
CRTSSProfileInterface CRTSSClient::m_profileInterface;
CString CRTSSClient::m_strInstallPath = "";
DWORD CRTSSClient::clientsNum = 0;
DWORD CRTSSClient::dwAppEntries = 0;
vector<std::pair<DWORD, string>> CRTSSClient::m_vszAppArr = {};


CRTSSClient::CRTSSClient(const char* setSlotOwner, int setClientPriority) {
	slotOwnerOSD = setSlotOwner;
	clientPriority = setClientPriority;
	
	InitRTSSInterface();
}

void CRTSSClient::InitRTSSInterface() {
	//init RivaTuner Statistics Server installation path - this needs to be occurring somewhere else...
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
	if (!CRTSSClient::m_profileInterface.IsInitialized())
		CRTSSClient::m_profileInterface.Init(m_strInstallPath);

	//If the profile interface initialized, then initialize sharedMemoryVersion as well
	if (m_profileInterface.IsInitialized()) {
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

		sharedMemoryVersion = dwResult;
	}
}

DWORD CRTSSClient::GetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty)
{
	DWORD dwProperty = 0;
	if (m_profileInterface.IsInitialized())
	{
		m_profileInterface.LoadProfile(lpProfile);

		m_profileInterface.GetProfileProperty(lpProfileProperty, (LPBYTE)&dwProperty, sizeof(dwProperty));
	}

	return dwProperty;
}

void CRTSSClient::IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement)
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

void CRTSSClient::SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty)
{
	if (m_profileInterface.IsInitialized())
	{
		m_profileInterface.LoadProfile(lpProfile);
		m_profileInterface.SetProfileProperty(lpProfileProperty, (LPBYTE)&dwProperty, sizeof(dwProperty));
		m_profileInterface.SaveProfile(lpProfile);
		m_profileInterface.UpdateProfiles();
	}
}

DWORD CRTSSClient::GetClientsNum() {
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
DWORD CRTSSClient::GetSharedMemoryVersion()
{
	
	if (!sharedMemoryVersion) {
		InitRTSSInterface();
	}

	return sharedMemoryVersion;
}
DWORD CRTSSClient::GetLastForegroundApp()
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
DWORD CRTSSClient::GetLastForegroundAppID()
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
DWORD CRTSSClient::GetAppArray() {
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");

	if (hMapFile)
	{
		LPVOID pMapAddr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		LPRTSS_SHARED_MEMORY pMem = (LPRTSS_SHARED_MEMORY)pMapAddr;

		if (pMem)
		{
			if ((pMem->dwSignature == 'RTSS') && (pMem->dwVersion >= 0x00020000)) {

				//Check if RTSS detected or removed any programs....
				int count = 0;
				for (DWORD dwEntry = 0; dwEntry < pMem->dwAppArrSize; dwEntry++) {
					RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY)((LPBYTE)pMem + pMem->dwAppArrOffset + dwEntry * pMem->dwAppEntrySize);
					if (strlen(pEntry->szName) && pEntry->dwProcessID != 0) {
						count++;
					}
					else if(strlen(pEntry->szName) == 0){
						break;
					}
				}

				//If so, add them to the map we'll use to populate the menu
				if (dwAppEntries != count) {
					dwAppEntries = 0;
					m_vszAppArr.clear();
					
					for (DWORD dwEntry = 0; dwEntry < pMem->dwAppArrSize; dwEntry++)
					{
						RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_APP_ENTRY)((LPBYTE)pMem + pMem->dwAppArrOffset + dwEntry * pMem->dwAppEntrySize);
						DEBUG_PRINT("PID: " + to_string(pEntry->dwProcessID) + " Name: " + pEntry->szName)
						if (strlen(pEntry->szName) && pEntry->dwProcessID != 0) {
							
							dwAppEntries++;
							string entryName = pEntry->szName;
							SL::RemoveExtension(entryName);
							SL::RemovePath(entryName);

							m_vszAppArr.push_back(std::pair(pEntry->dwProcessID, entryName));
						}
						else if (strlen(pEntry->szName) == 0) {
							break;
						}
					}
					
					for (auto const& [pid, pName] : m_vszAppArr) {
						DEBUG_PRINT("PID: " + to_string(pid) + " Name: " + pName)
					}
					
				}
			}
			//else return version error?
			UnmapViewOfFile(pMapAddr);
		}

		CloseHandle(hMapFile);
	}

	return dwAppEntries;
}

BOOL CRTSSClient::UpdateOSD(LPCSTR lpText) {
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
					DWORD dwEntry;
					if (clientPriority == 0) {
						dwEntry = 0;
					}
					else {
						dwEntry = 1;
					}
					for (dwEntry; dwEntry < pMem->dwOSDArrSize; dwEntry++)
						//allow primary OSD clients (e.g. EVGA Precision / MSI Afterburner) to use the first slot exclusively, so third party 
						//applications start scanning the slots from the second one - CHANGED THIS TO 0 SO I CAN BE PRIMARY BECAUSE I NEED THE CORNERS
					{
						RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);

						if (dwPass)
						{
							if (!strlen(pEntry->szOSDOwner))
								strcpy_s(pEntry->szOSDOwner, sizeof(pEntry->szOSDOwner), slotOwnerOSD);
						}

						//remember that strcmp returns 0 if the strings match... so the following if statement basically says if the strings match 
						if (!strcmp(pEntry->szOSDOwner, slotOwnerOSD))
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

							ownedSlot = dwEntry;
							
							//I really don't like how I'm storing this... I feel like I should be using a different var
							clientsNum = pMem->dwOSDArrSize;


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

void CRTSSClient::ReleaseOSD()
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
				for (DWORD dwEntry = 0; dwEntry < pMem->dwOSDArrSize; dwEntry++)
				{
					RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY pEntry = (RTSS_SHARED_MEMORY::LPRTSS_SHARED_MEMORY_OSD_ENTRY)((LPBYTE)pMem + pMem->dwOSDArrOffset + dwEntry * pMem->dwOSDEntrySize);
					if (!strcmp(pEntry->szOSDOwner, slotOwnerOSD))
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

DWORD CRTSSClient::EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags)
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

void CRTSSClient::GetOSDText(CGroupedString& osd, BOOL bFormatTagsSupported, BOOL bObjTagsSupported)
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

