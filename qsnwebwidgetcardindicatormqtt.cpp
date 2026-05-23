#include "qsnwebwidgetcardindicatormqtt.h"

QsnWebWidgetCardIndicatorMQTT::QsnWebWidgetCardIndicatorMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("MQTT topic"));
    mds = modules;

    // Listen for MQTT messages via global bus
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardIndicatorMQTT::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    // Load general options and I/O bindings (I/O not used here, but preserved)
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count;  // child items (none expected)

    // Apply MQTT-specific settings
    mqttTopic = optionsMap.value("MQTR", QString()).toString();       // Topic to read
    mqttItemJson = optionsMap.value("MQJV", QString()).toString();   // JSON field
    mqttItemType = static_cast<quint8>(optionsMap.value("MQT", 1).toInt()); // Data type
    mqttMultiplier = optionsMap.value("MQM", 1.0).toReal();
    if (mqttMultiplier == 0) mqttMultiplier = 1.0; // Avoid zero multiplier

    mqttMinValue = optionsMap.value("MQVM", QString()).toString();   // Min filter
    mqttMaxValue = optionsMap.value("MQVX", QString()).toString();   // Max filter
}

void QsnWebWidgetCardIndicatorMQTT::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;

    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_raw && !mqttTopic.isEmpty() && mqttTopic == container.info) {

            // Convert MQTT payload to SNIRF raw format
            QByteArray vol = QSNMQTTTopicMsgToRAW(container.Data, mqttItemType, mqttItemJson, mqttMultiplier);
            if (vol.isEmpty()) return;
            if (vol.at(0) != mqttItemType) return;

            // Validate result
            qreal v = QSNRAWtoReal(&vol, vol.at(0), 1);

            // Apply min/max filters (if configured)
            if (!mqttMinValue.isEmpty() && v < mqttMinValue.toDouble()) return;
            if (!mqttMaxValue.isEmpty() && v > mqttMaxValue.toDouble()) return;

            // Emit validated data
            emit onMQTT(vol, "BUSSERV_MGTT");
        }
    }

}

//void QsnWebWidgetCardIndicatorMQTT::writeToMQTTraw(bool state)
//{
//    if (!mds->supportMQTT) return;
////    if (mqttTopicWrite.isEmpty()) return;
//    QSNContainer container = newContainer();
//    container.role = QSNContainer::service;
//    container.Command = BUSSERV_MGTT;
//    container.Signal = BUSSERV_MGTT_publication_raw;
//    container.Sender = 0;
//    container.info = mqttTopicWrite;
//    if (mqttItemJson.isEmpty()) {
//        container.Data = state?mqttTextOn.toUtf8():mqttTextOff.toUtf8();
//    } else {
//       container.Data = QString("{\"%1\":\"%2\"}").arg(mqttItemJson, state?mqttTextOn:mqttTextOff).toUtf8();
//    }
//    mds->interface->snBUSInput(container, this);
//}



