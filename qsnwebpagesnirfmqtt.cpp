#include "qsnwebpagesnirfmqtt.h"

QsnWebPageSNIRFMQTT::QsnWebPageSNIRFMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("MQTT Device"));
    mds = modules;
    lastBatt = 0;
    lastValue = "";
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebPageSNIRFMQTT::fromStream(QDataStream *stream, QString path)
{
    int Count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> Count; //items

    mqttBattTopic = optionsMap.value("MQTT", QString()).toString();
    mqttBattJson = optionsMap.value("MQTJ", QString()).toString();
    mqttValueTopic = optionsMap.value("MQTVT", QString()).toString();
    mqttValueJson = optionsMap.value("MQTVJ", QString()).toString();
    timeout = optionsMap.value("MQTTO", 360).toInt() * 60;
}


void QsnWebPageSNIRFMQTT::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;

    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_raw && !mqttBattTopic.isEmpty() && mqttBattTopic == container.info) {
            QSNMQTTBattMsgProcess(container.Data);
        }

        if (container.Signal == BUSSERV_MGTT_reception_raw && !mqttValueTopic.isEmpty() && mqttValueTopic == container.info) {
            QSNMQTTValueMsgProcess(container.Data);
        }
    }

}

void QsnWebPageSNIRFMQTT::QSNMQTTBattMsgProcess(QString msg)
{
    QString vol;
    if (mqttBattJson.isEmpty()) {
        vol = msg;
    } else {
        QMap<QString, QString> options;
        if(!QSNParseJSON(msg, &options)) return;
        if (!options.contains(mqttBattJson)) return;
        vol = options.value(mqttBattJson);
    }
    bool ok = false;
    quint8 batt = vol.toUInt(&ok);
    if (!ok) return;
    if (batt < 0 || batt >100) {  // Дополнительная проверка диапазона
        qWarning() << "Invalid battery value:" << vol;
        return;
    }
    lastBatt = batt;
    emit onMQTT(this->objectName(), lastValue, lastBatt, 240, "BUSSERV_MGTT");
}

void QsnWebPageSNIRFMQTT::QSNMQTTValueMsgProcess(QString msg)
{
    QString vol;
    if (mqttValueJson.isEmpty()) {
        vol = msg;
    } else {
        QMap<QString, QString> options;
        if(!QSNParseJSON(msg, &options)) return;
        if (!options.contains(mqttValueJson)) return;
        vol = options.value(mqttValueJson);
    }

    lastValue = vol;
    emit onMQTT(this->objectName(), lastValue, lastBatt, timeout, "BUSSERV_MGTT");

}


