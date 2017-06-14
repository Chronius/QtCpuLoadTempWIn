#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include "SerialPort.h"
#include "network.h"
#include "KtoolGet.h"

#define RES 4

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitPerfCounter(); // Инициализация конструктора для получения загруженности CPU

    tmr = new QTimer(this); // Создаем объект класса QTimer и передаем адрес переменной
    tmr->setInterval(TIMER_DELAY); // Задаем интервал таймера

    SetupGraphic();
    CustomPlot(ui->plot_2);
    CustomPlot(ui->plot);


    ui->plot_2->addGraph();

    ui->plot_2->graph(0)->setPen(QPen(QColor(0, 255, 0), 2));
    ui->plot_2->graph(0)->setBrush(QBrush(QColor(34, 177, 76, 120)));
    ui->plot_2->graph(0)->setName("CPU Temp");

    ui->plot_2->graph(1)->setPen(QPen(QColor(255, 0, 0), 2));
    ui->plot_2->graph(1)->setBrush(QBrush(QColor(200, 34, 34, 120)));
    ui->plot_2->graph(1)->setName("SYS Temp");

    ui->plot->addGraph(); // add Graph 0
    ui->plot->addGraph(); // add Graph 1
    ui->plot->addGraph(); // add Graph 2
    ui->plot->addGraph(); // add Graph 3

    ui->plot->graph()->setPen(QPen(QColor(255, 0, 0), 2));
    ui->plot->graph(0)->setPen(QPen(QColor(0, 255, 0), 2));
    ui->plot->graph(1)->setPen(QPen(QColor(255, 128, 64), 2));
    ui->plot->graph(2)->setPen(QPen(QColor(82, 139, 152), 2));
    ui->plot->graph(3)->setPen(QPen(QColor(0, 255, 255), 2));

    ui->plot->graph()->setName("Core #1");
    ui->plot->graph(0)->setName("Core #2");
    ui->plot->graph(1)->setName("Core #3");
    ui->plot->graph(2)->setName("Core #4");
    ui->plot->graph(3)->setName("Total ");

    DisplayPlot(ui->plot);
    DisplayPlot(ui->plot_2);

    InfoSysTreeInit();


    //connect(PyStarter, SIGNAL(updateTree(QString,QString,QString)), InfoSysTreeView, SLOT(SetValue(QString,QString,QString)));
    //connect(PyStarter, SIGNAL(updatePlot(QString)), this, SLOT(updateTime(QString)));

    connect(tmr, SIGNAL(timeout()),this,SLOT(updateTime()));

    WrkMap["Память"] = new Worker("mem",0);
    WrkMap["Жесткие диски"] = new Worker("hard",0);
    WrkMap["Сетевые интерфейсы"] = new Worker("eth",0);


    tmr->start();

}

MainWindow::~MainWindow()
{

    delete ui;

}

void MainWindow::resizeEvent(QResizeEvent *)
{
    QList<int> sizes;
    sizes << ui->centralWidget->width()/3 << 2 * ui->centralWidget->width()/3;
    ui->splitter_2->setSizes(sizes);


#ifndef QT_DEBUG

    ui->treeWidget->setColumnWidth(0,3 * ui->treeWidget->width()/4);
    ui->treeWidget->setColumnWidth(1,ui->treeWidget->width()/4);

#endif

}

void MainWindow::UpdateTreeView(){

    qDebug()<<"Update tree view";

    foreach (QString key, WrkMap.keys()) {

        if(!WrkMap[key]->isProcessing()){

            QStringList report = WrkMap[key]->GetReport();

            QStringList outrep;

            int Delay = 0;

            if((key == "Жесткие диски")||
                (key == "Память")||
                    (key == "Сетевые интерфейсы")){

                for(int i = 1; i < report.length(); i+=2){

                    outrep.append(report[i]);

                }

            } else {

                outrep = report;

            }

            InfoSysTreeView->SetValue(key, outrep);

            if(key == "Жесткие диски"){

                continue;

            }
            if(key == "Память"){

                Delay = 8;

            }

            if(key == "Сетевые интерфейсы"){

                continue;

            }

            qDebug()<<"Starting script:" << key;

            WrkMap[key]->Start(Delay);

        }

    }




}


