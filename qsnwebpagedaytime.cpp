#include "qsnwebpagedaytime.h"

QsnWebPageDaytime::QsnWebPageDaytime(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Daytime"));
    widgetUrl = "/daytime";
    widgetIcon = "subicon-daytime";

    daytimeState = 2;
    isManual = false;
    beginDay = QTime(06, 00, 00);
    endDay = QTime(18, 00, 00);
    lastUpdateDate = QDate(2000, 1, 1);
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    mds->db->registryPublicData("daytime", stateData(), this);
}

void QsnWebPageDaytime::getFunctions(QStringList *functions, int , QString )
{   
    *functions << QString("$(\"#daystate\").click(function(){"
                          "sendState('action', 'daystate', ($(this).prop('checked')));"
                          "}); ");
}

void QsnWebPageDaytime::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#daystate').prop('checked', data.daystate);");
    *fjson << QString(" $('#sunrise').text(data.sunrise);");
    *fjson << QString(" $('#sunset').text(data.sunset);");
}

void QsnWebPageDaytime::getContents(QStringList *contents, int )
{  
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormSwitch(tr("Daytime"), daytimeState, "daystate");
    *contents << QsnBsFormLabel(tr("Sunrise"), beginDay.toString("hh:mm"), "sunrise");
    *contents << QsnBsFormLabel(tr("Sunset"), endDay.toString("hh:mm"), "sunset");
    *contents << QsnBsFormEnd();
}

void QsnWebPageDaytime::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "daystate") {
                QString state = options->value("state");
                if (state == QLatin1String("false")) toNotDay(false);
                if (state == QLatin1String("true")) toDay(false);
                isManual = true;
            }
        }
    }
    *returnItems << QString("\"daystate\": %1").arg(QSNBoolToText(daytimeState));
    *returnItems << QString("\"sunrise\": \"%1\"").arg( beginDay.toString("hh:mm"));
    *returnItems << QString("\"sunset\": \"%1\"").arg(endDay.toString("hh:mm"));
}

void QsnWebPageDaytime::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    YDBname = optionsMap.value("YDB", QString()).toString();
}

void QsnWebPageDaytime::receiveSignalIOIndex(int indexIO, QByteArray *)
{
    if (indexIO == 2) toDay(true);
    if (indexIO == 3) toNotDay(true);
    if (indexIO >= 2) isManual = true;
}

QString QsnWebPageDaytime::widgetState()
{
    if (daytimeState == 1) return tr("day");
    else return tr("not day");
}

void QsnWebPageDaytime::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 2) timeUpdate();
    //if (container.Command == 5 && container.Signal == 1) toNotDay(false, true);
    //        if (container.Command == 5) {
    //            QVariant night = mds->registry->value("night");
    //            if (night.isValid() && night.toBool()) toNotDay(false, true);
    //        }
}

void QsnWebPageDaytime::timeUpdate()
{
    if (lastUpdateDate < QDate::currentDate()) {
        beginDay = getSunTime(QDate::currentDate(), mds->latitude, mds->longitude, static_cast<double>(90.8333333333333), mds->timezone, false);
        endDay = getSunTime(QDate::currentDate(), mds->latitude, mds->longitude, static_cast<double>(90.8333333333333), mds->timezone, true);
    }

    QTime curTime = QTime::currentTime();

    if (curTime >= beginDay && curTime < endDay) {
        if (daytimeState == 1 && isManual) isManual = false;
        if (!isManual) toDay(false);
    }

    if (curTime < beginDay || curTime >= endDay) {
        if (daytimeState == 0 && isManual) isManual = false;
        if (!isManual) toNotDay(false);
    }

}

QString QsnWebPageDaytime::textState()
{
    if (daytimeState == 1) return QLatin1String("on");
    else return QLatin1String("off");
}

QString QsnWebPageDaytime::textTime()
{
    return QString(tr("from %1 to %2")).arg(beginDay.toString("hh:mm"), endDay.toString("hh:mm"));
}

bool QsnWebPageDaytime::daytimeSatus()
{
    return daytimeState;
}

void QsnWebPageDaytime::toDay(bool local)
{
    if (daytimeState == 1) return;
    daytimeState = 1;
    mds->db->registryPublicData("daytime", stateData(), this);
    QByteArray data;
    data[0] = 0;
    if (!local) mds->io->widgetReciveIndexSignal(0, &data, this);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("day"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("daytime"),
                                   false), this);
    sendDB(true);
}

