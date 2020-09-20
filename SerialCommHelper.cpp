// RCSerial.cpp: implementation of the CSerialCommHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "SerialCommHelper.h"
#include <Process.h>
#include <string>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CDebugDump	m_DebugFile;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define ATLTRACE6 m_DebugFile.Dump

void CSerialCommHelper::InvalidateHandle(HANDLE& hHandle)
{
	hHandle = INVALID_HANDLE_VALUE;
}
void CSerialCommHelper::CloseAndCleanHandle(HANDLE& hHandle)
{

	BOOL abRet = CloseHandle(hHandle);
	if (!abRet)
	{
		ASSERT(0);
		ATLTRACE6("CSerialCommHelper : Failed to open Close Handle %d :Last Error: %d", hHandle, GetLastError());
	}
	InvalidateHandle(hHandle);

}
CSerialCommHelper::CSerialCommHelper()
{

	InvalidateHandle(m_hThreadTerm);
	InvalidateHandle(m_hThread);
	InvalidateHandle(m_hThreadStarted);
	InvalidateHandle(m_hCommPort);
	InvalidateHandle(m_hDataRx);

	InitLock();
	m_eState = SS_UnInit;

	m_DebugFile.Init(_T("c:\\Temp\\"), _T("SerialIO"), _T("RCSERIAL:"));
}
CSerialCommHelper::~CSerialCommHelper()
{

	m_eState = SS_Unknown;
	DelLock();
}
HRESULT CSerialCommHelper::Init(std::string szPortName, DWORD dwBaudRate, BYTE byParity, BYTE byStopBits, BYTE byByteSize)
{

	HRESULT hr = S_OK;
	try
	{


		m_hDataRx = CreateEvent(0, 0, 0, 0);

		//open the COM Port
		m_hCommPort = ::CreateFile(szPortName.c_str(),
			GENERIC_READ | GENERIC_WRITE,//access ( read and write)
			0,	//(share) 0:cannot share the COM port						
			0,	//security  (None)				
			OPEN_EXISTING,// creation : open_existing
			FILE_FLAG_OVERLAPPED,// we want overlapped operation
			0// no templates file for COM port...
		);
		/*
		m_hCommPort = ::CreateFile(
			szPortName.c_str(),
			GENERIC_READ | GENERIC_WRITE,//access ( read and write)... MINE ONLY USES READ OR WRITE, BUT I'M ALSO OPENING A NEW CONNECTION(??) EVERY LOOP
			0,	//(share) 0:cannot share the COM port
			0,	//security  (None)
			OPEN_EXISTING,// creation : open_existing
			0,// we want overlapped operation ... NO WE DON'T... I DON'T THINK
			0// no templates file for COM port...
		);
		*/

		if (m_hCommPort == INVALID_HANDLE_VALUE)
		{
			ATLTRACE6("CSerialCommHelper : Failed to open COM Port Reason: %d", GetLastError());
			ASSERT(0);
			return E_FAIL;
		}


		ATLTRACE6("CSerialCommHelper : COM port opened successfully");

		//now start to read but first we need to set the COM port settings and the timeouts
		if (!::SetCommMask(m_hCommPort, EV_RXCHAR | EV_TXEMPTY))
		//if (!::SetCommMask(m_hCommPort, EV_RXCHAR | EV_ERR))
		{
			ASSERT(0);
			ATLTRACE6("CSerialCommHelper : Failed to Set Comm Mask Reason: %d", GetLastError());
			return E_FAIL;
		}
		ATLTRACE6("CSerialCommHelper : SetCommMask() success");

		//now we need to set baud rate etc,
		DCB dcb = { 0 };

		dcb.DCBlength = sizeof(DCB);

		if (!::GetCommState(m_hCommPort, &dcb))
		{
			ATLTRACE6("CSerialCommHelper : Failed to Get Comm State Reason: %d", GetLastError());
			return E_FAIL;
		}

		dcb.BaudRate = dwBaudRate;
		dcb.ByteSize = byByteSize;
		dcb.Parity = byParity;
		if (byStopBits == 1)
			dcb.StopBits = ONESTOPBIT;
		else if (byStopBits == 2)
			dcb.StopBits = TWOSTOPBITS;
		else
			dcb.StopBits = ONE5STOPBITS;

		dcb.fDsrSensitivity = 0;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fOutxDsrFlow = 0;


		if (!::SetCommState(m_hCommPort, &dcb))
		{
			ASSERT(0);
			ATLTRACE6("CSerialCommHelper : Failed to Set Comm State Reason: %d", GetLastError());
			return E_FAIL;
		}
		ATLTRACE6("CSerialCommHelper : Current Settings, (Baud Rate %d; Parity %d; Byte Size %d; Stop Bits %d", dcb.BaudRate,
			dcb.Parity, dcb.ByteSize, dcb.StopBits);



		//now set the timeouts ( we control the timeout overselves using WaitForXXX()
		COMMTIMEOUTS timeouts;

		timeouts.ReadIntervalTimeout = MAXDWORD;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 0;

		if (!SetCommTimeouts(m_hCommPort, &timeouts))
		{
			ASSERT(0);
			ATLTRACE6("CSerialCommHelper :  Error setting time-outs. %d", GetLastError());
			return E_FAIL;
		}
		//create thread terminator event...
		m_hThreadTerm = CreateEvent(0, 0, 0, 0);
		m_hThreadStarted = CreateEvent(0, 0, 0, 0);

		m_hThread = (HANDLE)_beginthreadex(0, 0, CSerialCommHelper::ThreadFn, (void*)this, 0, 0);

		DWORD dwWait = WaitForSingleObject(m_hThreadStarted, INFINITE);

		ASSERT(dwWait == WAIT_OBJECT_0);

		CloseHandle(m_hThreadStarted);

		InvalidateHandle(m_hThreadStarted);
		m_abIsConnected = true;

	}
	catch (...)
	{
		ASSERT(0);
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		m_eState = SS_Init;
	}
	return hr;

}
HRESULT CSerialCommHelper::Start()
{

	m_eState = SS_Started;
	return S_OK;

}
HRESULT CSerialCommHelper::Stop()
{


	m_eState = SS_Stopped;
	return S_OK;
}
HRESULT CSerialCommHelper::UnInit()
{
	HRESULT hr = S_OK;
	try
	{
		m_abIsConnected = false;
		SignalObjectAndWait(m_hThreadTerm, m_hThread, INFINITE, FALSE);
		CloseAndCleanHandle(m_hThreadTerm);
		CloseAndCleanHandle(m_hThread);
		CloseAndCleanHandle(m_hDataRx);
		CloseAndCleanHandle(m_hCommPort);
	}
	catch (...)
	{
		ASSERT(0);
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr))
		m_eState = SS_UnInit;
	return hr;
}
unsigned __stdcall CSerialCommHelper::ThreadFn(void* pvParam)
{

	CSerialCommHelper* apThis = (CSerialCommHelper*)pvParam;
	bool abContinue = true;
	DWORD dwEventMask = 0;

	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = CreateEvent(0, true, 0, 0);
	HANDLE arHandles[1];
	arHandles[0] = apThis->m_hThreadTerm;

	DWORD dwWait;
	SetEvent(apThis->m_hThreadStarted);
	while (abContinue)
	{

		BOOL abRet = ::WaitCommEvent(apThis->m_hCommPort, &dwEventMask, &ov);
		//BOOL abRet = ::WaitCommEvent(apThis->m_hCommPort, &dwEventMask, 0);
		if (!abRet)
		{
			ASSERT(GetLastError() == ERROR_IO_PENDING);
		}


		arHandles[1] = ov.hEvent;

		dwWait = WaitForMultipleObjects(2, arHandles, FALSE, INFINITE);
		switch (dwWait)
		{
		case WAIT_OBJECT_0:
		{
			_endthreadex(1);
		}
		break;
		case WAIT_OBJECT_0 + 1:
		{

		    
			DWORD dwMask;
			
			if (GetCommMask(apThis->m_hCommPort, &dwMask))
			{
				if (dwMask == EV_TXEMPTY)
				{
					AfxMessageBox("Data sent");
					ResetEvent(ov.hEvent);
					continue;
				}

			}
			

			//read data here...
			int iAccum = 0;

			apThis->m_theSerialBuffer.LockBuffer();

			try
			{
				std::string szDebug;
				BOOL abRet = false;

				DWORD dwBytesRead = 0;
				OVERLAPPED ovRead;
				memset(&ovRead, 0, sizeof(ovRead));
				ovRead.hEvent = CreateEvent(0, true, 0, 0);

				do
				{
					ResetEvent(ovRead.hEvent);
					char szTmp[1];
					int iSize = sizeof(szTmp);
					memset(szTmp, 0, sizeof szTmp);
					abRet = ::ReadFile(apThis->m_hCommPort, szTmp, sizeof(szTmp), &dwBytesRead, &ovRead);
					//abRet = ::ReadFile(apThis->m_hCommPort, szTmp, sizeof(szTmp), &dwBytesRead, 0);
					if (!abRet)
					{
						abContinue = FALSE;
						break;
					}
					if (dwBytesRead > 0)
					{
						apThis->m_theSerialBuffer.AddData(szTmp, dwBytesRead);
						iAccum += dwBytesRead;
					}
				} while (0);// dwBytesRead > 0 );
				CloseHandle(ovRead.hEvent);
			}
			catch (...)
			{
				ASSERT(0);
			}

			//if we are not in started state then we should flush the queue...( we would still read the data)
			if (apThis->GetCurrentState() != SS_Started)
			{
				iAccum = 0;
				apThis->m_theSerialBuffer.Flush();
			}

			apThis->m_theSerialBuffer.UnLockBuffer();

			ATLTRACE6(_T("RCSerial: Q Unlocked:"));
			if (iAccum > 0)
			{
				ATLTRACE6(_T("CSerialCommHelper(worker thread):  SetDataReadEvent() len:{%d} data:{%s}"), iAccum, (apThis->m_theSerialBuffer.GetData()).c_str());
				apThis->SetDataReadEvent();
			}
			ResetEvent(ov.hEvent);
		}
		break;
		}//switch
	}
	return 0;
}
HRESULT  CSerialCommHelper::CanProcess()
{

	switch (m_eState)
	{
	case SS_Unknown:ASSERT(0);return E_FAIL;
	case SS_UnInit:return E_FAIL;
	case SS_Started:return S_OK;
	case SS_Init:
	case SS_Stopped:
		return E_FAIL;
	default:ASSERT(0);

	}
	return E_FAIL;
}
HRESULT CSerialCommHelper::Write(const char* data, DWORD dwSize)
{
	HRESULT hr = CanProcess();
	if (FAILED(hr)) return hr;
	int iRet = 0;
	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = CreateEvent(0, true, 0, 0);
	DWORD dwBytesWritten = 0;
	//do
	{
		iRet = WriteFile(m_hCommPort, data, dwSize, &dwBytesWritten, &ov);
		if (iRet == 0)
		{
			WaitForSingleObject(ov.hEvent, INFINITE);
		}

	}//	while ( ov.InternalHigh != dwSize ) ;
	CloseHandle(ov.hEvent);
	std::string szData(data);
	ATLTRACE6(_T("RCSerial:Writing:{%s} len:{%d}"), (szData).c_str(), szData.size());

	return S_OK;
}
HRESULT CSerialCommHelper::Read_Upto(std::string& data, char chTerminator, long* alCount, long alTimeOut)
{
	HRESULT hr = CanProcess();
	if (FAILED(hr)) return hr;

	ATLTRACE6("CSerialCommHelper : CSerialCommHelper: Read_Upto called  ");
	try
	{

		std::string szTmp;
		szTmp.erase();
		long alBytesRead;

		bool abFound = m_theSerialBuffer.Read_Upto(szTmp, chTerminator, alBytesRead, m_hDataRx);

		if (abFound)
		{
			data = szTmp;
		}
		else
		{//there are either none or less bytes...
			long iRead = 0;
			bool abContinue = true;
			while (abContinue)
			{
				ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_Upto () making blocking read call  ");
				DWORD dwWait = ::WaitForSingleObject(m_hDataRx, alTimeOut);

				if (dwWait == WAIT_TIMEOUT)
				{
					ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_Upto () timed out in blocking read");
					data.erase();
					hr = E_FAIL;
					return hr;

				}

				bool abFound = m_theSerialBuffer.Read_Upto(szTmp, chTerminator, alBytesRead, m_hDataRx);


				if (abFound)
				{
					data = szTmp;
					ATLTRACE6("CSerialCommHelper : CSerialCommHelper: Read_Upto WaitForSingleObject  data:{%s}len:{%d}", ((szTmp)).c_str(), szTmp.size());
					return S_OK;
				}
				ATLTRACE6("CSerialCommHelper : CSerialCommHelper: Read_Upto WaitForSingleObject  not FOUND ");

			}
		}
	}
	catch (...)
	{
		ATLTRACE6(_T("CSerialCommHelperUnhandled exception"));
		ASSERT(0);
	}
	return hr;

}
HRESULT CSerialCommHelper::Read_N(std::string& data, long alCount, long  alTimeOut)
{
	HRESULT hr = CanProcess();

	if (FAILED(hr))
	{
		ASSERT(0);
		return hr;
	}

	ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_N called for %d bytes", alCount);
	try
	{

		std::string szTmp;
		szTmp.erase();

		ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_N (%d) locking the queue  ", alCount);

		int iLocal = m_theSerialBuffer.Read_N(szTmp, alCount, m_hDataRx);

		if (iLocal == alCount)
		{
			data = szTmp;
		}
		else
		{//there are either none or less bytes...
			long iRead = 0;
			int iRemaining = alCount - iLocal;
			while (1)
			{

				ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_N (%d) making blocking read() ", alCount);

				DWORD dwWait = WaitForSingleObject(m_hDataRx, alTimeOut);

				if (dwWait == WAIT_TIMEOUT)
				{
					ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_N (%d) timed out in blocking read", alCount);
					data.erase();
					hr = E_FAIL;
					return hr;

				}

				ASSERT(dwWait == WAIT_OBJECT_0);
				ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_N (%d) Woke Up from WaitXXX() locking Q", alCount);


				iRead = m_theSerialBuffer.Read_N(szTmp, iRemaining, m_hDataRx);
				iRemaining -= iRead;

				ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_N (%d) Woke Up from WaitXXX() Unlocking Q", alCount);


				if (iRemaining == 0)
				{
					ATLTRACE6("CSerialCommHelper : CSerialCommHelper : Read_N (%d) Woke Up from WaitXXX() Done reading ", alCount);
					data = szTmp;
					return S_OK;
				}
			}
		}
	}
	catch (...)
	{
		ATLTRACE6(_T("CSerialCommHelper Unhandled exception"));
		ASSERT(0);
	}
	return hr;

}
/*-----------------------------------------------------------------------
	-- Reads all the data that is available in the local buffer..
	does NOT make any blocking calls in case the local buffer is empty
-----------------------------------------------------------------------*/
HRESULT CSerialCommHelper::ReadAvailable(std::string& data)
{

	HRESULT hr = CanProcess();
	if (FAILED(hr)) return hr;
	try
	{
		std::string szTemp;
		bool abRet = m_theSerialBuffer.Read_Available(szTemp, m_hDataRx);

		data = szTemp;
	}
	catch (...)
	{
		ATLTRACE6(_T("CSerialCommHelper Unhandled exception in ReadAvailable()"));
		ASSERT(0);
		hr = E_FAIL;
	}
	return hr;

}