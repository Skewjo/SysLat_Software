#pragma once
#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#include<json/json.h>
#include<process.h>
#include<uuids.h>

#pragma comment(lib, "iphlpapi.lib")

#define MAX_NAME 256

class HardwareID
{
    bool dataExported = false;
    //Need to pull this out and put it in it's own hardware retrieval class
    //LPHW_PROFILE_INFOA lpHwProfileInfo = { 0 }; // why did this work??

    char                userName[UNLEN + 1] = { 0 };
    DWORD               userNameSize;
    char                computerName[UNLEN + 1] = { 0 };
    DWORD               computerNameSize;
    HW_PROFILE_INFOA	hwProfileInfo{ 0 };
    char*               m_pMac;

    //all of the following are used for the SID
    PSID                m_pSID = NULL;
    SID_NAME_USE        m_SIDType;
    char                lpDomain[MAX_NAME];
    DWORD i, dwSize = 0, dwResult = 0;

    
    
    
    void    GetUserAndComputerName();
    char*   GetMAC();
    void    GetMachineSID();
    void    CreateJSON();
    
    
public:
    //These 2 constructors are activated at the same time, right? This should fail or only call the first one I guess.
    //HardwareID() : m_pMac("") {};
    HardwareID() {
        GetUserAndComputerName();
        ::GetCurrentHwProfileA(&hwProfileInfo);
        m_pMac = GetMAC();
        GetMachineSID();
        CreateJSON();
    }
    ~HardwareID() {
        free(m_pMac);
    }
    void            ExportData(std::string path = ".\\SysLat_Logs");

    Json::Value     HardwareIDJSON;
};




