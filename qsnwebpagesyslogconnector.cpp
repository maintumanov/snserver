#include "qsnwebpagesyslogconnector.h"

QsnWebPageSyslogConnector::QsnWebPageSyslogConnector(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Syslog connector"));
    widgetUrl = "/syslogconnector";
    widgetIcon = "subicon-syslog";

    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    nameDevice = "SNSERVER";
    send_count = 0;
    connector_enable = true;
    connector_debug_enable = false;
}

QsnWebPageSyslogConnector::~QsnWebPageSyslogConnector()
{

}

void QsnWebPageSyslogConnector::endConfiguration()
{

}

void QsnWebPageSyslogConnector::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$(\"#enable\").click(function(){"
                          "sendState('action', 'enable', ($(this).prop('checked')));"
                          "}); ");
    *functions << QString("$(\"#debug\").click(function(){"
                          "sendState('action', 'debug', ($(this).prop('checked')));"
                          "}); ");
}

void QsnWebPageSyslogConnector::getFunctionsJSON(QStringList *fjson, int )
{  
    *fjson << QString(" $('#ip').text(data.ip);");
    *fjson << QString(" $('#port').text(data.port);");
    *fjson << QString(" $('#enable').prop('checked', data.enable);");
    *fjson << QString(" $('#debug').prop('checked', data.debug);");
}

void QsnWebPageSyslogConnector::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{

    if (options->contains("action")) {
        QString state = options->value("action");

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "enable") {
                connector_enable = options->value("state", "false") == "true";
                sendToLog(2, nameDevice, tr("Syslog connector is %1")
                          .arg(connector_enable?tr("enable"):tr("disable")));
            }

            if (options->value("id", "") == "debug") {
                connector_debug_enable = options->value("state", "false") == "true";
                sendToLog(connector_debug_enable?1:2, nameDevice, tr("Syslog debug mode %1")
                          .arg(connector_debug_enable?tr("enable"):tr("disable")));

            }
        }

        *returnItems << QString("\"ip\": \"%1\"").arg(syslogserver_address.toString());
        *returnItems << QString("\"port\": \"%1\"").arg(syslogserver_port);
        *returnItems << QString("\"enable\": %1").arg(QSNBoolToText(connector_enable));
        *returnItems << QString("\"debug\": %1").arg(QSNBoolToText(connector_debug_enable));


    }
}

void QsnWebPageSyslogConnector::getContents(QStringList *contents, int )
{
    *contents << QString("<div class=\"col-12 col-sm-10 ms-sm-auto\">");
    *contents << QsnBsFormSwitch(tr("Enable"), connector_enable, "enable");
    *contents << QsnBsFormSwitch(tr("Debug mode"), connector_debug_enable, "debug");
    *contents << QsnBsFormLabel(tr("Syslog server IP address"), syslogserver_address.toString(), "ip");
    *contents << QsnBsFormLabel(tr("Syslog server port"), QString::number(syslogserver_port), "port");
    *contents << QString("</div>");
}

void QsnWebPageSyslogConnector::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items

    syslogserver_address.setAddress(optionsMap.value("AD", QLatin1String("192.168.1.255")).toString());
    syslogserver_port = (optionsMap.value("PR", 514).toUInt());

    connector_debug_enable = mds->settings->value(QString("SysLogConnectorDebugMode"), false).toBool();
    connector_enable = mds->settings->value(QString("SysLogConnectorEnable"), false).toBool();
}

void QsnWebPageSyslogConnector::urlChanged(int )
{

}

QString QsnWebPageSyslogConnector::widgetState()
{
    if (!connector_enable) return tr("Disable");
    if (connector_debug_enable) return tr("Debug mode");
    return tr("Enable");
}

void QsnWebPageSyslogConnector::snBUSInput(QSNContainer container, QObject *sender)
{
    if (!connector_enable) return;
    QString text;
    QString tsender;
    quint8 wlevel = 2;
    if (container.role == QSNContainer::information) {
        wlevel = container.Command;
        if (wlevel > 4) wlevel = wlevel - 5;
        if (container.Sender == 0) {
            tsender = nameDevice;
            QStringList options = QString::fromUtf8(container.Data).split("/");
            if (options.count() > 1) text = (sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")) + " [" + options[1] + "] - " + container.info;
            else text = (sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")) + " - " + container.info;
        } else {
            tsender = QString::fromUtf8(container.Data);
            if (tsender.isEmpty()) tsender = tr("Unknown device");
            text = container.info;
        }
        sendToLog(wlevel, tsender, text);
    }

    if (connector_debug_enable) {
        if (container.role == QSNContainer::signal)
            sendToLog(2, QString("[BUS signal](%1):").arg(sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")), QString(" [%1] %2")
                      .arg(container.Signal)
                      .arg(QSNTypeRAWtoLabel(&container.Data)));

        if (container.role == QSNContainer::message)
            sendToLog(2, QString("[BUS message](%1):").arg(sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")), QString(" [%1] %2")
                      .arg(msgName(container.Command), QString(container.Data.toHex())));

        if (container.role == QSNContainer::service){
            if (container.Command == 2) return;
            sendToLog(2, QString("[BUS service](%1):").arg(sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")), QSNContainerToLogText(container, 2));
        }

        if (container.role == QSNContainer::registration) {
            QString msg;
            switch (container.Signal) {
            case 0: msg = tr("Registration request"); break;
            case 1: msg = tr("Unregistration request"); break;
            case 2: msg = tr("Registration successful"); break;
            case 3: msg = tr("Registration failed"); break;
            }
            sendToLog(2, QString("[BUS registration](%1):").arg(sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")), msg);
        }

        if (container.role == QSNContainer::authorization) {
            QString msg;
            switch (container.Signal) {
            case 1: msg = tr("Authorization successful"); break;
            case 0: msg = tr("Authorization failed"); break;
            }
            sendToLog(2, QString("[BUS authorization](%1):").arg(sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")), msg);
        }
    }
}

void QsnWebPageSyslogConnector::sendToLog(quint8 wlevel, QString source, QString text)
{
    if (text.isEmpty()) return;
    quint8 sl = 0;
    switch (wlevel) {
    case 0: sl = 2; break;
    case 1: sl = 4; break;
    case 2: sl = 6; break;
    case 3: sl = 6; break;
    case 4: sl = 6; break;
    case 5: sl = 7; break;
    }
    QByteArray datagram;
    datagram.append("<");
    datagram.append(QString::number(1 * 8 + sl).toUtf8());
    datagram.append(">");
    datagram.append(source.toUtf8());
    datagram.append(" [");
    datagram.append(mds->interface->deviceName().toUtf8());
    datagram.append("] ");
    datagram.append(text.toUtf8());
    send_count ++;
    serverSocket.writeDatagram(datagram, syslogserver_address, syslogserver_port);
    serverSocket.flush();
}

QString QsnWebPageSyslogConnector::msgName(quint8 type)
{
    switch (type)
    {
    case 0: return tr("get info");
    case 1: return tr("new device");
    case 2: return tr("get memory");
    case 3: return tr("read memory");
    case 4: return tr("set memory");
    case 5: return tr("write memory");
    case 6: return tr("set address");
    case 7: return tr("answer address");
    case 9: return tr("answer parametr");
    }
    return tr("not known");
}


