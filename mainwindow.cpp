#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include "SerialPort.h"
#include "network.h"
#include "KtoolGet.h"
#include "wmivideoget.h"

#define S_TO_LOAD 30
#define S_TO_PAUSE 30
#define S_TO_DELAY 10


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //InitPerfCounter(); // Инициализация конструктора для получения загруженности CPU

    tmr = new QTimer(this); // Создаем объект класса QTimer и передаем адрес переменной
    tmr->setInterval(TIMER_DELAY); // Задаем интервал таймера

    SetupGraphic();
    CustomPlot(ui->plot_2);
    CustomPlot(ui->plot);


    ui->plot_2->addGraph();
    ui->plot_2->addGraph();

    ui->plot_2->graph(0)->setPen(QPen(QColor(0, 255, 0), 2));
    ui->plot_2->graph(0)->setBrush(QBrush(QColor(34, 177, 76, 120)));
    ui->plot_2->graph(0)->setName("Core 1");

    ui->plot_2->graph(1)->setPen(QPen(QColor(255, 0, 0), 2));
    ui->plot_2->graph(1)->setBrush(QBrush(QColor(200, 34, 34, 120)));
    ui->plot_2->graph(1)->setName("Core 2");

    ui->plot_2->graph(2)->setPen(QPen(QColor(0, 0, 255), 2));
    ui->plot_2->graph(2)->setBrush(QBrush(QColor(34, 34, 200, 120)));
    ui->plot_2->graph(2)->setName("Package");

    {
        ui->plot_2->xAxis2->setLabel("Temperature");
        ui->plot_2->xAxis2->setLabelColor(QColor(255,255,255));

        ui->plot->xAxis2->setLabel("CPU Load");
        ui->plot->xAxis2->setLabelColor(QColor(255,255,255));
    }

    ui->plot->addGraph(); // add Graph 0
    ui->plot->addGraph(); // add Graph 1
    ui->plot->addGraph(); // add Graph 2
    ui->plot->addGraph(); // add Graph 3

    ui->plot->graph()->setPen(QPen(QColor(255, 0, 0), 2));
    ui->plot->graph(0)->setPen(QPen(QColor(0, 255, 0), 2));
    ui->plot->graph(1)->setPen(QPen(QColor(255, 128, 64), 2));
    ui->plot->graph(2)->setPen(QPen(QColor(82, 139, 152), 2));
    ui->plot->graph(3)->setPen(QPen(QColor(0, 255, 255), 2));

    ui->plot->graph()->setName("Average ");
    ui->plot->graph(0)->setName("Core #1");
    ui->plot->graph(1)->setName("Core #2");
    ui->plot->graph(2)->setName("Core #3");
    ui->plot->graph(3)->setName("Core #4");

    DisplayPlot(ui->plot);
    DisplayPlot(ui->plot_2);

    try{

        Wmi = new WmiManager();

    }

    catch( DWORD Exp){

        Wmi = NULL;

        qDebug()<<"Error:"<<Exp;


    }


    AddPushAutoButton();

    InfoSysTreeInit();


    connect(tmr, SIGNAL(timeout()),this,SLOT(updateTime()));

    tmr->start();

    EthThread = new Worker("hw_info.py eth", 0);

    EthThread->Start();

    TempThread = new Worker("CPU_Temp.py value", 0);

    TempThread->Start();


}

MainWindow::~MainWindow()
{

    delete ui;

}

void MainWindow::AddPushButton(){

    QPushButton *pushButton = new QPushButton(ui->centralWidget);

    int CoreCount=(int)PushButtonArray.size();

    pushButton->setObjectName(QString("Core%1").arg(CoreCount));

    pushButton->setCheckable(true);

    pushButton->setMaximumWidth(50);

    pushButton->setMaximumHeight(50);

    pushButton->setIcon(QIcon(QString(":/icon/core%1.png").arg(CoreCount)));

    pushButton->setIconSize(QSize(50,50));

    ui->horizontalLayout->addWidget(pushButton);

    PushButtonArray.append(pushButton);

    connect(pushButton,SIGNAL(released()),this,SLOT(SetCoreLoad()));

}

