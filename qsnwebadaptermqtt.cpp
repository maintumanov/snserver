#include "qsnwebadaptermqtt.h"

QsnWebAdapterMQTT::QsnWebAdapterMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    Server = new QTcpServer(this);
    mds = modules;
    widgetUrl = "/mqtt";
    setObjectName(tr("MQTT"));
    widgetIcon = "subicon-mqtt";
    widgetAccessRights = 2;

    Address = QHostAddress::Any;
    Port = 1883;
    serverState = false;
    serverEnable = false;
    authEnable = true;
    lastError = QString();

    rawEnable = true;
    jsonEnable = true;

    MQTTCore.sninterface = modules->interface;
    modules->supportMQTT = true;

    connect(Server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    connect(this, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebAdapterMQTT::~QsnWebAdapterMQTT()
{
    for (int i = MQTTCore.MQTTClients.count() - 1; i >= 0; i --) {
        MQTTCore.MQTTClients[i]->deleteLater();
        MQTTCore.MQTTClients.removeAt(i);
    }
    Server->deleteLater();
}

void QsnWebAdapterMQTT::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("MQTTO"))) {
            QsnWebAdapterMQTTOutput *ioo = new QsnWebAdapterMQTTOutput(static_cast<quint32>(id), modules(), this);
            ioo->fromStream(stream);
            ioo->setIDP(mds->interface->getDeviceAddress() + ((i & 0x000F) << 12));
            outputs.append(ioo);
        }

        if (sig == QString(QLatin1String("MQTTI"))) {
            QsnWebAdapterMQTTInput *ioi = new QsnWebAdapterMQTTInput(static_cast<quint32>(id), modules(), this);
            ioi->fromStream(stream);
            connect(ioi, SIGNAL(onWritePublishJson(QString,QString)), this, SLOT(onWritePublishJson(QString,QString)));
            inputs.append(ioi);
        }
    }

    Port = optionsMap.value("PR", 1883).toUInt();
    MQTTCore.login = optionsMap.value("LG", tr("admin")).toString();
    MQTTCore.password = optionsMap.value("PW", QLatin1String("signalnet")).toString();
    rawEnable = optionsMap.value("RW", false).toBool();
    jsonEnable = optionsMap.value("JS", true).toBool();
}

void QsnWebAdapterMQTT::getFunctions(QStringList *functions, int , QString )
{
    if (subscribeViewItem >= 0) {
        *functions << QString("function tablesubscribe(data) {"
                              "var table = $(\"#tablesub\");"
                              "table.empty();"
                              "$.each(data, function (i, item) {"
                              "table.append(\"<tr><td>\" + item.ss + \"</td>\" +"
                              "\"<td>\" + item.msg + \"</td></tr>\");"
                              "});}");
    } else {

        *functions << QString("function tabledev(data) {"
                              "var table = $(\"#tabledev\");"
                              "table.empty();"
                              "$.each(data, function (i, item) {"
                              "table.append(\"<tr><td><a href='mqtt?name=\" + item.id + \"'>\" + item.id + \"</a></td>\" +"
                              "\"<td>\" + item.ip + \"</td>\" +"
                              "\"<td>\" + item.ct + \"</td></tr>\");"
                              "});}");

        *functions << QString("function tableio(data) {"
                              "var table = $(\"#tableio\");"
                              "table.empty();"
                              "$.each(data, function (i, item) {"
                              "table.append(\"<tr><td>\" + item.dir + \"</td>\" +"
                              "\"<td>\" + item.nm + \"</td>\" +"
                              "\"<td>\" + item.dt + \"</td></tr>\");"
                              "});}");
    }
}

void QsnWebAdapterMQTT::getFunctionsJSON(QStringList *fjson, int )
{
    if (subscribeViewItem >= 0) {
        *fjson << QString(" tablesubscribe(data.sub);");
    } else {
        *fjson << QString(" $('#port').text(data.port);");
        *fjson << QString(" $('#open').text(data.open);");
        *fjson << QString(" $('#isjson').text(data.isjson);");
        *fjson << QString(" $('#israw').text(data.israw);");
        *fjson << QString(" tableio(data.io);");
        *fjson << QString(" tabledev(data.dev);");
    }
}

void QsnWebAdapterMQTT::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            if (subscribeViewItem >= 0) {
                *returnItems << QString("\"sub\": %1").arg(subscribsToJSON());
            } else {
                *returnItems << QString("\"port\": \"%1\"").arg(QString::number(Port));
                *returnItems << QString("\"open\": \"%1\"").arg(QSNBoolToYesNo(Server->isListening()));
                *returnItems << QString("\"isjson\": \"%1\"").arg(QSNBoolToYesNo(jsonEnable));
                *returnItems << QString("\"israw\": \"%1\"").arg(QSNBoolToYesNo(rawEnable));
                *returnItems << QString("\"dev\": %1").arg(connectionsToJSON());
                *returnItems << QString("\"io\": %1").arg(ioToJSON());
            }
        }
    }
}