void MainWindow::slotRangeChanged(const QCPRange &newRange)
{
    /* Если область видимости графика меньше одного дня,
     * то отображаем часы и минуты по Оси X,
     * в противном случае отображаем дату "День Месяц Год"
     * */
//    ui->plot->xAxis->setDateTimeFormat((newRange.size() <= 86400)? "hh:mm" : "dd MMM yy");
}
void MainWindow::InfoSysTreeInit()
{
    QList<TreeNode> TreeNodeList;

    QString output;
    QStringList listOutput;
    TreeNode Node;

    /*-----------------------------------------------------------------
     * DEBUG LINE
     *-----------------------------------------------------------------*/

//    KtlTemp = new KToolTempInfo();
//    QStringList Ktoollist = KtlTemp->GetSensorName();

//    if(Ktoollist.size())
//    {
//        listOutput = output.split("\r\n");

//        Node.Name = "Debug";
//        Node.Icon = QIcon(":/icon/cpu.png");

//        Node.List = Ktoollist;

//        TreeNodeList.append(Node);
//        Node.List.clear();
//    }

    /*-----------------------------------------------------------------
     * CPU
     *-----------------------------------------------------------------*/

    output = PyInfoRead("cpu");

    if (output != "")
    {
        listOutput = output.split("\r\n");

        Node.Name = "Процессор";
        Node.Icon = QIcon(":/icon/cpu.png");
        CoreNum = listOutput[1].toInt();

        for(int core_num = 0; core_num < CoreNum; core_num++)
        {
            Node.List << listOutput[0];
        }
        TreeNodeList.append(Node);
        Node.List.clear();
    }

    /*-----------------------------------------------------------------
     * RAM Memory
     *-----------------------------------------------------------------*/
    output = PyInfoRead("mem");
    listOutput = output.split("\r\n");

    if (output != "")
    {
        Node.Name = "Память";
        Node.Icon = QIcon(":/icon/Memory.png");

        for(int i=0; i < listOutput.size() - 1; i+=2){

            Node.List << listOutput[i];

        }
        TreeNodeList.append(Node);
        Node.List.clear();
    }

    /*-----------------------------------------------------------------
     * HARD DRIVE
     *-----------------------------------------------------------------*/
    output = PyInfoRead("hard");
    listOutput = output.split("\r\n");

    if (output != "")
    {
        Node.Name = "Жесткие диски";
        Node.Icon = QIcon(":/icon/hdd.png");

        for(int i=0; i < listOutput.size() - 1; i+=2){

            Node.List << listOutput[i];

        }
        TreeNodeList.append(Node);
        Node.List.clear();
    }

    /*-----------------------------------------------------------------
     * ETHERNET
     *-----------------------------------------------------------------*/
    output = PyInfoRead("eth");
    listOutput = output.split("\r\n");

    if (output != "")
    {
        listOutput.removeAt(listOutput.size()-1);

        Node.Name = "Сетевые интерфейсы";
        Node.Icon = QIcon(":/icon/network.png");

        for(int i=0; i < listOutput.size() - 1; i+=2){

            Node.List << listOutput[i];

        }

        TreeNodeList.append(Node);
        Node.List.clear();
    }


    /*-----------------------------------------------------------------
     * Serial Port
     *-----------------------------------------------------------------*/
    SerialPort SPort;
    QStringList SPortList = SPort.getListPorts(QString("RTSoft"));
    if(SPortList.size()){

        Node.Name = "Последовательные интерфейсы RS422/485";
        Node.Icon = QIcon(":/icon/serial_port.png");
        Node.List = SPortList;

        TreeNodeList.append(Node);

    }
    Node.List.clear();

    /*-----------------------------------------------------------------
     * Temperature sensor
     *-----------------------------------------------------------------*/

    KtlTemp = new KToolTempInfo();
    QStringList Ktoollist = KtlTemp->GetSensorName();

    if(Ktoollist.size())
    {
        listOutput = output.split("\r\n");

        Node.Name = "Датчики температуры";
        Node.Icon = QIcon(":/icon/Thermometer.ico");

        Node.List = Ktoollist;

        TreeNodeList.append(Node);
        Node.List.clear();
    }

//    output = PyInfoRead("temp");
//    listOutput = output.split("\r\n");

//    if (output != "")
//    {
//        Node.Name = "Датчики температуры";
//        Node.Icon = QIcon(":/icon/Thermometer.ico");

//        for(int i=0; i < listOutput.size() - 1; i+=2){

//            Node.List << listOutput[i];// + "\t" + listOutput[i + 1];

//        }
//        TreeNodeList.append(Node);

//        TreeNodeListTemp = Node.List;
//        Node.List.clear();
//    }
    /*-----------------------------------------------------------------
     * Voltage sensor
     *-----------------------------------------------------------------*/


    Ktl = new KToolInfo();
    QStringList Templist = Ktl->GetSensorName();

    if(Templist.size())
    {
        Node.Name = "Датчики напряжения питания";
        Node.Icon = QIcon(":/icon/cpu.png");

        Node.List = Templist;

        TreeNodeList.append(Node);
        TreeNodeListTemp = Node.List;
        Node.List.clear();
    }

//        output = PyInfoRead("volt");
//        listOutput = output.split("\r\n");
//    if (output != "")
//    {
//        Node.Name = "Датчики напряжения питания";
//        Node.Icon = QIcon(":/icon/cpu.png");

//        for(int i=0; i < listOutput.size() - 1; i+=2){

//            Node.List << listOutput[i];// + "\t" + listOutput[i + 1];

//        }
//        TreeNodeList.append(Node);
//        TreeNodeListVolt = Node.List;
//        Node.List.clear();
//    }
        InfoSysTreeView = new InfoSysTree(ui->treeWidget, TreeNodeList);


}