void QsnWebPageDaytime::toNotDay(bool local)
{
    if (daytimeState == 0) return;
    daytimeState = 0;
    mds->db->registryPublicData("daytime", stateData(), this);
    QByteArray data;
    data[0] = 0;
    if (!local) mds->io->widgetReciveIndexSignal(1, &data, this);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("not day"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("daytime"),
                                   false), this);
    sendDB(false);
}

QTime QsnWebPageDaytime::getSunTime(QDate dt, double latitude, double longitude, double zenith, qint8 localOffset, bool sunset)
{
    quint16 N;
    double LngHour;
    double M;
    double L;
    double RA;
    double Lquadrant;
    double RAquadrant;
    double sinDec;
    double cosDec;
    double HCos;
    double H;
    double LocalT;
    double UT;
    double t = 0;
    // 1. first calculate the day of the year
    N = static_cast<quint16>(dt.dayOfYear());
    // 2. convert the longitude to hour value and calculate an approximate time
    LngHour = longitude / 15;
    if (sunset) t = static_cast<double>(N) + ((static_cast<double>(18) - LngHour) / static_cast<double>(24));
    else t = static_cast<double>(N) + ((static_cast<double>(6) - LngHour) / static_cast<double>(24));
    // 3. calculate the Sun's mean anomaly
    M = (static_cast<double>(0.9856) * t) - static_cast<double>(3.289);
    // 4. calculate the Sun's true longitude
    double rd = static_cast<double>(3.1415926535) / static_cast<double>(180);
    double dg = static_cast<double>(180) / static_cast<double>(3.1415926535);
    L = M + (static_cast<double>(1.916) * sin(M * rd)) + (static_cast<double>(0.020) * sin(static_cast<double>(2) * M * rd)) + static_cast<double>(282.634);
    while (L >= 360)  L = L - 360;
    while (L < 0) L = L + 360;
    // 5a. calculate the Sun's right ascension
    RA = atan(static_cast<double>(0.91764) * tan(L * rd)) * dg;
    while (RA >= 360)  RA = RA - 360;
    while (RA <= 360) RA = RA + 360;
    // 5b. right ascension value needs to be in the same quadrant as L
    Lquadrant = floor(L / 90) * 90;
    RAquadrant = floor(RA / 90) * 90;
    RA = RA + (Lquadrant - RAquadrant);
    // 5c. right ascension value needs to be converted into hours
    RA = RA / 15;
    // 6. calculate the Sun's declination
    sinDec = static_cast<double>(0.39782) * sin(L * rd);
    cosDec = cos(asin(sinDec));
    // 7a. calculate the Sun's local hour angle
    HCos = (cos(zenith * rd) - (sinDec * sin(latitude * rd))) / (cosDec * cos(latitude * rd));
    if (HCos > 1 || HCos < -1) return QTime(0,0,59); //TODO
    // 7b. finish calculating H and convert into hours
    if (sunset) H = acos(HCos) * dg;
    else H = static_cast<double>(360) - acos(HCos) * dg;
    H = H / 15;
    // 8. calculate local mean time of rising/setting
    LocalT = H + RA - (static_cast<double>(0.06571) * t) - static_cast<double>(6.622);
    // 9. adjust back to UTC
    UT = LocalT - LngHour;
    while (UT >= 24)  UT = UT - 24;
    while (UT < 0) UT = UT + 24;
    // 10. convert UT value to local time zone of latitude/longitude
    t = UT + localOffset;
    while (t >= 24)  t = t - 24;
    while (t < 0) t = t + 24;
    quint8 hour = static_cast<quint8>(t);
    quint8 min = static_cast<quint8>((t - hour) * 0.6 * 100);
    return QTime(hour, min, 0);
}

void QsnWebPageDaytime::sendDB(bool state)
{
    QByteArray d;
    d.append(1);
    QSNBoolToRAW(&d, 1, state);
    mds->db->writeRAWtoYBD(YDBname, &d, QDateTime::currentDateTime(), objectName(), this);
}

QByteArray QsnWebPageDaytime::stateData()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, daytimeState);
    QSNByteToRAW(&data, 2, 12);
    return data;
}

