#pragma once
#include <json/json.h>

// 12-28-2020
// The default values in the following 4 structures get overwritten when ReadPreferences() is called
struct SysLatOptions {
	std::string					m_PortSpecifier = "COM3";
	time_t						m_maxTestDuration = 0;
	int							m_maxLogs = 10;
	std::string					m_LogDir = ".\\";
	bool						m_bDarkMode = false;
};

struct PrivacyOptions {
	bool						m_bAutoCheckUpdates = true;
	bool						m_bAutoExportLogs = true;
	bool						m_bAutoUploadLogs = true;
	bool						m_bAutoUploadMachineID = true;
	bool						m_bAutoUploadMachineInfo = true;
};

struct DebugOptions {
	bool						m_bDebugMode = false; //save to config
	bool						m_bTestUploadMode = false; //change name?
	bool						m_bSysLatInOSD = false;
};

struct RTSSOptions {
	// text color
	// box colors??
	DWORD						m_positionX = 0;
	DWORD						m_positionY = 0;
	bool						m_bPositionManualOverride = false;
	int							m_internalX = 0;
	int							m_internalY = 0;
};

class SysLatPreferences
{

	//TestCtrl::TestCtrl(std::vector<CSysLatData*>* p_previousSLD, CWnd* pParent /*=nullptr*/)
	//	: CDialogEx(IDD_TESTCTRL_DIALOG, pParent), m_pPreviousSLD(p_previousSLD)
	//{
	//}
	//HardwareID() {
	//	GetUserAndComputerName();
	//	::GetCurrentHwProfileA(&hwProfileInfo);
	//	m_pMac = GetMAC();
	//	GetMachineSID();
	//	CreateJSON();
	//}
	Json::Value m_JSONPreferences;

	void						WriteSysLatOptions();
	void						WritePrivacyOptions();
	void						WriteDebugOptions();
	void						WriteRTSSOptions();

	void						ReadSysLatOptions();
	void						ReadPrivacyOptions();
	void						ReadDebugOptions();
	void						ReadRTSSOptions();

public:
	SysLatPreferences() {
		ReadPreferences();
	}

	//The use of "./" in filepaths in the following 2 functions makes the program create the file where it is run from... not where the actual executable is located...
	void						WritePreferences();
	void						ReadPreferences();

	SysLatOptions				m_SysLatOptions;
	PrivacyOptions				m_PrivacyOptions;
	DebugOptions				m_DebugOptions;
	RTSSOptions					m_RTSSOptions;
};

