#ifndef QSNWEBADAPTERTCPSERVER_H
#define QSNWEBADAPTERTCPSERVER_H

#include <QObject>
#include <QDebug>
#include "qsnwebadapter.h"
#include "qsnwebauthorization.h"
#include "qsntcpserver.h"
#include "qsnglobalmodules.h"
#include "qsnimageconvert.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebAdapterTCPserver : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterTCPserver(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebAdapterTCPserver();
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

private slots:
    void authorizationRequest(QString login, QString password, QString addr, QObject *sender);
    void serverUpdated();

private:
    QsnTCPserver *tcpServer;
    QsnGlobalModules *mds;
    QString labelName;
    int updateIndex;
    int indexFromAddr(int addr);
    QString clientsToJSON();
    QString lastErrorTable(QString lasterror, quint16 devaddr);
};

#endif // QSNWEBADAPTERTCPSERVER_H
