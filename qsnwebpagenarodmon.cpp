#include "qsnwebpagenarodmon.h"

QsnWebPageNarodmon::QsnWebPageNarodmon(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Narodmon"));
    widgetUrl = "/narodmon";
    widgetIcon = "subicon-narodmon";

    clientSocket = new QUdpSocket();
    lastSendTime = QDateTime(QDate(2000,1,1), QTime(0,0,0));
    nameDevice = "Signalnet";
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(&dnslookup, SIGNAL(finished()), this, SLOT(lookupFinished()));

}

QsnWebPageNarodmon::~QsnWebPageNarodmon()
{

}

void QsnWebPageNarodmon::endConfiguration()
{

}

void QsnWebPageNarodmon::getFunctions(QStringList *functions, int, QString )
{
    *functions << QString("function tablelive(data) {"
                          "var table = $(\"#nmtable\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td>\" + item.tm + \"</td>\" +"
                          "\"<td>\" + item.tp + \"</td>\" +"
                          "\"<td>\" + item.dt + \"</td></tr>\");"
                          "});}");

    *functions << QString("$(\"#nmsnd\").click(function(){"
                          "sendState('action', 'send', true);"
                          "}); ");

}

void QsnWebPageNarodmon::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#nmaddr').text(data.nmaddr);");
    *fjson << QString(" $('#nmaddrip').text(data.nmaddrip);");
    *fjson << QString(" $('#nmport').text(data.nmport);");
    *fjson << QString(" $('#nmmac').text(data.nmmac);");
    *fjson << QString(" $('#nmls').text(data.nmls);");
    *fjson << QString(" tablelive(data.sndlist);");
}

void QsnWebPageNarodmon::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{   
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("action") && options->value("id", "") == "send") sendToNarodmon();

        *returnItems << QString("\"nmport\": %1").arg(QString::number(Port));
        *returnItems << QString("\"nmaddr\": \"%1\"").arg(Address);
        *returnItems << QString("\"nmaddrip\": \"%1\"").arg(ipaddress.toString());
        *returnItems << QString("\"nmmac\": \"%1\"").arg(MAC);
        *returnItems << QString("\"nmls\": \"%1\"").arg(lastSendTime.date() == QDate(2000,1,1)?tr("none"):lastSendTime.toString("dd.MM.yyyy hh:mm:ss"));
        *returnItems << QString("\"sndlist\": %1").arg(sendToJSON());
    }
}

void QsnWebPageNarodmon::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("status", tr("STATUS"), !values.count());
    *contents << QsnBsTapsTabAdd("sended", tr("SENDED"), values.count());
    *contents << QsnBsTapsPanesBegin();
    *contents << QsnBsTapsPanelBegin("status", !values.count());
    *contents << QsnBsFormLabel(tr("Address"), Address, "nmaddr");
    *contents << QsnBsFormLabel(tr("Address IP"), ipaddress.toString(), "nmaddrip");
    *contents << QsnBsFormLabel(tr("Port"), QString::number(Port), "nmport");
    *contents << QsnBsFormLabel(tr("MAC"), MAC, "nmmac");
    *contents << QsnBsFormLabel(tr("Last send"), lastSendTime.date() == QDate(2000,1,1)?tr("none"):lastSendTime.toString("dd.MM.yyyy hh:mm:ss"), "nmls");
    *contents << QsnBsFormButton(tr("Send now"), QLatin1String("nmsnd"), QString("btn-info"));
    *contents << QsnBsTapsPanelEnd();


    *contents << QsnBsTapsPanelBegin("sended", values.count());
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Time stamp"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Type"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Data"), QString());
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("nmtable");

    for (int i = 0; i < values.count(); i ++) {
        *contents << QsnBsFormTableBodyRowAdd(values[i].shtamp.toString("dd.MM.yyyy hh:mm:ss"),
                                              QSNTypePrefix(values[i].type),
                                              values[i].value);
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
    *contents << QsnBsTapsPanesEnd();
}

void QsnWebPageNarodmon::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    // set options
    MAC = optionsMap.value("MAC", QLatin1String("AABBCCDDEEFF")).toString();
    Address = optionsMap.value("ADR", QLatin1String("narodmon.ru")).toString();
    Port = static_cast<quint16>(optionsMap.value("PRT", 8283).toUInt());
    ipaddress = QHostAddress::Null;
    dnslookup.setType(QDnsLookup::ANY);
    dnslookup.setName(Address);
    dnslookup.lookup();
    configUpdateEnd();
}

void QsnWebPageNarodmon::urlChanged(int )
{

}

void QsnWebPageNarodmon::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO == 0) addValue(data);
}

QString QsnWebPageNarodmon::widgetState()
{
    int c = 0;
    for (int i = 0; i < values.count(); i ++)
        if (values[i].issend) c ++;
    return QString("%1 %2/%3").arg(tr("Sended")).arg(c).arg(values.count());
}

char QsnWebPageNarodmon::widgetNotifState()
{
    if (ipaddress.toString().isNull()) return 'c';
    return 'n';
}

