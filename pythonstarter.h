#ifndef PYTHONSTARTER
#define PYTHONSTARTER

#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include "InfoSysTree.h"

class PythonStarter : public QObject
 {
    Q_OBJECT

private:

    QString Argc;
    QStringList Report;
    int Delay;

public:

    PythonStarter(QString argc, int delay){

        Argc = argc;
        Delay = delay;

    }

    void SetDelay(int delay){

        Delay = delay;

    }

    QStringList GetReport(){

        return Report;

    }

signals:

    void finished();

public slots:

    void PyInfoRead(){

        QProcess process;

        Report.clear();

//        QThread::sleep(Delay);

        do{

            if (Argc.isEmpty()) {

                break;


            }

#ifdef QT_DEBUG

    QString CurrentPath = QDir::currentPath();

#else

    QString CurrentPath = QDir::currentPath();

#endif

//            process.start( "python " + CurrentPath + "//hw_info.py " + Argc);
            process.start( "python " + CurrentPath + "//" + Argc);

            if( !process.waitForStarted() || !process.waitForFinished() ) {

                break;

            }

            Report = QString(process.readAllStandardOutput()).split("\n");

            Report.removeAt(Report.size()-1);

            qDebug()<<Report;
            QThread::msleep(500);

        } while(true);

        emit finished();

    }

};

class Worker : public QObject {

    Q_OBJECT

private:

    PythonStarter* PyStart;
    QThread* thread;

public:

Worker::Worker(QString arg, int delay){

    PyStart = new PythonStarter(arg,delay);
    thread = new QThread;

    PyStart->moveToThread(thread);

    connect(thread, SIGNAL(started()), PyStart, SLOT(PyInfoRead()));

    connect(PyStart, SIGNAL(finished()), thread, SLOT(quit()));

    //connect(this, SIGNAL(stopAll()), worker, SLOT(stop()));

    //connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

    //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

}

QStringList GetReport(){

    return PyStart->GetReport();

}

void Start(){

    thread->start();

}

void Stop(){

    thread->wait(10);

    delete thread;
    delete PyStart;

}

bool isProcessing(){

    return thread->isRunning();

}


signals:

    //void stopAll();

public slots:

    //void stop(){



    //}

};


#endif // PYTHONSTARTER

