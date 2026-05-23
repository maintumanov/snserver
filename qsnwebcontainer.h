#ifndef QSNWEBCONTAINER_H
#define QSNWEBCONTAINER_H

#include <QObject>
#include <QTextStream>
#include <QDebug>
#include <QTextCodec>
#include <QDateTime>
#include <QDir>
#include <QTcpSocket>
#include "qsnbsshapes.h"
#include "qsnwebsslserver.h"
#include "qsnwebsocket.h"
#include "qsnweb.h"
#include "qsnwebpage.h"
#include "qsnwebadapter.h"
#include "qsnwebadaptercanv5.h"
#include "qsnwebadapteruartcan.h"
#include "qsnwebadapterring.h"
#include "qsnwebadaptertcpserver.h"
#include "qsnwebadapterudp.h"
#include "qsnwebadapterudpserver.h"
#include "qsnwebpagepzem.h"
#include "qsnglobalmodules.h"
#include "qsnwebpage404.h"
#include "qsnwebpageabout.h"
#include "qsnwebpagelog.h"
#include "qsnwebpagealert.h"
#include "qsnwebwidgetstartpage.h"
#include "qsnwebpagemail.h"
#include "qsnwebpagegraph.h"
#include "qsnwebpageenergy.h"
#include "qsnwebpageschedule.h"
#include "qsnwebpageusers.h"
#include "qsnwebpagecontrolpanel.h"
#include "qsnwebpagemessages.h"
#include "qsnwebpagegsm.h"
#include "qsnwebpagedoorbell.h"
#include "qsnwebpageowntracks.h"
#include "qsnwebpagekeenetic.h"
#include "qsnwebpagenarodmon.h"
#include "qsnwebpagesecurityalarm.h"
#include "qsnimageconvert.h"
#include "qsnwebpagenotification.h"
#include "qsnwebpagelogconnector.h"
#include "qsnwebpagesyslogconnector.h"
#include "qsnwebpageyeelight.h"
#include "qsnwebadaptermqtt.h"
#include "qsnwebpagewifisniffer.h"
#include "qsnwebadaptersynch.h"
#include "qsnwebpagepresence.h"
#include "qsnwebpagenighttime.h"
#include "qsnwebpagedaytime.h"
#include "qsnwebpagesnirf.h"

class QsnWebSocket;
class QsnWebSSLServer;

class QsnWebContainer : public QsnWeb
{
    Q_OBJECT
public:
    explicit QsnWebContainer(QObject *parent = Q_NULLPTR );
    ~QsnWebContainer();
    void authorizationAttempt(QString login, QString password, int *aindex, QString *error);
    QString authorizationCooke(int authorizationAttempt, bool isSession);
    int accountIndexFromCookeID(qint64 cookeID);
    bool isAuthorizationEnable();
    quint16 webPort();

signals:
    void snBUSOutput(QSNContainer container, QObject *sender);
    void changeWebPort(quint16 port);

public slots:
    void snBUSInput(QSNContainer container, QObject *sender);
    void getHtml(QString url, QByteArray *content, int userID = -1);
    void getJSON(QString url, QByteArray *input, QByteArray *output);
    void getAuthorizationHtml(QString url, QByteArray *content);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void updateConfiguration();
    void setDefaultConfiguration();
    void createDefaulConfiguration(QByteArray *conf);
    void eventDispalyLog(QString text);
    // tcpserver
    void actionConnect();
    void actionDisconnect();
    void setListenWebPorts(quint16 port, quint16 sslport);
    QsnGlobalModules *gmodules();

private slots:
    void saveSettings();
    void configureClear();
    void timerInitDelay();
    // webserver
    void newConnection();
    void newConnectionSSL();
    void closeConnection(QsnWebSocket *socket);

private:

    QTcpServer *Server;
    QsnWebSSLServer *ServerSSL;
    QHostAddress Address;
    quint16 portWeb;
    quint16 portSSLWeb;
    bool serverState;
    bool serverIsStart;
    bool notyfyLaunch;

    QList<QsnWebSocket*> ServerClients;
    qint64 versionID;

    //=====
    quint32 timeInterval;
   // QString title;
    int devId;

    QList<QsnWeb*> widgets;
    QList<QsnWeb*> constWidgets;
    QsnWebAdapterTCPserver *tcpServer;
    QsnWebAdapterUDPserver *udpServer;
    QsnWebAdapterUDP *udpClient;
    QsnWebAdapterUartCan *uartCan;
    QsnWebPage404 *pageInfo;
    QsnWebPageAbout *pageAbout;
    QsnWebPageLog *pageLog;
    QsnWebPageGraph *pageGraph;
    QsnWebPageUsers *pageUsers;
    QsnWebPageControlPanel *pageSettings;
    QsnWebWidgetStartPage *startPage;
    QsnWebPageLogConnector *LogCon;
    QsnGlobalModules modules;
    QsnWeb* getWidgetFromUrl(QString url);

    void loadSettings();
    void checkSupportSSL();

};

#endif // QSNWEBCONTAINER_H
