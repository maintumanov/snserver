#include "qsnwebadapterudp.h"

QsnWebAdapterUDP::QsnWebAdapterUDP(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("UDP client"));
    widgetUrl = "/udpclient";
    widgetIcon = "subicon-udpclient";

    udpClient = new QsnUDPclient(modules->interface);
    udpClient->setRepeatMode();
    connect(udpClient, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), udpClient, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebAdapterUDP::~QsnWebAdapterUDP()
{
    disconnect(udpClient, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    disconnect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), udpClient, SLOT(snBUSInput(QSNContainer,QObject*)));
    udpClient->deleteLater();
}

void QsnWebAdapterUDP::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$(\"#udplog\").click(function(){"
                          "sendState('action', 'udplog', ($(this).prop('checked')));"
                          "}); ");
}

void QsnWebAdapterUDP::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#udpsadr').text(data.udpsadr);");
    *fjson << QString(" $('#udpsport').text(data.udpsport);");
    *fjson << QString(" $('#udpsrcport').text(data.udpsrcport);");
    *fjson << QString(" $('#udpcon').text(data.udpcon);");
    *fjson << QString(" $('#udpstate').text(data.udpstate);");
    *fjson << QString(" $('#udplog').prop('checked', data.udplog);");
}

void QsnWebAdapterUDP::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"udpsadr\": \"%1\"").arg(udpClient->getAddress());
            *returnItems << QString("\"udpsport\": \"%1\"").arg(QString::number(udpClient->getServerPort()));
            *returnItems << QString("\"udpsrcport\": \"%1\"").arg(QString::number(udpClient->getSourcePort()));
            *returnItems << QString("\"udpcon\": \"%1\"").arg(QSNBoolToYesNo(udpClient->isConnected()));
            *returnItems << QString("\"udpstate\": \"%1\"").arg(lastError(udpClient->error()));
            *returnItems << QString("\"udplog\": %1").arg(QSNBoolToText(udpClient->logEnable()));
        }

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "udplog") udpClient->setLogEnable(options->value("state", "false") == "true");
        }
    }
}

void QsnWebAdapterUDP::getContents(QStringList *contents, int )
{
    *contents << QString("<div class=\"col-12 col-sm-10 offset-sm-1\">");
    *contents << QsnBsFormLabel(tr("Connected"), QSNBoolToYesNo(udpClient->isConnected()), "udpcon");
    *contents << QsnBsFormLabel(tr("Server address"), udpClient->getAddress(), "udpsadr");
    *contents << QsnBsFormLabel(tr("Server port"), QString::number(udpClient->getServerPort()), "udpsport");
    *contents << QsnBsFormLabel(tr("Source port"), QString::number(udpClient->getSourcePort()), "udpsrcport");
    *contents << QsnBsFormLabel(tr("State"), lastError(udpClient->error()), "udpstate");
    *contents << QsnBsFormCheckbox(tr("Allow logging"), udpClient->logEnable(), "udplog");
    *contents << QString("</div>");
}

void QsnWebAdapterUDP::fromStream(QDataStream *stream)
{
    int Count;
    QMap<QString, QVariant> optionsMap;
    QString opName;
    QVariant opValue;
    QString sig;
    *stream >> sig;
    *stream >> Count;
    for (int i = 0; i < Count; i ++) {
        *stream >> opName;
        opValue.load(*stream);
        optionsMap.insert(opName, opValue);
    }

    *stream >> Count; //io
    *stream >> Count; //items

    udpClient->setServerPort(static_cast<quint16>(optionsMap.value("SP", 29550).toUInt()));
    udpClient->setSourcePort(static_cast<quint16>(optionsMap.value("OP", 29548).toUInt()));
    udpClient->setAddress(optionsMap.value("ADR", QLatin1String("192.168.1.255")).toString());
    udpClient->setKey(optionsMap.value("KEY", QLatin1String("signalnet")).toString());
    udpClient->setEnabled(true);
}

void QsnWebAdapterUDP::setDefault()
{
    udpClient->setServerPort(29550);
    udpClient->setSourcePort(29548);
    udpClient->setAddress(QLatin1String("192.168.1.255"));
    udpClient->setEnabled(true);
}

void QsnWebAdapterUDP::adapterDisconnect()
{
    udpClient->actionDisconnect();
}

void QsnWebAdapterUDP::adapterConnect()
{
    udpClient->actionConnect(mds->interface->getDeviceAddress());
}

QString QsnWebAdapterUDP::widgetState()
{
    if (!udpClient->error().isEmpty()) return udpClient->error();
    if (udpClient->isConnected()) return tr("connected");
    else  return tr("not connected");
}

char QsnWebAdapterUDP::widgetNotifState()
{
    if (!udpClient->error().isEmpty()) return 'c';
    if (udpClient->isConnected()) return 'n';
    return 'w';
}

QString QsnWebAdapterUDP::lastError(QString lasterror)
{
//    if (lasterror.isEmpty()) return QString("<a href='log?name=devicelog_udpadapter'>OK</a>");
//    return QString("<a href='log?name=devicelog_udpadapter'>%2</a>").arg(lasterror);
    if (lasterror.isEmpty()) return QString("OK");
    return QString("%1").arg(lasterror);
}