void QsnWebAdapterMQTT::getContents(QStringList *contents, int )
{
    if (subscribeViewItem >= 0) {
        *contents << QsnBsFormBegin();
        *contents << QString("<h3>%1</h3>").arg(tr("Subscribe") + " - " + MQTTCore.MQTTClients.at(subscribeViewItem)->clientID);
        *contents << QsnBsFormTableBegin();
        *contents << QsnBsFormTableTheadBegin();
        *contents << QsnBsFormTableTheadAdd(tr("Subscribe"));
        *contents << QsnBsFormTableTheadAdd(tr("Last data"));
        *contents << QsnBsFormTableTheadEnd();
        *contents << QsnBsFormTableBodyBegin("tablesub");
        QString id = MQTTCore.MQTTClients[subscribeViewItem]->clientID;
        for (int i = 0; i < MQTTCore.subscribes.count(); i ++) {
            if (MQTTCore.subscribes.at(i).id == id)
                *contents << QsnBsFormTableBodyRowAdd(MQTTCore.subscribes.at(i).filter.join('/'),
                                                      QSNEscapingHTMLCharacters(MQTTCore.subscribes.at(i).lastMSG));
        }
        *contents << QsnBsFormTableBodyEnd();
        *contents << QsnBsFormTableEnd();
        *contents << QsnBsFormEnd();
        return;
    }

    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("info", tr("INFO"), !MQTTCore.MQTTClients.count());
    *contents << QsnBsTapsTabAdd("clients", tr("CLIENTS"), MQTTCore.MQTTClients.count());
    *contents << QsnBsTapsTabAdd("io", tr("IO"));
    *contents << QsnBsTapsPanesBegin();

    *contents << QsnBsTapsPanelBegin("info", !MQTTCore.MQTTClients.count());
    *contents << QsnBsFormLabel(tr("Port"), QString::number(Port), "port");
    *contents << QsnBsFormLabel(tr("The port is open"), QSNBoolToYesNo(Server->isListening()), "open");
    *contents << QsnBsFormLabel(tr("Format JSON"), QSNBoolToYesNo(jsonEnable), "isjson");
    *contents << QsnBsFormLabel(tr("Format RAW"), QSNBoolToYesNo(rawEnable), "israw");
    *contents << QsnBsTapsPanelEnd();


    *contents << QsnBsTapsPanelBegin("clients", MQTTCore.MQTTClients.count());
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Client ID"));
    *contents << QsnBsFormTableTheadAdd(tr("IP address"));
    *contents << QsnBsFormTableTheadAdd(tr("Connect time"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tabledev");
    for (int i = 0; i < MQTTCore.MQTTClients.count(); i ++) {
        *contents << QsnBsFormTableBodyRowAdd(QString("<a href='mqtt?name=%1'>%1</a>").arg(MQTTCore.MQTTClients[i]->clientID),
                                              QSNCleanIP(MQTTCore.MQTTClients[i]->clientIPAddress().toString()),
                                              QSNUpTime(MQTTCore.MQTTClients[i]->connectTime));
    }
    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanelBegin("io");
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("IO"));
    *contents << QsnBsFormTableTheadAdd(tr("Name"));
    *contents << QsnBsFormTableTheadAdd(tr("Data"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tableio");

    for (int i = 0; i < outputs.count(); i ++)
        *contents << QsnBsFormTableBodyRowAdd(tr("Output"),
                                              outputs.at(i)->objectName(),
                                              outputs.at(i)->getLabelLastData());
    for (int i = 0; i < inputs.count(); i ++)
        *contents << QsnBsFormTableBodyRowAdd(tr("Input"),
                                              inputs.at(i)->objectName(),
                                              inputs.at(i)->getLabelLastData());

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
    *contents << QsnBsTapsPanesEnd();
}

void QsnWebAdapterMQTT::setDefault()
{
    Port = 1883;
}

quint16 QsnWebAdapterMQTT::getPort()
{
    return Port;
}

void QsnWebAdapterMQTT::adapterDisconnect()
{
    if (serverState == false) return;

    Server->close();

    for (int i = MQTTCore.MQTTClients.count() - 1; i >= 0; i --) {

        MQTTCore.MQTTClients[i]->deleteLater();
        MQTTCore.MQTTClients.removeAt(i);
    }

    serverState = false;
    emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                       tr("Closing port"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("mqtt"),
                                       false), this);
}

void QsnWebAdapterMQTT::adapterConnect()
{
    if (serverState == true) return;
    if (!Server->listen(QHostAddress::Any, Port) && serverState == false) {
        emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_caution,
                                           tr("No open port %1").arg(Port),
                                           BUSSERV_LOG_CATEGORY_module,
                                           QString("mqtt"),
                                           false), this);
        return;
    }
    serverState = true;
    emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                       tr("Open port %1").arg(Port),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("mqtt"),
                                       false), this);
}

