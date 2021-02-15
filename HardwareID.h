#pragma once

#include "StdAfx.h"

#define MAX_NAME 256

class HardwareID
{
    bool m_bDataExported = false;
    //Need to pull this out and put it in it's own hardware retrieval class
    //LPHW_PROFILE_INFOA lpHwProfileInfo = { 0 }; // why did this work??

    char                userName[UNLEN + 1] = { 0 };
    DWORD               userNameSize;
    char                computerName[UNLEN + 1] = { 0 };
    DWORD               computerNameSize;
    HW_PROFILE_INFO 	hwProfileInfo{ 0 };
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
    //HardwareID() : m_pMac("") {};
    HardwareID() {
        GetUserAndComputerName();
        ::GetCurrentHwProfile(&hwProfileInfo);
        m_pMac = GetMAC();
        GetMachineSID();
        CreateJSON();
    }
    ~HardwareID() {
        free(m_pMac);
    }
    void            ExportData(string path = ".\\SysLat_Logs");

    Json::Value     HardwareIDJSON;
};
