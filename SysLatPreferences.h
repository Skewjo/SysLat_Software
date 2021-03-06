#pragma once
// 12-28-2020
// The default values in the following 4 structures get overwritten when ReadPreferences() is called
struct SysLatOptions {
	string						m_targetApp = "dota2";
	//vector<string>			m_favoriteTargets;
	string						m_PortSpecifier = "COM3";
	int							m_maxTestDuration = 15; //does this need to be of type "duration<int>" ???
	int							m_maxLogs = 10;
	string						m_LogDir;// = ".\\SysLat_Logs\\";
	bool						m_bDarkMode = false;
};

struct PrivacyOptions {
	bool						m_bFirstRun = true;
	bool						m_bRunOnStartup = true;
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

	//TestCtrl::TestCtrl(vector<CSysLatData*>* p_previousSLD, CWnd* pParent /*=nullptr*/)
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
	Json::Value					m_JSONPreferences;

	CHAR						pathToSysLat[MAX_PATH];
	string						pathOnly;
	
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
		GetModuleFileName(NULL, pathToSysLat, MAX_PATH);
		pathOnly = pathToSysLat;
		SL::RemoveFileNameFromPath(pathOnly);
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

