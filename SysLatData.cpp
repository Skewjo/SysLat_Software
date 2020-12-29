#include "stdafx.h"
#include "SysLatData.h"
#include<fstream>

CSysLatData::CSysLatData() {
	//sld = { 0 }; //this does not 0 out the values in the "m_allResults" array...also, I can't initialize a struct this way when it contains a vector??
	m_Mutex = CreateMutex(NULL, FALSE, NULL);

	m_startTime = time(0);
	ctime_s(m_startDate, sizeof(m_startDate), &m_startTime);
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

int CSysLatData::GetMedian() {
	return sld.m_a_MovingAverage[50]; //THIS ISN'T RIGHT YOU DUMMY - going to have to implement a new data structure if I want this value
}
int CSysLatData::GetMedianEVR() {
	return sld.m_a_MovingAverage[50]; //going to have to keep another array of movingAverageEVR
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

void CSysLatData::SetEndTime() {
	time(&m_endTime);
	ctime_s(m_endDate, sizeof(m_endDate), &m_endTime);
}

void CSysLatData::UpdateSLD(unsigned int loopCounter, const CString& sysLatResults, std::string RTSSWindow, std::string activeWindow)
{
	BOOL success = AcquireSLDMutex();		// begin the sync access to fields
	if (!success)
		return;

	sld.m_counter = loopCounter+1;

	m_strSysLatResultsComplete = sysLatResults;

	int systemLatency = 0;
	if (!m_strSysLatResultsComplete.IsEmpty()) {
		systemLatency = StrToInt(m_strSysLatResultsComplete);
		sld.m_allResults.push_back(systemLatency);
		sld.m_v_strRTSSWindow.push_back(RTSSWindow);
		sld.m_v_strActiveWindow.push_back(activeWindow);
		sld.m_systemLatencyTotal += systemLatency;
		sld.m_systemLatencyAverage = static_cast<double>(sld.m_systemLatencyTotal) / sld.m_counter; //when I try to cast one of these to a double, it appears to get the program out of sync and shoots the displayed syslat up quite a bit... - working now?

		if (systemLatency > 3 && systemLatency < 100 && RTSSWindow == activeWindow) {
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

void CSysLatData::CreateJSONSLD() {
	Json::Value resultsSize(Json::arrayValue);
	Json::Value resultsArray(Json::arrayValue);
	
	//These 3 values were being used to send ALL test data. I think it's overkill.
	resultsSize.append(sld.m_allResults.size());
	resultsSize.append(sld.m_v_strRTSSWindow.size());
	resultsSize.append(sld.m_v_strActiveWindow.size());

	//This block of code would keep the 3 arrays of data found in the SYSLAT_DATA struct seperate in the JSON. They are currently formatted to be an array of arrays to make the data easier to read.
	for (int i = 0; i < sld.m_counter; i++ ) {
		resultsArray.append(Json::Value(sld.m_allResults[i]));
	}
	Json::Value RTSSArray(Json::arrayValue);
	for (int i = 0; i < sld.m_counter; i++) {
		RTSSArray.append(Json::Value(sld.m_v_strRTSSWindow[i]));
	}
	Json::Value activeArray(Json::arrayValue);
	for (int i = 0; i < sld.m_counter; i++) {
		activeArray.append(Json::Value(sld.m_v_strActiveWindow[i]));
	}

	////This block was for making a 2d JSON array and was kind of stupid.
	//for (int i = 0; i < sld.m_allResults.size(); i++) {
	//	Json::Value subResultsArray(Json::arrayValue);
	//	subResultsArray.append(Json::Value(i));
	//	subResultsArray.append(Json::Value(sld.m_allResults[i]));
	//	subResultsArray.append(Json::Value(sld.m_v_strTargetWindow[i]));
	//	subResultsArray.append(Json::Value(sld.m_v_strActiveWindow[i]));
	//	resultsArray.append(subResultsArray);
	//}

	//Add elapsed time(duration?) at some point
	
	struct tm* startTimeUTC = gmtime(&m_startTime); //Apparently(and the documentation doesn't reveal this FYI), gmtime is a static object(???) so if I don't set it right before I output it, I get the wrong thing.
	char* startDateUTC = asctime(startTimeUTC);

	jsonSLD["MetaData"]["SysLatVersion"] = "v0.0.1";
	jsonSLD["MetaData"]["RTSSVersion"] = "vPLACEHOLDER";
	jsonSLD["MetaData"]["SysLatVersion"] = "v0.0.1";
	jsonSLD["MetaData"]["TargetApplication"] = "PLACEHOLDER";
	jsonSLD["MetaData"]["StartTimeUTC"] = startDateUTC;
	//THIS NEEDS TO BE MOVED!! WHY DID I PUT IT HERE??
	struct tm* endTimeUTC = gmtime(&m_endTime);
	char *endDateUTC = asctime(endTimeUTC);
	jsonSLD["MetaData"]["EndTimeUTC"] = endDateUTC;
	jsonSLD["MetaData"]["StartTimeLocal"] = m_startDate;
	jsonSLD["MetaData"]["EndTimeLocal"] = m_endDate;
	//NumProcBegin: Int
	//NumProcEnd : Int
	//ProcListBegin : [String]
	//ProcListEnd : [String]
	//NumServBegin : Int
	//NumServEnd : Int
	//ServListBegin : [String]
	//ServListEnd : [String]
	
	jsonSLD["AggregateData"]["EVRCounter"] = sld.m_counterEVR;
	jsonSLD["AggregateData"]["EVRSystemLatencyTotal"] = sld.m_systemLatencyTotalEVR;
	jsonSLD["AggregateData"]["EVRsystemLatencyAverage"] = sld.m_systemLatencyAverageEVR;
	jsonSLD["AggregateData"]["SysLatTestCount"] = sld.m_counter;
	jsonSLD["AggregateData"]["SystemLatencyTotal"] = sld.m_systemLatencyTotal;
	jsonSLD["AggregateData"]["SystemLatencyAverage"] = sld.m_systemLatencyAverage;

	jsonSLD["SysLatData"]["SysLatResultSize"] = resultsSize;
	jsonSLD["SysLatData"]["SysLatResults"] = resultsArray;
	jsonSLD["SysLatData"]["RTSSWindow"] = RTSSArray;
	jsonSLD["SysLatData"]["ActiveWindow"] = activeArray;
}

void CSysLatData::ExportData(int testNumber) {
	std::ofstream exportData;
	exportData.open("./logs/sld_export" + std::to_string(testNumber) + ".json");

	if (exportData.is_open()) {
		exportData << jsonSLD;
		dataExported = true;
	}
	else {
		m_strError += "Unable to open file";
	}

	exportData.close();
}