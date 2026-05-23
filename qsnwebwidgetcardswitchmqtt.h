#ifndef QSNWEBWIDGETCCARDSWITCHMQTT_H
#define QSNWEBWIDGETCCARDSWITCHMQTT_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

class QsnWebWidgetCardSwitchMQTT : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardSwitchMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);


public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void onStateChange(bool state, QString source);

signals:
    void onMQTT(bool state, QString source);

private:
    QByteArray lastData;
    QsnGlobalModules *mds;
    QString mqttTopicRead;
    QString mqttTopicWrite;
    QString mqttItemJson;
    QString mqttTextOn;
    QString mqttTextOff;

    void readFromMQTTraw(QByteArray *data);
    void writeToMQTTraw(bool state);

};

#endif // QSNWEBWIDGETCCARDSWITCHMQTT_H
