#ifndef QSNTCPSERVER_H
#define QSNTCPSERVER_H

#include <QCoreApplication>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "qsninterface.h"

class QsnTCPSocket;

class QsnTCPserver : public QObject
{
    Q_OBJECT
public:
    QList<QsnTCPSocket *> TCPClients;

    explicit QsnTCPserver(QsnInterface *interface, QObject *parent = Q_NULLPTR);
    ~QsnTCPserver();
    void setAuthEnable(bool enable);
    bool isAuthEnable();
    quint16 getPort();
    bool isConnected();
    bool isEnabled();
    QString error();

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void authorizationRequest(QString login, QString password, QString addr ,QObject *sender);
    void stateChanged();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void responseAuthentication(bool access, QObject *sender);
    void actionConnect();
    void actionDisconnect();
    void setEnabled(bool enable);
    void setPort(quint16 port);

private slots:
    void newConnection();
    void closeConnection(QsnTCPSocket *socket);

private:
    QTcpServer *Server;
    QHostAddress Address;
    quint16 Port;
    bool authEnable;
    bool serverState;
    bool serverEnable;
    QString lastError;
    QsnInterface *sninterface;
    void serviceProcessing(QSNContainer container);
 //   void info(quint8 warningLevel, QString text, bool dispView = false);
    void connectedChange(bool state);
};


class QsnTCPSocket : public QObject
{
    Q_OBJECT
public:
    explicit QsnTCPSocket(QTcpSocket *Socket, QsnInterface *interface, bool authEn, QObject *parent = Q_NULLPTR);
    ~QsnTCPSocket();
    QTcpSocket* clientSocket;
    quint16 clientDevAddress();
    QHostAddress clientIPAddress();
    QString clientName();
    QDateTime connectionTime();
    QString clientLastError();

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void authorizationRequest(QString login, QString password, QString addr, QObject *sender);
    void closeConnected(QsnTCPSocket *socket);
    void eventDBGchLevelText(quint8 level, QString text, QObject *sender);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void responseAuthentication(bool access);
    void slotDisconnect();

private slots:
    void readyRead();
    void slotDisconnected();

private:
    bool authState;
    bool authEnable;
    QString login;
    QsnInterface *sninterface;
    quint8 caseMode;
    quint32 blockSize;
    quint16 cDeviceAddress;
    QDateTime cConnectionTime;
    QString cName;
    QString cLastError;

    void writeSize(quint32 size);
    void readSize();
    void readData();
    void sendData(QByteArray *block);
    void readAuth(QDataStream *stream);
    void state_1();
    void state_2();

};

#endif // QSNTCPSERVER_H
