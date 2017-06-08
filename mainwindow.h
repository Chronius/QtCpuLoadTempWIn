#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"

#include <QTimer>
#include <QTime>

#include <QString>

#include "CpuLoad/stdafx.h"

#include "CpuLoad/CpuPerfomance.h"
#include <QVector>
#include <iterator>
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

    QMap<QString, int> QMapCorePerfFoo;
    std::map<std::string, int> MapCorePerfFoo;
    CpuPerfomance CpuPerf;
    QVector<double>::iterator income_p;
    QVector <double> GraphTimes;
    QVector <double> GraphLoadCpu;
private slots:
    void slotRangeChanged (const QCPRange &newRange);
    void setupPlot(QCustomPlot * plot);
    void SetupGraphic();
    void CustomPlot(QCustomPlot * plot);
    void DisplayPlot(QCustomPlot * plot);
    void updateTime();          // Слот для обновления времени на экране
    void InitPerfCounter();
    void PlotLoadCpu();
    void PlotTempCpu(QString str);

    QString PyTestRead();
};

#endif // MAINWINDOW_H
