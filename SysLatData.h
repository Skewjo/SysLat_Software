#pragma once
#ifndef SYSLATDATA_H
#define SYSLATDATA_H

//#define MAX_TESTS										3600
#define MOVING_AVERAGE									100
#define EVR_MIN											3
#define EVR_MAX											100

typedef struct SYSLAT_DATA {
	//should these be pre-pended with m_ or not?
	//int					m_totalTests[MAX_TESTS];
	int					m_counter;
	int					m_systemLatencyTotal;
	double				m_systemLatencyAverage;
	int					m_counterEVR;
	int					m_systemLatencyTotalEVR; //EVR stands for expected value range
	double				m_systemLatencyAverageEVR;
	int					m_aMovingAverage[MOVING_AVERAGE];//if I end up using a "total tests" var, then this should probably just be split into 2 pointers pointing at the "head" of the totalTests var and head-100 or something...
	int					m_systemLatencyMedian;
	int					m_systemLatencyMedianEVR;
	int					m_systemLatencyMax;
	int					m_systemLatencyMin;
	int					m_systemLatencyMaxEVR;
	int					m_systemLatencyMinEVR;
}SYSLAT_DATA;

class CSysLatData
{
protected: 
	//static int instancesThisSession; //Do I need to keep track of the number of these objects that I've created?
	SYSLAT_DATA			sld;
	CString				m_strSysLatResultsComplete;
	HANDLE				m_Mutex = NULL;
	CString				m_strError;
	
public:
	CSysLatData(); //this constructor only opens a mutex... does the destructor need to close the mutex? Also, do I need to init the struct in the constructor? It should init to all 0's off the bat...
	//using getters and setters for all of these seems stupid...
	int					GetCounter();
	int					GetTotal();
	double				GetAverage();
	int					GetCounterEVR();
	int					GetTotalEVR();
	double				GetAverageEVR();
	CString				GetStringResult();

	//int*				GetMovingAverage(); //????
	int					GetMedian();
	int					GetMedianEVR();
	int					GetMax();
	int					GetMin();
	int					GetMaxEVR();
	int					GetMinEVR();

	//double				CalculateMovingAverage(); //this function would be for calculating it from scratch...
	//double				UpdateMovingAverage(); //this function would be used if I'm updating the moving average every time I get a new value

	
	void				UpdateSLD(unsigned int loopCounter, const CString& arduinoResults);

	//mutex functions?
	void					CheckSLDMutex();
	BOOL					AcquireSLDMutex();
	void					ReleaseSLDMutex();
	void					CloseSLDMutex();

	void AppendError(const CString& error);

	//data IO functions?
	//BOOL					ExportData(); //to CSV or something? What's the 
};
#endif