#ifndef QSNWEBADAPTERMQTTINPUT_H
#define QSNWEBADAPTERMQTTINPUT_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebAdapterMQTTInput : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebAdapterMQTTInput(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    QString getTopic();
    QString getJsonValueItem();
    QString getLabelLastData();

signals:
    void onWritePublishJson(QString topic, QString json);

private:
    QsnGlobalModules *mds;
    QString subscriptionTopic;
    QString valueName;
    quint8 valueType;
    QByteArray valueData;
};


#endif // QSNWEBADAPTERMQTTINPUT_H