void QsnWebAdapterMQTT::urlChanged(int )
{
    subscribeViewItem = -1;
    if (isKey("name")) {
        QString name = getValue("name");
        for (int i = MQTTCore.MQTTClients.count() - 1; i >= 0; i --)
            if (MQTTCore.MQTTClients[i]->clientID == name) subscribeViewItem = i;
    }
}

QString QsnWebAdapterMQTT::widgetState()
{
    if (!lastError.isEmpty()) return lastError;
    if (serverState) return tr("conn: %1").arg(MQTTCore.MQTTClients.count());
    else return tr("close");
}

char QsnWebAdapterMQTT::widgetNotifState()
{
    if (!lastError.isEmpty()) return 'c';
    if (serverState) return (MQTTCore.MQTTClients.count() > 0?'s':'n');
    else return 'c';
}

void QsnWebAdapterMQTT::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) timeCheck();
    if (container.Command == BUSSERV_MGTT) {
       if (container.Signal == BUSSERV_MGTT_publication_raw) onWritePublish(container.info, &container.Data);
       if (container.Signal == BUSSERV_MGTT_publication_sn) onWritePublishPSN(container.info, &container.Data);
    }
}

void QsnWebAdapterMQTT::onSubscribe(QString filter, QString id, quint8 qos)
{
    for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --)
        if (MQTTCore.subscribes.at(i).filter.join('/') == filter && MQTTCore.subscribes.at(i).id == id) {
            emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                               tr("Subscription %2 to %1 already exists").arg(filter, id),
                                               BUSSERV_LOG_CATEGORY_module,
                                               QString(),
                                               true), this);
            onRequestFromWidgets(filter);
            return;
        }

    QsnMQTTFilter f;
    f.QoS = qos;
    f.filter = getTopicFromString(filter);
    f.id = id;
    MQTTCore.subscribes.append(f);
    emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                       tr("Subscribe %2 to %1").arg(filter, id),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString(),
                                       true), this);
    onRequestFromWidgets(filter);
}

void QsnWebAdapterMQTT::onUnsubscribe(QString filter, QString id)
{
    for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --)
        if (MQTTCore.subscribes.at(i).filter.join('/') == filter && MQTTCore.subscribes.at(i).id == id) MQTTCore.subscribes.removeAt(i);
}

void QsnWebAdapterMQTT::onRequestFromWidgets(QString topic)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_request_sn;
    container.Sender = 0;
    container.info = topic;
    MQTTCore.sninterface->snBUSInput(container, this);

    onRequestFromLocalWidgets(topic);
}

void QsnWebAdapterMQTT::onReadPublish(QString topic, QByteArray msg, QString)
{
    QStringList tp = getTopicFromString(topic);
    for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --) {
        if (/*id != MQTTCore.subscribes.at(i).id &&*/ compareTopics(tp, MQTTCore.subscribes.at(i).filter)) {
            MQTTCore.subscribes[i].lastMSG = msg;
            MQTTCore.subscribes[i].lastTopic = topic;
            publish(topic, msg, MQTTCore.subscribes.at(i).id);
        }
    }

    for (int i = 0; i < outputs.count(); i ++)
        outputs.at(i)->reciveTopic(topic, msg);

    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Sender = 0;

    if (topic.indexOf("snserver") == 0) {
        container.Signal = BUSSERV_MGTT_reception_sn;
        if (jsonEnable && topic.length() - topic.lastIndexOf("json", -1, Qt::CaseInsensitive) == 4) {
            container.info = topic;
            container.info.remove(topic.length() - 5, 5);
            if (QSNJsonToRAW(&container.Data, QString::fromUtf8(msg))) {
                MQTTCore.sninterface->snBUSInput(container, this);
                emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                                   tr("Read JSON to %1").arg(topic),
                                                   BUSSERV_LOG_CATEGORY_module,
                                                   QString(),
                                                   true), this);
            } else   {  emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_caution,
                                                           tr("Read JSON ERROR to %1, %2").arg(topic, QString::fromUtf8(msg)),
                                                           BUSSERV_LOG_CATEGORY_module,
                                                           QString(),
                                                           true), this);
            }
        } else if (rawEnable && topic.length() - topic.lastIndexOf("json", -1, Qt::CaseInsensitive) != 4) {
            container.info = topic;
            quint8 type = container.info.lastIndexOf('/');
            if (type < 1) {
                emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_caution,
                                                   tr("Read RAW ERROR (No data available)"),
                                                   BUSSERV_LOG_CATEGORY_module,
                                                   QString(),
                                                   true), this);
                return;
            }
            QString stype = container.info.right(container.info.count() - type - 1);
            container.info.remove(type, container.info.count() - type);
            type = QSNLatianNameToType(stype);
            if (type == 63) {
                emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_caution,
                                                   tr("Read RAW ERROR to %1, %2").arg(topic, QString::fromUtf8(msg)),
                                                   BUSSERV_LOG_CATEGORY_module,
                                                   QString(),
                                                   true), this);
                return;
            }
            QSNStringToRAW(&container.Data, type, 1, QString::fromUtf8(msg), QString());
            MQTTCore.sninterface->snBUSInput(container, this);
            emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                               tr("Read RAW to %1").arg(topic),
                                               BUSSERV_LOG_CATEGORY_module,
                                               QString(),
                                               true), this);
        }
    } else {
        container.Signal = BUSSERV_MGTT_reception_raw;
        container.info = topic;
        container.Data = msg;
        MQTTCore.sninterface->snBUSInput(container, this);
    }
}

