#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitPerfCounter(); // Инициализация конструктора для получения загруженности CPU

    tmr = new QTimer(this); // Создаем объект класса QTimer и передаем адрес переменной
    tmr->setInterval(TIMER_DELAY); // Задаем интервал таймера
    connect(tmr, SIGNAL(timeout()), this, SLOT(updateTime())); // Подключаем сигнал таймера к нашему слоту
    tmr->start(); // Запускаем таймер

    SetupGraphic();
    CustomPlot(ui->plot_2);
    CustomPlot(ui->plot);


    ui->plot_2->graph(0)->setPen(QPen(QColor(0, 255, 0), 2));
    ui->plot_2->graph(0)->setBrush(QBrush(QColor(34, 177, 76, 120)));
    ui->plot_2->graph(0)->setName("Temp");

    ui->plot->addGraph(); // add Graph 0
    ui->plot->addGraph(); // add Graph 1
    ui->plot->addGraph(); // add Graph 2
    ui->plot->addGraph(); // add Graph 3
//    ui->plot->graph()->setPen(QPen(Qt::red));
//    ui->plot->graph(0)->setPen(QPen(Qt::green));
//    ui->plot->graph(1)->setPen(QPen(Qt::darkYellow));
//    ui->plot->graph(2)->setPen(QPen(Qt::darkCyan));
//    ui->plot->graph(3)->setPen(QPen(Qt::magenta));

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotRangeChanged(const QCPRange &newRange)
{
    /* Если область видимости графика меньше одного дня,
     * то отображаем часы и минуты по Оси X,
     * в противном случае отображаем дату "День Месяц Год"
     * */
//    ui->plot->xAxis->setDateTimeFormat((newRange.size() <= 86400)? "hh:mm" : "dd MMM yy");
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
//    graphic->setName("CPU Usage");       // Устанавливаем

    ui->plot_2->addPlottable(graphic2);
//    graphic2->setName("CPU2 Usage");

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

    s = PyTestRead();

    PlotTempCpu(s);
    qDebug()<< "QLOAD" << s <<endl;
    QMapCorePerfFoo = CpuPerf.QGetPerfomance();
    for (auto it = QMapCorePerfFoo.begin(); it != QMapCorePerfFoo.end(); ++it)
    {
        qDebug()<< "QMapMainDebug"<<it.key()<< it.value();
        s += "Core #" + it.key() + "  " + QString::number(it.value())+"\n";
        GraphLoadCpu.append(it.value());
    }

    ui->textBrowser->setText(s);
    MapCorePerfFoo.clear();

    PlotLoadCpu();

    ui->plot_2->graph()->addData(foo, QMapCorePerfFoo.begin().value());
    GraphLoadCpu.clear();
    ui->plot->replot();

    ui->plot_2->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->plot_2->xAxis->setRange(foo-60, foo);
    ui->plot_2->replot();
    foo += 1;

}

void MainWindow::PlotLoadCpu()
{
    ui->plot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->plot->xAxis->setRange(foo-60, foo);

    //Добавление новых значений на график
    ui->plot->graph()->addData(foo, GraphLoadCpu[0]);
    ui->plot->graph(0)->addData(foo, GraphLoadCpu[1]);
    ui->plot->graph(1)->addData(foo, GraphLoadCpu[2]);
    ui->plot->graph(2)->addData(foo, GraphLoadCpu[3]);
    ui->plot->graph(3)->addData(foo, GraphLoadCpu[4]);
}

void MainWindow::PlotTempCpu(QString str)
{
    QString load;
    load.append(str.at(2));

    qDebug()<< "LOAD" << load << endl;
}

void MainWindow::InitPerfCounter()
{
    CpuPerf.CpuPerfomanceInit();
}


QString MainWindow::PyTestRead()
{
    QProcess process;

//    QString CurrentPath = QDir::currentPath();
//    QString CurrentPath = QDir::current().absolutePath();
    QString CurrentPath = "C:\\Users\\abuzarov_bv\\Documents\\Graphicv2";
    //process.start( "python " + CurrentPath + "//CpuLoad//CPU_load.py");

    process.start( "python " + CurrentPath + "//CPU_load.py");

    if( !process.waitForStarted() || !process.waitForFinished() ) {
        return 0;
    }

    //while(!process.waitForStarted() || !process.waitForFinished());
    QString output(process.readAllStandardOutput());
    return output;
    qDebug() << process.readAllStandardError();
    qDebug() << process.readAllStandardOutput();
}

