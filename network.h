#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QNetworkInterface>
#include <QNetworkConfigurationManager>
#include <QUuid>
#include <QDebug>

class NetWork : public QObject
{
    Q_OBJECT
public:
    explicit NetWork(QObject *parent = 0){

        QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    }

    QStringList getListNet(QString &Manufacture){

        QStringList List;

        QString items;

        QNetworkConfigurationManager mngr;
        QList<QNetworkConfiguration> conf = mngr.allConfigurations();

        for (int i = 0; i < conf.size(); i++)
        {
            qDebug()<< "name" <<conf[i].identifier() <<endl;

        }

        QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

        for (int i = 0; i < ifaces.size(); i++)
        {

            qDebug()<< ifaces[i].name() << "Name" << ifaces[i].humanReadableName() <<endl;

        }

        return List;
    }

signals:

public slots:
};

#endif // NETWORK_H
