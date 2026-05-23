#ifndef QSNWEBADAPTERMQTT_H
#define QSNWEBADAPTERMQTT_H

#include <QDebug>
#include "qsnwebadapter.h"
#include "qsnwebauthorization.h"
#include "qsntcpserver.h"
#include "qsnglobalmodules.h"
#include "qsnimageconvert.h"
#include "qsnweb.h"
#include "qsnwebadaptermqttoutput.h"
#include "qsnwebadaptermqttinput.h"
#include "qsnbsshapes.h"

class QsnMQTTSocket;

struct QsnMQTTFilter {
    QString id;
    QStringList filter;
    quint8 QoS;
    QByteArray lastMSG;
    QString lastTopic;
};

struct QsnMQTTCore {
    QList<QsnMQTTSocket*> MQTTClients;
    QString login;
    QString password;
    QsnInterface *sninterface;
    QList<QsnMQTTFilter> subscribes;
};

class QsnWebAdapterMQTT : public QsnWeb
{
    Q_OBJECT
public:
    //QList<QsnMQTTSocket*> MQTTClients;

    QsnWebAdapterMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebAdapterMQTT();
    void fromStream(QDataStream *stream);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void setDefault();
    quint16 getPort();
    void adapterDisconnect();
    void adapterConnect();
    void urlChanged(int accountIndex);
    QString widgetState();
    char widgetNotifState();

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void serverUpdated();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void onSubscribe(QString filter, QString id, quint8 qos);
    void onUnsubscribe(QString filter, QString id);
    void onRequestFromWidgets(QString topic);
    void onReadPublish(QString topic, QByteArray msg, QString id);
    void onWritePublish(QString topic, QByteArray *msg);
    void onWritePublishPSN(QString topic, QByteArray *msg);
    void onWritePublishJson(QString topic, QString items);
    void onClearSeanse(QsnMQTTSocket *socket);

private slots:
    void newConnection();
    void closeConnection(QsnMQTTSocket *socket);

private:
    QsnMQTTCore MQTTCore;
    QTcpServer *Server;
    QHostAddress Address;
    quint16 Port;
    QVector<QsnWebAdapterMQTTOutput*> outputs;
    QList<QsnWebAdapterMQTTInput*> inputs;
    bool serverState;
    bool authEnable;
    bool serverEnable;
    bool rawEnable;
    bool jsonEnable;
    bool ioView;
    int subscribeViewItem;

    QString lastError;
    QsnGlobalModules *mds;

    void connectedChange(bool state);
    QStringList getTopicFromString(QString topic);
    bool compareTopics(QStringList  topic, QStringList  subscribe);
    void publish(QString topic, QByteArray msg, QString id);
    void timeCheck();
    QString subscribsToJSON();
    QString connectionsToJSON();
    QString ioToJSON();
    void onRequestFromLocalWidgets(QString topic);
};

//======================================================================
class QsnMQTTSocket : public QObject
{
    Q_OBJECT
public:
    explicit QsnMQTTSocket(QTcpSocket *Socket, QsnMQTTCore *core, QsnInterface *interface, QObject *parent = Q_NULLPTR);
    ~QsnMQTTSocket();
    QTcpSocket* clientSocket;
    QHostAddress clientIPAddress();

    //public property
    QString clientID;
    QString sessionLastError;
    quint16 remainingTime;
    quint16 sessionActiveTime;
    QString sessionLastWishTheme;
    QString sessionLastWishMsg;
    bool isLastWish;
    bool isClearSeanse;
    QDateTime connectTime;

signals:
    void closeConnected(QsnMQTTSocket *socket);
    void onSubscribe(QString filter, QString id, quint8 qos);
    void onUnsubscribe(QString filter, QString id);
    void onReadPublish(QString topic, QByteArray msg, QString id);
    void onClearSeanse(QsnMQTTSocket *socket);

public slots:
    void slotDisconnect();
    void publish(QString topic, QByteArray msg);
    void onTimeCheck();

private slots:
    void readyRead();
    void slotDisconnected();


private:
    QsnMQTTCore *MQTTcore;

    quint8 currentState;
    quint8 packetType;
    quint8 packetFlags;
    quint16 packetID;
    quint32 packetSize;
    quint32 packetSizeMult;
    QsnInterface *iface;

    void state_begin();
    void state_getSize();
    void state_processing();
    bool state_connect();
    bool state_publish();
    bool state_subscribe();
    bool state_unsubscribe();
    bool state_pingreq();
    bool state_disconnect();

    void state_connack(quint8 retCode);
    void state_puback();
    void state_suback(quint8 retCode);
    void state_unsuback();
    void state_pingresp();

    quint8 readByte();
    quint16 readWord();
    QString readString();
    QByteArray readByteArray();
    void baAddWord(QByteArray *ba, quint16 word);
    void baAddString(QByteArray *ba, QString str);
    void baAddSize(QByteArray *ba, int size);
};

#endif // QSNWEBADAPTERMQTT_H
