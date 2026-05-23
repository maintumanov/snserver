#ifndef QSNWEBWIDGETCARDTHERMOSTATINDICATOR_H
#define QSNWEBWIDGETCARDTHERMOSTATINDICATOR_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebWidgetCardThermostatIndicator : public QsnWeb
{
    Q_OBJECT

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

public:
    QsnWebWidgetCardThermostatIndicator(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    quint8 state();

private:
    QsnGlobalModules *mds;
    quint8 mode;
    bool istate;
};

#endif // QSNWEBWIDGETCARDTHERMOSTATINDICATOR_H
