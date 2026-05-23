#include "qsnwebwidgetcardindicatormqttstate.h"

QsnWebWidgetCardIndicatorMQTTState::QsnWebWidgetCardIndicatorMQTTState(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("MQTT topic"));
    mds = modules;
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    lastState = 255; // Unknown/initial state (outside 0–1 range)

}

void QsnWebWidgetCardIndicatorMQTTState::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; // child items (none expected)

    mqttTopic = optionsMap.value("MQTR", QString()).toString();
    mqttItemJson = optionsMap.value("MQJV", QString()).toString();
    mqttValueTrue = optionsMap.value("MQST", "1").toString();
    mqttValueFalse = optionsMap.value("MQSF", "0").toString();
    mqttSubType = static_cast<quint8>(optionsMap.value("MQSP", 0).toInt());
    mqttOnlyChange = optionsMap.value("MQSC", true).toBool(); // Default: emit only on change
}


void QsnWebWidgetCardIndicatorMQTTState::snBUSInput(QSNContainer container, QObject *sender)
{

    if (sender == this) return;
    if (container.role != QSNContainer::service) return;

    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_raw && !mqttTopic.isEmpty() && mqttTopic == container.info) {
            QSNMQTTTopicMsgProcess(container.Data);
        }
    }
}

void QsnWebWidgetCardIndicatorMQTTState::QSNMQTTTopicMsgProcess(QString msg)
{
    QString vol;
    quint8 newState = 255;

    // Extract value from JSON or full payload
    if (mqttItemJson.isEmpty()) {
        vol = msg;
    } else {
        QMap<QString, QString> options;
        if(!QSNParseJSON(msg, &options)) return;
        if (!options.contains(mqttItemJson)) return;
        vol = options.value(mqttItemJson);
    }

    // Map to binary state
    if (vol == mqttValueTrue) newState = 1;
    if (vol == mqttValueFalse) newState = 0;

    // Always refresh parent's activity timer
    emit onActive("BUSSERV_MGTT");
    if (newState == 255) return;

    // Skip if no change and "only change" mode is enabled
    if (mqttOnlyChange && newState == lastState) return;
    lastState = newState;

    // Build 3-byte SNIRF-like packet:
    // [type=1][bool][subtype]
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, newState);
    QSNByteToRAW(&data, 2, mqttSubType);
    emit onMQTT(data, "BUSSERV_MGTT");
}
