#pragma once

constexpr size_t MOVING_AVERAGE = 100;
constexpr size_t EVR_MIN = 3;
constexpr size_t EVR_MAX = 100;

struct SYSLAT_DATA {
	struct Statistics {
		std::size_t counter = 0;
		int total = 0;
		double average = 0.0;
		int median = 0;
		int max = 0;
		int min = 0;
		double approxMovingAvg = 0.0;
	};

	vector<int>			m_allResults;
	vector<string>		m_v_strRTSSWindow;
	vector<string>		m_v_strActiveWindow;
	
	Statistics			m_statistics;
	Statistics			m_statisticsEVR;
};

class CSysLatData
{
protected: 
	SYSLAT_DATA			m_sld;
	Json::Value			m_JSONsld; //this is basically a second copy of the data... will probably eat up a BOATLOAD of memory for no reason. There's got to be a better way...
	int					systemLatency = 0;

	std::mutex			m_Mutex;
	string				m_strError = "";

	const system_clock::time_point	m_startTime = system_clock::now();
	system_clock::time_point		m_endTime;
	duration<int>					m_testDuration; 

	double				CalculateMovingAverage(double currentAvg, int input);

public:
	const SYSLAT_DATA&	GetData() {return m_sld;}
	const Json::Value&	GetJSONData() { return m_JSONsld; }
	const int&			GetSystemLatency() { return systemLatency; }
	const duration<int>	GetCurrentDuration() { return duration_cast<duration<int>>(system_clock::now() - m_startTime); } //if this is called for an old test, it could give an incorrect value...
	const duration<int>& GetTestDuration() { return m_testDuration; }

	
	void				SetEnd();
	void				UpdateSLD(unsigned int loopCounter, const string& sysLatResults, string RTSSWindow, string activeWindow, DWORD fgPID, DWORD rtssPID);
	void				AppendError(const string& error);
	
	void				CreateJSONSLD(); // I think I may need to make this and the following functions return bools or ints based on whether or not they failed.
	void				ExportData(int testNumber, string path = ".\\SysLat_Logs", int totalLogs = 10000);

	string				m_RTSSVersion = "0.0.0";
	string				m_targetApp = "Unknown";
	string				m_boxAnchor = "Unknown";

	bool				m_bDataExported = false;
	bool				m_bDataUploaded = false;
};
