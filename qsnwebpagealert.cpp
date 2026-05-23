#include "qsnwebpagealert.h"

QsnWebPageAlert::QsnWebPageAlert(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Alarm Center"));
    widgetUrl = "/alert";
    widgetIcon = "subicon-attention";
    lastChange = QDateTime::fromString("2000-01-01_00-00-00", QLatin1String("yyyy-MM-dd_hh-mm-ss"));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebPageAlert::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("function tablealerts(data) {"
                          "var table = $(\"#tablealerts\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "var levstyle = \"\";"
                          "if (item.level) levstyle = \"  class='caution'\";"
                          "table.append(\"<tr\" + levstyle + \"><td>\" + item.state + \"</td>\" +"
                          "\"<td>\" + item.locale + \"</td>\" +"
                          "\"<td>\" + item.date + \"</td></tr>\");"
                          "});}");

    *functions << QString("$(\"#bcancel\").click(function(){"
                          "sendState('action', 'cancel', 'cancel');"
                          "}); ");

}

void QsnWebPageAlert::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" tablealerts(data.events);");
}

void QsnWebPageAlert::getContents(QStringList *contents, int )
{
    *contents << QsnBsFormBegin();
    *contents << QsnBsPageTitleBegin(tr("Alerts"));
    *contents << QsnBsButton(tr("Cancel"), "bcancel", "btn-danger");
    *contents << QsnBsPageTitleEnd();

    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Alert"));
    *contents << QsnBsFormTableTheadAdd(tr("Locale"));
    *contents << QsnBsFormTableTheadAdd(tr("Time"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tablealerts");


    for (int i = 0; i < events.count(); i ++) {
        *contents << QsnBsFormTableBodyRowAlertAdd(events[i].alarm.notificationType%2,
                                                   events[i].time.toString("dd.MM.yy hh:mm:ss"),
                                                   textStateNotif(&events[i].alarm),
                                                   events[i].eventLocale);
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsFormEnd();
}

void QsnWebPageAlert::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("action"))   {
            if (options->value("id", "") == "cancel") {
                QString state = options->value("state");
                if (state == QLatin1String("cancel")) setCancel();
            }
        }
    }

    *returnItems << QString("\"events\": %1").arg(eventsToJSON());
}

QString QsnWebPageAlert::eventsToJSON()
{
    QString ret = "[";
    for (int i = 0; i < events.count(); i ++) {
        if (ret.count() > 1) ret += ",";
        ret += "{";
        ret += QString("\"level\": %1,").arg(QSNBoolToText(events[i].alarm.notificationType%2));
        ret += QString("\"state\": \"%1\",").arg(textStateNotif(&events[i].alarm));
        ret += QString("\"date\": \"%1\",").arg(events[i].time.toString("dd.MM.yy hh:mm:ss"));
        ret += QString("\"locale\": \"%1\"").arg(events[i].eventLocale);
        ret += "}";

    }
    ret += "]";
    return ret;
}

void QsnWebPageAlert::addWidget(QsnWeb *widget)
{
    widgets.append(widget);
}

void QsnWebPageAlert::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    //if (indexIO != 1) return;
    QSNNotification notification;
    notification.idp = 0;
    switch (indexIO) {
    case 1: notification = QSNRAWtoNotification(data, 1); break;
    case 2: notification.notificationType = 0; notification.idp = mds->interface->getDeviceAddress(); break;
    case 3: notification.notificationType = 1; notification.idp = mds->interface->getDeviceAddress(); break;
    case 4: notification.notificationType = 3; notification.idp = mds->interface->getDeviceAddress(); break;
    case 5: notification.notificationType = 5; notification.idp = mds->interface->getDeviceAddress(); break;
    case 6: notification.notificationType = 7; notification.idp = mds->interface->getDeviceAddress(); break;
    case 7: notification.notificationType = 9; notification.idp = mds->interface->getDeviceAddress(); break;
    case 8: notification.notificationType = 20; break;
    default: return;
    }
    stateSet(notification, true);
}

void QsnWebPageAlert::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
}

void QsnWebPageAlert::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command != BUSSERV_ALERT ) return;
    QSNNotification notif;
    notif.notificationType = container.Signal;
    notif.idp = container.Data.toInt();
    notif.notificationSource = container.Sender;
   // if (container.Signal == BUSSERV_ALERT_cancel ) setCancel();
    if (stateSet(notif, true)) sendToOutput(notif);


}

