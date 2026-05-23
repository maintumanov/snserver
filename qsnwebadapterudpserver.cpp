#include "qsnwebadapterudpserver.h"

QsnWebAdapterUDPserver::QsnWebAdapterUDPserver(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    updateIndex = 0;
    setObjectName(tr("UDP server"));
    widgetUrl = "/udpserver";
    widgetIcon = "subicon-udpserver";
    udpServer = new QsnUDPserver(modules->interface);
    connect(udpServer, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), udpServer, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(udpServer, SIGNAL(stateChanged()), this, SLOT(serverUpdated()));

}

QsnWebAdapterUDPserver::~QsnWebAdapterUDPserver()
{
    disconnect(udpServer, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    disconnect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), udpServer, SLOT(snBUSInput(QSNContainer,QObject*)));
    //    delete udpServer;
    udpServer->deleteLater();
}

void QsnWebAdapterUDPserver::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("function tablelive(data) {"
                          "var table = $(\"#tablebody\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td>\" + item.nm + \"</td>\" +"
                          "\"<td>\" + item.sn + \"</td>\" +"
                          "\"<td>\" + item.ip + \"</td>\" +"
                          "\"<td>\" + item.ut + \"</td>\" +"
                          "\"<td>\" + item.lm + \"</td></tr>\");"
                          "});}");

    *functions << QString("$(\"#udplog\").click(function(){"
                          "sendState('action', 'udplog', ($(this).prop('checked')));"
                          "}); ");
}

void QsnWebAdapterUDPserver::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#udpport').text(data.udpport);");
    *fjson << QString(" $('#udpauth').text(data.udpauth);");
    *fjson << QString(" $('#udpopen').text(data.udpopen);");
    *fjson << QString(" $('#udplog').prop('checked', data.udplog);");
    *fjson << QString(" tablelive(data.dev);");
}

void QsnWebAdapterUDPserver::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "udplog") udpServer->setLogEnable(options->value("state", "false") == "true");
        }

        *returnItems << QString("\"udpport\": \"%1\"").arg(QString::number(udpServer->getPort()));
        *returnItems << QString("\"udpopen\": \"%1\"").arg(QSNBoolToYesNo(udpServer->isConnected()));
        *returnItems << QString("\"udplog\": %1").arg(QSNBoolToText(udpServer->logEnable()));
        *returnItems << QString("\"dev\": %1").arg(clientsToJSON());
    }
}

void QsnWebAdapterUDPserver::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("info", tr("INFO"), !udpServer->clients.count());
    *contents << QsnBsTapsTabAdd("devices", tr("DEVICES"), udpServer->clients.count());
    *contents << QsnBsTapsPanesBegin();
    *contents << QsnBsTapsPanelBegin("info", !udpServer->clients.count());
    *contents << QsnBsFormLabel(tr("Port"), QString::number(udpServer->getPort()), "udpport");
    *contents << QsnBsFormLabel(tr("The port is open"), QSNBoolToYesNo(udpServer->isConnected()), "udpopen");
    *contents << QsnBsFormCheckbox(tr("Allow logging"), udpServer->logEnable(), "udplog");
    *contents << QsnBsTapsPanelEnd();


    *contents << QsnBsTapsPanelBegin("devices", udpServer->clients.count());
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Name"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("SN address"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("IP address"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Up time"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Last msg"), QString());
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tablebody");

    for (int i = 0; i < udpServer->clients.count(); i ++) {
        *contents << QsnBsFormTableBodyRowAdd(udpServer->clients[i].name,
                                              QString::number(udpServer->clients[i].devAddress),
                                              QSNCleanIP(QSNCleanIP(udpServer->clients[i].ipAddress.toString())),
                                              QSNUpTime(udpServer->clients[i].connectTime),
                                              lastErrorTable(udpServer->clients[i].lastError,
                                                             udpServer->clients[i].devAddress));
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
}

void QsnWebAdapterUDPserver::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    *stream >> count; //io
    *stream >> count; //items

    quint16 nPort = static_cast<quint16>(optionsMap.value("PR", 29550).toUInt());
    QString nKey = optionsMap.value("KEY", QLatin1String("signalnet")).toString();
    udpServer->setLogEnable(optionsMap.value("log", false).toBool());

    if (udpServer->getKey() != nKey || udpServer->getPort() != nPort) {
        if (udpServer->isConnected()) adapterDisconnect();
        udpServer->setPort(nPort);
        udpServer->setKey(nKey);
    }
    udpServer->setEnabled(true);
}

void QsnWebAdapterUDPserver::setDefault()
{
    udpServer->setPort(29550);
    udpServer->setEnabled(true);
}

void QsnWebAdapterUDPserver::adapterDisconnect()
{
    udpServer->actionDisconnect();
}

void QsnWebAdapterUDPserver::adapterConnect()
{
    udpServer->actionConnect(mds->interface->getDeviceAddress());
}

void QsnWebAdapterUDPserver::urlChanged(int )
{

}

QString QsnWebAdapterUDPserver::widgetState()
{
    if (!udpServer->error().isEmpty()) return udpServer->error();
    if (udpServer->isConnected()) return tr("conn: %1").arg(udpServer->clients.count());
    else return tr("close");
}

char QsnWebAdapterUDPserver::widgetNotifState()
{
    if (!udpServer->error().isEmpty()) return 'c';
    if (udpServer->isConnected()) return (udpServer->clients.count()>0?'s':'n');
    return 'w';
}

void QsnWebAdapterUDPserver::serverUpdated()
{
    updateIndex ++;
}

int QsnWebAdapterUDPserver::indexFromAddr(int addr)
{
    if (addr == -1) return -1;
    for (int i = 0; i < udpServer->clients.count(); i ++)
        if (udpServer->clients[i].devAddress == addr) return i;
    return -1;
}

QString QsnWebAdapterUDPserver::clientsToJSON()
{
    QString ret = "[";
    for (int i = 0; i < udpServer->clients.count(); i ++) {
        ret += "{";
        ret += QString("\"nm\": \"%1\",").arg(udpServer->clients[i].name);
        ret += QString("\"sn\": \"%1\",").arg(udpServer->clients[i].devAddress);
        ret += QString("\"ip\": \"%1\",").arg(QSNCleanIP(udpServer->clients[i].ipAddress.toString()));
        ret += QString("\"ut\": \"%1\",").arg(QSNUpTime(udpServer->clients[i].connectTime));
        ret += QString("\"lm\": \"%1\"").arg(lastErrorTable(udpServer->clients[i].lastError, udpServer->clients[i].devAddress));
        ret += "}";
        if (i != udpServer->clients.count() - 1) ret += ",";
    }
    ret += "]";

    return ret;
}

QString QsnWebAdapterUDPserver::lastErrorTable(QString lasterror, quint16 devaddr)
{
    if (lasterror.isEmpty()) return QString("<a href='log?name=devicelog_a%1'>OK</a>").arg(devaddr);
    return QString("<a href='log?name=devicelog_a%1'>%2</a>").arg(devaddr).arg(lasterror);
}

