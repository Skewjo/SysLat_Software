// SerialBuffer.cpp: implementation of the CSerialBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerialBuffer.h"
#include "SerialCommHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

USE_DEBUGDUMP

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerialBuffer::CSerialBuffer()
{
	Init();
}
void CSerialBuffer::Init()
{
	::InitializeCriticalSection(&m_csLock);
	m_abLockAlways = true;
	m_iCurPos = 0;
	m_alBytesUnRead = 0;
	m_szInternalBuffer.erase();

}
CSerialBuffer::~CSerialBuffer()
{
	::DeleteCriticalSection(&m_csLock);

}
void CSerialBuffer::AddData(char ch)
{
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) AddData(char) called "), GetCurrentThreadId());
	m_szInternalBuffer += ch;
	m_alBytesUnRead += 1;

}
void CSerialBuffer::AddData(std::string& szData, int iLen)
{
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) AddData(%s,%d) called "), GetCurrentThreadId(), szData.c_str(), iLen);
	m_szInternalBuffer.append(szData.c_str(), iLen);
	m_alBytesUnRead += iLen;
}
void CSerialBuffer::AddData(char* strData, int iLen)
{
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) AddData(char*,%d) called "), GetCurrentThreadId(), iLen);
	ASSERT(strData != NULL);
	m_szInternalBuffer.append(strData, iLen);
	m_alBytesUnRead += iLen;

}
void CSerialBuffer::AddData(std::string& szData)
{
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) AddData(%s) called "), GetCurrentThreadId(), szData.c_str());
	m_szInternalBuffer += szData;
	m_alBytesUnRead += szData.size();
}
void CSerialBuffer::Flush()
{
	LockBuffer();
	m_szInternalBuffer.erase();
	m_alBytesUnRead = 0;
	m_iCurPos = 0;
	UnLockBuffer();
}
long CSerialBuffer::Read_N(std::string& szData, long  alCount, HANDLE& hEventToReset)
{
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) Read_N() called "), GetCurrentThreadId());
	ASSERT(hEventToReset != INVALID_HANDLE_VALUE);


	LockBuffer();
	long alTempCount = min(alCount, m_alBytesUnRead);
	long alActualSize = GetSize();

	szData.append(m_szInternalBuffer, m_iCurPos, alTempCount);

	m_iCurPos += alTempCount;

	m_alBytesUnRead -= alTempCount;
	if (m_alBytesUnRead == 0)
	{
		ClearAndReset(hEventToReset);
	}

	UnLockBuffer();
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) Read_N returned %d bytes (data:%s)  "), GetCurrentThreadId(), alTempCount, ((szData)).c_str());
	return alTempCount;
}
bool CSerialBuffer::Read_Available(std::string& szData, HANDLE& hEventToReset)
{
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) Read_Upto called "), GetCurrentThreadId());

	LockBuffer();
	szData += m_szInternalBuffer;

	ClearAndReset(hEventToReset);

	UnLockBuffer();

	ATLTRACE6(_T("CSerialBuffer : (tid:%d) Read_Available returned (data:%s)  "), GetCurrentThreadId(), ((szData)).c_str());
	return (szData.size() > 0);
}
void CSerialBuffer::ClearAndReset(HANDLE& hEventToReset)
{
	m_szInternalBuffer.erase();
	m_alBytesUnRead = 0;
	m_iCurPos = 0;
	::ResetEvent(hEventToReset);

}
bool CSerialBuffer::Read_Upto(std::string& szData, char chTerm, long& alBytesRead, HANDLE& hEventToReset)
{
	return Read_Upto_FIX(szData, chTerm, alBytesRead, hEventToReset);

	ATLTRACE6(_T("CSerialBuffer : (tid:%d) Read_Upto called "), GetCurrentThreadId());

	LockBuffer();

	alBytesRead = 0;
	bool abFound = false;
	if (m_alBytesUnRead > 0)
	{//if there are some bytes un-read...

		int iActualSize = GetSize();


		for (int i = m_iCurPos; i < iActualSize; ++i)
		{
			alBytesRead++;
			szData.append(m_szInternalBuffer, i, 1);
			m_alBytesUnRead -= 1;
			if (m_szInternalBuffer[i] == chTerm)
			{
				abFound = true;
				break;
			}
		}
		if (m_alBytesUnRead == 0)
		{
			ClearAndReset(hEventToReset);
		}
		else
		{
			//if we are here it means that there is still some data in the local buffer and
			//we have already found what we want... maybe this is ok but we want to catch this
			//scenario --- fix is in TCP/ip SocketBuffer.
			ASSERT(0);
		}
	}

	UnLockBuffer();
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) Read_Upto returned %d bytes (data:%s)  "), GetCurrentThreadId(), alBytesRead, ((szData)).c_str());
	return abFound;
}
bool CSerialBuffer::Read_Upto_FIX(std::string& szData, char chTerm, long& alBytesRead, HANDLE& hEventToReset)
{
	ATLTRACE6(_T("CSerialBuffer : (tid:%d) Read_Upto called "), GetCurrentThreadId());

	LockBuffer();
	alBytesRead = 0;


	bool abFound = false;
	if (m_alBytesUnRead > 0)
	{//if there are some bytes un-read...

		int iActualSize = GetSize();
		int iIncrementPos = 0;
		for (int i = m_iCurPos; i < iActualSize; ++i)
		{
			//szData .append ( m_szInternalBuffer,i,1);
			szData += m_szInternalBuffer[i];
			m_alBytesUnRead -= 1;
			if (m_szInternalBuffer[i] == chTerm)
			{
				iIncrementPos++;
				abFound = true;
				break;
			}
			iIncrementPos++;
		}
		m_iCurPos += iIncrementPos;
		if (m_alBytesUnRead == 0)
		{
			ClearAndReset(hEventToReset);
		}
	}
	UnLockBuffer();
	return abFound;
}