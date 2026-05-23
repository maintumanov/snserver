#ifndef QSNTCPCLIENT_H
#define QSNTCPCLIENT_H

#include <QObject>
#include <QtCore>
#include <QTcpSocket>
#include "qsntcpclient.h"
#include "qsnshapes.h"

class QsnTCPclient : public QObject
{
    Q_OBJECT

public:
    explicit QsnTCPclient(QObject *parent = 0);
    ~QsnTCPclient();
    bool isEnabled();
    void setEnabled(bool enable);
    quint16 getPort();
    void setPort(quint16 port);
    QString getAddress();
    void setAddress(QString address);
    bool isConnected();
    void setLoginAndPassword(QString clientlogin, QString pass);
    QString getLogin();
    QString getPassword();
    void setDeviceName(QString name);

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = 0);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = 0);

    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void saveSettingsStream(QDataStream *stream);
    void loadSettingsStream(QDataStream *stream);
    void connectToServer(quint16 devaddress);
    void disconnectFromServer();

private slots:
    void readSize();
    void writeSize(quint32 size);
    void readyRead();
    void disconnected();
    void sendBlock(QByteArray *block);
    void readedBlock(QByteArray *block);
    void connectedChange(bool state);

private:
    quint16 deviceAddress;
	bool clientEnable;
    QString clientaddress;
    quint16 clientport;
    QTcpSocket *clientSocket;
    quint8 caseMode;
    quint32 dataBlockSize;
    bool authorized;
    QString clientlogin;
    QString clientpassword;
	QTime timeDelay;
    QString deviceName;

    void readAnswerPath();
    void serviceProcessing(QSNContainer container);
    void info(quint8 warningLevel, QString text);
    void authorizationRequest();
    void readAuth(QDataStream *stream);
    void state_1();
    void state_2();

};

#endif // QSNTCPCLIENT_H
