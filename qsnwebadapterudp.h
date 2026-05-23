#ifndef QSNWEBADAPTERUDP_H
#define QSNWEBADAPTERUDP_H

#include <QObject>
#include <QDebug>
#include "qsnudpclient.h"
#include "qsnglobalmodules.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebAdapterUDP : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterUDP(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebAdapterUDP();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void setDefault();
    void adapterDisconnect();
    void adapterConnect();
    QString widgetState();
    char widgetNotifState();

private slots:


private:
    QsnUDPclient *udpClient;
    QsnGlobalModules *mds;
    QString lastError(QString lasterror);

};

#endif // QSNWEBADAPTERUDP_H
