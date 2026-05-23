#ifndef QsnUDPclient_H
#define QsnUDPclient_H

#include <QObject>
#include <QtCore>
#include <QUdpSocket>
#include "qsnshapes.h"

class QsnUDPclient : public QObject
{
    Q_OBJECT

public:
    explicit QsnUDPclient(QObject *parent = Q_NULLPTR);
    ~QsnUDPclient();
    bool isEnabled();
    void setEnabled(bool enable);
    quint16 getServerPort();
    void setServerPort(quint16 port);
    quint16 getSourcePort();
    void setSourcePort(quint16 port);
    QString getAddress();
    void setAddress(QString address);
    QString getKey();
    void setKey(QString key);
    bool isConnected();
    void setDeviceName(QString name);
    void saveSettingsStream(QDataStream *stream);
    void loadSettingsStream(QDataStream *stream);
    void setLogEnable(bool enable);
    bool logEnable();

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void connectLost();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void actionConnect(quint16 address);
    void actionDisconnect();
    void setRepeatMode();
    QString error();

private slots:
    void readyRead();
    void readedBlock(QByteArray *block);
    void connectedChange(bool state);
    void timeout();

private:
    bool isLog;
	bool clientEnable;
    quint16 deviceAddress;
    quint16 serverport;
    quint16 sourceport;
    QString clientaddress;
    QString clientkey;
    QUdpSocket *clientSocket;
    bool autoRepeatMode;
    QString deviceName;
    QString lastError;
    quint16 registredTimeOut;

    void registrationAnswer(QSNContainer *container);
    void registration(bool registration = true);
    void serviceProcessing(QSNContainer container);
  //  void info(quint8 warningLevel, QString text, bool dispView = false);
    void checkDeviceExist();
    bool checkingContainerForRestrictions(QSNContainer *container);
};

#endif // QsnUDPclient_H
