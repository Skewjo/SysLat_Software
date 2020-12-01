#pragma once
#ifndef SYSLATDATA_H
#define SYSLATDATA_H

#include<vector>
#include<string>
#include<json/json.h>

//#define MAX_TESTS										500
#define MOVING_AVERAGE									100
#define EVR_MIN											3
#define EVR_MAX											100

typedef struct SYSLAT_DATA {
	std::vector<int>	m_allResults;
	std::vector<std::string>	m_v_strTargetWindow;
	std::vector<std::string>	m_v_strActiveWindow;
	int					m_counter = 0;
	int					m_systemLatencyTotal = 0;
	double				m_systemLatencyAverage = 0;
	int					m_counterEVR = 0;
	int					m_systemLatencyTotalEVR = 0; //EVR stands for expected value range
	double				m_systemLatencyAverageEVR = 0;
	int					m_a_MovingAverage[MOVING_AVERAGE] = { 0 };//if I end up using a "total tests" var, then this should probably just be split into 2 pointers pointing at the "head" of the totalTests var and head-100 or something...
	int					m_systemLatencyMedian = 0;
	int					m_systemLatencyMedianEVR = 0;
	int					m_systemLatencyMax = 0;
	int					m_systemLatencyMin = 0;
	int					m_systemLatencyMaxEVR = 0;
	int					m_systemLatencyMinEVR = 0;
}SYSLAT_DATA;

class CSysLatData
{
protected: 
	SYSLAT_DATA			sld;
	CString				m_strSysLatResultsComplete;
	HANDLE				m_Mutex = NULL;
	CString				m_strError;
	time_t				m_startTime, m_endTime;
	char				m_startDate[26], m_endDate[26];

	
	
public:
	CSysLatData(); //this constructor only opens a mutex... does the destructor need to close the mutex? Also, do I need to init the struct in the constructor? It should init to all 0's off the bat...
	
	Json::Value			jsonSLD; //this is basically a second copy of the data... will probably eat up a BOATLOAD of memory for no reason. There's got to be a better way...

	//using getters and setters for all of these seems stupid...
	int					GetCounter();
	int					GetTotal();
	double				GetAverage();
	int					GetCounterEVR();
	int					GetTotalEVR();
	double				GetAverageEVR();
	CString				GetStringResult();

	int					GetMedian();
	int					GetMedianEVR();
	int					GetMax();
	int					GetMin();
	int					GetMaxEVR();
	int					GetMinEVR();
	//int*				GetMovingAverage(); //????

	//double			CalculateMovingAverage(); //this function would be for calculating it from scratch...
	//double			UpdateMovingAverage(); //this function would be used if I'm updating the moving average every time I get a new value
	void				SetEndTime();
	
	void				UpdateSLD(unsigned int loopCounter, const CString& sysLatResults, std::string targetWindow, std::string activeWindow);
	
	//mutex functions
	void				CheckSLDMutex();
	BOOL				AcquireSLDMutex();
	void				ReleaseSLDMutex();
	void				CloseSLDMutex();

	void				AppendError(const CString& error);

	// I think I need to make the following 2 functions return BOOLs or INTs based on whether or not they failed.
	void				CreateJSONSLD();
	void				ExportData(int testNumber); 

	bool				dataExported = false;
	bool				dataUploaded = false;
};
#endif