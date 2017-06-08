#include "stdafx.h"


#include "CpuPerfomance.h"
#include <QString>
#include <QDebug>

CpuPerfomance::CpuPerfomance()
{
}


CpuPerfomance::~CpuPerfomance()
{	
//	if (NULL != bstrNameSpace)
//	{
//		SysFreeString(bstrNameSpace);
//	}

	if (NULL != apEnumAccess)
	{
		for (i = 0; i < dwNumReturned; i++)
		{
			if (apEnumAccess[i] != NULL)
			{
				apEnumAccess[i]->Release();
				apEnumAccess[i] = NULL;
			}
		}
		delete[] apEnumAccess;
	}
	if (NULL != pWbemLocator)
	{
		pWbemLocator->Release();
	}
	if (NULL != pNameSpace)
	{
		pNameSpace->Release();
	}
	if (NULL != pEnum)
	{
		pEnum->Release();
	}
	if (NULL != pConfig)
	{
		pConfig->Release();
    }/*
    if (NULL != pRefresher)
	{
		pRefresher->Release();
    }*/

//	CoUninitialize();
}

int CpuPerfomance::CpuPerfomanceInit()
{

    /*
    if (FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		CoUninitialize();
		return hr;
	}
    */
	if (FAILED(hr = CoInitializeSecurity(
		NULL,
		-1,
		NULL,
		NULL,
		RPC_C_AUTHN_LEVEL_NONE,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, 0)))
	{
		CoUninitialize();
		return hr;
	}

	if (FAILED(hr = CoCreateInstance(
		CLSID_WbemLocator,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		(void**)&pWbemLocator)))
	{
		CoUninitialize();
		return hr;
	}

	// Connect to the desired namespace.
    //bstrNameSpace = SysAllocString(L"\\\\.\\root\\cimv2");

    wchar_t bstrNameSpace[100] = {0};
    QString foostring = "\\\\.\\root\\cimv2";
    foostring.toWCharArray(bstrNameSpace);



	if (NULL == bstrNameSpace)
	{
		hr = E_OUTOFMEMORY;
		CoUninitialize();
		return hr;
	}
	if (FAILED(hr = pWbemLocator->ConnectServer(
		bstrNameSpace,
		NULL, // User name
		NULL, // Password
		NULL, // Locale
		0L,   // Security flags
		NULL, // Authority
		NULL, // Wbem context
		&pNameSpace)))
	{
        qDebug() << "Could not connect. Error code = 0x"
            << hr << endl;

		pWbemLocator->Release();
		CoUninitialize();
		return hr;      // Program has failed.
	}

    qDebug() << "ConnectServer Ok:"
        << hr << endl;

	pWbemLocator->Release();
	pWbemLocator = NULL;
    //SysFreeString(bstrNameSpace);
    //bstrNameSpace = NULL;

	if (FAILED(hr = CoCreateInstance(
		CLSID_WbemRefresher,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWbemRefresher,
		(void**)&pRefresher)))
	{
		CoUninitialize();
		return hr;      // Program has failed.
	}

	if (FAILED(hr = pRefresher->QueryInterface(
		IID_IWbemConfigureRefresher,
		(void **)&pConfig)))
	{
		pRefresher->Release();
		CoUninitialize();
		return hr;      // Program has failed.
	}

	if (FAILED(hr = pConfig->AddEnum(
		pNameSpace,
		L"Win32_PerfFormattedData_PerfOS_Processor",
		0,
		NULL,
		&pEnum,
		&lID)))
	{
		CoUninitialize();
		return hr;      // Program has failed.
	}

	pConfig->Release();
	pConfig = NULL;

	return 0;
}

void CpuPerfomance::GetPerfomanceCore(int NumCore)
{
}

