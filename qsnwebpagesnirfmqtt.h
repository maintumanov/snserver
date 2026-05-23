#ifndef QSNWEBPAGESNIRFQTTSTATE_H
#define QSNWEBPAGESNIRFQTTSTATE_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

class QsnWebPageSNIRFMQTT : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageSNIRFMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream, QString path = QString());
    void writeToMQTTPSN(QByteArray data);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

signals:
    void onMQTT(QString name, QString value, quint8 batt, quint16 timeout, QString source);

private:
    QsnGlobalModules *mds;
    QString mqttBattTopic;
    QString mqttBattJson;
    QString mqttValueTopic;
    QString mqttValueJson;
    quint8 lastBatt;
    QString lastValue;
    int timeout;

    void QSNMQTTBattMsgProcess(QString msg);
    void QSNMQTTValueMsgProcess(QString msg);
};

#endif // QSNWEBPAGESNIRFQTTSTATE_H
