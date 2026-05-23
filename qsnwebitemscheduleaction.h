#ifndef QSNWEBITEMSCHEDULEACTION_H
#define QSNWEBITEMSCHEDULEACTION_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebItemScheduleAction : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebItemScheduleAction(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);
    void widgetRunAction(int ioIndex);

signals:
    void readPower(QByteArray *data);

private:
    QsnGlobalModules *mds;

};


#endif // QSNWEBITEMSCHEDULEACTION_H
