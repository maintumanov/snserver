#ifndef QSNWEBITEMPRESENCETIME_H
#define QSNWEBITEMPRESENCETIME_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebItemPresenceTime : public QsnWeb
{
    Q_OBJECT
public:   
    QsnWebItemPresenceTime(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);

signals:
    void presenceAction(QString source, qint64 presenceTime, qint64 altPresenceTime);

public slots:
    //----BM

private:

    QsnGlobalModules *mds;
    qint64 presencetime;
    qint64 presencetimealt;


};

#endif // QSNWEBITEMABSENCETIME_H
