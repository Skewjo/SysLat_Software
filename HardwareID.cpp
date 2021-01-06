#include "stdafx.h"
#include "HardwareID.h"

void HardwareID::GetUserAndComputerName() {
    userNameSize = UNLEN + 1;
    GetUserName(userName, &userNameSize);
    DEBUG_PRINT(userName)

    computerNameSize = UNLEN + 1;
    GetComputerName(computerName, &computerNameSize);
    DEBUG_PRINT(computerName)
}

char* HardwareID::GetMAC() {
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
    char* mac_addr = (char*)malloc(18);

    AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (AdapterInfo == NULL) {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        free(mac_addr);
        return NULL; // it is safe to call free(NULL)
    }

    // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
        if (AdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            free(mac_addr);
            return NULL;
        }
    }

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        // Contains pointer to current adapter info
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        do {
            // technically should look at pAdapterInfo->AddressLength
            //   and not assume it is 6.
            sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
                pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
            printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
            // print them all, return the last one.
            // return mac_addr;

            printf("\n");
            pAdapterInfo = pAdapterInfo->Next;
        } while (pAdapterInfo);
    }
    free(AdapterInfo);
    return mac_addr; // caller must free.
}


//incomplete
void HardwareID::GetMachineSID() {
    //m_pSID
    //LookupAccountName(computerName, m_pSID, computerName, computerNameSize, );
    LookupAccountSid(computerName, m_pSID, userName, &dwSize, lpDomain, &dwSize, &m_SIDType);
    //ConvertSidToStringSid();

}

void HardwareID::CreateJSON() {
    //BenchmarkDatasets: [BenchmarkDataset!] @relation(name: "dataSetOwnerHWID")
    //UserMachineInfo: UserMachineInfo @relation(name: "MI_HWID")
    HardwareIDJSON["UserMachineID"]["ComputerName"] = computerName;
    HardwareIDJSON["UserMachineID"]["ComputerUserName"] = userName;
    HardwareIDJSON["UserMachineID"]["HardwareID"] = hwProfileInfo.szHwProfileGuid;
    HardwareIDJSON["UserMachineID"]["MACAddress"] = m_pMac;
    //IPAddress : String // THIS WILL NEED TO BE ADDED BY THE WEBSITE, I THINK?? IDK
    //MachineSID : String
    //UserProfileName : String

}

void HardwareID::ExportData(string path) {
    std::ofstream exportData;
    exportData.open(path + "\\HardwareID.json");

    if (exportData.is_open()) {
        exportData << HardwareIDJSON;
        dataExported = true;
    }
    else {
        DEBUG_PRINT("\nError exporting hardwareID file.\n")
    }

    exportData.close();
}
