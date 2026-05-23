#include "qsnwebwidgetcardindicatormqttalert.h"

QsnWebWidgetCardIndicatorMQTTAlert::QsnWebWidgetCardIndicatorMQTTAlert(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("MQTT topic"));
    mds = modules;
    if (mds && mds->interface) {
        connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    }
    lastState = 2; // Unknown/initial state
}

void QsnWebWidgetCardIndicatorMQTTAlert::fromStream(QDataStream *stream, QString path)
{
    int Count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> Count; // child items (none expected)

    mqttTopic = optionsMap.value("MQTR", QString()).toString();
    mqttItemJson = optionsMap.value("MQJV", QString()).toString();
    mqttValueTrue = optionsMap.value("MQAT", "1").toString();
    mqttValueFalse = optionsMap.value("MQAF", "0").toString();
    mqttAlert = optionsMap.value("MQAL", 0).toInt();
    mqttAddr = optionsMap.value("MQAA", 4000).toInt();
    // ⚠️ Option "MQSC" ("Only change value") is loaded in UI but NOT read here!
    // This explains why `mqttOnlyChange` is unused — it's missing from deserialization.
}


void QsnWebWidgetCardIndicatorMQTTAlert::snBUSInput(QSNContainer container, QObject *sender)
{

    if (sender == this) return;
    if (container.role != QSNContainer::service) return;

    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_raw && !mqttTopic.isEmpty() && mqttTopic == container.info) {
            MsgProcess(container.Data);
        }
    }
}

void QsnWebWidgetCardIndicatorMQTTAlert::MsgProcess(QString msg)
{
    QByteArray data;
    QString vol;
    quint8 newState = 2; // unknown

    // Extract value from JSON or use full payload
    if (mqttItemJson.isEmpty()) {
        vol = msg;
    } else {
        QMap<QString, QString> options;
        if(!QSNParseJSON(msg, &options)) return;
        if (!options.contains(mqttItemJson)) return;
        vol = options.value(mqttItemJson);
    }

    // Map string to state
    if (vol == mqttValueTrue) newState = 1;
    if (vol == mqttValueFalse) newState = 0;

    // Always emit onActive() to refresh parent's confidence timer
    emit onActive("BUSSERV_MGTT");

    // Skip if no change (debounce)
    if (newState == lastState) return;
    lastState = newState;

    // Build notification packet
    QSNNotification n;
    n.idp = mqttAddr;
    // Notification type encoding:
    // - Bit 0: 1 = active, 0 = inactive
    // - Bits 1+: alert type index
    n.notificationType = (mqttAlert << 1) + 1;
    if (!newState) n.notificationType ++;
    n.notificationSource = 0;

    // Serialize to 14-byte raw packet
    QSNByteToRAW(&data, 0, 14);
    QSNNotificationToRAW(&data, 1, n);

    // Broadcast system alert
    AlertSend(n);

    // Notify parent indicator
    emit onMQTT(data, "BUSSERV_MGTT_SN");
}

void QsnWebWidgetCardIndicatorMQTTAlert::AlertSend(const QSNNotification &nitification)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_ALERT;
    container.Sender = nitification.notificationSource;
    container.Signal = nitification.notificationType;
    container.info = QString();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << nitification.idp;  // Only ID is sent in alert payload
    container.Data = data;
    mds->interface->snBUSInput(container, this);
}


