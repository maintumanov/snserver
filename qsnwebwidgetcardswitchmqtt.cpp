#include "qsnwebwidgetcardswitchmqtt.h"

QsnWebWidgetCardSwitchMQTT::QsnWebWidgetCardSwitchMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("MQTT topic"));
    mds = modules;
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    //danger
}

void QsnWebWidgetCardSwitchMQTT::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    mqttTopicRead = optionsMap.value("TR", QString()).toString();
    mqttTopicWrite = optionsMap.value("TW", QString()).toString();
    mqttItemJson = optionsMap.value("JV", QString()).toString();
    mqttTextOn = optionsMap.value("TN", QLatin1String("ON")).toString();
    mqttTextOff = optionsMap.value("TF", QLatin1String("OFF")).toString();
    if (mqttTopicWrite.isEmpty()) mqttTopicWrite = mqttTopicRead;
}


void QsnWebWidgetCardSwitchMQTT::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;

    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_raw && !mqttTopicRead.isEmpty() && mqttTopicRead == container.info) readFromMQTTraw(&container.Data);
    }
}

void QsnWebWidgetCardSwitchMQTT::onStateChange(bool state, QString )
{
    writeToMQTTraw(state);
}

void QsnWebWidgetCardSwitchMQTT::readFromMQTTraw(QByteArray *data)
{
    if (mqttTopicRead.isEmpty()) return;
    QByteArray raw = QSNMQTTTopicMsgToRAW(QString::fromUtf8(*data), 1, mqttItemJson, 1);
    emit onMQTT(QSNRAWtoBool(&raw, 1), "BUSSERV_MGTT");
}

void QsnWebWidgetCardSwitchMQTT::writeToMQTTraw(bool state)
{
    if (!mds->supportMQTT) return;
    if (mqttTopicWrite.isEmpty()) return;
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_publication_raw;
    container.Sender = 0;
    container.info = mqttTopicWrite;
    if (mqttItemJson.isEmpty()) {
        container.Data = state?mqttTextOn.toUtf8():mqttTextOff.toUtf8();
    } else {
        container.Data = QString("{\"%1\":\"%2\"}").arg(mqttItemJson, state?mqttTextOn:mqttTextOff).toUtf8();
    }
    mds->interface->snBUSInput(container, this);
}



