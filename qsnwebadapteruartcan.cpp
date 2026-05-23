#include "qsnwebadapteruartcan.h"

QsnWebAdapterUartCan::QsnWebAdapterUartCan(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Uart CAN"));
    widgetUrl = "/uartcan";
    widgetIcon = "subicon-uartcan";
    mds = modules;
    connect(&uartcan, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), &uartcan, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebAdapterUartCan::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#conn').text(data.conn);");
    *fjson << QString(" $('#port').text(data.port);");
    *fjson << QString(" $('#state').text(data.state);");
    *fjson << QString(" $('#msgreciv').text(data.msgreciv);");
    *fjson << QString(" $('#msgsent').text(data.msgsent);");
}

void QsnWebAdapterUartCan::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"conn\": \"%1\"").arg(QSNBoolToYesNo(uartcan.isConnected()));
            *returnItems << QString("\"port\": \"%1\"").arg(uartcan.getPortName());
            *returnItems << QString("\"state\": \"%1\"").arg(lastError(uartcan.error()));
            *returnItems << QString("\"msgsent\": \"%1\"").arg(uartcan.statMsgSent());
            *returnItems << QString("\"msgreciv\": \"%1\"").arg(uartcan.statMsgRecived());
        }
    }
}

void QsnWebAdapterUartCan::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormLabel(tr("Connected"), QSNBoolToYesNo(uartcan.isConnected()), "conn");
    *contents << QsnBsFormLabel(tr("Port name"), uartcan.getPortName(), "port");
    *contents << QsnBsFormLabel(tr("State"), lastError(uartcan.error()), "state");
    *contents << QsnBsFormLabel(tr("Packages sent"), QString().number(uartcan.statMsgSent()), "msgsent");
    *contents << QsnBsFormLabel(tr("Received packages"), QString().number(uartcan.statMsgRecived()), "msgreciv");
    *contents << QsnBsFormEnd();
}

void QsnWebAdapterUartCan::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    *stream >> count; //io
    *stream >> count; //items

    uartcan.setPortName(optionsMap.value("PR", QLatin1String("COM1")).toString());
    uartcan.setEnabled(true);
}

void QsnWebAdapterUartCan::adapterDisconnect()
{
    uartcan.closePort();
}

void QsnWebAdapterUartCan::adapterConnect()
{
    uartcan.openPort(mds->interface->getDeviceAddress());
}

QString QsnWebAdapterUartCan::widgetState()
{
    if (!uartcan.error().isEmpty()) return uartcan.error() + "!";
    if (uartcan.isConnected()) return tr("connected");
    return tr("not connected");
}

char QsnWebAdapterUartCan::widgetNotifState()
{
    if (!uartcan.error().isEmpty()) return 'c';
    if (uartcan.isConnected()) return 'n';
    return 'w';
}

QString QsnWebAdapterUartCan::lastError(QString lasterror)
{
//    if (lasterror.isEmpty()) return QString("<a href='log?name=devicelog_a%1'>OK</a>").arg(uartcan.objectName());
//    return QString("<a href='log?name=devicelog_udpadapter'>%1</a>").arg(lasterror);
    if (lasterror.isEmpty()) return QString("OK");
    return lasterror;
}
