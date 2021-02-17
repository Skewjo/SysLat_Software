#include "stdafx.h"
#include "MachineInfo.h"

using Microsoft::WRL::ComPtr;

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) noexcept : result(hr) {}

        const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}

void MachineInfo::SetOSInfo() {
    double osver = 0.0;

    NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);

    OSVERSIONINFOEXW osInfo;

    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

    if (NULL != RtlGetVersion)
    {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
        osver = osInfo.dwMajorVersion;
    }

    std::ostringstream stream;
    stream <<  osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion  << " Build: " << osInfo.dwBuildNumber;

    OSName.append(stream.str());
    
    DEBUG_PRINT("\n")
    DEBUG_PRINT("Major Version: " + osInfo.dwMajorVersion)
    DEBUG_PRINT("Minor Version: " + osInfo.dwMinorVersion)
    DEBUG_PRINT("Build Number: " + osInfo.dwBuildNumber)
    

}

void MachineInfo::SetCPUInfo() {
    // 4 is essentially hardcoded due to the __cpuid function requirements.
    // NOTE: Results are limited to whatever the sizeof(int) * 4 is...
    std::array<int, 4> integerBuffer = {};
    constexpr size_t sizeofIntegerBuffer = sizeof(int) * integerBuffer.size();

    std::array<char, 64> charBuffer = {};

    // The information you wanna query __cpuid for.
    // https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=vs-2019
    constexpr std::array<int, 3> functionIds = {
        // Manufacturer
        //  EX: "Intel(R) Core(TM"
        0x8000'0002,
        // Model
        //  EX: ") i7-8700K CPU @"
        0x8000'0003,
        // Clockspeed
        //  EX: " 3.70GHz"
        0x8000'0004
    };


    for (int id : functionIds)
    {
        __cpuid(integerBuffer.data(), id);
        std::memcpy(charBuffer.data(), integerBuffer.data(), sizeofIntegerBuffer);
        CPU += string(charBuffer.data());
    }

}

void MachineInfo::SetVideoCardInfo() {
    ComPtr<IDXGIFactory1> dxgiFactory;
    DX::ThrowIfFailed(
        CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf()))
    );

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0;
        SUCCEEDED(dxgiFactory->EnumAdapters1(
            adapterIndex,
            adapter.ReleaseAndGetAddressOf()));
        adapterIndex++)
    {
        DXGI_ADAPTER_DESC1 desc;
        DX::ThrowIfFailed(adapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }
        
        size_t i;
        const size_t BUFFER_SIZE = 256;
        char tempStdChar[BUFFER_SIZE];
        wcstombs_s(&i, tempStdChar, (size_t)BUFFER_SIZE, desc.Description, (size_t)BUFFER_SIZE);
        Videocard = tempStdChar;
        // desc.VendorId: VID
        // desc.DeviceId: PID
        // desc.Description: name string seen above
    }
}

void MachineInfo::SetMOBOInfo() {
    std::wstring regSubKey;
    #ifdef _WIN64 // Manually switching between 32bit/64bit for the example. Use dwFlags instead.
    regSubKey = L"HARDWARE\\WOW6432Node\\DESCRIPTION\\System\\BIOS\\";
    #else
    regSubKey = L"HARDWARE\\DESCRIPTION\\System\\BIOS\\";
    #endif
    std::wstring regValue(L"BaseBoardProduct");
    std::wstring valueFromRegistry;
    try
    {
        valueFromRegistry = GetStringValueFromHKLM(regSubKey, regValue);
    }
    catch (std::exception& e)
    {
        string errorString = e.what();
        DEBUG_PRINT(errorString)
    }
    #ifdef _DEBUG
    DEBUG_PRINT("Motherboard info: ")
    OutputDebugStringW(valueFromRegistry.c_str());
    OutputDebugString("\n");
    #endif
    
    size_t i;
    const int bufferSize = 256;
    char temp[bufferSize];

    //wcstombs(temp, desc.Description, bufferSize);
    //wcstombs_s(&i, tempStdChar, (size_t)BUFFER_SIZE, desc.Description, (size_t)BUFFER_SIZE);

    wcstombs_s(&i, temp, bufferSize, valueFromRegistry.c_str(), bufferSize);

    Motherboard = temp;
}

void MachineInfo::SetMachineInfo() {
    GetSystemInfo(&siSysInfo);

    std::ostringstream stream;
    stream << "OEM ID: " << siSysInfo.dwOemId;
    
    stream << "  Number of processors: " << siSysInfo.dwNumberOfProcessors << "\n";
    stream << "  Page size: " << siSysInfo.dwPageSize << "\n";
    stream << "  Processor type: " << siSysInfo.dwProcessorType << "\n";
    stream << "  Minimum application address: " << siSysInfo.lpMinimumApplicationAddress << "\n";
    stream << "  Maximum application address: " << siSysInfo.lpMaximumApplicationAddress << "\n";
    stream << "  Active processor mask: " << siSysInfo.dwActiveProcessorMask << "\n";

    DEBUG_PRINT(stream.str())

    //CPU.append(stream.str());
}

void MachineInfo::CreateJSON() {
    //UserMachineID: UserMachineID @relation(name: "MI_HWID")
    //BenchmarkDatasets: [BenchmarkDataset!] @relation(name: "dataSetOwnerMI")
    MachineInfoJSON["UserMachineInfo"]["OperatingSystem"] = OSName;
    MachineInfoJSON["UserMachineInfo"]["CPU"] = CPU;
    MachineInfoJSON["UserMachineInfo"]["Videocard"] = Videocard;
    //Drive : String
    //Memory : String
    MachineInfoJSON["UserMachineInfo"]["Motherboard"] = Motherboard;
    //DisplayMonitor : String

}

void MachineInfo::ExportData(string path) {
    std::ofstream exportData;
    exportData.open(path + "\\MachineInfo.json");

    if (exportData.is_open()) {
        exportData << MachineInfoJSON;
        m_bDataExported = true;
    }
    else {
        DEBUG_PRINT("Error exporting machineInfo file.")
    }

    exportData.close();
}

std::wstring MachineInfo::GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue)
{
    size_t bufferSize = 0xFFF; // If too small, will be resized down below.
    std::wstring valueBuf; // Contiguous buffer since C++11.
    valueBuf.resize(bufferSize);
    auto cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
    auto rc = RegGetValueW(
        HKEY_LOCAL_MACHINE,
        regSubKey.c_str(),
        regValue.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        (void*)(valueBuf.data()),
        &cbData
    );
    while (rc == ERROR_MORE_DATA)
    {
        // Get a buffer that is big enough.
        cbData /= sizeof(wchar_t);
        if (cbData > static_cast<DWORD>(bufferSize))
        {
            bufferSize = static_cast<size_t>(cbData);
        }
        else
        {
            bufferSize *= 2;
            cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
        }
        valueBuf.resize(bufferSize);
        rc = RegGetValueW(
            HKEY_LOCAL_MACHINE,
            regSubKey.c_str(),
            regValue.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            (void*)(valueBuf.data()),
            &cbData
        );
    }
    if (rc == ERROR_SUCCESS)
    {
        cbData /= sizeof(wchar_t);
        valueBuf.resize(static_cast<size_t>(cbData - 1)); // remove end null character
        return valueBuf;
    }
    else
    {
        throw std::runtime_error("Windows system error code: " + to_string(rc));
    }
}