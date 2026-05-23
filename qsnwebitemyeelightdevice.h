#ifndef QSNWEBWIDGETYEELAMP_H
#define QSNWEBWIDGETYEELAMP_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QtNetwork/QHostInfo>
#include "qsnweb.h"
#include "qsnwebitemyeelightscenario.h"

class QsnWebItemYeelightDevice : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebItemYeelightDevice(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebItemYeelightDevice();
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void actionRMCode(QByteArray code);
    void setAddress(QHostAddress address, quint16 port);
    void checkConnected();
    QHostAddress deviceIP();
    quint16 devicePort();
    QString deviceID();
    void switchOff();
    void setBright(quint8 b);
    void setTemperature(quint16 t);
    QString widgetState();
    bool isConnected();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void checkTime();

private slots:
    void MSGCheck();
    void readyRead();
    void applyScenarioLamp(quint8 bright, quint16 colorTemp);
    void tcpconnected();
    void tcpdisconnected();

private:
    QList<QsnWebItemYeelightScenario*> scenarios;
    QTcpSocket *TCPSocket;
    QsnGlobalModules *mds;
    QString devID;
    QHostAddress devIP;
    QString settingIP;
    quint16 devPort;
    QList <QString> MSGqueue;

    quint8 lampbright;
    quint16 lamptemp;

    QByteArray rmOn;
    QByteArray rmOff;
    QByteArray rmSwitch;

    bool switchState;
    int connectState;
    int connectAttempt;
    int timeCheckConnectet;


    void turnOn();
    void turnOff();
    void setStateOn();
    void setStateOff();
    void getProperty();
    void readProperty(QString property);
    void readParams(QString params);
    void MSGAdd(QString msg);
    void beginConnect(QHostAddress address, quint16 port);


};

#endif // QSNWEBWIDGETYEELIGHTDESKLAMP_H
