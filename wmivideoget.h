#ifndef WMIVIDEOGET_H
#define WMIVIDEOGET_H

#include <QObject>
#include <QDebug>
#include <QByteArray>
#include <QException>
#include <qt_windows.h>

#include <wbemcli.h>

//class WmiException : public QException {

//public:

//    void raise() const { throw *this; }

//    WmiException *clone() const { return new WmiException(*this); }

//};

class WmiRootManager {

public:

    WmiRootManager(){

        pSvc = NULL;

        HRESULT hres;

//        hres =  CoInitializeSecurity(NULL,
//                                      -1,                          // COM authentication
//                                      NULL,                        // Authentication services
//                                      NULL,                        // Reserved
//                                      RPC_C_AUTHN_LEVEL_NONE,   // Default authentication
//                                      RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
//                                      NULL,                        // Authentication info
//                                      EOAC_NONE,                   // Additional capabilities
//                                      NULL                         // Reserved
//                                      );

//         if (FAILED(hres))
//         {
//             qDebug() << "Failed to initialize security. Error code = 0x"
//                        << hex
//                            << hres
//                                << endl;
//             CoUninitialize();

//             throw hres;

//         }

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
         hres = pLoc->ConnectServer((const BSTR)(QString("root\\wmi").toStdWString().data()), // Object path of WMI namespace
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

         qDebug() << "Connected to ROOT\\WMI WMI namespace" << endl;


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
          return;   // Program successfully completed.
    }

    ~WmiRootManager(){

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
                             Report.append(((ulong)vtProp.boolVal)?"Activefoo":"InActivefoo");
                      break;
                     case CIM_BOOLEAN:
                            Report.append(((ulong)vtProp.boolVal)?"Active":"InActive");
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



private:

    IWbemServices           *pSvc;
    IWbemLocator            *pLoc;
    IEnumWbemClassObject    *pEnumerator;
    IWbemClassObject        *pclsObj;


};


#endif // WMIVIDEOGET_H