void QsnWebAdapterMQTT::onWritePublish(QString topic, QByteArray *msg)
{
    QByteArray mqttMSG;
    QString mqttTopic;
    QStringList tp;

    // publish to subscribe
    mqttTopic = topic;
    mqttMSG = *msg;
    tp = getTopicFromString(mqttTopic);
    for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --)
        if (compareTopics(tp, MQTTCore.subscribes.at(i).filter)) {
            MQTTCore.subscribes[i].lastMSG = mqttMSG;
            MQTTCore.subscribes[i].lastTopic = topic;
            publish(mqttTopic, mqttMSG, MQTTCore.subscribes[i].id);
        }

}

void QsnWebAdapterMQTT::onWritePublishPSN(QString topic, QByteArray *msg)
{
    QByteArray mqttMSG;
    QString mqttTopic;
    QStringList tp;
    // publish to subscribe
    if (jsonEnable) {
        mqttMSG = QSNRAWToJSON(msg).toUtf8();
        mqttTopic = topic+ "/json";
        tp = getTopicFromString(mqttTopic);
        for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --)
            if (compareTopics(tp, MQTTCore.subscribes.at(i).filter)) {
                MQTTCore.subscribes[i].lastMSG = mqttMSG;
                MQTTCore.subscribes[i].lastTopic = topic;
                publish(mqttTopic, mqttMSG, MQTTCore.subscribes[i].id);
            }
    }
    // publish from raw
    if (!rawEnable) return;
    if (msg->count() == 0) return;
    mqttTopic = QSNTypeLatianName(msg->at(0));
    if (mqttTopic.isEmpty()) return;
    mqttTopic = topic;
    mqttTopic = mqttTopic + "/" + QSNTypeLatianName(msg->at(0));
    mqttMSG = QSNRAWToValueString(msg, 1).toUtf8();
    tp = getTopicFromString(mqttTopic);
    for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --)
        if (compareTopics(tp, MQTTCore.subscribes.at(i).filter)) {
            MQTTCore.subscribes[i].lastMSG = mqttMSG;
            MQTTCore.subscribes[i].lastTopic = topic;
            publish(mqttTopic, mqttMSG, MQTTCore.subscribes[i].id);
        }
}

void QsnWebAdapterMQTT::onWritePublishJson(QString topic, QString items)
{
    QStringList tp;
    // publish to subscribe
    tp = getTopicFromString(topic);
    for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --)
        if (compareTopics(tp, MQTTCore.subscribes.at(i).filter)) {
            MQTTCore.subscribes[i].lastMSG = items.toUtf8();
            MQTTCore.subscribes[i].lastTopic = topic;
            publish(topic, items.toUtf8(), MQTTCore.subscribes[i].id);
        }
}

void QsnWebAdapterMQTT::onClearSeanse(QsnMQTTSocket *socket)
{
    // отключениеклиентов с таким же id
    for (int i = MQTTCore.MQTTClients.count() - 1; i >= 0; i --)
        if (MQTTCore.MQTTClients.at(i)->clientID == socket->clientID && MQTTCore.MQTTClients.at(i) != socket) {
            emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_warning,
                                               tr("Disconnecting an existing connection with the same ID %1").arg(socket->clientID),
                                               BUSSERV_LOG_CATEGORY_module,
                                               QString(),
                                               true), this);
            MQTTCore.MQTTClients.at(i)->slotDisconnect();
        }

    for (int i = MQTTCore.subscribes.count() - 1; i >= 0; i --)
        if (MQTTCore.subscribes.at(i).id == socket->clientID) MQTTCore.subscribes.removeAt(i);
}