QMap<QString, int> CpuPerfomance::QGetPerfomance()
{
    dwNumReturned = 0;
    dwNumObjects = 0;

    if (FAILED(hr = pRefresher->Refresh(0)))
    {
        return QmapCore;
    }

    hr = pEnum->GetObjects(0L,
        dwNumObjects,
        apEnumAccess,
        &dwNumReturned);
    // If the buffer was not big enough,
    // allocate a bigger buffer and retry.
    if (hr == WBEM_E_BUFFER_TOO_SMALL
        && dwNumReturned > dwNumObjects)
    {
        apEnumAccess = new IWbemObjectAccess*[dwNumReturned];
        if (NULL == apEnumAccess)
        {
            hr = E_OUTOFMEMORY;
            return QmapCore;
        }
        SecureZeroMemory(apEnumAccess,
            dwNumReturned * sizeof(IWbemObjectAccess*));
        dwNumObjects = dwNumReturned;

        if (FAILED(hr = pEnum->GetObjects(0L,
            dwNumObjects,
            apEnumAccess,
            &dwNumReturned)))
        {
            return QmapCore;
        }
    }
    else
    {
        if (hr == WBEM_S_NO_ERROR)
        {
            hr = WBEM_E_NOT_FOUND;
            return QmapCore;
        }
    }

    // First time through, get the handles.
    if (0 == x)
    {
        CIMTYPE VirtualBytesType;
        CIMTYPE ProcessHandleType;
        if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
            L"PercentProcessorTime",
            &VirtualBytesType,
            &lVirtualBytesHandle)))
        {
            return QmapCore;
        }

        if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
            L"Name",
            &ProcessHandleType,
            &lIDProcessHandle)))
        {
            return QmapCore;
        }
        x += 1;
    }

    for (i = 0; i < dwNumReturned; i++)
    {
        if (FAILED(hr = apEnumAccess[i]->ReadDWORD(
            lVirtualBytesHandle,
            &dwVirtualBytes)))
        {
            break;
        }
        apEnumAccess[i]->ReadPropertyValue(lIDProcessHandle, lBufferSize, &lNumBytes, buf);
        std::wstring str(reinterpret_cast<wchar_t *>(buf), sizeof(buf) / sizeof(buf[0]));
        std::string foo(reinterpret_cast<const char *> (buf), sizeof(buf) / sizeof(buf[0]));
        QString foo_debug = QString::fromWCharArray(str.c_str());

        QmapCore.insert(foo_debug, dwVirtualBytes);
        MapCorePerf.insert(std::pair<std::string, int>(foo, dwVirtualBytes));
//			MapFoo.insert(std::pair<std::string, int>(foo, dwVirtualBytes));
        memset(buf, 0, sizeof(buf));

//        qDebug() << foo_debug << dwVirtualBytes;

        // Done with the object
        apEnumAccess[i]->Release();

    }
    for (auto it = QmapCore.begin(); it != QmapCore.end(); ++it)
    {
//        qDebug()<< "MapCoreDebug"<<it.key()<< it.value();
    }

    return QmapCore;
}

std::map<std::string, int> CpuPerfomance::GetPerfomance()
{
	//for (x = 0; x < 1; x++)
	{
		dwNumReturned = 0;
		dwNumObjects = 0;

        if (FAILED(hr = pRefresher->Refresh(0)))
        {
            return MapCorePerf;
        }

        hr = pEnum->GetObjects(0L,
			dwNumObjects,
			apEnumAccess,
			&dwNumReturned);
		// If the buffer was not big enough,
		// allocate a bigger buffer and retry.
		if (hr == WBEM_E_BUFFER_TOO_SMALL
			&& dwNumReturned > dwNumObjects)
		{
			apEnumAccess = new IWbemObjectAccess*[dwNumReturned];
			if (NULL == apEnumAccess)
			{
				hr = E_OUTOFMEMORY;
				return MapCorePerf;
			}
			SecureZeroMemory(apEnumAccess,
				dwNumReturned * sizeof(IWbemObjectAccess*));
			dwNumObjects = dwNumReturned;

			if (FAILED(hr = pEnum->GetObjects(0L,
				dwNumObjects,
				apEnumAccess,
				&dwNumReturned)))
			{
				return MapCorePerf;
			}
		}
		else
		{
			if (hr == WBEM_S_NO_ERROR)
			{
				hr = WBEM_E_NOT_FOUND;
				return MapCorePerf;
			}
		}

		// First time through, get the handles.
		if (0 == x)
		{
			CIMTYPE VirtualBytesType;
			CIMTYPE ProcessHandleType;
			if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
				L"PercentProcessorTime",
				&VirtualBytesType,
				&lVirtualBytesHandle)))
			{
				return MapCorePerf;
			}

			if (FAILED(hr = apEnumAccess[0]->GetPropertyHandle(
				L"Name",
				&ProcessHandleType,
				&lIDProcessHandle)))
			{
				return MapCorePerf;
			}
            x += 1;
		}


//		std::map<std::string, int> MapFoo;
		for (i = 0; i < dwNumReturned; i++)
		{
			if (FAILED(hr = apEnumAccess[i]->ReadDWORD(
				lVirtualBytesHandle,
				&dwVirtualBytes)))
			{
				break;
			}
			apEnumAccess[i]->ReadPropertyValue(lIDProcessHandle, lBufferSize, &lNumBytes, buf);
			std::wstring str(reinterpret_cast<wchar_t *>(buf), sizeof(buf) / sizeof(buf[0]));
            std::string foo(reinterpret_cast<const char *> (buf), sizeof(buf) / sizeof(buf[0]));
            QString foo_debug = QString::fromWCharArray(str.c_str());

            QmapCore.insert(foo_debug, dwVirtualBytes);
			MapCorePerf.insert(std::pair<std::string, int>(foo, dwVirtualBytes));
//			MapFoo.insert(std::pair<std::string, int>(foo, dwVirtualBytes));
			memset(buf, 0, sizeof(buf));

            qDebug() << foo_debug << dwVirtualBytes;

			//wprintf(L"Name %s is loaded %lu %%\n",
			//	buf, dwVirtualBytes);

			// Done with the object
			apEnumAccess[i]->Release();

		}
        for (auto it = QmapCore.begin(); it != QmapCore.end(); ++it)
        {

            qDebug()<< "MapCoreDebug"<<it.key()<< it.value();
        }

		return MapCorePerf;

		for (auto it = MapCorePerf.begin(); it != MapCorePerf.end(); ++it)
		{
			std::cout << it->first << " : " << it->second << std::endl;
		}

		MapCorePerf.clear();
		// Sleep for a second.
		//Sleep(1000);
	}
	// exit loop here
}
