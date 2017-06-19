#ifndef MEMORYUSAGE_H
#define MEMORYUSAGE_H

#include <windows.h>
#include <QDebug>

class MemoryInfo
{

public:

    MemoryInfo()
    {


    }

    static QString GetValue(){

        MEMORYSTATUSEX statex;

        statex.dwLength = sizeof (statex);
        GlobalMemoryStatusEx (&statex);

        GlobalMemoryStatusEx (&statex);

        qDebug() << QString::number(statex.dwMemoryLoad) << endl;

//        qDebug() << QString::number(statex.ullTotalPhys - statex.ullAvailPhys) << endl;

        return QString::number(statex.dwMemoryLoad);
    }

private:

};
#endif // MEMORYUSAGE_H
