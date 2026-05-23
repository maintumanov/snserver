#ifndef QSNWEBADAPTERCANV5_H
#define QSNWEBADAPTERCANV5_H

#include <QObject>
#include <QDebug>
#include "qsnglobalmodules.h"
#include "qsncanv5.h"
#include "qsninterface.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"


class QsnWebAdapterCANv5 : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterCANv5(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getFunctions(QStringList *functions, int, QString);
    void getFunctionsJSON(QStringList *fjson, int);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void adapterDisconnect();
    void adapterConnect();
    QString widgetState();

private slots:


private:
    QSnCanV5 CANv5;
    QsnGlobalModules *mds;
    QString lastError(QString lasterror);
};

#endif // QSNWEBADAPTERCANV5_H
