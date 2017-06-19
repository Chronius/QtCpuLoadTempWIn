#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"

#include <QTimer>
#include <QTime>

#include <QString>
#include <QThread>
#include <QPushButton>

#include "CpuLoad/stdafx.h"

#include "CpuLoad/CpuPerfomance.h"
#include <QVector>
#include <iterator>
#include "InfoSysTree.h"

#include "memoryusage.h"
#include "ktoolget.h"
#include "KToolTemp.h"
#include "wmimanager.h"      т

#include "pythonstarter.h"


#define TIMER_DELAY 1000

#define SEC_TO_MSEC 1000

#define NET_MAC_FILTER ("58:75:21")
#define NET_ADAPTER_FILTER ("I210")
//#define NET_ADAPTER_FILTER ("")
#define RS_ADAPTER_FILTER (QString("RTSoft"))

class CoreLoadThread : public QThread {

    Q_OBJECT

public:

    CoreLoadThread(int CoreNum){

        this->CoreNum = CoreNum;

    }

protected:

    void run(){

        State = true;

        while(State){

            double pi;

            //Yield();


            for(double i=0; i<100000; i++){

                pi += (1/(pow(16,i)))*(4/((8*i)+1) - 2/((8*i)+4) - 1/((8*i)+5) - 1/((8*i)+6));

                if(!State){

                    break;

                }

            }

        }

    }

private:

    volatile bool State;

    int CoreNum;

public slots:

    void Stop(){

        State = false;

    }

signals:


};



class Thread : public QThread {

    Q_OBJECT

public:

    Thread(){

        HRESULT hres;

        try{


            Wmi = new WmiManager();

        }

        catch( DWORD Exp){

            Wmi = NULL;

            qDebug()<<"Error:"<<Exp;

            throw Exp;

        }


    }

protected:

    void run(){

        State = true;

        while(State){

            QStringList Name = Wmi->GetValue("Win32_PerfFormattedData_Tcpip_NetworkInterface", "Name");

            QStringList SpeedList = Wmi->GetValue("Win32_PerfFormattedData_Tcpip_NetworkInterface", "BytesTotalPerSec");

            QStringList Bandwidth = Wmi->GetValue("Win32_PerfFormattedData_Tcpip_NetworkInterface", "CurrentBandwidth");


            //QStringList Coonections = Wmi->GetValue("Win32_NetworkConnection", "ConnectionType");

            QStringList EthSpeed;

            for(int i = 0; i < Name.size(); i++) {

                if(Name[i].indexOf(NET_ADAPTER_FILTER)>=0){

                    int NetUsage = (int)(((double)(SpeedList[i].toLongLong()*8)/Bandwidth[i].toLongLong())*100);

                    NetUsage = (NetUsage>100)?100:NetUsage;

                    EthSpeed << ((NetUsage<0)?"":(QString::number(NetUsage) + " %"));

                    //EthSpeed << QString("%1 %").arg((int)(((double)SpeedList[i].toLongLong())*8));

                }

            }

            emit SetValue("Сетевые интерфейсы", EthSpeed);

            sleep(1);

        }

    }

private:

    volatile bool        State;
    WmiManager   *Wmi;

public slots:

    void Stop(){

        State = false;

    }

signals:

    void SetValue(QString Node, QStringList Values);
    void SetNameValue(QString Node, QString Value);

};


namespace Ui {

class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void AddPushButton();
    void InitTextLabel();
    void AddPushAutoButton();
private:
    Ui::MainWindow *ui;
    QCustomPlot *customPlot;    // Объявляем графическое полотно
    QCPGraph *graphic;          // Объявляем график
    QCPGraph *graphic2;
    QTimer *tmr;                // Адресная переменная таймера

    QTimer *tmrAutoLoad;

    const int tmr_s_elapsed = 1;
    int foo = 0;
    int TimesCount = 0;
    int CoreNum = 0;
    bool first_load = true;
//    QSharedPointer<QCPAxisTickerTime> timeTicker;

    QMap<QString, int> QMapCorePerfFoo;
    std::map<std::string, int> MapCorePerfFoo;
    CpuPerfomance CpuPerf;


    QStringList TreeNodeListTemp;
    QStringList TreeNodeListVolt;

    QVector<int>::iterator income_p;
    QVector <int> GraphTimes;
    QVector <int> GraphLoadCpu;
    QVector <int> GraphTempCpu;
    InfoSysTree  *InfoSysTreeView;

    QVector <int> GraphLoadCpuAvr;

    KToolInfo *Ktl;
    KToolTempInfo *KtlTemp;
    WmiManager    *Wmi;
    Worker        *EthThread;
    Worker        *TempThread;

    QVector<QPushButton *> PushButtonArray;
    QLabel *TimeLabel;

    Thread *PeriodThread;

    QMap<int, QProcess *> CoreLoadThreads;

    void resizeEvent(QResizeEvent *);

    void UpdateTreeView();

public slots:

    void updateTime();          // Слот для обновления времени на экране

private slots:
    void slotRangeChanged (const QCPRange &newRange);
    void setupPlot(QCustomPlot * plot);
    void SetupGraphic();
    void CustomPlot(QCustomPlot * plot);
    void DisplayPlot(QCustomPlot * plot);

    void InitPerfCounter();
    void PlotLoadCpu();
    void PlotTempCpu(QString str);

    void InfoSysTreeInit();
    void UpdateTreeSensor();

    QString PyInfoRead(QString argc);

    void SetCoreLoad();
    void SetAutoMode();

    void runAutoLoad();
};

#endif // MAINWINDOW_H
