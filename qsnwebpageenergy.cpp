#include "qsnwebpageenergy.h"

QsnWebPageEnergy::QsnWebPageEnergy(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;

    setObjectName(tr("Electricity"));
    widgetUrl = "/energy";
    widgetIcon = "subicon-energy";
    isminmax = false;
    iscolumns = false;
    issmoothing = true;
    maxColor = "#F78773";
    minColor = "#73B87A";
    defaultColor = "#455187";
    dbName = "energy";
    period = 0;
    cost = 0;
    requestQueue = 0;
    isEOM = false;
    lastDay = tr("Waiting for data");
    lastWeek = lastDay;
    lastMonth = lastDay;
    sanitationTimeout = EnergyPageTimeSanitation;
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));

}

QsnWebPageEnergy::~QsnWebPageEnergy()
{

}

void QsnWebPageEnergy::getJavaScript(QStringList *, int )
{
    //if (!dbName.isEmpty()) *script << QString("var gvar;");
}

void QsnWebPageEnergy::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("function tabletotal(data) {"
                          "var table = $(\"#tabletotal\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td><a href='/\" + item.link + \"'>\" + item.per + \"</a></td>\" +"
                          "\"<td>\" + item.data + \"</td></tr>\");"
                          "});}");

    *functions << QString("function tableudevies(data) {"
                          "var table = $(\"#tabledevs\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td><a href='/\" + item.link + \"'>\" + item.name + \"</td>\" +"
                          "\"<td>\" + item.data + \"</td></tr>\");"
                          "});}");
}

void QsnWebPageEnergy::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" tabletotal(data.total);");
    *fjson << QString(" tableudevies(data.devices);");
}