void QsnWebAdapterMQTT::newConnection()
{
    QsnMQTTSocket *MQTTSocket = new QsnMQTTSocket(Server->nextPendingConnection(), &MQTTCore, mds->interface, this);
    emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                       tr("New connection from %1").arg(QSNCleanIP(MQTTSocket->clientIPAddress().toString())),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString(),
                                       true), this);
    MQTTCore.MQTTClients.append(MQTTSocket);
    connect(MQTTSocket, SIGNAL(closeConnected(QsnMQTTSocket*)), this, SLOT(closeConnection(QsnMQTTSocket*)));
    connect(MQTTSocket, SIGNAL(onSubscribe(QString, QString, quint8)), this, SLOT(onSubscribe(QString, QString, quint8)));
    connect(MQTTSocket, SIGNAL(onUnsubscribe(QString, QString)), this, SLOT(onUnsubscribe(QString, QString)));
    connect(MQTTSocket, SIGNAL(onClearSeanse(QsnMQTTSocket*)), this, SLOT(onClearSeanse(QsnMQTTSocket*)));
    connect(MQTTSocket, SIGNAL(onReadPublish(QString, QByteArray, QString)), this, SLOT(onReadPublish(QString, QByteArray, QString)));
    emit serverUpdated();
}

void QsnWebAdapterMQTT::closeConnection(QsnMQTTSocket *socket)
{
    emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                       tr("Close connection from %1").arg(QSNCleanIP(socket->clientIPAddress().toString())),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString(),
                                       true), this);
    if (socket->isClearSeanse) onClearSeanse(socket);
    for (int i = MQTTCore.MQTTClients.count() - 1; i >= 0; i --)
        if (MQTTCore.MQTTClients[i] == socket) {
            if (MQTTCore.MQTTClients[i]->isLastWish) {
                onReadPublish(MQTTCore.MQTTClients[i]->sessionLastWishTheme, MQTTCore.MQTTClients[i]->sessionLastWishMsg.toUtf8(), MQTTCore.MQTTClients[i]->clientID);
                emit snBUSOutput(QSNLogToContainer(BUSSERV_LOG_LEVEL_warning,
                                                   tr("Publication of the last will %1 (%2 -> %3)").arg(QSNCleanIP(socket->clientIPAddress().toString()), MQTTCore.MQTTClients[i]->sessionLastWishTheme, MQTTCore.MQTTClients[i]->sessionLastWishMsg),
                                                   BUSSERV_LOG_CATEGORY_module,
                                                   QString(),
                                                   true), this);
            }
            MQTTCore.MQTTClients[i]->deleteLater();
            MQTTCore.MQTTClients.removeAt(i);
        }
    emit serverUpdated();
}

QStringList QsnWebAdapterMQTT::getTopicFromString(QString topic)
{
    QStringList t;
    int index = -1;
    QString tsrc = topic;

    do {
        index = tsrc.indexOf('/');
        if (index > 0) t.append(tsrc.left(index));
        tsrc.remove(0, index + 1);
    } while (index != -1);
    if (!tsrc.isEmpty()) t.append(tsrc);
    return t;
}

bool QsnWebAdapterMQTT::compareTopics(QStringList topic, QStringList subscribe)
{
    int count = topic.count();
    if (subscribe.count() < count) count = subscribe.count();
    for (int i = 0; i < count; i ++) {
        if (subscribe.at(i) == '#') return true;
        if (subscribe.at(i) == '+') continue;
        if (subscribe.at(i) != topic.at(i)) return false;
    }
    if (topic.count() != subscribe.count()) return false;
    return true;
}

void QsnWebAdapterMQTT::publish(QString topic, QByteArray msg, QString id)
{
    for (int i = MQTTCore.MQTTClients.count() - 1; i >= 0; i --)
        if (MQTTCore.MQTTClients.at(i)->clientID == id) {
            MQTTCore.MQTTClients.at(i)->publish(topic, msg);
            return;
        }
}

void QsnWebAdapterMQTT::timeCheck()
{
    for (int i = MQTTCore.MQTTClients.count() - 1; i >= 0; i --)
        MQTTCore.MQTTClients.at(i)->onTimeCheck();
    for (int i = 0; i < outputs.count(); i ++)
        outputs.at(i)->timeCheck();
}

QString QsnWebAdapterMQTT::subscribsToJSON()
{
    QString id = MQTTCore.MQTTClients[subscribeViewItem]->clientID;
    QString ret = "[";
    for (int i = 0; i < MQTTCore.subscribes.count(); i ++)
        if (MQTTCore.subscribes.at(i).id == id) {
            ret += "{";
            ret += QString("\"ss\": \"%1\",").arg(MQTTCore.subscribes.at(i).filter.join('/'));
            ret += QString("\"msg\": \"%1\"").arg(QSNEscapingHTMLCharacters(MQTTCore.subscribes.at(i).lastMSG));
            ret += "}";
            if (i < MQTTCore.subscribes.count() - 1) ret += ",";
        }
    ret += "]";
    return ret;
}

