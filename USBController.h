#pragma once
#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H
class CUSBController
{
	/*
protected:
	
	//These vars aren't actually in use anywhere yet...
	const CString& PortSpecifier;
	DWORD BaudRate = CBR_9600; //9600 Baud
	/*other Baud options:
	CBR_110             110
	CBR_300             300
	CBR_600             600
	CBR_1200            1200
	CBR_2400            2400
	CBR_4800            4800
	CBR_9600            9600
	CBR_14400           14400
	CBR_19200           19200
	CBR_38400           38400
	CBR_56000           56000
	CBR_57600           57600
	CBR_115200          115200
	CBR_128000          128000
	CBR_256000          256000
	
	BYTE ByteSize = 8; //8 data bits
	BYTE Parity = NOPARITY; //no parity
	BYTE StopBits = ONESTOPBIT; //1 stop
	*/
public:
	HANDLE				OpenComPort(const CString& PortSpecifier);
	void				CloseComPort(HANDLE hPort);
	bool				IsComPortOpened(HANDLE hPort);
	int					ReadByte(HANDLE port);
};
#endif
