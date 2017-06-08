#pragma once

#include <comutil.h>
#define _WIN32_DCOM
#include <iostream>

#include <qt_windows.h>
#include <wbemcli.h>
//#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include <map> 
#include <string>

#include <QMap>


class CpuPerfomance
{
private:
	byte buf[15];		// Name Core
	long lBufferSize = 15;
	long lNumBytes;

	HRESULT                 hr = S_OK;
	IWbemRefresher          *pRefresher = NULL;
	IWbemConfigureRefresher *pConfig = NULL;
	IWbemHiPerfEnum         *pEnum = NULL;
	IWbemServices           *pNameSpace = NULL;
	IWbemLocator            *pWbemLocator = NULL;
	IWbemObjectAccess       **apEnumAccess = NULL;
        WCHAR                    *bstrNameSpace = NULL;
	long                    lID = 0;
	long                    lVirtualBytesHandle = 0;
	long                    lIDProcessHandle = 0;
	DWORD                   dwVirtualBytes = 0;
	DWORD                   dwProcessId = 0;
	DWORD                   dwNumObjects = 0;
	DWORD                   dwNumReturned = 0;
	DWORD                   i = 0;
	int                     x = 0;
	std::map<std::string, int> MapCorePerf;
    QMap<QString, int> QmapCore;

public:
	CpuPerfomance();
	~CpuPerfomance();
	int CpuPerfomanceInit();
	void GetPerfomanceCore(int NumCore);
	std::map<std::string, int> GetPerfomance();
    QMap<QString, int> QGetPerfomance();
};
