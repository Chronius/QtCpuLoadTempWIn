#ifndef KTOOLGET_H
#define KTOOLGET_H

#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QMap>
#include <iostream>
class KToolInfo
{

public:

    KToolInfo(){

        this->sense_count = 0;
        QProcess process;
        QStringList Report;

        #ifdef QT_DEBUG

            CurrentPath = QDir::currentPath() + "//..//";

        #else

            CurrentPath = QDir::currentPath() + "//";

        #endif

        process.start( CurrentPath + "//ktool32.exe volt GetVoltageSensorCount");

        if( !process.waitForStarted() || !process.waitForFinished() ) {

            throw std::runtime_error("err in run ktool32");

        }

        Report = QString(process.readAllStandardOutput()).split("\r\n");
        Report.removeAt(Report.size()-1);
        QString count = "count: ";

        this->sense_count = Report[0].midRef(Report[0].lastIndexOf(count)+count.size()).toInt();
    }

    QStringList GetInfo()
    {
        QProcess process;
        QStringList Report;
        QStringList output;
        QString sensor_name;
        int sensor_count;
        float sensor_value;

        process.start( CurrentPath + "//ktool32.exe volt GetVoltageSensorCount");

        if( !process.waitForStarted() || !process.waitForFinished() ) {

            return Report;

        }

        Report = QString(process.readAllStandardOutput()).split("\r\n");
        Report.removeAt(Report.size()-1);
        QString count = "count: ";

        sensor_count = Report[0].midRef(Report[0].lastIndexOf(count)+count.size()).toInt();

        count = "name: ";
        int res;
        for (int i = 0; i < sensor_count; i++)
        {
            process.start( CurrentPath + "//ktool32.exe volt GetVoltageSensorInfo " + QString::number(i));

            if( !process.waitForStarted() || !process.waitForFinished() ) {

                return Report;

            }
            Report = QString(process.readAllStandardOutput()).split("\r\n");
            for (int j = 0; j < Report.size(); j++)
            {
                 qDebug()<<Report[j];
                 res = Report[j].lastIndexOf(count);
                if (res != -1)
                {
                    Report[j].remove(0, count.size());
                    sensor_name = Report[j];
                    sensor_value = GetSensorValue(i);
                    output.append(sensor_name + " " + QString::number(sensor_value));
                }
            }
        }
        qDebug()<<output;
        return output;
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
            process.start( CurrentPath + "//ktool32.exe volt GetVoltageSensorInfo " + QString::number(i));

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

    float GetSensorValue(int num)
    {
        QProcess process;
        QStringList Report;
        QString regStr = "value:";
        bool ok;
        int res;
        process.start( CurrentPath + "//ktool32.exe volt GetVoltageSensorValue " + QString::number(num));

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


private:
    int sense_count;
    QString CurrentPath;


    QString GetValueString(int num)
    {
        QProcess process;
        QStringList Report;
        QString regStr = "value:";

        int res;
        process.start( CurrentPath + "//ktool32.exe volt GetVoltageSensorValue " + QString::number(num));

        if( !process.waitForStarted() || !process.waitForFinished() ) {

            return QString();
        }

        Report = QString(process.readAllStandardOutput()).split("\r\n");
        for (int j = 0; j < Report.size(); j++)
        {
            qDebug() << Report[j];

            res = Report[j].indexOf(regStr);

            if (res != -1)
            {
                return Report[j].remove(0, res + regStr.size());
            }
        }
        return QString();
    }

void exeption(){

}

};
#endif // KTOOLGET_H
