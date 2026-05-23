#include "qsnwebpagenighttime.h"

QsnWebPageNighttime::QsnWebPageNighttime(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Nighttime"));
    widgetUrl = "/nighttime";
    widgetIcon = "subicon-nighttime";
    mds = modules;
    isLog = false;
    stateNight = 2;
    isManualNight = false;
    isManualNotNight = false;
    beginNight = QTime(22, 00, 00);
    endNight = QTime(06, 00, 00);
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebPageNighttime::getFunctions(QStringList *functions, int , QString )
{

    *functions << QString("$(\"#nightstate\").click(function(){"
                          "sendState('action', 'nightstate', ($(this).prop('checked')));"
                          "}); ");

    *functions << QString("$(\"#nightTimeBegin\").change(function(){"
                          "sendState('action', 'beginnight', ($(this).val()));"
                          "}); ");

    *functions << QString("$(\"#nightTimeEnd\").change(function(){"
                          "sendState('action', 'endnight', ($(this).val()));"
                          "}); ");

}

void QsnWebPageNighttime::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#nightstate').prop('checked', data.nightstate);");
    *fjson << QString(" if ($('#nightTimeBegin').val() != data.timebegin) {$('#nightTimeBegin').val(data.timebegin)};");
    *fjson << QString(" if ($('#nightTimeEnd').val() != data.timeend) {$('#nightTimeEnd').val(data.timeend)}; ");
    *fjson << QString("iot.setClassID('nightTimeBegin', data.timebegininvalid, 'is-invalid');");
    *fjson << QString("iot.setClassID('nightTimeEnd', data.timeendinvalid, 'is-invalid');");
}

void QsnWebPageNighttime::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormSwitch(tr("Night time"), stateNight, "nightstate");
    *contents << QsnBsFormTextEdit(tr("Beginning"), beginNight.toString("hh:mm"), "nightTimeBegin", "time", !isBeginValid());
    *contents << QsnBsFormTextEdit(tr("Ending"), endNight.toString("hh:mm"), "nightTimeEnd", "time", !isEndValid());
    *contents << QsnBsFormEnd();
}


void QsnWebPageNighttime::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "nightstate") {
                QString state = options->value("state");
                if (state == QLatin1String("false")) toNotNight(false, true);
                if (state == QLatin1String("true")) toNight(false, true);
            }
        }

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "beginnight") {
                QString state = options->value("state");
                beginNight = QTime::fromString(state.replace("%3A", ":"), "HH:mm");
                mds->settings->setValue(QString("%1_nightTimeBegin").arg(itemID()), beginNight);
            }
        }

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "endnight") {
                QString state = options->value("state");
                endNight = QTime::fromString(state.replace("%3A", ":"), "HH:mm");
                mds->settings->setValue(QString("%1_nightTimeEnd").arg(itemID()), endNight);
            }
        }

    }

    *returnItems << QString("\"nightstate\": %1").arg(QSNBoolToText(stateNight));
    *returnItems << QString("\"timebegin\": \"%1\"").arg(beginNight.toString("hh:mm"));
    *returnItems << QString("\"timebegininvalid\": %1").arg(QSNBoolToText(!isBeginValid()));
    *returnItems << QString("\"timeend\": \"%1\"").arg(endNight.toString("hh:mm"));
    *returnItems << QString("\"timeendinvalid\": %1").arg(QSNBoolToText(!isEndValid()));
}

void QsnWebPageNighttime::getItemJSON(QStringList *jsonItems)
{
    *jsonItems << QString("sw%1vol: '%2'").arg(itemID()).arg(textState());
}

void QsnWebPageNighttime::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    beginNight = optionsMap.value("BG", QTime(22,00)).toTime();
    endNight = optionsMap.value("EN", QTime(06,00)).toTime();
    beginNight = mds->settings->value(QString("%1_nightTimeBegin").arg(itemID()), beginNight).toTime();
    endNight = mds->settings->value(QString("%1_nightTimeEnd").arg(itemID()), endNight).toTime();
}

void QsnWebPageNighttime::receiveSignalIOIndex(int indexIO, QByteArray *)
{
        if (indexIO == 2) toNight(true, true);
        if (indexIO == 3) toNotNight(true, true);
}

QString QsnWebPageNighttime::widgetState()
{
    if (!isEndValid() || !isBeginValid()) return tr("incorrect settings");
    if (stateNight == 1) return tr("night");
    else return tr("not night");
}

void QsnWebPageNighttime::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) timeUpdate();
    if (container.Command == BUSSERV_MODULE_STATE_CHANGE) {
        //        QVariant day = mds->registry->value("state_nighttime");
        //        if (day.isValid() && day.toBool()) toNotNight(false, true);
    }
}

void QsnWebPageNighttime::timeUpdate()
{
    QTime curTime = QTime::currentTime();

        if (isBeginValid() && (curTime >= beginNight || curTime < endNight)) {
            if (!isManualNotNight) toNight(false);
            if (isManualNight && stateNight == 1) isManualNight = false;
        }

        if (isEndValid() && (curTime < beginNight && curTime >= endNight)) {
            if (!isManualNight) toNotNight(false);
            if (isManualNotNight && stateNight == 0) isManualNotNight = false;
        }

}

bool QsnWebPageNighttime::nighttimeStatus()
{
    return stateNight;
}

QString QsnWebPageNighttime::textState()
{
    if (stateNight == 1) return QLatin1String("on");
    else return QLatin1String("off");
}

QString QsnWebPageNighttime::textTime()
{
    return QString(tr("from %1 to %2")).arg(beginNight.toString("hh:mm"), endNight.toString("hh:mm"));
}

void QsnWebPageNighttime::toNight(bool local, bool isManual)
{
    if (stateNight == 1) return;
    if (isManual) isManualNight = true;
    isManualNotNight = false;
    stateNight = 1;
    QByteArray data = stateData();
    if (!local) mds->io->widgetReciveIndexSignal(0, &data, this);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("night"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("nighttime"),
                                   false), this);
    sendDB(&data);
}

void QsnWebPageNighttime::toNotNight(bool local, bool isManual)
{
    if (stateNight == 0) return;
    if (isManual) isManualNotNight = true;
    isManualNight = false;
    stateNight = 0;

    QByteArray data = stateData();
    if (!local) mds->io->widgetReciveIndexSignal(1, &data, this);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("not night"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("nighttime"),
                                   false), this);
    sendDB(&data);
}

void QsnWebPageNighttime::sendDB(QByteArray *data)
{
    if (YDBname.isEmpty()) mds->db->registryPublicData("nighttime", *data, this);
    else mds->db->writeRAWtoYBD(YDBname, data, QDateTime::currentDateTime(), objectName(), this);
}

QByteArray QsnWebPageNighttime::stateData()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, stateNight);
    QSNByteToRAW(&data, 2, 11);
    return data;
}

bool QsnWebPageNighttime::isBeginValid()
{
    return beginNight >= QTime(15, 0, 0);
}

bool QsnWebPageNighttime::isEndValid()
{
    return endNight < QTime(12, 0, 0);
}