/******************************************************************************************************************/
/* Setup the plot area */
/******************************************************************************************************************/
void MainWindow::SetupGraphic()
{
    // Инициализируем график и привязываем его к Осям
    graphic = new QCPGraph(ui->plot->xAxis, ui->plot->yAxis);
    graphic2 = new QCPGraph(ui->plot_2->xAxis, ui->plot_2->yAxis);

    ui->plot->addPlottable(graphic);  // Устанавливаем график на полотно

    ui->plot_2->addPlottable(graphic2);

    graphic->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
    graphic->setLineStyle(QCPGraph::lsLine);

    graphic2->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
    graphic2->setLineStyle(QCPGraph::lsLine);
}

void MainWindow::setupPlot(QCustomPlot * plot)
{
    plot->clearItems();                                                              // Remove everything from the plot

    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    plot->setBackground(plotGradient);

    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));
    plot->axisRect()->setBackground(axisRectGradient);

    plot->setBackground(QBrush(QColor(48,47,47)));
}

void MainWindow::CustomPlot(QCustomPlot * plot)
{
    plot->setInteraction(QCP::iRangeZoom,true);   // Включаем взаимодействие удаления/приближения
    plot->setInteraction(QCP::iRangeDrag, true);  // Включаем взаимодействие перетаскивания графика
    plot->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
    plot->axisRect()->setRangeZoom(Qt::Horizontal);   // Включаем удаление/приближение только по горизонтальной оси
    plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);   // Подпись координат по Оси X в качестве Даты и Времени
    plot->xAxis->setDateTimeFormat("hh:mm:ss");  // Устанавливаем формат даты и времени

    plot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    plot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));

    plot->xAxis2->setVisible(true);
    plot->yAxis2->setVisible(true);
    plot->xAxis2->setTicks(false);
    plot->yAxis2->setTicks(false);
    plot->xAxis2->setTickLabels(false);
    plot->yAxis2->setTickLabels(false);

    plot->xAxis->setTickLabelColor(QColor(Qt::white));
    plot->yAxis->setTickLabelColor(QColor(Qt::white));
    plot->legend->setVisible(true);   //Включаем Легенду графика
    // Устанавливаем Легенду в левый верхний угол графика
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);

    setupPlot(plot);
    /* Подключаем сигнал от Оси X об изменении видимого диапазона координат
     * к СЛОТу для переустановки формата времени оси.
     * */
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            this, SLOT(slotRangeChanged(QCPRange)));
}

void MainWindow::DisplayPlot(QCustomPlot * plot)
{
    foo = QDateTime::currentDateTime().toTime_t();

    plot->xAxis->setDateTimeFormat("hh:mm:ss");
    plot->xAxis->setRange(foo, foo+60);

    plot->yAxis->setRange(0, 100);
    plot->replot();           // Отрисовываем график
}

