#include "stdafx.h"
#include "USBController.h"
HANDLE CUSBController::OpenComPort(const CString& PortSpecifier)
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
void CUSBController::CloseComPort(HANDLE hPort)
{
	PurgeComm(hPort, PURGE_RXCLEAR);    // it is not clear whether the purge is needed on each read of the byte, or only when we need to close the port
	CloseHandle(hPort);
}
bool CUSBController::IsComPortOpened(HANDLE hPort)
{
	return hPort != INVALID_HANDLE_VALUE;
}
int CUSBController::ReadByte(HANDLE hPort)
{
	int retVal;

	BYTE Byte;
	DWORD dwBytesTransferred;
	if (FALSE == ReadFile(hPort, &Byte, 1, &dwBytesTransferred, 0)) //read 1
		retVal = 0x101;
	retVal = Byte;

	return retVal;
}
