#include "qsnwebitemenergydevice.h"

QsnWebItemEnergyDevice::QsnWebItemEnergyDevice(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName("Electrical device");
    mds = modules;

    period = 0;
    cost = 0;
    numIcon = 0;
    isminmax = false;
    iscolumns = false;
    issmoothing = false;
    lastData = 0;
}

void QsnWebItemEnergyDevice::getContents(QStringList *contents, int )
{
    *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/graph?name=%2&minutes=%3%4'>%1</a>")
                                    .arg(objectName(), dbName)
                                    .arg(getMinutes(period))
                                    .arg(linkOptions()),
                                    getLabelData(getMinutes(period)));
}

void QsnWebItemEnergyDevice::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items
    numIcon = optionsMap.value("IC", 0).toInt();
    dbName = optionsMap.value("YDB", QString()).toString();
}

void QsnWebItemEnergyDevice::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO != 0) return;
    if (data->count() < 7) return;
    if (data->at(0) != 19) return;
    emit readPower(data);
    setPower(data);
}

void QsnWebItemEnergyDevice::setChartMinMax(bool enable)
{
    isminmax = enable;
}

void QsnWebItemEnergyDevice::setChartSmoothing(bool enable)
{
    issmoothing = enable;
}

void QsnWebItemEnergyDevice::setChartColumns(bool enable)
{
    iscolumns = enable;
}

void QsnWebItemEnergyDevice::setPeriod(int per)
{
    period = per;
}

void QsnWebItemEnergyDevice::setCost(quint32 icost)
{
    cost = icost;
}

void QsnWebItemEnergyDevice::setCU(QString icu)
{
    CU = icu;
}

void QsnWebItemEnergyDevice::setAddrss(quint16 addr)
{
    devAddress = addr;
}

quint16 QsnWebItemEnergyDevice::address()
{
    return devAddress;
}

QString QsnWebItemEnergyDevice::location()
{
    if (devLocation.isEmpty()) return modules()->locations->location(0);
    return devLocation;
}

void QsnWebItemEnergyDevice::setPower(QByteArray *data)
{
    mds->db->writeRAWtoYBD(dbName, data, QDateTime::currentDateTime(), objectName(), this);
    if (devLocation.isEmpty()) devLocation = modules()->locations->location(QSNTypeRAWtoIDP(data) & 0x0FFF);
}

void QsnWebItemEnergyDevice::setDBname(QString name)
{
    dbName = name;
}

bool QsnWebItemEnergyDevice::isDBfound()
{
    QFile file;
    file.setFileName(QSNHomeSubPath("server", "db").absoluteFilePath(QString("%1_%2.ybd").arg(dbName).arg(QDate::currentDate().year())));
    return file.exists();
}

QString QsnWebItemEnergyDevice::getJSONContent(bool last)
{
    QString ret = "{";
    ret += QString("\"name\":\"%1\",").arg(objectName());
    ret += QString("\"data\":\"%1\",").arg(last?getLabelLastData():getLabelData(getMinutes(period)));
    ret += QString("\"link\":\"graph?name=%1&minutes=%2%3\"").arg(dbName).arg(getMinutes(period)), linkOptions();
    ret += "}";
    return ret;
}

QString QsnWebItemEnergyDevice::getLabelData(int minutes)
{
    QsnDB::dbSeries data;
    data.end = QDateTime::currentDateTime();
    data.type = 19;

    data.seriesCount = 1;
    data.name = dbName;
    data.begin = data.end.addSecs(- minutes * 60);
    mds->db->requestDataFromYBD(&data);
    lastData = data.series[0].vol.toDouble();

    return getLabelLastData();
}

QString QsnWebItemEnergyDevice::getLabelLastData()
{
    QString scost;
    if (cost > 0) scost = QString(",%1%2").arg(lastData * cost / 100000000, 0, 'f', 2).arg(CU);
    QByteArray d = QSNVariantToRAW(lastData, 19);
    return QString("%1%2").arg(QSNRAWtoScaledVolume(&d), scost);
}

QString QsnWebItemEnergyDevice::linkOptions()
{
    QString ret = "";
    if (isminmax) ret += "&minmax";
    if (iscolumns) ret += "&columns";
    if (!issmoothing) ret += "&nosmoothing";
    return ret;
}

int QsnWebItemEnergyDevice::getMinutes(int p)
{
    switch (p) {
    case 0: return 1440;
    case 1: return 10080;
    case 2: return QDate::currentDate().daysInMonth() * 1440;
    }
    return 1440;
}





