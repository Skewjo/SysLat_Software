#pragma once
#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "user32.lib")
#include<json/json.h>

class MachineInfo
{
    bool dataExported = false;

    SYSTEM_INFO siSysInfo;
    
    std::string OEMID;
    std::string OSName = "Windows ";

    std::string CPU;
    std::string Videocard;
    std::string Drive;
    std::string Memory;
    std::string Motherboard;



    void    SetOSInfo();
    void    SetCPUInfo();
    void    SetVideoCardInfo();
    //std::string Drive;
    //std::string Memory;
    void    SetMOBOInfo();

    //THIS FUNCTION IS UNUSED ATM - IT'S KIND OF WORTHLESS
    void    SetMachineInfo();

    void    CreateJSON();

    //The following method can be used to retrieve any string value from the registry - will be useful for things other than just grabbing the MOBO name.
    std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue);

public:
    MachineInfo() {
        SetOSInfo();
        SetCPUInfo();
        SetVideoCardInfo();
        SetMOBOInfo();

        CreateJSON();
    }
    void            ExportData();

    Json::Value     MachineInfoJSON;
};