void MainWindow::updateTime()
{
    ui->label->setText(QTime::currentTime().toString());

    foo = QDateTime::currentDateTime().toTime_t();

    QString s;

    /*if (s != "")
    {
        PlotTempCpu(s);
    }*/

    QMapCorePerfFoo = CpuPerf.QGetPerfomance();
    for (auto it = QMapCorePerfFoo.begin(); it != QMapCorePerfFoo.end(); ++it)
    {
        s += "Core #" + it.key() + "  " + QString::number(it.value())+"\n";
        GraphLoadCpu.append(it.value());
    }
    MapCorePerfFoo.clear();

    PlotLoadCpu();

    GraphLoadCpu.clear();
    ui->plot->replot();

    ui->plot_2->xAxis->setRange(foo-60, foo);
    ui->plot_2->replot();
    foo += 1;

    UpdateTreeSensor();

    UpdateTreeView();
}

void MainWindow::PlotLoadCpu()
{
    ui->plot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->plot->xAxis->setRange(foo-60, foo);

    //Добавление новых значений на график
//    ValList << QString::number((int)GraphLoadCpu[0]) + " %";

//    ui->plot->graph(0)->addData(foo, GraphLoadCpu[1]);
//    ValList << QString::number((int)GraphLoadCpu[1]) + " %";

//    ui->plot->graph(1)->addData(foo, GraphLoadCpu[2]);
//    ui->plot->graph(2)->addData(foo, GraphLoadCpu[3]);
//    ui->plot->graph(3)->addData(foo, GraphLoadCpu[4]);

    QStringList ValList;

    for(int core_num = 0; core_num < CoreNum; core_num++)
    {

       if(core_num == 0){

            ui->plot->graph()->addData(foo, GraphLoadCpu[core_num]);

       }else{

            ui->plot->graph(core_num-1)->addData(foo, GraphLoadCpu[core_num]);

       }

        ValList << QString::number((int)GraphLoadCpu[core_num]) + " %";

    }

    InfoSysTreeView->SetValue("Процессор",ValList);

}

void MainWindow::UpdateTreeSensor()
{
    QStringList ValList;
    double value;

    for(int i = 0; i < Ktl->GetSensorCount(); i++)
    {
        ValList << QString::number(Ktl->GetSensorValue(i));
    }

    InfoSysTreeView->SetValue("Датчики напряжения питания", ValList);
    ValList.clear();

    for(int i = 0; i < KtlTemp->GetSensorCount(); i++)
    {
        value = KtlTemp->GetSensorValue(i);
        ValList << QString::number(value);
        GraphTempCpu.append(value);
    }

    ui->plot_2->graph()->addData(foo, GraphTempCpu[0]);
    ui->plot_2->graph(0)->addData(foo, GraphTempCpu[1]);

    GraphTempCpu.clear();

    InfoSysTreeView->SetValue("Датчики температуры", ValList);

}

void MainWindow::PlotTempCpu(QString str)
{
    str.chop(1);
    QStringList load = str.split("\r\n");;
    QString listString;
    double dec;
    bool ok;
    for (int i = 0; i < load.size()-1; i += 2)
    {
          dec = load[i+1].toDouble(&ok);
          if (ok)
          {
              qDebug()<< "PlotTempCpu " + load[i] << dec;
              GraphTempCpu.append(dec);

//              emit updateTree("Датчики температуры", load[i], QString::number(dec));

//              emit updateTree("Датчики температуры", load[i], QString::number(dec));
//              emit updateTree("Датчики температуры", load[i], QString::number(10));
          }
    }
    ui->plot_2->graph()->addData(foo, GraphTempCpu[0]);
    ui->plot_2->graph(0)->addData(foo, GraphTempCpu[1]);

    GraphTempCpu.clear();
}

void MainWindow::InitPerfCounter()
{
    CpuPerf.CpuPerfomanceInit();
}


QString MainWindow::PyInfoRead(QString argc)
{
    QString output = "";
    QProcess process;

    //QString CurrentPath = "C:\\Users\\abuzarov_bv\\Documents\\Graphicv2";

#ifdef QT_DEBUG

    QString CurrentPath = QDir::currentPath() + "//..//";

#else

    QString CurrentPath = QDir::currentPath() + "//..//";

#endif

    process.start( "python " + CurrentPath + "//hw_info.py " + argc);
    if( !process.waitForStarted() || !process.waitForFinished() ) {
        return output;
    }
    output.append(process.readAllStandardOutput());
    qDebug() << "PyInfoRead" <<output << endl;
    return output;
}
