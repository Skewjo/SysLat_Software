#pragma once
#ifndef RTSSCLIENT_H
#define RTSSCLIENT_H
#include "GroupedString.h"
#include "RTSSProfileInterface.h"

//RTSS Client related functions
//Non-static members are basically object identifiers while static members are objects linking to either the RTSS shared memory or the RTSS profile interface
class CRTSSClient
{
protected:
	const char*						slotOwnerOSD; //if this is const... it can still be modified once by the constructor, right?
	int								clientPriority = 1;//should this be initialized here... or in the constructor? Is this considered a default value?
	//static DWORD clientsNum; //this could store stale data, so we want to fetch the number of clients every time we need it
	static DWORD					sharedMemoryVersion;
	//DWORD						GetRTSSSimpleProperty(); //I think I need to figure out how to pass a function pointer or something to make this work...

public:
	
	CRTSSClient(const char* setSlotOwner = "RTSSSharedMemorySample", int setClientPriority = 1);
	static void						InitRTSSInterface();
	static CRTSSProfileInterface	m_profileInterface;
	static CString					m_strInstallPath;
	static void						IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement);
	static void						SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty);


	DWORD							EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags);
	//The following 4 functions could be (kind-of) combined into one "GetRTSSAttribute" function and then these accessor functions could be one line long
	//These should probably also be static because there is only going to be one setting for either of the two clients I'm going to have in this program
	static DWORD					GetClientsNum();
	static DWORD					GetSharedMemoryVersion();
	static DWORD					GetLastForegroundApp();
	static DWORD					GetLastForegroundAppID();
	BOOL							UpdateOSD(LPCSTR lpText);
	void							ReleaseOSD();



	//I think the following function will be unnecessary
	void						GetOSDText(CGroupedString& osd, BOOL bFormatTagsSupported, BOOL bObjTagsSupported);


	//~CRTSSClient();
};
#endif