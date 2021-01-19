#include "stdafx.h"
#include "SysLatPreferences.h"

void SysLatPreferences::WritePreferences() {
	WriteSysLatOptions();
	WritePrivacyOptions();
	WriteDebugOptions();
	WriteRTSSOptions();

	std::ofstream exportPreferences;
	exportPreferences.open(pathOnly + "\SysLatPreferences.json");

	if (exportPreferences.is_open()) {
		exportPreferences << m_JSONPreferences;
		//dataExported = true;
	}
	//else {
		//m_strError += "Unable to open file";
	//}

	exportPreferences.close();
}
void SysLatPreferences::WriteSysLatOptions() {
	m_JSONPreferences["SysLatOptions"]["LastSelectedTargetApp"] = m_SysLatOptions.m_targetApp;
	m_JSONPreferences["SysLatOptions"]["PortSpecifier"] = m_SysLatOptions.m_PortSpecifier;
	m_JSONPreferences["SysLatOptions"]["MaxTestDuration"] = m_SysLatOptions.m_maxTestDuration;
	m_JSONPreferences["SysLatOptions"]["MaxLogs"] = m_SysLatOptions.m_maxLogs;
	m_JSONPreferences["SysLatOptions"]["LogDir"] = m_SysLatOptions.m_LogDir;
	m_JSONPreferences["SysLatOptions"]["DarkMode"] = m_SysLatOptions.m_bDarkMode;
}
void SysLatPreferences::WritePrivacyOptions() {
	m_JSONPreferences["PrivacyOptions"]["FirstRun"] = m_PrivacyOptions.m_bFirstRun;
	m_JSONPreferences["PrivacyOptions"]["RunOnStartup"] = m_PrivacyOptions.m_bRunOnStartup;
	m_JSONPreferences["PrivacyOptions"]["AutoCheckUpdates"] = m_PrivacyOptions.m_bAutoCheckUpdates;
	m_JSONPreferences["PrivacyOptions"]["AutoExportLogs"] = m_PrivacyOptions.m_bAutoExportLogs;
	m_JSONPreferences["PrivacyOptions"]["AutoUploadLogs"] = m_PrivacyOptions.m_bAutoUploadLogs;
	m_JSONPreferences["PrivacyOptions"]["AutoUploadMachineID"] = m_PrivacyOptions.m_bAutoUploadMachineID;
	m_JSONPreferences["PrivacyOptions"]["AutoUploadMachineInfo"] = m_PrivacyOptions.m_bAutoUploadMachineInfo;
}
void SysLatPreferences::WriteDebugOptions() {
	m_JSONPreferences["DebugOptions"]["DebugMode"] = m_DebugOptions.m_bDebugMode;
	m_JSONPreferences["DebugOptions"]["TestUploadMode"] = m_DebugOptions.m_bTestUploadMode;
	m_JSONPreferences["DebugOptions"]["SysLatInOSD"] = m_DebugOptions.m_bSysLatInOSD;
}
void SysLatPreferences::WriteRTSSOptions() {
	//the following 2 keep getting written as int or unsigned int max value for some reason...
	//Changed cast from "unsigned int" to "int", but now it's showing as "-1" for each when I think they should be 0 or 1... may need to flip the negative bit before writing or something?
	//But that seems dumb...
	//Actually this might be working correctly. Not sure yet.
	m_JSONPreferences["RTSSOptions"]["positionX"] = static_cast<int>(m_RTSSOptions.m_positionX);
	m_JSONPreferences["RTSSOptions"]["positionY"] = static_cast<int>(m_RTSSOptions.m_positionY);
	m_JSONPreferences["RTSSOptions"]["PositionManualOverride"] = m_RTSSOptions.m_bPositionManualOverride;
	m_JSONPreferences["RTSSOptions"]["internalX"] = m_RTSSOptions.m_internalX;
	m_JSONPreferences["RTSSOptions"]["internalY"] = m_RTSSOptions.m_internalY;
}


