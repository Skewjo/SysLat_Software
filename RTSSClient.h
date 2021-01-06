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
	int								clientPriority = 1;//should this be initialized here... or in the constructor? Is this considered a default value?
	const char*						slotOwnerOSD; //if this is const... it can still be modified once by the constructor, right?
	static DWORD					sharedMemoryVersion;
	//DWORD							GetRTSSSimpleProperty(); //I think I need to figure out how to pass a function pointer or something to make this work...

public:
	//I don't really like the following 2 members are public, but I don't feel like creating getters for them...
	DWORD							ownedSlot;
	static DWORD					clientsNum;
	static CRTSSProfileInterface	m_profileInterface;
	static CString					m_strInstallPath;
	static DWORD					dwAppEntries;	
	static vector< std::pair<DWORD, string>>	m_vszAppArr;
	
	CRTSSClient(const char* setSlotOwner = "RTSSSharedMemorySample", int setClientPriority = 1);
	static void						InitRTSSInterface();
	static DWORD					GetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty);
	static void						IncProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, LONG dwIncrement);
	static void						SetProfileProperty(LPCSTR lpProfile, LPCSTR lpProfileProperty, DWORD dwProperty);
	
	//The following 4 functions could be (kind-of) combined into one "GetRTSSAttribute"("GetRTSSSimpleProperty"?) function and then these accessor functions could be one line long
	//These should probably also be static because there is only going to be one setting for either of the two clients I'm going to have in this program
	static DWORD					GetClientsNum();
	static DWORD					GetSharedMemoryVersion();
	static DWORD					GetLastForegroundApp();
	static DWORD					GetLastForegroundAppID();
	static DWORD					GetAppArray();

	BOOL							UpdateOSD(LPCSTR lpText);
	void							ReleaseOSD();
	DWORD							EmbedGraph(DWORD dwOffset, FLOAT* lpBuffer, DWORD dwBufferPos, DWORD dwBufferSize, LONG dwWidth, LONG dwHeight, LONG dwMargin, FLOAT fltMin, FLOAT fltMax, DWORD dwFlags);

	//I think the following function will be unnecessary
	void							GetOSDText(CGroupedString& osd, BOOL bFormatTagsSupported, BOOL bObjTagsSupported);


	//~CRTSSClient();
};
#endif