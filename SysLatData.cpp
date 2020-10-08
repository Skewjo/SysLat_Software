#include "stdafx.h"
#include "SysLatData.h"

CSysLatData::CSysLatData() {
	sld = { 0 };
	m_Mutex = CreateMutex(NULL, FALSE, NULL);
}

int CSysLatData::GetCounter() {
	return sld.m_counter;
}
int	CSysLatData::GetTotal() {
	return sld.m_systemLatencyTotal;
}
double CSysLatData::GetAverage() {
	return sld.m_systemLatencyAverage;
}
int	CSysLatData::GetCounterEVR() {
	return sld.m_counterEVR;
}
int	CSysLatData::GetTotalEVR() {
	return sld.m_systemLatencyTotalEVR;
}
double CSysLatData::GetAverageEVR() {
	return sld.m_systemLatencyAverageEVR;
}
//int*				GetMovingAverage(); //????

int CSysLatData::GetMedian() {
	return sld.m_aMovingAverage[50]; //THIS ISN'T RIGHT YOU DUMMY - going to have to implement a new data structure if I want this value
}
int CSysLatData::GetMedianEVR() {
	return sld.m_aMovingAverage[50]; //going to have to keep another array of movingAverageEVR
}
int CSysLatData::GetMax() {
	return sld.m_systemLatencyMax;
}
int CSysLatData::GetMin() {
	return sld.m_systemLatencyMin;
}
int CSysLatData::GetMaxEVR() {
	return sld.m_systemLatencyMaxEVR;
}
int CSysLatData::GetMinEVR() {
	return sld.m_systemLatencyMinEVR;
}
CString CSysLatData::GetStringResult() {
	return m_strSysLatResultsComplete;
}

/*
double CSysLatData::CalculateMovingAverage() {
} 
double CSysLatData::UpdateMovingAverage() {
}
*/


void CSysLatData::UpdateSLD(unsigned int loopCounter, const CString& sysLatResults)
{
	BOOL success = AcquireSLDMutex();		// begin the sync access to fields
	if (!success)
		return;

	sld.m_counter = loopCounter;

	m_strSysLatResultsComplete = sysLatResults;

	int systemLatency = 0;
	if (!m_strSysLatResultsComplete.IsEmpty()) {
		systemLatency = StrToInt(m_strSysLatResultsComplete);
		sld.m_systemLatencyTotal += systemLatency;
		sld.m_systemLatencyAverage = static_cast<double>(sld.m_systemLatencyTotal) / loopCounter; //when I try to cast one of these to a double, it appears to get the program out of sync and shoots the displayed syslat up quite a bit...

		if (systemLatency > 3 && systemLatency < 100) {
			sld.m_counterEVR++;
			sld.m_systemLatencyTotalEVR += systemLatency;
			sld.m_systemLatencyAverageEVR = static_cast<double>(sld.m_systemLatencyTotalEVR) / sld.m_counterEVR;
		}
	}

	ReleaseSLDMutex();		// end the sync access to fields
}

void CSysLatData::CheckSLDMutex() {
	if (m_Mutex == NULL)
	{
		AppendError("Error: Failed to create mutex");
	}
}
BOOL CSysLatData::AcquireSLDMutex() {
	if (m_Mutex != NULL)
	{
		return WAIT_ABANDONED != WaitForSingleObject(m_Mutex, INFINITE);
	}

	return TRUE;
}
void CSysLatData::ReleaseSLDMutex() {
	if (m_Mutex != NULL)
	{
		ReleaseMutex(m_Mutex);
	}
}
void CSysLatData::CloseSLDMutex() {
	if (m_Mutex != NULL)
	{
		CloseHandle(m_Mutex);
		m_Mutex = NULL;
	}
}

void CSysLatData::AppendError(const CString& error)
{
	//AcquireSLDMutex();

	if (!m_strError.IsEmpty())
		m_strError.Append("\n");
	m_strError.Append(error);
	m_strError.Append("\n");

	//ReleaseSLDMutex();
}

//data IO functions?
//BOOL CSysLatData::ExportData() {//to CSV or something? JSON is Michael's favorite...
//}