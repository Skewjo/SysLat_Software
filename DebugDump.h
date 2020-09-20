// DebugDump.h: interface for the CDebugDump class.
#pragma once
#include <fstream>
#include <string>

#define GAP _T("     ")

class CDebugDump
{
private:
	char m_szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	std::ofstream m_fileDump;
	std::string m_szTitle;
	std::string m_szFileName;
	std::string m_szDir;

	BOOL m_bTraceToFile;
	BOOL m_bTraceToDebug;
public:
	void SetTraceToFile(BOOL bOnOff)
	{
		if (bOnOff == FALSE && m_bTraceToFile != bOnOff)
		{
			if (m_fileDump.is_open())
			{
				std::string szTimeStamp = GetTime();
				m_fileDump << _T("Stopped  log on computer: ") << m_szComputerName <<
					_T(" at ") << szTimeStamp.c_str() << std::endl;
				m_fileDump << _T("------------------------------------------------------------------------------------------------------") << std::endl << std::endl;
			}
		}
		m_bTraceToFile = bOnOff;
	};
	void SetTraceToDebug(BOOL bOnOff)
	{
		m_bTraceToDebug = bOnOff;
	};

	CDebugDump()
	{
		m_bTraceToFile = TRUE;
		m_bTraceToDebug = TRUE;
	}
	virtual ~CDebugDump()
	{
		if (m_fileDump.is_open())
		{
			std::string szTimeStamp = GetTime();			m_fileDump << std::endl << _T("Logging Ended : ")  _T(" at ") << szTimeStamp.c_str() << std::endl;
			m_fileDump << _T("------------------------------------------------------------------------------------------------------") << std::endl;m_fileDump.close();
		}
	};

	inline void	Init(std::string szDir = _T("c:\\Temp\\"), std::string szFile = _T("SERIALCOMM"), std::string szTitle = _T("SerialComm:"))
	{
		m_szDir = szDir;
		m_szFileName = szFile;
		m_szTitle = szTitle;
		std::string szFullPath(szDir);
		szFullPath.append(szFile);
		DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
		if (!GetComputerName(m_szComputerName, &dwSize))
			dwSize = GetLastError();

		if (!m_fileDump.is_open())
		{
			std::string szTimeStamp = GetTime();
			szFullPath.append(GetTime(false));
			szFullPath.append(".dmp");
			m_fileDump.open(szFullPath.c_str());

			m_fileDump << _T("Starting log on computer: ") << m_szComputerName <<
				_T(" at ") << szTimeStamp.c_str() << std::endl;
			m_fileDump << _T("------------------------------------------------------------------------------------------------------") << std::endl << std::endl;
		}
	}
	inline void Dump(LPCSTR lpszFormat, ...)
	{
		try
		{

			va_list args;
			va_start(args, lpszFormat);

			int nBuf;
			char szBuffer[8024];

			nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);

			ASSERT(nBuf < sizeof(szBuffer)); //Output truncated as it was > sizeof(szBuffer)

			std::string szTimeStamp = GetTime();

			char szThreadId[100];
			sprintf(szThreadId, "[%d] ", GetCurrentThreadId());


			if (m_bTraceToFile)
			{
				m_fileDump << szThreadId << m_szTitle << GAP << szTimeStamp.c_str() << GAP
					<< szBuffer << std::endl;
			}
			if (m_bTraceToDebug)
			{
				std::string szTemp(szThreadId);
				szTemp.append(m_szTitle);
				szTemp.append(szBuffer);
				szTemp.append("\n");
				TRACE(szTemp.c_str());
			}
			va_end(args);
		}
		catch (...)
		{
			ASSERT(0);
		}
	}



	inline std::string  GetTime(bool bLongTime = true)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		char szTime[100];
		if (bLongTime) sprintf(szTime, "%d/%d/%d  %d::%d::%d", tm.wMonth, tm.wDay, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond);
		else sprintf(szTime, "%d%d%d_%d_%d_%d", tm.wMonth, tm.wDay, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond);
		std::string sTime(szTime);
		return sTime;
	}
};