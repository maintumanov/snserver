#ifndef QSNWEBADAPTERSYNCH_H
#define QSNWEBADAPTERSYNCH_H

#include <QDebug>
#include "qsnwebadapter.h"
#include "qsnwebauthorization.h"
#include "qsntcpserver.h"
#include "qsnglobalmodules.h"
#include "qsnimageconvert.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnPSocket;

class QsnWebAdapterSynch : public QsnWeb
{
    Q_OBJECT
public:
    //QList<QsnMQTTSocket*> MQTTClients;

    QsnWebAdapterSynch(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebAdapterSynch();
    void fromStream(QDataStream *stream);
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void setDefault();
    quint16 getPort();
    void adapterDisconnect();
    void adapterConnect();
    QString widgetState();

signals:

public slots:
    void onLog(quint8 level, QString msg);


private slots:
    void newConnection();
    void closeConnection(QsnPSocket *socket);

private:

    QList<QsnPSocket*> TCPClients;
    QTcpServer *Server;
    QHostAddress Address;
    quint16 Port;
    QString login;
    QString password;
    QDateTime lastConnect;
    QsnGlobalModules *mds;
    QString lastConnectText();
};

//======================================================================
class QsnPSocket : public QObject
{
    Q_OBJECT
public:
    explicit QsnPSocket(QTcpSocket *Socket, QObject *parent = Q_NULLPTR);
    ~QsnPSocket();
    QTcpSocket* clientSocket;
    QHostAddress clientIPAddress();
    QDateTime connectionTime();
    QString clientLastError();
    void setLoginPassword(QString l, QString p);

signals:
    //---- For interface module
    void closeConnected(QsnPSocket *socket);

public slots:
    //----BM
    void slotDisconnect();

private slots:
    void readyRead();
    void slotDisconnected();

private:
    bool authState;
    QString login;
    QString password;
    quint8 caseMode;
    quint32 blockSize;
    QDateTime connectTime;
//    QString cName;
    QString lastError;

    void writeSize(quint32 size);
    void readSize();
    void readData();
    void sendData(QByteArray *block);
    void state_1();
    void state_2();
    void requestPatternsList();
    void requestPatternsListRead(QString path, QString *item);
    void requestDownload(QDataStream *stream);
    void requestUpload(QDataStream *stream);
    void requestAuth(QDataStream *stream);
    void accessAnswer();

};

#endif // QSNWEBADAPTERSYNCH_H