void QsnWebPageNarodmon::snBUSInput(QSNContainer container, QObject *)
{
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        configUpdateBegin();
    }
}

void QsnWebPageNarodmon::lookupFinished()
{
    QList<QDnsHostAddressRecord> records = dnslookup.hostAddressRecords();
    if (records.count() > 0) ipaddress = records.at(0).value();
    else ipaddress = QHostAddress(Address);
}

QString QsnWebPageNarodmon::sendToJSON()
{ 
    QString ret = "[";
    for (int i = 0; i < values.count(); i ++) {
        ret += "{";
        ret += QString("\"tm\": \"%1\",").arg(values[i].shtamp.toString("dd.MM.yyyy hh:mm:ss"));
        ret += QString("\"tp\": \"%1\",").arg(QSNTypePrefix(values[i].type));
        ret += QString("\"dt\": \"%1\"").arg(values[i].value);
        ret += "}";
        if (i != values.count() - 1) ret += ",";
    }
    ret += "]";

    return ret;
}

void QsnWebPageNarodmon::sendToNarodmon()
{
    if (ipaddress == QHostAddress::Null) return;

    QByteArray block;
    block.append(QString("#%1#%2\n").arg(MAC, nameDevice).toLocal8Bit());
    block.append(QString("#LAT#%1\n").arg(mds->latitude).toLocal8Bit());
    block.append(QString("#LNG#%1\n").arg(mds->longitude).toLocal8Bit());

    for (int i = 0; i < values.count(); i ++) {
        if (values[i].shtamp.secsTo(QDateTime::currentDateTime()) > 600 ) continue;
        values[i].issend = true;
        block.append(QString("#%1#%2#%3\n")
                     .arg(nmTypeFromType(values[i].type),
                          QSNAllocateNumberFromString(values[i].value),
                          QSNTypePrefix(values[i].type))
                     .toLocal8Bit());
    }
    block.append(QString("##").toUtf8());
    clientSocket->writeDatagram(block, ipaddress, Port);
    clientSocket->flush();
    lastSendTime = QDateTime::currentDateTime();

}

void QsnWebPageNarodmon::addValue(QByteArray *data)
{
    if (data->count() < 2) return;
    quint8 type = static_cast<quint8>(data->at(0));
    int index = indexValueFromType(type);
    if (index < 0) {
        if (nmTypeFromType(type).isEmpty()) return;
        valueItem vol;
        vol.type = type;
        vol.value = QSNRAWtoScaledVolume(data);
        vol.shtamp = QDateTime::currentDateTime();
        vol.issend = false;
        values.append(vol);
    } else {
        values[index].value = QSNRAWtoScaledVolume(data);
        values[index].shtamp = QDateTime::currentDateTime();
    }
    if (lastSendTime.secsTo(QDateTime::currentDateTime()) >= 300 ) sendToNarodmon();
}

int QsnWebPageNarodmon::indexValueFromType(quint8 type)
{
    for (int i = 0; i < values.count(); i ++)
        if (values[i].type == type) return i;
    return -1;
}

QString QsnWebPageNarodmon::nmTypeFromType(quint8 type)
{
    switch (type) {
    case 9: return QString("T1");
    case 10: return QString("W1");
    case 11: return QString("I1");
    case 12: return QString("U1");
    case 13: return QString("TIME");
    case 19: return QString("WH");
    case 20: return QString("H1");
    case 21: return QString("P1");
    case 24: return QString("DEG");
    case 25: return QString("L1");
    }
    return QString();
}

void QsnWebPageNarodmon::configUpdateBegin()
{
    mds->db->tempSettings.insert("narodmonCount", values.count());
    for (int i = 0; i < values.count(); i ++) {
        mds->db->tempSettings.insert(QString("narodmon%1type").arg(i), values.at(i).type);
        mds->db->tempSettings.insert(QString("narodmon%1issend").arg(i), values.at(i).issend);
        mds->db->tempSettings.insert(QString("narodmon%1shtamp").arg(i), values.at(i).shtamp);
        mds->db->tempSettings.insert(QString("narodmon%1value").arg(i), values.at(i).value);
    }
    mds->db->tempSettings.insert("narodmonlastSendTime", lastSendTime);
}

void QsnWebPageNarodmon::configUpdateEnd()
{
    int count = mds->db->tempSettings.value(QString("narodmonCount"), 0).toInt();
    valueItem vi;
    for (int i = 0; i < count; i ++) {
        vi.type = mds->db->tempSettings.value(QString("narodmon%1type").arg(i), 0).toUInt();
        vi.issend = mds->db->tempSettings.value(QString("narodmon%1issend").arg(i), false).toBool();
        vi.shtamp = mds->db->tempSettings.value(QString("narodmon%1shtamp").arg(i),  QDateTime::currentDateTime()).toDateTime();
        vi.value = mds->db->tempSettings.value(QString("narodmon%1value").arg(i), QString()).toString();
        values.append(vi);
    }

    lastSendTime = mds->db->tempSettings.value(QString("narodmonlastSendTime"),  lastSendTime).toDateTime();
}
