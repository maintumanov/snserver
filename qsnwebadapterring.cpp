#include "qsnwebadapterring.h"

QsnWebAdapterRing::QsnWebAdapterRing(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Ring"));
    widgetUrl = "/ring";
    widgetIcon = "subicon-ring";
    mds = modules;
    connect(&ring, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), &ring, SLOT(snBUSInput(QSNContainer,QObject*)));
}
void QsnWebAdapterRing::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#conn').text(data.conn);");
    *fjson << QString(" $('#port').text(data.port);");
    *fjson << QString(" $('#state').text(data.state);");
}

void QsnWebAdapterRing::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"conn\": \"%1\"").arg(ring.isConnected());
            *returnItems << QString("\"port\": \"%1\"").arg(ring.getPortName());
            *returnItems << QString("\"state\": \"%1\"").arg(lastError(ring.error()));
        }
    }
}

void QsnWebAdapterRing::getContents(QStringList *contents, int )
{
    *contents << QString("<div class=\"col-12 col-sm-10 ms-sm-auto\">");
    *contents << QsnBsFormLabel(tr("Connected"), QSNBoolToYesNo(ring.isConnected()), "conn");
    *contents << QsnBsFormLabel(tr("Port name"), ring.getPortName(), "port");
    *contents << QsnBsFormLabel(tr("State"), lastError(ring.error()), "state");
    *contents << QString("</div>");
}

void QsnWebAdapterRing::fromStream(QDataStream *stream)
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

    ring.setPortName(optionsMap.value("PR", "COM1").toString());
    ring.setEnabled(true);
}

void QsnWebAdapterRing::adapterDisconnect()
{
    ring.closePort();
}

void QsnWebAdapterRing::adapterConnect()
{
    ring.openPort(mds->interface->getDeviceAddress());
}

QString QsnWebAdapterRing::widgetState()
{
    if (!ring.error().isEmpty()) return ring.error();
    if (ring.isConnected()) return tr("connected");
    else  return tr("not connected");
}

char QsnWebAdapterRing::widgetNotifState()
{
    if (!ring.error().isEmpty()) return 'c';
    if (ring.isConnected()) return 'n';
    return 'w';
}

QString QsnWebAdapterRing::lastError(QString lasterror)
{
    if (lasterror.isEmpty()) return QString("<a href='log?name=devicelog_a%1'>OK</a>");
    return QString("<a href='log?name=devicelog_udpadapter'>%2</a>").arg(lasterror);
}
