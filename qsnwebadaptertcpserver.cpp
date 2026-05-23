#include "qsnwebadaptertcpserver.h"

QsnWebAdapterTCPserver::QsnWebAdapterTCPserver(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("TCP server"));
    widgetUrl = "/tcpserver";
    widgetIcon = "subicon-tcpserver";
    tcpServer = new QsnTCPserver(modules->interface, this);
    connect(tcpServer, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), tcpServer, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(tcpServer, SIGNAL(authorizationRequest(QString,QString,QString,QObject*)), this, SLOT(authorizationRequest(QString,QString,QString,QObject*)));
    connect(tcpServer, SIGNAL(stateChanged()), this, SLOT(serverUpdated()));
}

QsnWebAdapterTCPserver::~QsnWebAdapterTCPserver()
{
    disconnect(tcpServer, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    disconnect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), tcpServer, SLOT(snBUSInput(QSNContainer,QObject*)));
    disconnect(tcpServer, SIGNAL(authorizationRequest(QString,QString,QString,QObject*)), this, SLOT(authorizationRequest(QString,QString,QString,QObject*)));
    tcpServer->deleteLater();
}

void QsnWebAdapterTCPserver::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    *stream >> count; //io
    *stream >> count; //items

    tcpServer->setPort(static_cast<quint16>(optionsMap.value("PR", 8888).toUInt()));
    tcpServer->setEnabled(true);
}

void QsnWebAdapterTCPserver::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("function tablelive(data) {"
                          "var table = $(\"#tablebody\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td>\" + item.sn + \"</td>\" +"
                          "\"<td>\" + item.ip + \"</td>\" +"
                          "\"<td>\" + item.ut + \"</td>\" +"
                          "\"<td>\" + item.lm + \"</td></tr>\");"
                          "});}");
}

void QsnWebAdapterTCPserver::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#tcpport').text(data.tcpport);");
    *fjson << QString(" $('#tcpauth').text(data.tcpauth);");
    *fjson << QString(" $('#tcpopen').text(data.tcpopen);");
    *fjson << QString(" tablelive(data.dev);");
}

void QsnWebAdapterTCPserver::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"tcpport\": \"%1\"").arg(QString::number(tcpServer->getPort()));
            *returnItems << QString("\"tcpauth\": \"%1\"").arg(QSNBoolToYesNo(mds->auth->isTCPAuthorization()));
            *returnItems << QString("\"tcpopen\": \"%1\"").arg(QSNBoolToYesNo(tcpServer->isConnected()));
            *returnItems << QString("\"dev\": %1").arg(clientsToJSON());
        }

    }
}

