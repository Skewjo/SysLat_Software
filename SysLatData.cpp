#include "StdAfx.h"
#include "SysLatData.h"

void CSysLatData::SetEnd() {
	m_endTime = system_clock::now();
	m_testDuration = duration_cast<duration<int>>(m_endTime - m_startTime);
}

void CSysLatData::UpdateSLD(unsigned int loopCounter, const string& sysLatResults, string RTSSWindow, string activeWindow, DWORD fgPID, DWORD rtssPID)
{
	m_Mutex.lock();
	m_sld.m_statistics.counter = loopCounter+1;

	if (sysLatResults.size() != 0) {
		systemLatency = stoi(sysLatResults);
		m_sld.m_allResults.push_back(systemLatency);
		m_sld.m_v_strRTSSWindow.push_back(RTSSWindow);
		m_sld.m_v_strActiveWindow.push_back(activeWindow);
		m_sld.m_statistics.total += systemLatency;
		m_sld.m_statistics.average = static_cast<double>(m_sld.m_statistics.total) / m_sld.m_statistics.counter; //when I try to cast one of these to a double, it appears to get the program out of sync and shoots the displayed syslat up quite a bit... - working now?
		m_sld.m_statistics.approxMovingAvg = CalculateMovingAverage(m_sld.m_statistics.average, systemLatency);
		if (systemLatency > 3 && systemLatency < 100 && fgPID == rtssPID) {
			m_sld.m_statisticsEVR.counter++;
			m_sld.m_statisticsEVR.total += systemLatency;
			m_sld.m_statisticsEVR.average = static_cast<double>(m_sld.m_statisticsEVR.total) / m_sld.m_statisticsEVR.counter;
			m_sld.m_statisticsEVR.approxMovingAvg = CalculateMovingAverage(m_sld.m_statisticsEVR.average, systemLatency);
		}
	}

	m_Mutex.unlock();
}

double CSysLatData::CalculateMovingAverage(double currentAvg, int input) {
	double newMovingAvg = currentAvg - (currentAvg/ MOVING_AVERAGE);
	newMovingAvg += static_cast<double>(input)/MOVING_AVERAGE;

	return newMovingAvg;
}

void CSysLatData::AppendError(const string& error)
{
	if (m_strError.size() != 0)
		m_strError += "\n";
	m_strError += error;
	m_strError += "\n";
}

void CSysLatData::CreateJSONSLD() {
	Json::Value resultsSize(Json::arrayValue);
	Json::Value resultsArray(Json::arrayValue);

	//These 3 values were being used to send ALL test data. I think it's overkill.
	resultsSize.append(m_sld.m_allResults.size());
	resultsSize.append(m_sld.m_v_strRTSSWindow.size());
	resultsSize.append(m_sld.m_v_strActiveWindow.size());

	//This block of code would keep the 3 arrays of data found in the SYSLAT_DATA struct seperate in the JSON. They are currently formatted to be an array of arrays to make the data easier to read.
	//MAJOR ERROR HERE - NOT SURE WHY, BUT IT'S OCCURRED IN BOTH THE FIRST AND SECOND LOOPS SO FAR FOR ME
	for (size_t i = 0; i < m_sld.m_allResults.size(); i++) {
		resultsArray.append(Json::Value(m_sld.m_allResults[i]));
	}
	Json::Value RTSSArray(Json::arrayValue);
	for (size_t i = 0; i < m_sld.m_v_strRTSSWindow.size(); i++) {
		RTSSArray.append(Json::Value(m_sld.m_v_strRTSSWindow[i]));
	}
	Json::Value activeArray(Json::arrayValue);
	for (size_t i = 0; i < m_sld.m_v_strActiveWindow.size(); i++) {
		activeArray.append(Json::Value(m_sld.m_v_strActiveWindow[i]));
	}

	m_JSONsld["MetaData"]["SysLatVersion"] = "v1.0.1";
	m_JSONsld["MetaData"]["RTSSVersion"] = m_RTSSVersion; //this is getting changed from 0 to 131088??
	m_JSONsld["MetaData"]["TargetApplication"] = m_targetApp;
	m_JSONsld["MetaData"]["StartTimeUTC"] = format("%a %b %R:%OS %Y\n", m_startTime);
	m_JSONsld["MetaData"]["EndTimeUTC"] = format("%a %b %R:%OS %Y\n", m_endTime);
	m_JSONsld["MetaData"]["Duration"] = format("%R:%OS", m_testDuration);
	//m_JSONsld["MetaData"]["TestAnchor"] = m_boxAnchor; //Having a value in the in the test data that doesn't exist in the schema is not allowed apparently...

	//NumProcBegin: Int
	//NumProcEnd : Int
	//ProcListBegin : [String]
	//ProcListEnd : [String]
	//NumServBegin : Int
	//NumServEnd : Int
	//ServListBegin : [String]
	//ServListEnd : [String]
	
	m_JSONsld["AggregateData"]["EVRCounter"] = m_sld.m_statisticsEVR.counter;
	m_JSONsld["AggregateData"]["EVRSystemLatencyTotal"] = m_sld.m_statisticsEVR.total;
	m_JSONsld["AggregateData"]["EVRsystemLatencyAverage"] = m_sld.m_statisticsEVR.average;
	m_JSONsld["AggregateData"]["SysLatTestCount"] = m_sld.m_statistics.counter;
	m_JSONsld["AggregateData"]["SystemLatencyTotal"] = m_sld.m_statistics.total;
	m_JSONsld["AggregateData"]["SystemLatencyAverage"] = m_sld.m_statistics.average;

	m_JSONsld["SysLatData"]["SysLatResultSize"] = resultsSize;
	m_JSONsld["SysLatData"]["SysLatResults"] = resultsArray;
	m_JSONsld["SysLatData"]["RTSSWindow"] = RTSSArray;
	m_JSONsld["SysLatData"]["ActiveWindow"] = activeArray;
}

void CSysLatData::ExportData(int testNumber, string path, int totalLogs) {
	std::ofstream exportData;
	//string startDateUTC = format("%F", m_startTime);

	//COUNT NUMBER OF LOGS HERE
	//string path = "/path/to/directory";
	int count = 0;
	std::filesystem::directory_entry oldest_file;
	std::regex rx("SL_Log_.*");

	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::filesystem::path filename = std::filesystem::path(entry).filename();
		if (count == 0) {

			oldest_file = entry;
		}
		if (entry.exists() && std::regex_match(filename.string(), rx)) {
			if (entry.last_write_time() > oldest_file.last_write_time()) {
				oldest_file = entry;
			}
			count++;
		}
	}
	
	DEBUG_PRINT("TotalLogs: " + to_string(totalLogs))

	//TODO delete more than 1 at a time
	if (count > totalLogs && oldest_file.exists()) {
		std::filesystem::remove(oldest_file);
	}
	exportData.open( path + "\\SL_Log_" + to_string(testNumber) + ".json");//+ "_" + startDateUTC - on second thought... I don't think I need the date in the file name

	if (exportData.is_open()) {
		exportData << m_JSONsld;
		m_bDataExported = true;
	}
	else {
		m_strError += "Unable to open file";
	}

	exportData.close();
}
