#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = 0){


    }

    QStringList getListPorts(QString &Manufacture){

        QStringList List;

        QList<QSerialPortInfo> SerialPortInfo = QSerialPortInfo::availablePorts();

        foreach (QSerialPortInfo SerialInfo, SerialPortInfo) {

            if((Manufacture.isEmpty())||(Manufacture == SerialInfo.manufacturer())){

                List << QString("%1 (%2)").arg(SerialInfo.description()).arg(SerialInfo.portName());

            }

        }

        return List;

    }

signals:

public slots:
};

#endif // SERIALPORT_H
