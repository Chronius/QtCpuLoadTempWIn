#ifndef WMIMANAGER_H
#define WMIMANAGER_H

#include <QObject>
#include <QDebug>
#include <QByteArray>
#include <QException>
#include <qt_windows.h>

#include <wbemcli.h>

class WmiException : public QException {

public:

    void raise() const { throw *this; }

    WmiException *clone() const { return new WmiException(*this); }

};

class WmiManager {

public:

    WmiManager(){

        pSvc = NULL;

        HRESULT hres;


        hres =  CoInitializeEx(0, COINIT_MULTITHREADED);

        if (FAILED(hres)){

            qDebug() << "Failed to initialize COM library. Error code = 0x"
                       << hex
                           << hres
                               << endl;

            //throw hres;

        }


        hres =  CoInitializeSecurity(NULL,
                                      -1,                          // COM authentication
                                      NULL,                        // Authentication services
                                      NULL,                        // Reserved
                                      RPC_C_AUTHN_LEVEL_NONE,   // Default authentication
                                      RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
                                      NULL,                        // Authentication info
                                      EOAC_NONE,                   // Additional capabilities
                                      NULL                         // Reserved
                                      );

         if (FAILED(hres))
         {
             qDebug() << "Failed to initialize security. Error code = 0x"
                        << hex
                            << hres
                                << endl;
             CoUninitialize();

             throw hres;

         }

         hres = CoCreateInstance(   CLSID_WbemLocator,
                                    0,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IWbemLocator,
                                    (LPVOID *) &pLoc);


         if (FAILED(hres)){

             qDebug() << "Failed to create IWbemLocator object."
                        << " Err code = 0x"
                            << hex
                                << hres
                                    << endl;

             CoUninitialize();

             throw hres;


         }
                                                    /*(QString("root\\wmi")*/
         hres = pLoc->ConnectServer((const BSTR)(QString("ROOT\\CIMV2").toStdWString().data()), // Object path of WMI namespace
                                      NULL,                    // User name. NULL = current user
                                      NULL,                    // User password. NULL = current
                                      0,                       // Locale. NULL indicates current
                                      NULL,                    // Security flags.
                                      0,                       // Authority (e.g. Kerberos)
                                      0,                       // Context object
                                      &pSvc                    // pointer to IWbemServices proxy
                                      );

         if (FAILED(hres))
         {
             qDebug() << "Could not connect. Error code = 0x"
                        << hex
                            << hres
                                << endl;

             pLoc->Release();

             CoUninitialize();

             throw hres;

         }

         qDebug() << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


         hres = CoSetProxyBlanket(  pSvc,                        // Indicates the proxy to set
                                    RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
                                    RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
                                    NULL,                        // Server principal name
                                    RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
                                    RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                                    NULL,                        // client identity
                                    EOAC_NONE                    // proxy capabilities
                                 );

         if (FAILED(hres)){

             qDebug() << "Could not set proxy blanket. Error code = 0x"
                        << hex
                            << hres
                                << endl;

             pSvc->Release();

             pLoc->Release();

             CoUninitialize();

             throw hres;

         }

         if (FAILED(hres = CoCreateInstance(
             CLSID_WbemRefresher,
             NULL,
             CLSCTX_INPROC_SERVER,
             IID_IWbemRefresher,
             (void**)&pRefresher)))
         {
             CoUninitialize();
             throw hres;      // Program has failed.
         }

         (hres = pRefresher->QueryInterface(
                      IID_IWbemConfigureRefresher,
                      (void **)&pConfig));
         if (FAILED(hres))
         {
             pRefresher->Release();
             CoUninitialize();
             throw hr;      // Program has failed.
         }

         if (FAILED(hres = pConfig->AddEnum(
             pSvc,
             L"Win32_PerfFormattedData_PerfOS_Processor",
             0,
             NULL,
             &pEnum,
             &lID)))
         {
             CoUninitialize();
             throw hr;      // Program has failed.
         }

         pConfig->Release();
         pConfig = NULL;

          return;   // Program successfully completed.

    }

    ~WmiManager(){

        pEnum->Release();
        pConfig->Release();

        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        pclsObj->Release();
        CoUninitialize();

    }

    QStringList GetValue(QString StructName, QString Parameter){

        // QStringList RequestList << "Win32_Processor" <<

        QStringList Report;

        QString Request = "SELECT * FROM " + StructName;

        DWORD hres = pSvc->ExecQuery( (const BSTR)(QString("WQL").toStdWString().data()),
                                 (const BSTR)Request.toStdWString().data(),
                                 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                 NULL,
                                 &pEnumerator);

         if (FAILED(hres)){

             qDebug() << "Query for operating system name failed."
                        << " Error code = 0x"
                            << hex
                                << hres
                                    << endl;

             pSvc->Release();

             pLoc->Release();

             CoUninitialize();

             return Report;

         }

         ULONG uReturn = 0;

         while (pEnumerator){

             HRESULT hr = pEnumerator->Next(WBEM_INFINITE,
                                            1,
                                            &pclsObj,
                                            &uReturn);

             if(0 == uReturn){

                 break;

             }

             VARIANT vtProp;

             //QVariant data()

             CIMTYPE Type;

             hr = pclsObj->Get((const BSTR)(Parameter.toStdWString().data()), 0, &vtProp , &Type, 0);

             try {

                 switch (Type) {
                     case CIM_STRING:

                        Report.append(QString::fromWCharArray((wchar_t*)vtProp.bstrVal));

                     break;

                     case CIM_SINT32:
                     case CIM_UINT32:
                     case CIM_BOOLEAN:

                        Report.append(QString::number((int)vtProp.bstrVal));

                     break;

                    case CIM_SINT64:

                        Report.append(QString::number((qint64)vtProp.bstrVal));

                    break;

                    case CIM_UINT64:

                        Report.append(QString::fromWCharArray((wchar_t*)vtProp.bstrVal));

                    break;

                    default:

                        Report.append(QString());

                    break;

                 }

             }

             catch (...){

                Report.append(QString());

             }

         }

         return Report;

    }

    QMap<QString, int> QGetPerfomance()
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
            memset(buf, 0, sizeof(buf));
            apEnumAccess[i]->Release();
        }
        return QmapCore;
    }

private:

    IWbemServices           *pSvc;
    IWbemLocator            *pLoc;
    IEnumWbemClassObject    *pEnumerator;
    IWbemClassObject        *pclsObj;

    byte buf[15];		// Name Core
    long lBufferSize = 15;
    long lNumBytes;

    HRESULT                 hr = S_OK;
    IWbemRefresher          *pRefresher = NULL;
    IWbemConfigureRefresher *pConfig = NULL;
    IWbemHiPerfEnum         *pEnum = NULL;
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

};

#endif // WMIMANAGER_H