void QsnWebPageEnergy::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("total", tr("TOTAL"), true);
    *contents << QsnBsTapsTabAdd("dev", tr("DEVICES"), false);
    *contents << QsnBsTapsPanesBegin();

    *contents << QsnBsTapsPanelBegin("total", true);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Period"));
    *contents << QsnBsFormTableTheadAdd(tr("Consumption"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tabletotal");

    *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/graph?name=%2&minutes=%3%4'>%1</a>").arg(tr("Last hour"), dbName).arg(60).arg(linkOptions()), lastVolue);
    *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/graph?name=%2&minutes=%3%4'>%1</a>").arg(tr("Per day"), dbName).arg(1440).arg(linkOptions()), lastDay);
    *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/graph?name=%2&minutes=%3%4'>%1</a>").arg(tr("Per month"), dbName).arg(43200).arg(linkOptions()), lastMonth);

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanelBegin("dev", false);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Name"));
    *contents << QsnBsFormTableTheadAdd(tr("Consumption"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tabledevs");

    QStringList l;
    for (int i = 0; i < devices.count(); i ++)
        if (!l.contains(devices[i]->location())) l << devices[i]->location();

    for (int h = 0; h < l.count(); h ++) {
        for (int i = 0; i < devices.count(); i ++)
            if (devices[i]->location() == l.at(h)) {
                devices[i]->getContents(contents, -1);
            }
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanesEnd();
}

void QsnWebPageEnergy::actionItem(QString, QMap<QString, QString> *, QStringList * returnItems, qint64 )
{
    *returnItems << QString("\"total\": %1").arg(totalToJSON());
    *returnItems << QString("\"devices\": %1").arg(devicesToJSON());
}

void QsnWebPageEnergy::fromStream(QDataStream *stream)
{
    int Count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    int id;

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    // set options
    dbName = optionsMap.value("YDB", "energy").toString();
    isminmax = optionsMap.value("MM", false).toBool();
    iscolumns = optionsMap.value("CL", true).toBool();
    period = optionsMap.value("PR", 0).toInt();
    cost = static_cast<quint32>(optionsMap.value("CS", 2.7).toDouble() * 100);
    CU = optionsMap.value("CP", QString("₽")).toString();
    upperBound = optionsMap.value("UB", QString()).toString();
    lowerBound = optionsMap.value("LB", QString()).toString();
    //Есть другие параметры

    mds->io->loadIOFromStream(stream, this);

    *stream >> Count; //items
    for (int i = 0; i < Count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("Eldevice"))) {
            QsnWebItemEnergyDevice *dev = new QsnWebItemEnergyDevice(static_cast<quint32>(id), modules(), this);
            dev->fromStream(stream);
            dev->setChartColumns(iscolumns);
            dev->setChartMinMax(isminmax);
            dev->setChartSmoothing(issmoothing);
            dev->setPeriod(period);
            dev->setCost(cost);
            dev->setCU(CU);
            connect(dev, SIGNAL(readPower(QByteArray*)), this, SLOT(devicePower(QByteArray*)));
            devices.append(dev);
        }
    }

    lastVolue = getLabelData(60);
}

void QsnWebPageEnergy::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (data->count() < 7) return;
    if (data->at(0) != 19) return;
    if (indexIO == 0) addPowerData(data);
    if (indexIO == 1) addMeterData(data);
}

void QsnWebPageEnergy::endConfiguration()
{
    QsnDB::dbFileList list;
    mds->db->listYBD(&list);
    quint16 addr;
    QsnWebItemEnergyDevice *dev;
    for (int i = 0; i < list.items.count(); i ++) {
        if (list.items[i].name.indexOf(dbName) == -1) continue;
        addr = nameToAddress(list.items[i].name);
        if (addr == 0) continue;
        dev = new QsnWebItemEnergyDevice(static_cast<quint32>(65535 - addr), modules(), this);
        dev->setChartColumns(iscolumns);
        dev->setChartMinMax(isminmax);
        dev->setChartSmoothing(issmoothing);
        dev->setPeriod(period);
        dev->setCost(cost);
        dev->setCU(CU);
        dev->setObjectName(list.items[i].label);
        dev->setAddrss(addr);
        dev->setDBname(list.items[i].name);
        devices.append(dev);
    }
}

QString QsnWebPageEnergy::widgetState()
{
    return lastVolue;
}

void QsnWebPageEnergy::devicePower(QByteArray *data)
{
    mds->db->writeRAWtoYBD(dbName, data, QDateTime::currentDateTime(), objectName(), this);
}

void QsnWebPageEnergy::snBUSInput(QSNContainer container, QObject *)
{
    if (container.Command == 2) {
        sanitationTimeout --;
        if (!sanitationTimeout) {
            deviceSanitation();
            sanitationTimeout = EnergyPageTimeSanitation;
        }
    }
}

QString QsnWebPageEnergy::getLabelData(int minutes)
{
    QsnDB::dbSeries data;
    data.end = QDateTime::currentDateTime();
    data.begin = data.end.addSecs(- minutes * 60);
    data.name = dbName;
    data.type = 19;
    data.seriesCount = 1;
    mds->db->requestDataFromYBD(&data);
    //    if (minutes == 60) lastHour = data.series[0].vol.toDouble();
    //    if (minutes == getMinutes(0)) lastDay = data.series[0].vol.toDouble();
    //    if (minutes == getMinutes(1)) lastWeek = data.series[0].vol.toDouble();
    //    if (minutes == getMinutes(2)) lastMonth = data.series[0].vol.toDouble();

    //    QString volType = QSNTypePostFix(data.type);
    //    if (!volType.isEmpty()) volType = QString("(%1)").arg(volType);

    QString scost;
    if (cost > 0) scost = QString(", %1%2").arg(data.series[0].vol.toDouble() * cost / 100000000, 0, 'f', 2).arg(CU);
    QByteArray d = QSNVariantToRAW(data.series[0].vol.toDouble(), 19);
    return QString("%1%2").arg(QSNRAWtoScaledVolume(&d), scost);
}

QString QsnWebPageEnergy::linkOptions()
{
    QString ret = "";
    if (isminmax) ret += "&minmax";
    if (iscolumns) ret += "&columns";
    if (!upperBound.isEmpty()) ret += "&ubound=" + upperBound;
    if (!lowerBound.isEmpty()) ret += "&lbound=" + lowerBound;
    return ret;
}

int QsnWebPageEnergy::getMinutes(int p)
{
    switch (p) {
    case 0: return 1440;
    case 1: return 10080;
    case 2: return QDate::currentDate().daysInMonth() * 1440;
    }
    return 1440;
}

QString QsnWebPageEnergy::getPeriodLabel(int p)
{
    switch (p) {
    case 0: return tr("day");
    case 1: return tr("month");
    }
    return tr("day");
}

void QsnWebPageEnergy::addPowerData(QByteArray *data)
{
    if (!isEOM) {
        mds->db->writeRAWtoYBD(dbName, data, QDateTime::currentDateTime(), objectName(), this);
        lastVolue = getLabelData(60);
    }
    quint16 IDP = QSNTypeRAWtoIDP(data);
    if (IDP == 0) return;
    for (int i = 0; i < devices.count(); i ++)
        if (devices[i]->address() == IDP) {
            devices[i]->setObjectName(modules()->locations->getLabelFromIDP(IDP));
            devices[i]->setPower(data);
            return;
        }

    QString name = modules()->locations->getLabelFromIDP(IDP);
    if (name.isEmpty()) name = QString(tr("Device %1")).arg(IDP);
    QsnWebItemEnergyDevice *dev = new QsnWebItemEnergyDevice(static_cast<quint32>(65535 - IDP), modules(), this);
    dev->setChartColumns(iscolumns);
    dev->setChartMinMax(isminmax);
    dev->setChartSmoothing(issmoothing);
    dev->setPeriod(period);
    dev->setCost(cost);
    dev->setCU(CU);
    dev->setObjectName(name);
    dev->setAddrss(IDP);
    dev->setDBname(QString("%1_dev_%2").arg(dbName).arg(IDP));
    dev->setPower(data);
    devices.append(dev);
}

void QsnWebPageEnergy::addMeterData(QByteArray *data)
{
    mds->db->writeRAWtoYBD(dbName, data, QDateTime::currentDateTime(), objectName(), this);
    lastVolue = getLabelData(60);
    isEOM = true;
}

QString QsnWebPageEnergy::totalToJSON()
{
    switch(requestQueue) {
    case 0:
        lastVolue = getLabelData(60);
        break;
    case 1:
        lastDay = getLabelData(getMinutes(0));
        break;
    case 2:
        lastMonth = getLabelData(getMinutes(2));
        break;
    }
    QString ret = "[";

    ret += "{";
    ret += QString("\"per\":\"%1\",").arg(tr("Last hour"));
    ret += QString("\"data\":\"%1\",").arg(lastVolue);
    ret += QString("\"link\":\"graph?name=%1&minutes=%2%3\"").arg(dbName).arg(60).arg(linkOptions());
    ret += "},";

    ret += "{";
    ret += QString("\"per\":\"%1\",").arg(tr("Per day"));
    ret += QString("\"data\":\"%1\",").arg(lastDay);
    ret += QString("\"link\":\"graph?name=%1&minutes=%2%3\"").arg(dbName).arg(10080).arg(linkOptions());
    ret += "},";

    ret += "{";
    ret += QString("\"per\":\"%1\",").arg(tr("Per month"));
    ret += QString("\"data\":\"%1\",").arg(lastMonth);
    ret += QString("\"link\":\"graph?name=%1&minutes=%2%3\"").arg(dbName).arg(43200).arg(linkOptions());
    ret += "}";

    requestQueue ++;
    if (requestQueue > devices.count() + 4) requestQueue = 0;

    ret += "]";
    return ret;
}

QString QsnWebPageEnergy::devicesToJSON()
{
    QStringList l;
    for (int i = 0; i < devices.count(); i ++)
        if (!l.contains(devices[i]->location())) l << devices[i]->location();

    QString ret = "[";
    for (int h = 0; h < l.count(); h ++) {
        for (int i = 0; i < devices.count(); i ++)
            if (devices[i]->location() == l.at(h)) {
                if (ret.count() > 1) ret += ",";
                ret += devices[i]->getJSONContent(requestQueue - 4 == i);
            }
    }
    ret += "]";
    return ret;
}

quint16 QsnWebPageEnergy::nameToAddress(QString name)
{
    QString s = name;
    //    int p = s.indexOf(QDateTime::currentDateTime().toString("yyyy"));
    //    if (p == -1) return 0;
    //    s.remove(p, 4);
    bool ok = false;
    int p = QSNAllocateNumberFromString(s).toInt(&ok, 10);
    if (!ok) return 0;
    return static_cast<quint16>(p);
}

void QsnWebPageEnergy::deviceSanitation()
{
    for (int i = devices.count() - 1; i >= 0; i --)
        if (!devices[i]->isDBfound()) devices.removeAt(i);
}