QString QsnWebPageAlert::iconStateNotif(QSNNotification *event)
{
    switch (event->notificationType) {
    case 0: return QLatin1String();
    case 1: return QLatin1String("pFire.svg");
    case 2: return QLatin1String("pFireG.svg");
    case 3: return QLatin1String("pWater.svg");
    case 4: return QLatin1String("pWaterG.svg");
    case 5: return QLatin1String("pPower.svg");
    case 6: return QLatin1String("pPowerG.svg");
    case 7: return QLatin1String("pOxygen.svg");
    case 8: return QLatin1String("pOxygenG.svg");
    case 9: return QLatin1String("pGas.svg");
    case 10: return QLatin1String("pGasG.svg");
    }
    return QString();
}

QString QsnWebPageAlert::textStateNotif(QSNNotification *event)
{
    switch (event->notificationType) {
    case 0: return QString(tr("Cancel"));
    case 1: return QString(tr("Fire"));
    case 2: return QString(tr("No fire"));
    case 3: return QString(tr("Leaks water"));
    case 4: return QString(tr("Water leakage is stopped"));
    case 5: return QString(tr("Cut electricity"));
    case 6: return QString(tr("Power has"));
    case 7: return QString(tr("Low oxygen levels"));
    case 8: return QString(tr("Oxygen OK"));
    case 9: return QString(tr("Gas leak"));
    case 10: return QString(tr("No gas leaks"));
    }
    return QString();
}

bool QsnWebPageAlert::stateSet(QSNNotification alert, bool loged)
{
    if (QDateTime::currentDateTime() < lastChange.addSecs(2) && lastEventType == alert.notificationType) {
        lastChange = QDateTime::currentDateTime();
        return false;
    }
    statusSet(alert.notificationType);
    lastChange = QDateTime::currentDateTime();
    lastEventType = alert.notificationType;
    lastLocation = mds->locations->location(alert.idp & 0x0FFF);
    addEvent(alert);
    switch (alert.notificationType) {
    case 0:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Cancel alerts"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        break;
    case 1:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       tr("Fire{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(!)" + tr("Fire"));
        break;
    case 2:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("The fire stopped{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(i)" + tr("The fire stopped"));
        break;
    case 3:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       tr("Water leak{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(!)" + tr("Water leak"));
        break;
    case 4:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Water leakage is stopped{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(i)" + tr("Water leakage is stopped"));
        break;
    case 5:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_warning,
                                       tr("No power supply{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("/!\\" + tr("No power supply"));
        break;
    case 6:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("The power supply is restored{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(i)" + tr("The power supply is restored"));
        break;
    case 7:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       tr("Poor air quality{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(!)" + tr("Poor air quality"));
        break;
    case 8:
        if (loged) log(WL_INFORMATION, tr("The quality of the Air is normal{%1}").arg(lastLocation), QLatin1String("alert"), objectName());
        sendMessage("(i)" + tr("The quality of the Air is normal"));
        break;
    case 9:
        if (loged) log(WL_CAUTION, tr("Gas leak{%1}").arg(lastLocation), QLatin1String("alert"), objectName());
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       tr("Gas leak{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(!)" + tr("Gas leak"));
        break;
    case 10:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Gas leak stopped{%1}").arg(lastLocation),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("alert"),
                                       false), this);
        sendMessage("(i)" + tr("Gas leak stopped"));
        break;
    case 20:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Doorbell"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("doorbel"),
                                       false), this);
        sendMessage("(i)" + tr("Doorbell"));
        break;
    }

    return true;
}

void QsnWebPageAlert::statusSet(quint8 status)
{
    if (status == 0) modules()->alert_status = 0;
    else if (status & 1) modules()->alert_status |= 1 << (status >> 1);
    else modules()->alert_status &= ~(1 << ((status - 1) >> 1));
}

void QsnWebPageAlert::setCancel()
{
    QByteArray data;
    QSNNotification notification;
    notification.notificationType = 0;
    notification.idp = mds->interface->getDeviceAddress();
    notification.notificationSource = 0;
    if (!stateSet(notification)) return;
    QSNByteToRAW(&data, 0, 14);
    QSNNotificationToRAW(&data, 1, notification);
    mds->io->widgetReciveIndexSignal(0, &data, this);
//    events.clear();
}

void QsnWebPageAlert::sendToOutput(QSNNotification alert)
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 14);
    QSNNotificationToRAW(&data, 1, alert);
    mds->io->widgetReciveIndexSignal(0, &data, this);
}

void QsnWebPageAlert::setClear()
{
    events.clear();
}

void QsnWebPageAlert::sendMessage(QString message)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 3;
    container.Signal = 3;
    container.Sender = 0;
    container.info = message;
    if (!lastLocation.isEmpty()) container.info += QString("{%1}").arg(lastLocation);
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageAlert::addEvent(QSNNotification event)
{
    recentEvent ev;
    ev.time = QDateTime::currentDateTime();
    ev.alarm = event;
    //lastLocation = mds->locations->location(event.deviceAddress);
    ev.eventLocale = lastLocation;
    events.insert(0, ev);
    if (events.count() > 20) events.removeLast();
}
