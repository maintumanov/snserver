#ifndef QSNUDPSERVER_H
#define QSNUDPSERVER_H

#define QSNUDPSERVER_TIMEOUT 300

#include <QObject>
#include <QtCore>
#include <QUdpSocket>
#include "qsnshapes.h"

class QsnUDPserver : public QObject
{
    Q_OBJECT

public:
    struct clientRec {
        QHostAddress ipAddress;
        quint16 port;
        quint16 devAddress;
        QDateTime checkTime;
        QDateTime connectTime;
        QString lastError;
        QString name;
    };

    struct clientIP {
        QHostAddress ipAddress;
        quint16 port;
    };

    QList<clientRec> clients;

    explicit QsnUDPserver(QObject *parent = Q_NULLPTR);
    ~QsnUDPserver();
    bool isEnabled();
    void setEnabled(bool enable);
    quint16 getPort();
    void setPort(quint16 port);
    QString getKey();
    void setKey(QString key);
    bool isConnected();
    QString error();
    void setLogEnable(bool enable);
    bool logEnable();

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void writeLog(QString msg, QString name, QString title);
    void stateChanged();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void actionConnect(quint16 address);
    void actionDisconnect();
private slots:
    void readyRead();
    void connectedChange(bool state);

private:
    bool isLog;
    bool serverEnable;
    quint16 serverPort;
    quint16 deviceAddress;
    QString encryptKey;
    QUdpSocket *serverSocket;
    QList<clientRec> udpExternalTables;
    quint16 busExternalDeviceAddress;
    QString lastError;
    void loadClients();
    void unregisteredClients();
    void registredClientAnswer(QHostAddress address, quint16 port, bool success = true);
    void registredClient(QHostAddress address, quint16 port, quint16 devAddr, QString name);
    void unregistredClient(QHostAddress address, quint16 port);
    void serviceProcessing(QSNContainer container);
    void changeSNAddress(QHostAddress address, quint16 port, QSNContainer *container);

    void setDeviceName(quint16 address, QString name);
    QString deviceName(quint16 address);
    void setDeviceError(quint16 address, QString error);
    void checkDeviceExist();
};

#endif // QSNUDPSERVER_H