QString QsnWebAdapterMQTT::connectionsToJSON()
{
    QString ret = "[";
    for (int i = 0; i < MQTTCore.MQTTClients.count(); i ++) {
        ret += "{";
        ret += QString("\"id\": \"%1\",").arg(MQTTCore.MQTTClients[i]->clientID);
        ret += QString("\"ip\": \"%1\",").arg(QSNCleanIP(MQTTCore.MQTTClients[i]->clientIPAddress().toString()));
        ret += QString("\"ct\": \"%1\"").arg(QSNUpTime(MQTTCore.MQTTClients[i]->connectTime));
        ret += "}";
        if (i < MQTTCore.MQTTClients.count() - 1) ret += ",";
    }
    ret += "]";
    return ret;
}

QString QsnWebAdapterMQTT::ioToJSON()
{
    QString ret = "[";
    for (int i = 0; i < outputs.count(); i ++) {
        ret += "{";
        ret += QString("\"dir\": \"%1\",").arg(tr("Output"));
        ret += QString("\"nm\": \"%1\",").arg(outputs.at(i)->objectName());
        ret += QString("\"dt\": \"%1\"").arg(outputs.at(i)->getLabelLastData());
        ret += "}";
        if (i < outputs.count() - 1) ret += ",";
    }

    for (int i = 0; i < inputs.count(); i ++) {
        ret += "{";
        ret += QString("\"dir\": \"%1\",").arg(tr("Input"));
        ret += QString("\"nm\": \"%1\",").arg(inputs.at(i)->objectName());
        ret += QString("\"dt\": \"%1\"").arg(inputs.at(i)->getLabelLastData());
        ret += "}";
        if (i != inputs.count() - 1) ret += ",";
    }

    ret += "]";
    return ret;
}

void QsnWebAdapterMQTT::onRequestFromLocalWidgets(QString topic)
{
    struct mq {
        QString topic;
        QString item;
    };

    QVector<mq> items;
        for (int i = 0; i < outputs.count(); i ++)
            if (QSNMQTTTopicFilterCompare(outputs.at(i)->getTopic(), topic)) {
                mq m;
                m.topic = outputs.at(i)->getTopic();
                m.item = outputs.at(i)->getJsonValueItem();
                items.append(m);
            }

    for (int i = 0; i < inputs.count(); i ++)
        if (QSNMQTTTopicFilterCompare(inputs.at(i)->getTopic(), topic)) {
            mq m;
            m.topic = inputs.at(i)->getTopic();
            m.item = inputs.at(i)->getJsonValueItem();
            items.append(m);
        }
    QString json;
    while (!items.isEmpty()) {
        json = "{" + items.at(0).item;
        for (int i = items.count() - 1; i > 0; i --)
            if (items.at(0).topic == items.at(i).topic) {
                json += "," + items.at(i).item;
                items.removeAt(i);
            }

        json += "}";
        if (json != "{}") onWritePublishJson(items.at(0).topic, json);
        items.removeAt(0);
    }
}


//==================================================================================================

QsnMQTTSocket::QsnMQTTSocket(QTcpSocket *Socket, QsnMQTTCore *core, QsnInterface *interface, QObject *)
{
    setObjectName(QString(tr("snMQTT broker")));
    clientSocket = Socket;
    MQTTcore = core;
    currentState = 0;
    sessionActiveTime = 60;
    remainingTime = 0;
    isLastWish = 0;
    iface = interface;
    connectTime = QDateTime::currentDateTime();
    connect(Socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(Socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
}

QsnMQTTSocket::~QsnMQTTSocket()
{
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    clientSocket->deleteLater();
}

QHostAddress QsnMQTTSocket::clientIPAddress()
{
    return clientSocket->peerAddress();
}

void QsnMQTTSocket::slotDisconnect()
{
    clientSocket->close();
}

void QsnMQTTSocket::publish(QString topic, QByteArray msg)
{
    int size = topic.toUtf8().count() + 2 + msg.count();
    QByteArray pubPack;
    pubPack.append(0b00110000);
    baAddSize(&pubPack, size);
    baAddString(&pubPack, topic);
    pubPack.append(msg);
    clientSocket->write(pubPack);
    clientSocket->flush();
}

void QsnMQTTSocket::readyRead()
{
    switch (currentState)
    {
    case 0:state_begin(); break;
    case 1:state_getSize(); break;
    case 2:state_processing(); break;
    }
}

void QsnMQTTSocket::slotDisconnected()
{
    emit closeConnected(this);
    clientSocket->close();
    //this->deleteLater();
}

void QsnMQTTSocket::onTimeCheck()
{
    if (remainingTime == 0) return;
    remainingTime --;
    if (remainingTime != 0) return;
    iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                        tr("Connection time expired (%1)").arg(clientIPAddress().toString()),
                                        BUSSERV_LOG_CATEGORY_module,
                                        QString(),
                                        true), this);
    state_connack(1);
    slotDisconnected();
}

