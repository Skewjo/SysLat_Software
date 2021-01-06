#pragma once
#ifndef USBCONTROLLER_H
#define USBCONTROLLER_H



struct SSerInfo {
	SSerInfo() : bUsbDevice(FALSE) {}
	CString strDevPath;          // Device path for use with CreateFile()
	CString strPortName;         // Simple name (i.e. COM1)
	CString strFriendlyName;     // Full name to be displayed to a user
	BOOL bUsbDevice;             // Provided through a USB connection?
	CString strPortDesc;         // friendly name without the COMx
};


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
	
	//---------------------------------------------------------------
	// Helpers for enumerating the available serial ports.
	// These throw a CString on failure, describing the nature of
	// the error that occurred.

	void				EnumPortsWdm(CArray<SSerInfo, SSerInfo&>& asi);
	void				EnumPortsWNt4(CArray<SSerInfo, SSerInfo&>& asi);
	void				EnumPortsW9x(CArray<SSerInfo, SSerInfo&>& asi);
	void				SearchPnpKeyW9x(HKEY hkPnp, BOOL bUsbDevice, CArray<SSerInfo, SSerInfo&>& asi);

public:
	
	HANDLE				OpenComPort(const CString& PortSpecifier);
	void				CloseComPort(HANDLE hPort);
	bool				IsComPortOpened(HANDLE hPort);
	int					ReadByte(HANDLE port);

	//1-6-21
	//This only half works on my system, so I also have to use "_WINBASE_::GetCommPorts()", but it does give me the "friendly name", which is what I really need.
	void				EnumSerialPorts(CArray<SSerInfo, SSerInfo&>& asi, BOOL bIgnoreBusyPorts);
	
	
};
#endif




	

	