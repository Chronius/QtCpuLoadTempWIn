#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"

#include <QTimer>
#include <QTime>

#include <QString>
#include <QThread>

#include "CpuLoad/stdafx.h"

#include "CpuLoad/CpuPerfomance.h"
#include <QVector>
#include <iterator>
#include "InfoSysTree.h"
#include "pythonstarter.h"
#include "ktoolget.h"
#include "KToolTemp.h"


#define TIMER_DELAY 1000

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QCustomPlot *customPlot;    // Объявляем графическое полотно
    QCPGraph *graphic;          // Объявляем график
    QCPGraph *graphic2;
    QTimer *tmr;                // Адресная переменная таймера

    int foo = 0;
    int TimesCount = 0;
    int CoreNum = 0;

    QMap<QString, int> QMapCorePerfFoo;
    std::map<std::string, int> MapCorePerfFoo;
    CpuPerfomance CpuPerf;


    QStringList TreeNodeListTemp;
    QStringList TreeNodeListVolt;

    QVector<double>::iterator income_p;
    QVector <double> GraphTimes;
    QVector <double> GraphLoadCpu;
    QVector <double> GraphTempCpu;
    InfoSysTree  *InfoSysTreeView;

    Worker *wrk;
    KToolInfo *Ktl;
    KToolTempInfo *KtlTemp;

    QMap<QString, Worker*> WrkMap;

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

};

#endif // MAINWINDOW_H
