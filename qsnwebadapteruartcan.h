#ifndef QSNWEBADAPTERUARTCAN_H
#define QSNWEBADAPTERUARTCAN_H

#include <QObject>
#include <QDebug>
#include "qsnuartcan.h"
#include "qsninterface.h"
#include "qsnglobalmodules.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebAdapterUartCan : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterUartCan(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctionsJSON(QStringList *fjson, int);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void adapterDisconnect();
    void adapterConnect();
    QString widgetState();
    char widgetNotifState();

private slots:

private:
    QSnUartCan uartcan;
    QsnGlobalModules *mds;
    QString lastError(QString lasterror);

};

#endif // QSNWEBADAPTERUARTCAN_H
