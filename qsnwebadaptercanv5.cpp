#include "qsnwebadaptercanv5.h"
#include "qsnwebadaptercanv5.h"

QsnWebAdapterCANv5::QsnWebAdapterCANv5(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Adapter CANv5"));
    widgetUrl = "/canv5";
    widgetIcon = "subicon-canv5";
    mds = modules;
    connect(&CANv5, SIGNAL(snBUSOutput(QSNContainer,QObject*)), mds->interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), &CANv5, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebAdapterCANv5::getFunctions(QStringList *, int , QString )
{

}

void QsnWebAdapterCANv5::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#conn').text(data.conn);");
    *fjson << QString(" $('#port').text(data.port);");
    *fjson << QString(" $('#state').text(data.state);");
}

void QsnWebAdapterCANv5::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"conn\": \"%1\"").arg(QSNBoolToYesNo(CANv5.isConnected()));
            *returnItems << QString("\"port\": \"%1\"").arg(CANv5.getPortName());
            *returnItems << QString("\"state\": \"%1\"").arg(lastError(CANv5.error()));
        }
    }
}

void QsnWebAdapterCANv5::getContents(QStringList *contents, int )
{
    *contents << QString("<div class=\"col-12 col-sm-10 ms-sm-auto\">");
    *contents << QsnBsFormLabel(tr("Connected"), QSNBoolToYesNo(CANv5.isConnected()), "conn");
    *contents << QsnBsFormLabel(tr("Port name"), CANv5.getPortName(), "port");
    *contents << QsnBsFormLabel(tr("State"), lastError(CANv5.error()), "state");
    *contents << QString("</div>");
}

void QsnWebAdapterCANv5::fromStream(QDataStream *stream)
{
    int count;
    QMap<QString, QVariant> optionsMap;
    QString opName;
    QVariant opValue;
    QString sig;
    *stream >> sig;
    *stream >> count;

    for (int i = 0; i < count; i ++) {
        *stream >> opName;
        opValue.load(*stream);
        optionsMap.insert(opName, opValue);
    }

    *stream >> count; //io
    *stream >> count; //items

    CANv5.setPortName(optionsMap.value("PR", QLatin1String("COM1")).toString());
    CANv5.setEnabled(true);
}

void QsnWebAdapterCANv5::adapterDisconnect()
{
    CANv5.closePort();
}

void QsnWebAdapterCANv5::adapterConnect()
{
    CANv5.openPort();
}

QString QsnWebAdapterCANv5::widgetState()
{
    if (!CANv5.error().isEmpty()) return CANv5.error();
    if (CANv5.isConnected()) return tr("connected");
    else  return tr("not connected");
}

QString QsnWebAdapterCANv5::lastError(QString lasterror)
{
    if (lasterror.isEmpty()) return QString("<a href='log?name=devicelog_a%1'>OK</a>");
    return QString("<a href='log?name=devicelog_udpadapter'>%2</a>").arg(lasterror);
}
