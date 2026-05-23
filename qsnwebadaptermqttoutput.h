#ifndef QSNWEBADAPTERMQTTOUTPUT_H
#define QSNWEBADAPTERMQTTOUTPUT_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebAdapterMQTTOutput : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterMQTTOutput(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);
    void reciveTopic(QString topic, QByteArray msg);
    QString getTopic();
    QString getJsonValueItem();
    void setIDP(quint16 idp);
    QString getLabelLastData();
    void timeCheck();

signals:
    void readPower(QByteArray *data);

private:
    QsnGlobalModules *mds;
    //double lastData;
    QString subscriptionTopic;
    QString valueJson;
    quint8 valueType;
    quint16 outputIDP;
    quint16 repeatTimeout;
    quint16 repeatTimeoutSet;
    quint16 repeatTimeoutMinute;
    QByteArray valueData;

};


#endif // QSNWEBADAPTERMQTTOUTPUT_H