void QsnMQTTSocket::state_begin() {
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    clientSocket->read(&p, 1);
    packetType = (quint8)p >> 4;
    packetFlags = p & 0b00001111;
    packetSizeMult = 1;
    packetSize = 0;
    currentState = 1;
    readyRead();
}

void QsnMQTTSocket::state_getSize()
{
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    quint8 b;
    clientSocket->read(&p, 1);
    b = p;
    packetSize += (b & 127) * packetSizeMult;
    packetSizeMult *= 128;
    if (b < 128) currentState = 2;
    readyRead();
}

void QsnMQTTSocket::state_processing()
{
    if (clientSocket->bytesAvailable() < packetSize) return;
    remainingTime = sessionActiveTime;
    switch(packetType) {
    case 1: state_connect(); break;
    case 3: state_publish(); break;
    case 8: state_subscribe(); break;
    case 10: state_unsubscribe(); break;
    case 12: state_pingreq(); break;
    case 14: state_disconnect(); break;
    }
    currentState = 0;
    readyRead();
}

bool QsnMQTTSocket::state_connect()
{
    QString sessionUserName;
    QString sessionPassword;
    quint8 sessionFlags;
    quint16 size = 0;
    size = readWord();
    //read MQTT
    if (packetSize < 6) return false;
    if (size != 4) return false;
    if (readByte() != 77) return false;
    if (readByte() != 81) return false;
    if (readByte() != 84) return false;
    if (readByte() != 84) return false;
    //read protocol level
    if (packetSize < 1) return false;
    if (readByte() != 4) {
        iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                            tr("Unsupported protocol version (%1)").arg(clientIPAddress().toString()),
                                            BUSSERV_LOG_CATEGORY_module,
                                            QString("mqtt"),
                                            false), this);
        state_connack(1);
        slotDisconnected();
        return false;
    }
    sessionFlags =readByte();
    if (sessionFlags & 0b00000001) {
        iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                            tr("Protocol error (%1)").arg(clientIPAddress().toString()),
                                            BUSSERV_LOG_CATEGORY_module,
                                            QString("mqtt"),
                                            false), this);
        state_connack(1);
        slotDisconnected();
        return true;
    }
    //read active time
    if (packetSize < 1) return false;
    sessionActiveTime = readWord() + 2;
    //read ID
    if (packetSize < 2) return false;
    clientID = readString();
    //read тема последней воли
    quint8 sessionQoS = (sessionFlags >> 3) & 0b00000011;
    isLastWish = sessionFlags & 0b00000100;
    if (isLastWish) {
        sessionLastWishTheme = readString();
        sessionLastWishMsg = readString();
    }

    isLastWish = sessionFlags & 0b00000010;
    if (isLastWish) emit onClearSeanse(this);

    //read user name
    if (packetSize < 2) return false;
    if (sessionFlags & 0b10000000) sessionUserName = readString();
    //read password
    if (packetSize < 2) return false;
    if (sessionFlags & 0b01000000) sessionPassword = readString();

    if (sessionQoS > 1) {

        state_connack(1);
        slotDisconnected();
        iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_warning,
                                            tr("Unsupported QoS level (%1)").arg(clientIPAddress().toString()),
                                            BUSSERV_LOG_CATEGORY_module,
                                            QString("mqtt"),
                                            false), this);
        return true;
    }

    if (!MQTTcore->login.isEmpty() && (sessionUserName != MQTTcore->login || sessionPassword != MQTTcore->password)) {
        iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_warning,
                                            tr("Authorization failed (%1)").arg(clientIPAddress().toString()),
                                            BUSSERV_LOG_CATEGORY_module,
                                            QString("mqtt"),
                                            false), this);
        state_connack(4);
        slotDisconnected();
        return true;
    }

    iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                        tr("Connect successfully %1, ID: %2").arg(clientIPAddress().toString()),
                                        BUSSERV_LOG_CATEGORY_module,
                                        QString("mqtt"),
                                        false), this);

    state_connack(0);
    return true;
}

