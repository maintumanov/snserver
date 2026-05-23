#ifndef QSNWEBADAPTERUDPSERVER_H
#define QSNWEBADAPTERUDPSERVER_H

#include <QObject>
#include <QDebug>
#include "qsnudpserver.h"
#include "qsnglobalmodules.h"
#include "qsnimageconvert.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebAdapterUDPserver : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterUDPserver(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebAdapterUDPserver();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void setDefault();
    void adapterDisconnect();
    void adapterConnect();
    void urlChanged(int accountIndex);
    QString widgetState();
    char widgetNotifState();

private slots:
    void serverUpdated();

private:
    QsnUDPserver *udpServer;
    QsnGlobalModules *mds;
    int updateIndex;
    int indexFromAddr(int addr);
    QString clientsToJSON();
    QString lastErrorTable(QString lasterror, quint16 devaddr);


};

#endif // QSNWEBADAPTERUDPSERVER_H
