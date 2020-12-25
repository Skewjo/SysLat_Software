#pragma once
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

public:
	void						WritePreferences();
	void						ReadPreferences();

	SysLatPreferences() {
		ReadPreferences();
	}

//Members:
	typedef struct SysLatOptions {
		CString						m_PortSpecifier = "COM3";
		time_t						m_maxTestDuration;
		int							m_maxLogs = 10;
		CString						m_LogDir = ".//SysLat_Logs/";
		bool						m_bDarkMode = false;
	};

	typedef struct PrivacyOptions {
		bool						m_bAutoCheckUpdates = true;
		bool						m_bAutoExportLogs = true;
		bool						m_bAutoUploadLogs = true;
		bool						m_bAutoUploadMachineID = true;
		bool						m_bAutoUploadMachineInfo = true;
	};

	typedef struct DebugOptions{
		bool						m_bDebugMode = false; //save to config
		bool						m_bTestUploadMode = false; //change name?
		bool						m_bSysLatInOSD = false;
	};

	typedef struct RTSSOptions{
		// text color
		// box colors??
		DWORD						m_positionX = 0;
		DWORD						m_positionY = 0;
		bool						m_bPositionManualOverride = false;
		int							m_internalX = 0;
		int							m_internalY = 0;
	};
};