void QsnWebAdapterTCPserver::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("info", tr("INFO"), !tcpServer->TCPClients.count());
    *contents << QsnBsTapsTabAdd("devices", tr("DEVICES"), tcpServer->TCPClients.count());
    *contents << QsnBsTapsPanesBegin();

    *contents << QsnBsTapsPanelBegin("info", !tcpServer->TCPClients.count());
    *contents << QsnBsFormLabel(tr("Port"), QString::number(tcpServer->getPort()), "tcpport");
    *contents << QsnBsFormLabel(tr("Identification required"), QSNBoolToYesNo(mds->auth->isTCPAuthorization()), "tcpauth");
    *contents << QsnBsFormLabel(tr("The port is open"), QSNBoolToYesNo(tcpServer->isConnected()), "tcpopen");
    *contents << QsnBsTapsPanelEnd();


    *contents << QsnBsTapsPanelBegin("devices", tcpServer->TCPClients.count());

    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("SN address"));
    *contents << QsnBsFormTableTheadAdd(tr("IP address"));
    *contents << QsnBsFormTableTheadAdd(tr("Up time"));
    *contents << QsnBsFormTableTheadAdd(tr("Last msg"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tablebody");

    for (int i = 0; i < tcpServer->TCPClients.count(); i ++) {
        *contents << QsnBsFormTableBodyRowAdd(QString::number(tcpServer->TCPClients[i]->clientDevAddress()),
                                              QSNCleanIP(tcpServer->TCPClients[i]->clientIPAddress().toString()),
                                              QSNUpTime(tcpServer->TCPClients[i]->connectionTime()),
                                              lastErrorTable(tcpServer->TCPClients[i]->clientLastError(),
                                                             tcpServer->TCPClients[i]->clientDevAddress()));
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();


    *contents << QsnBsTapsPanelEnd();
}

void QsnWebAdapterTCPserver::setDefault()
{
    tcpServer->setPort(8888);
    tcpServer->setEnabled(true);
}

quint16 QsnWebAdapterTCPserver::getPort()
{
    return tcpServer->getPort();
}

void QsnWebAdapterTCPserver::adapterDisconnect()
{
    tcpServer->actionDisconnect();
}

void QsnWebAdapterTCPserver::adapterConnect()
{
    tcpServer->actionConnect();
}

void QsnWebAdapterTCPserver::urlChanged(int )
{

}

QString QsnWebAdapterTCPserver::widgetState()
{
    if (!tcpServer->error().isEmpty()) return tcpServer->error();
    if (tcpServer->isConnected()) return tr("conn: %1").arg(tcpServer->TCPClients.count());
    else return tr("close");
}

char QsnWebAdapterTCPserver::widgetNotifState()
{
    if (!tcpServer->error().isEmpty()) return 'c';
    if (tcpServer->isConnected()) return (tcpServer->TCPClients.count()>0?'s':'n');
    return 'w';
}

void QsnWebAdapterTCPserver::authorizationRequest(QString login, QString password, QString addr, QObject *sender)
{
    if (!mds->auth->isTCPAuthorization()) {
        tcpServer->responseAuthentication(true, sender);
        mds->interface->snBUSInput(QSNLogToContainer(
                                     BUSSERV_LOG_LEVEL_caution,
                                     tr("Connection without authorization{%1}").arg(addr),
                                     BUSSERV_LOG_CATEGORY_adapter,
                                     QString("tcpserver"),
                                     true), this);
    } else {
        if (mds->auth->accountIndex(login, password) != -1) {
            mds->interface->snBUSInput(QSNLogToContainer(
                                         BUSSERV_LOG_LEVEL_caution,
                                         tr("Authentication is successful{%1}").arg(addr),
                                         BUSSERV_LOG_CATEGORY_adapter,
                                         QString("tcpserver"),
                                         true), this);
            tcpServer->responseAuthentication(true, sender);
        } else {
            tcpServer->responseAuthentication(false, sender);
            mds->interface->snBUSInput(QSNLogToContainer(
                                         BUSSERV_LOG_LEVEL_caution,
                                         tr("Authorization failed{%1}").arg(addr),
                                         BUSSERV_LOG_CATEGORY_adapter,
                                         QString("tcpserver"),
                                         true), this);
        }
    }
}

void QsnWebAdapterTCPserver::serverUpdated()
{
    updateIndex ++;
}

int QsnWebAdapterTCPserver::indexFromAddr(int addr)
{
    if (addr == -1) return -1;
    for (int i = 0; i < tcpServer->TCPClients.count(); i ++)
        if (tcpServer->TCPClients[i]->clientDevAddress() == addr) return i;
    return -1;
}

QString QsnWebAdapterTCPserver::clientsToJSON()
{   
    QString ret = "[";
    for (int i = 0; i < tcpServer->TCPClients.count(); i ++) {
        ret += "{";
        ret += QString("\"sn\": \"%1\",").arg(tcpServer->TCPClients[i]->clientDevAddress());
        ret += QString("\"ip\": \"%1\",").arg(QSNCleanIP(tcpServer->TCPClients[i]->clientIPAddress().toString()));
        ret += QString("\"ut\": \"%1\",").arg(QSNUpTime(tcpServer->TCPClients[i]->connectionTime()));
        ret += QString("\"lm\": \"%1\"").arg(lastErrorTable(tcpServer->TCPClients[i]->clientLastError(), tcpServer->TCPClients[i]->clientDevAddress()));
        ret += "}";
        if (i != tcpServer->TCPClients.count() - 1) ret += ",";
    }
    ret += "]";

    return ret;
}

QString QsnWebAdapterTCPserver::lastErrorTable(QString lasterror, quint16 devaddr)
{
    if (lasterror.isEmpty()) return QString("<a href='log?name=devicelog_a%1'>OK</a>").arg(devaddr);
    return QString("<a href='log?name=devicelog_a%1'>%2</a>").arg(devaddr).arg(lasterror);
}