void MainWindow::SetCoreLoad(){

    QObject* obj=QObject::sender();

    if (QPushButton *tb=qobject_cast<QPushButton *>(obj)){

        QString ObName = tb->objectName();

        int CoreNum = QString(ObName.remove("Core")).toInt();

        if(tb->isChecked()){

            qDebug()<<tb->objectName()<<" "<<CoreNum << "start";

            CoreLoadThreads[CoreNum]->start(QString(QDir::currentPath() + "//core_load.exe %1").arg(CoreNum));

        }else{

            qDebug()<<tb->objectName()<<" "<<CoreNum << "stop";

            CoreLoadThreads[CoreNum]->kill();

            //CoreLoadThreads[CoreNum]->waitForFinished(1000);

        }

    }

}

void MainWindow::AddPushAutoButton(){

    QPushButton *pushButton = new QPushButton(ui->centralWidget);

    pushButton->setObjectName(QString("Auto"));

    pushButton->setCheckable(true);

    pushButton->setMaximumWidth(50);

    pushButton->setMaximumHeight(50);


    pushButton->setText("AUTO");

    pushButton->setIconSize(QSize(50,50));

    ui->horizontalLayout->addWidget(pushButton);

    connect(pushButton,SIGNAL(released()),this,SLOT(SetAutoMode()));


    tmrAutoLoad = new QTimer;

    tmrAutoLoad->setInterval(S_TO_DELAY * tmr_s_elapsed * SEC_TO_MSEC);

    connect(tmrAutoLoad, SIGNAL(timeout()),this,SLOT(runAutoLoad()));

}
/*Слот отвечающий за нажатие на кнопку AUTO*/
void MainWindow::SetAutoMode(){

    QObject* obj=QObject::sender();
    if (QPushButton *tb=qobject_cast<QPushButton *>(obj)){

        if(tb->isChecked())
        {

            qDebug()<<tb->objectName()<<" "<< "start";

            for (int i = 0; i < PushButtonArray.size(); i++)
            {
                if (PushButtonArray[i]->isChecked())
                {
                    PushButtonArray[i]->setChecked(false);
                    emit PushButtonArray[i]->released();
                }

                PushButtonArray[i]->setEnabled(false);

            }

            tmrAutoLoad->start();

        }
        else
        {

            tmrAutoLoad->stop();

            for (int i = 0; i < PushButtonArray.size(); i++)
            {
                if (PushButtonArray[i]->isChecked())
                {
                    PushButtonArray[i]->setChecked(false);
                    emit PushButtonArray[i]->released();
                }

                PushButtonArray[i]->setEnabled(true);

            }

            qDebug()<<tb->objectName()<<" "<< "stop";

        }

    }


}
/*Слот для загрузки в авто режиме*/
void MainWindow::runAutoLoad()
{
    qDebug()<<"runAutoLoad";
    if(tmrAutoLoad->interval() == S_TO_DELAY * tmr_s_elapsed * SEC_TO_MSEC)
    {
        for (int i  = 0; i < CoreNum; i++)
        {
            if (CoreLoadThreads[i]->state() == QProcess::ProcessState::Running)
            {
                continue;
            }
            else
            {

                if (i == CoreNum - 1)
                {
                    tmrAutoLoad->setInterval(S_TO_LOAD * tmr_s_elapsed * SEC_TO_MSEC);
                }

                PushButtonArray[i]->setChecked(true);
                emit PushButtonArray[i]->released();

                break;

            }
        }
    }
    else
    {
        tmrAutoLoad->setInterval((S_TO_DELAY+1) * tmr_s_elapsed * SEC_TO_MSEC);
        for (int i  = 0; i < CoreNum; i++)
        {
            if (i == CoreNum - 1)
            {
                tmrAutoLoad->setInterval(S_TO_DELAY * tmr_s_elapsed * SEC_TO_MSEC);
                tmrAutoLoad->stop();
                tmrAutoLoad->start();
                first_load != first_load;
            }
            if (CoreLoadThreads[i]->state() == QProcess::ProcessState::Running)
            {
                PushButtonArray[i]->setChecked(false);
                emit PushButtonArray[i]->released();
                break;
            }
        }
    }
}

