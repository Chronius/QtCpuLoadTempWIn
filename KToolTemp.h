#ifndef KTOOLTEMPGET_H
#define KTOOLTEMPGET_H

#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QMap>
#include <iostream>
class KToolTempInfo
{

public:

    KToolTempInfo(){

        this->sense_count = 0;
        QProcess process;
        QStringList Report;

        #ifdef QT_DEBUG

            CurrentPath = QDir::currentPath() + "//..//";

        #else

            CurrentPath = QDir::currentPath() + "//";

        #endif

        process.start( CurrentPath + "//ktool32.exe temp GetTempSensorCount");

        if( !process.waitForStarted() || !process.waitForFinished() ) {

            throw(std::runtime_error("err in run ktool32"));

        }

        Report = QString(process.readAllStandardOutput()).split("\r\n");
        Report.removeAt(Report.size()-1);
        QString count = "count: ";

        this->sense_count = Report[0].midRef(Report[0].lastIndexOf(count)+count.size()).toInt();
    }


    QStringList GetSensorName()
    {
        QProcess process;
        QStringList Report;
        QStringList output;
        QString sensor_name;
        QString RegStr = "name: ";

        int res;

        for (int i = 0; i < this->sense_count; i++)
        {
            process.start( CurrentPath + "//ktool32.exe temp GetTempSensorInfo " + QString::number(i));

            if( !process.waitForStarted() || !process.waitForFinished() ) {

                return Report;

            }
            Report = QString(process.readAllStandardOutput()).split("\r\n");
            for (int j = 0; j < Report.size(); j++)
            {
                qDebug()<<Report[j];
                res = Report[j].indexOf(RegStr);

                if (res != -1)
                {
                    Report[j].remove(0, RegStr.size());
                    sensor_name = Report[j];
                    output.append(sensor_name);
                }
            }
        }
        qDebug()<<output;
        return output;
    }

    QStringList GetSensorPythonName()
    {
        QProcess process;
        QStringList Report;
        QStringList output;
        QString sensor_name;

        int res;

        process.start("python " +  CurrentPath + "//CPU_Temp.py name");

        if( !process.waitForStarted() || !process.waitForFinished() ) {

            return Report;

        }
        Report = QString(process.readAllStandardOutput()).split("\r\n");
        Report.removeAt(Report.size()-1);
        for (int j = 0; j < Report.size(); j++)
        {
            qDebug()<<Report[j];

            sensor_name = Report[j];
            output.append(sensor_name);
            sense_count_python++;
        }

        qDebug()<<output;
        return output;
    }

    float GetSensorValue(int num)
    {
        QProcess process;
        QStringList Report;
        QString regStr = "value:";
        bool ok;
        int res;
        process.start( CurrentPath + "//ktool32.exe temp GetTempSensorValue " + QString::number(num));

        if( !process.waitForStarted() || !process.waitForFinished() ) {

            return 0;
        }

        Report = QString(process.readAllStandardOutput()).split("\r\n");


        for (int j = 0; j < Report.size(); j++)
        {
            qDebug() << Report[j];

            res = Report[j].indexOf(regStr);

            if (res != -1)
            {
                Report[j].remove(0, res + regStr.size());
                Report[j].toFloat(&ok);
                if(ok)
                {
                    qDebug() << Report[j].toFloat();
                    return (Report[j].toFloat())/1000;
                }
                else
                    return 0;
            }
        }
        return 0;
    }

    int GetSensorCount()
    {
        return this->sense_count;
    }

    int   GetSensorCountPython()
    {
        return this->sense_count_python;
    }


private:
    int sense_count;
    int sense_count_python = 0;
    QString CurrentPath;


};
#endif // KTOOLTEMPGET_H
