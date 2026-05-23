#ifndef QSNWEBWIDGETYEELIGHTDESKLAMPSCENARIO_H
#define QSNWEBWIDGETYEELIGHTDESKLAMPSCENARIO_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebItemYeelightScenario : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebItemYeelightScenario(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);

signals:
    void applyScenarioLamp(quint8 bright, quint16 colorTemp);

private:
    QsnGlobalModules *mds;
    quint8 lampbright;
    quint16 lamptemp;

};


#endif // QSNWEBWIDGETYEELIGHTDESKLAMPSCENARIO_H
