#ifndef QSNWEBADAPTERRING_H
#define QSNWEBADAPTERRING_H

#include <QObject>
#include <QDebug>
#include "qsnring.h"
#include "qsninterface.h"
#include "qsnglobalmodules.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebAdapterRing : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterRing(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
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
    QSnRing ring;
    QsnGlobalModules *mds;
    QString lastError(QString lasterror);

};

#endif // QSNWEBADAPTERRING_H