bool QsnMQTTSocket::state_publish()
{
    if (packetSize < 4) return false;
    //read Тема
    QString themeName = readString();
    if (((packetFlags & 0b00000110) >> 1)) packetID = readWord();

    QByteArray msg = readByteArray();
    emit onReadPublish(themeName, msg, clientID);
    iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                        tr("Publish %1 to %2 [%3]").arg(clientID, themeName, msg),
                                        BUSSERV_LOG_CATEGORY_module,
                                        QString(),
                                        true), this);
    return true;
}

bool QsnMQTTSocket::state_subscribe()
{
    if (packetSize < 4) return false;
    packetID = readWord();
    QString filter;
    quint8 qos;
    //Загрузка фильтров
    QsnMQTTFilter f;
    while (packetSize) {
        filter = readString();
        qos = readByte();
        emit onSubscribe(filter, clientID, qos);
    }
    state_suback(0);
    return true;
}

bool QsnMQTTSocket::state_unsubscribe()
{
    if (packetSize < 4) return false;
    packetID = readWord();
    QString filter;
    //Загрузка фильтров
    while (packetSize) {
        filter = readString();
        emit onUnsubscribe(filter, clientID);
        iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                            tr("Unsubscribe %1 to %2").arg(clientID, filter),
                                            BUSSERV_LOG_CATEGORY_module,
                                            QString(),
                                            true), this);
    }
    state_unsuback();
    return true;
}

bool QsnMQTTSocket::state_pingreq()
{
    state_pingresp();
    return true;
}

bool QsnMQTTSocket::state_disconnect()
{
    iface->snBUSInput(QSNLogToContainer(BUSSERV_LOG_LEVEL_information,
                                        tr("Disconnect %1").arg(clientID),
                                        BUSSERV_LOG_CATEGORY_module,
                                        QString(),
                                        true), this);
    slotDisconnected();
    return true;
}

void QsnMQTTSocket::state_connack(quint8 retCode)
{
    QByteArray ack;
    ack.append(0b00100000);
    ack.append(0b00000010);
    ack.append((char)0);
    ack.append(retCode);
    clientSocket->write(ack);
    clientSocket->flush();
}

void QsnMQTTSocket::state_puback()
{
    QByteArray ack;
    ack.append(0b01000000);
    ack.append((char)2);
    ack.append((char)0);
    ack.append((char)0);
    clientSocket->write(ack);
    clientSocket->flush();
}

void QsnMQTTSocket::state_suback(quint8 retCode)
{
    QByteArray ack;
    ack.append(0b10010000);
    ack.append((char)3);
    ack.append(packetID >> 8);
    ack.append(packetID & 0x00ff);
    ack.append(retCode);
    clientSocket->write(ack);
    clientSocket->flush();
}

void QsnMQTTSocket::state_unsuback()
{
    QByteArray ack;
    ack.append(0b10010000);
    ack.append((char)2);
    ack.append(packetID >> 8);
    ack.append(packetID & 0x00ff);
    clientSocket->write(ack);
    clientSocket->flush();
}

void QsnMQTTSocket::state_pingresp()
{
    QByteArray ack;
    ack.append(0b11010000);
    ack.append((char)0);
    clientSocket->write(ack);
    clientSocket->flush();
}

quint8 QsnMQTTSocket::readByte()
{
    char p;
    clientSocket->read(&p, 1);
    packetSize -= 1;
    return static_cast<qint8>(p);
}

quint16 QsnMQTTSocket::readWord()
{
    char p;
    clientSocket->read(&p, 1);
    quint16 w = 0;
    w += static_cast<qint16>(p) << 8;
    clientSocket->read(&p, 1);
    w += static_cast<qint16>(p);
    packetSize -= 2;
    return w;
}

QString QsnMQTTSocket::readString()
{
    quint16 size = readWord();
    if (size == 0) return QString();
    if (size > packetSize) return QString();
    QByteArray st;
    for (int i = 0; i < size; i ++) st.append(readByte());
    return QString::fromUtf8(st);
}

QByteArray QsnMQTTSocket::readByteArray()
{
    if (packetSize <= 0) return QByteArray();
    QByteArray ba;
    int size = packetSize;
    for (int i = 0; i < size; i ++) ba.append(readByte());
    return ba;
}

void QsnMQTTSocket::baAddWord(QByteArray *ba, quint16 word)
{
    ba->append(word >> 8);
    ba->append(word &0x00FF);
}

void QsnMQTTSocket::baAddString(QByteArray *ba, QString str)
{
    QByteArray ptop = str.toUtf8();
    baAddWord(ba, ptop.count());
    ba->append(ptop);
}

void QsnMQTTSocket::baAddSize(QByteArray *ba, int size)
{
    int m = 0;
    int b = 0;
    do {
        m = size / 128;
        b = size - m * 128;
        if (m) b = b + 128;
        size = m;
        ba->append(b);
    } while (m > 0);
}







