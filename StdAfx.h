// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__6D63CA46_F2CF_40D3_B925_F1C75DFBFC28__INCLUDED_)
#define AFX_STDAFX_H__6D63CA46_F2CF_40D3_B925_F1C75DFBFC28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxwinappex.h>	// Added 12-31-2020 to make "folder browse" component of the preferences dialog work
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#define ATLTRACE6 m_DebugFile.Dump
#define USE_DEBUGDUMP extern CDebugDump m_DebugFile;

#include "afxdialogex.h"
#include "afxeditbrowsectrl.h"
#include <algorithm>
#include <Assert.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <io.h>
#include <json/json.h>
#include <map>
#include <mutex>
#include <process.h>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <time.h>
#include <vector>
#include <WinBase.h>
#include <winternl.h>
#include <wrl/client.h> //graphics card info for MachineInfo.cpp

#pragma comment(lib, "DXGI.lib") //graphics card info
#pragma comment (lib, "OneCore.lib") //Used to enumerate COM ports

using std::string;
using std::vector;
using std::map;
using std::string_view;
using std::to_string;

namespace SL
{
	inline void OutputDebug(string OutputString){
		OutputDebugString(OutputString.c_str());
		OutputDebugString("\n");
	}

	//file name string cleaner helper functions
	inline void RemoveExtension(string& inputString) {
		size_t pos = inputString.find(".exe");
		inputString.replace(pos, inputString.size(), "");
	}
	
	inline void RemovePath(string& inputString) {
		size_t pos = inputString.rfind("\\");
		inputString.replace(0, pos + 1, "");
	}

	inline void RemoveFileNameFromPath(string& inputString) {
		size_t pos = inputString.rfind("\\");
		inputString.replace(pos + 1, inputString.size(), "");
	}

	inline void RemoveSpaces(string& inputString) {
		size_t pos;
		while ((pos = inputString.find(" ")) != string::npos) {
			inputString.replace(pos, 1, "");
		}
	}
}

#if _DEBUG
#define DEBUG_PRINT(x)  SL::OutputDebug(x);
#else
#define DEBUG_PRINT(x)   
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__6D63CA46_F2CF_40D3_B925_F1C75DFBFC28__INCLUDED_)