void MainWindow::InitTextLabel(){

    TimeLabel = new QLabel(ui->centralWidget);
    TimeLabel->setMaximumSize(QSize(16777215, 50));
    QFont font1;
    font1.setFamily(QStringLiteral("Arial"));
    font1.setPointSize(24);
    TimeLabel->setFont(font1);
    TimeLabel->setTextFormat(Qt::AutoText);
    ui->horizontalLayout->addWidget(TimeLabel);

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

void MainWindow::UpdateTreeView()
{

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
//    WmiRootManager WmiRoot;
//    listOutput = WmiRoot.GetValue("WmiMonitorConnectionParams", "Active");
//    listOutput << WmiRoot.GetValue("WmiMonitorConnectionParams", "InstanceName");
//    listOutput << WmiRoot.GetValue("WmiMonitorConnectionParams", "VideoOutputTechnology");

//    qDebug() << listOutput << endl;

    /*-----------------------------------------------------------------
     * CPU
     *-----------------------------------------------------------------*/

    QString CPUName = Wmi->GetValue("Win32_Processor", "Name")[0];

    CoreNum = Wmi->GetValue("Win32_Processor", "NumberOfLogicalProcessors")[0].toInt();

    Node.Name = "Процессор";
    Node.Icon = QIcon(":/icon/cpu.png");

    for(int core_num = 0; core_num < CoreNum; core_num++){

        Node.List << CPUName;
        AddPushButton();

        CoreLoadThreads[core_num] = new QProcess();

    }

    TreeNodeList.append(Node);
    Node.List.clear();

    InitTextLabel();

    TimeLabel->setText(QTime::currentTime().toString());

    /*-----------------------------------------------------------------
     * RAM Memory
     *-----------------------------------------------------------------*/
    //output = PyInfoRead("mem");
    //listOutput = output.split("\r\n");

    MemoryInfo Meminfo;
    Meminfo.GetValue();

    Node.Name = "Память";
    Node.Icon = QIcon(":/icon/Memory.png");

    QStringList Name = Wmi->GetValue("Win32_PhysicalMemory", "Manufacturer");


    QStringList Capacity = Wmi->GetValue("Win32_PhysicalMemory", "Capacity");

    for(int i=0; i < Name.size(); i++){

        if (Name[i].indexOf("0112") >= 0)
        {
            Name[i] = "DRAM Apacer Technology";
        }

        qDebug()<< Capacity[i].toULongLong()/(1024*1024*1024);

        Node.List << QString("%1 %2 Gb").arg(Name[i]).arg((int)(Capacity[i].toULongLong()/(1024*1024*1024)));

    }

    TreeNodeList.append(Node);
    Node.List.clear();

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


        Name = Wmi->GetValue("Win32_PerfFormattedData_Tcpip_NetworkInterface", "Name");

        QStringList EthName;

        for(int i = 0; i < Name.size(); i++) {

            if(Name[i].indexOf(NET_ADAPTER_FILTER)>=0){

                EthName << Name[i];

            }

        }

        Node.Name = "Сетевые интерфейсы";
        Node.Icon = QIcon(":/icon/network.png");

        Node.List = EthName;

        TreeNodeList.append(Node);
        Node.List.clear();


    /*-----------------------------------------------------------------
     * Serial Port
     *-----------------------------------------------------------------*/
    SerialPort SPort;
    QStringList SPortList = SPort.getListPorts(QString(RS_ADAPTER_FILTER));
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

    do {

        try {

            KtlTemp = new KToolTempInfo();

        }

        catch (...) {

            KtlTemp = NULL;

            break;
        }


        QStringList Ktoollist = KtlTemp->GetSensorName();
        Ktoollist = KtlTemp->GetSensorPythonName();

        if(Ktoollist.size())
        {
            listOutput = output.split("\r\n");

            Node.Name = "Датчики температуры";
            Node.Icon = QIcon(":/icon/Thermometer.ico");

            Node.List = Ktoollist;

            TreeNodeList.append(Node);
            Node.List.clear();
        }

    }

    while(false);


    /*-----------------------------------------------------------------
     * Voltage sensor
     *-----------------------------------------------------------------*/

    do {

        try {

            Ktl = new KToolInfo();

        }

        catch (...) {

            Ktl = NULL;

            break;
        }

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

    }while(false);

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

    ui->plot->yAxis->setRange(0, 125);
    ui->plot->yAxis->setRange(0, 125);

    ui->plot_2->yAxis->setRange(20, 100);
    ui->plot_2->yAxis->setRange(20, 100);
    plot->replot();           // Отрисовываем график
}

void MainWindow::updateTime()
{
    /*DEBUG LINE*/
    {
//        QStringList output = EthThread->GetReport();
        QStringList Report = EthThread->GetReport();

        qDebug()<< "EthThread"<<Report;

//        Report.removeAt(Report.size()-1);

        QStringList Name;
        QStringList SpeedList;
        QStringList Bandwidth;

        for (int i = 0; i < Report.size(); i++)
        {
            Name << Report[i++];
            SpeedList << Report[i++];
            Bandwidth << Report[i];
        }

        QStringList EthSpeed;

        for(int i = 0; i < Name.size(); i++) {

            if(Name[i].indexOf(NET_ADAPTER_FILTER)>=0){

                int NetUsage = (int)(((double)(SpeedList[i].toLongLong()*8)/Bandwidth[i].toLongLong())*100);

                NetUsage = (NetUsage>100)?100:NetUsage;

                EthSpeed << ((NetUsage<0)?"Не активен":(QString::number(NetUsage) + " %"));

            }
        }

        InfoSysTreeView->SetValue("Сетевые интерфейсы", EthSpeed);
    }


    TimeLabel->setText(QTime::currentTime().toString());
//    TimeLabel->setText(QString::number(foo));


    QMap <QString, int> QMapCorePerfFoo = Wmi->QGetPerfomance();
    for (auto it = QMapCorePerfFoo.begin(); it != QMapCorePerfFoo.end(); ++it)
    {
        GraphLoadCpu.append(it.value());
    }
    QMapCorePerfFoo.clear();


    PlotLoadCpu();

    UpdateTreeSensor();

    ui->plot->replot();

    ui->plot_2->xAxis->setRange(foo-60, foo);
    ui->plot_2->replot();
    foo += 1;

    //UpdateTreeView();

    InfoSysTreeView->SetNameValue("Память", MemoryInfo::GetValue() + " %");

    InfoSysTreeView->SetNameValue("Процессор", QString::number(GraphLoadCpu[CoreNum]) + " %");


    GraphLoadCpu.clear();
}

void MainWindow::PlotLoadCpu()
{
    ui->plot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->plot->xAxis->setRange(foo-60, foo);


    QStringList ValList;


        ui->plot->graph()->addData(foo, GraphLoadCpu[CoreNum]);

        for(int core_num = 0; core_num < CoreNum; core_num++)
        {

            ui->plot->graph(core_num)->addData(foo, GraphLoadCpu[core_num]);

            ValList << QString::number((int)GraphLoadCpu[core_num]) + " %";

        }

    InfoSysTreeView->SetValue("Процессор",ValList);

}

void MainWindow::UpdateTreeSensor()
{
    QStringList ValList;
    double value;

    if(Ktl){

        for(int i = 0; i < Ktl->GetSensorCount(); i++)
        {
            ValList << QString::number(Ktl->GetSensorValue(i));
        }

    }

    InfoSysTreeView->SetValue("Датчики напряжения питания", ValList);
    ValList.clear();



    QStringList TempReport = TempThread->GetReport();

    qDebug()<< "TempThread"<<TempReport;

    if(KtlTemp){

        for(int i = 0; i < KtlTemp->GetSensorCountPython(); i++)
        {
            value = TempReport[i].toDouble();
            ValList << QString::number(value);
            GraphTempCpu.append(value);

//            value = KtlTemp->GetSensorValue(i);
//            ValList << QString::number(value);
//            GraphTempCpu.append(value);
        }

        ui->plot_2->graph(0)->addData(foo, GraphTempCpu[0]);
        ui->plot_2->graph(1)->addData(foo, GraphTempCpu[1]);
        ui->plot_2->graph(2)->addData(foo, GraphTempCpu[2]);

//        ui->plot_2->graph()->addData(foo, GraphTempCpu[0]);
//        ui->plot_2->graph(0)->addData(foo, GraphTempCpu[1]);

    }

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