void SysLatPreferences::ReadPreferences() {
	std::ifstream importPreferences;
	bool check = CreateDirectory((pathOnly + "SysLat_Logs").c_str(), NULL);
	importPreferences.open(pathOnly + "/SysLatPreferences.json");

	if (importPreferences.is_open()) {
		importPreferences >> m_JSONPreferences;
		//dataExported = true;
	}
	//else {
		//m_strError += "Unable to open file";
	//}

	importPreferences.close();

	ReadSysLatOptions();
	ReadPrivacyOptions();
	ReadDebugOptions();
	ReadRTSSOptions();
}

//All these values are being set by default when they're instantiated AND when they're read... seems like overkill?
void SysLatPreferences::ReadSysLatOptions() {
	m_SysLatOptions.m_targetApp = m_JSONPreferences["SysLatOptions"].get("LastSelectedTargetApp", "dota2").asString();
	m_SysLatOptions.m_PortSpecifier = m_JSONPreferences["SysLatOptions"].get("PortSpecifier", "COM3").asString();
	m_SysLatOptions.m_maxTestDuration = m_JSONPreferences["SysLatOptions"].get("MaxTestDuration", 15).asInt();
	m_SysLatOptions.m_maxLogs = m_JSONPreferences["SysLatOptions"].get("MaxLogs", 15).asInt();
	m_SysLatOptions.m_LogDir = m_JSONPreferences["SysLatOptions"].get("LogDir", pathOnly + "\SysLat_Logs").asString();
	m_SysLatOptions.m_bDarkMode = m_JSONPreferences["SysLatOptions"].get("DarkMode", false).asBool();
}
void SysLatPreferences::ReadPrivacyOptions() {
	m_PrivacyOptions.m_bFirstRun = m_JSONPreferences["PrivacyOptions"].get("FirstRun", true).asBool();
	m_PrivacyOptions.m_bRunOnStartup = m_JSONPreferences["PrivacyOptions"].get("RunOnStartup", true).asBool();
	m_PrivacyOptions.m_bAutoCheckUpdates = m_JSONPreferences["PrivacyOptions"].get("AutoCheckUpdates", true).asBool();
	m_PrivacyOptions.m_bAutoExportLogs = m_JSONPreferences["PrivacyOptions"].get("AutoExportLogs", true).asBool();
	m_PrivacyOptions.m_bAutoUploadLogs = m_JSONPreferences["PrivacyOptions"].get("AutoUploadLogs", true).asBool();
	m_PrivacyOptions.m_bAutoUploadMachineID = m_JSONPreferences["PrivacyOptions"].get("AutoUploadMachineID", true).asBool();
	m_PrivacyOptions.m_bAutoUploadMachineInfo = m_JSONPreferences["PrivacyOptions"].get("AutoUploadMachineInfo", true).asBool();
}
void SysLatPreferences::ReadDebugOptions() {
	m_DebugOptions.m_bDebugMode = m_JSONPreferences["DebugOptions"].get("DebugMode", false).asBool();
	m_DebugOptions.m_bTestUploadMode = m_JSONPreferences["DebugOptions"].get("TestUploadMode", false).asBool();
	m_DebugOptions.m_bSysLatInOSD = m_JSONPreferences["DebugOptions"].get("SysLatInOSD", false).asBool();
}
void SysLatPreferences::ReadRTSSOptions() {
	m_RTSSOptions.m_positionX = m_JSONPreferences["RTSSOptions"].get("positionX", 1).asInt(); //does this need to default to 1 or 0? I think (1,1) represents the bottom right corner
	m_RTSSOptions.m_positionY = m_JSONPreferences["RTSSOptions"].get("positionY", 1).asInt();
	m_RTSSOptions.m_bPositionManualOverride = m_JSONPreferences["RTSSOptions"].get("PositionManualOverride", false).asBool();
	m_RTSSOptions.m_internalX = m_JSONPreferences["RTSSOptions"].get("internalX", 0).asInt(); //Uhh... one of these 2 definitely needs a default value... of like 10 or something?
	m_RTSSOptions.m_internalY = m_JSONPreferences["RTSSOptions"].get("internalY", 0).asInt();
}
