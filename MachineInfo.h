#pragma once

class MachineInfo
{
    bool m_bDataExported = false;

    SYSTEM_INFO siSysInfo;
    
    string OEMID;
    string OSName = "Windows ";

    string CPU;
    string Videocard;
    string Drive;
    string Memory;
    string Motherboard;



    void    SetOSInfo();
    void    SetCPUInfo();
    void    SetVideoCardInfo();
    //string Drive;
    //string Memory;
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
    void            ExportData(string path = ".\\SysLat_Logs");

    Json::Value     MachineInfoJSON;
};

