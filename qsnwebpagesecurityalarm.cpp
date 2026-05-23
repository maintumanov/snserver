#include "qsnwebpagesecurityalarm.h"

QsnWebPageSecurityAlarm::QsnWebPageSecurityAlarm(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    widgetUrl = "/security";
    widgetIcon = "subicon-security";
    setObjectName(tr("Security alarm"));
    widgetAccessRights = 1;

    mds->alarm_status = 0;
    isPenetration = false;
    isProtection = false;
    isItActive = false;
    lastLocation = QString();
    lastChange = QDateTime::fromString("2000-01-01_00-00-00", QLatin1String("yyyy-MM-dd_hh-mm-ss"));
    Mask = 0;
    cancelTime = 0;

    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    //mds->auth->addPermission("alarmNotification", tr("Security"), false);
}

void QsnWebPageSecurityAlarm::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("function tablealerts(data) {"
                          "var table = $(\"#tablealerts\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "var levstyle = \"\";"
                          "if (item.level = 0) levstyle = \"  class='caution'\";"
                          "if (item.level = 1) levstyle = \"  class='warning'\";"
                          "table.append(\"<tr\" + levstyle + \"><td>\" + item.msg + \"</td>\" +"
                          "\"<td>\" + item.locale + \"</td>\" +"
                          "\"<td>\" + item.time + \"</td></tr>\");"
                          "});}");

    *functions << QString("$(\"#arming\").click(function(){"
                          "sendState('action', 'arming', true);"
                          "}); ");

    *functions << QString("$(\"#secretly\").click(function(){"
                          "sendState('action', 'secretly', true);"
                          "}); ");

    *functions << QString("$(\"#disarm\").click(function(){"
                          "sendState('action', 'disarm', true);"
                          "}); ");

    *functions << QString("$(\"#cancel\").click(function(){"
                          "sendState('action', 'cancel', true);"
                          "}); ");

}

void QsnWebPageSecurityAlarm::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#state').text(data.state);");
    *fjson << QString(" tablealerts(data.events);");

}
void QsnWebPageSecurityAlarm::getContents(QStringList *contents, int )
{
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("pstate", tr("STATE"), true);
    *contents << QsnBsTapsTabAdd("events", tr("EVENTS"), false);
    *contents << QsnBsTapsPanesBegin();
    *contents << QsnBsTapsPanelBegin("pstate", true);
    *contents << QsnBsFormLabel(tr("Alarm status"), textState(), "state");
    *contents << QsnBsFormGroupButtonBegin();
    *contents << QsnBsFormGroupButton(tr("Arming"), "arming", "btn-danger");
    *contents << QsnBsFormGroupButton(tr("Secretly"), "secretly", "btn-danger");
    *contents << QsnBsFormGroupButton(tr("Disarm"), "disarm", "btn-primary");
    *contents << QsnBsFormGroupButton(tr("Cancel"), "cancel", "btn-secondary");
    *contents << QsnBsFormGroupButtonEnd();
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanelBegin("events", false);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Event"));
    *contents << QsnBsFormTableTheadAdd(tr("Locale"));
    *contents << QsnBsFormTableTheadAdd(tr("Time"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tablealerts");

    for (int i = 0; i < events.count(); i ++) {
        *contents << QsnBsFormTableBodyRowAdd(textStateNotif(&events[i].alarm),
                                              events[i].eventLocale,
                                              events[i].time.toString("dd.MM.yy hh:mm:ss"));
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
    *contents << QsnBsTapsPanesEnd();
}

void QsnWebPageSecurityAlarm::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("action") && options->value("id", "") == "arming")  setArming();
        if (state == QLatin1String("action") && options->value("id", "") == "secretly")  setArmingSecretly();
        if (state == QLatin1String("action") && options->value("id", "") == "disarm")  setDisarming();
        if (state == QLatin1String("action") && options->value("id", "") == "cancel")  setCancel();
    }
    *returnItems << QString("\"state\": \"%1\"").arg(widgetState());
    *returnItems << QString("\"events\": %1").arg(eventsToJSON());
}

void QsnWebPageSecurityAlarm::addWidget(QsnWeb *widget)
{
    widgets.append(widget);
}

void QsnWebPageSecurityAlarm::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    QSNNotification notification;
    if (indexIO == 1) {
        notification = QSNRAWtoNotification(data, 1);
        stateSet(notification, true);
    }

    if (indexIO == 2) setArming();
    if (indexIO == 3) setDisarming();
    if (indexIO == 4) setPenetration();
    if (indexIO == 5) setCancel();
    if (indexIO == 6) setArmingSecretly();

    //    if (!stateSet(notification, true)) return;
    //    bool alarmState = isProtection;
    //    if (!isProtection && alarmState && isDomination && isItActive) {
    //        setArming();
    //        setPenetration();
    //    }

}

void QsnWebPageSecurityAlarm::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items

    isItActive = mds->settings->value(QString("securityalarm_it"), isItActive).toBool();
    isSecretly = mds->settings->value(QString("securityalarm_secretly"), isSecretly).toBool();
    Mask = static_cast<quint8>(optionsMap.value("Mask", 0).toUInt());
    isLog = optionsMap.value("log", false).toBool();
}

QString QsnWebPageSecurityAlarm::widgetState()
{
    if (!isProtection) return tr("not active");
    if (!isPenetration) return tr("active");
    return tr("alarm");
}

char QsnWebPageSecurityAlarm::widgetNotifState()
{
    if (!isProtection) return 'n';
    if (!isPenetration) return 'w';
    return 'c';
}

void QsnWebPageSecurityAlarm::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 4) if (mds->settings->value(QString("securityalarm_state"), false).toBool()) setArming();
    if (container.Command == 2 && cancelTime) {
        cancelTime --;
        if (cancelTime == 0 && isPenetration) setCancel();
    }
    if (container.Command != 9 ) return;
    if (container.Signal == 0) setCancel();
    if (container.Signal == 1) setArming();
    if (container.Signal == 2) setArmingSecretly();
    if (container.Signal == 3) setDisarming();
    if (container.Signal == 4) setPenetration();

}

QString QsnWebPageSecurityAlarm::textState()
{
    if (!isProtection) return tr("Alarm is not active");
    if (isPenetration) return tr("Penetration");
    if (isSecretly) return tr("Alarm active (secretly)");
    return tr("Alarm active");
}

QString QsnWebPageSecurityAlarm::bool2text(bool state)
{
    if (state) return QLatin1String("true");
    return QLatin1String("false");
}

QString QsnWebPageSecurityAlarm::bool2visable(bool visable)
{
    if (visable) return QLatin1String("block");
    return QLatin1String("none");
}

QString QsnWebPageSecurityAlarm::iconState()
{
    if (!isProtection) return QLatin1String("pDisarm.svg");
    if (!isPenetration) return QLatin1String("pAlarm.svg");
    return QLatin1String("pPenetration.svg");
}

QString QsnWebPageSecurityAlarm::iconStateNotif(QSNNotification *alarm)
{
    switch (alarm->notificationType) {
    case 10: return QLatin1String("pAlarm.svg");
    case 11: return QLatin1String("pDisarm.svg");
    case 12: return QLatin1String("pPenetration.svg");
    case 13: return QLatin1String("pAlarm.svg");
    case 14: return QLatin1String("pAlarm.svg");
    }
    return QString();
}

QString QsnWebPageSecurityAlarm::textStateNotif(QSNNotification *alarm)
{
    switch (alarm->notificationType) {
    case 10: return QString(tr("Arming"));
    case 11: return QString(tr("Disarming"));
    case 12: return QString(tr("Penetration"));
    case 13: return QString(tr("Cancel the alarm"));
    case 14: return QString(tr("Arming (secretly)"));
    }
    return QString();
}

QString QsnWebPageSecurityAlarm::cancelVisable()
{
    if (!isProtection) return QLatin1String("none");
    if (!isPenetration) return QLatin1String("none");
    return QLatin1String("block");
}

bool QsnWebPageSecurityAlarm::stateSet(QSNNotification alarm, bool )
{
    if (QDateTime::currentDateTime() < lastChange.addSecs(1)) {
        lastChange = QDateTime::currentDateTime();
        return false;
    }
    QByteArray data;
    lastChange = QDateTime::currentDateTime();
    if (alarm.notificationType >= 10) addEvent(alarm);
    switch (alarm.notificationType) {
    case 10:
        if (isProtection) break;
        isPenetration = false;
        isProtection = true;
        isSecretly = false;
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Arming"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("security"),
                                       false), this);
        mds->settings->setValue(QString("securityalarm_state"), true);
        mds->settings->setValue(QString("securityalarm_it"), isItActive);
        mds->settings->setValue(QString("securityalarm_secretly"), isSecretly);
        mds->io->widgetReciveIndexSignal(7, &data, this);
        break;
    case 11:
        if(!isProtection) break;
        isPenetration = false;
        isProtection = false;
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Disarm"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("security"),
                                       false), this);
        mds->settings->setValue(QString("securityalarm_state"), false);
        break;
    case 12:
        if (!isPenetration) sendMessage("(!)" + tr("Penetration"));
        isPenetration = true;
        if (!lastLocation.isEmpty()) mds->interface->snBUSInput(QSNLogToContainer(
                                                                    BUSSERV_LOG_LEVEL_information,
                                                                    tr("Penetration{%1}").arg(lastLocation),
                                                                    BUSSERV_LOG_CATEGORY_module,
                                                                    QString("security"),
                                                                    false), this);;
        if (lastLocation.isEmpty()) mds->interface->snBUSInput(QSNLogToContainer(
                                                                   BUSSERV_LOG_LEVEL_information,
                                                                   tr("Penetration{%1}").arg(tr("Home")),
                                                                   BUSSERV_LOG_CATEGORY_module,
                                                                   QString("security"),
                                                                   false), this);

        cancelTime = 60;
        break;
    case 13:
        isPenetration = false;
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Cancel the alarm"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("security"),
                                       false), this);
        break;
    case 14:
        if (isProtection) break;
        isPenetration = false;
        isProtection = true;
        isSecretly = true;
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Arming (secretly)"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("security"),
                                       false), this);
        mds->settings->setValue(QString("securityalarm_state"), true);
        mds->settings->setValue(QString("securityalarm_it"), isItActive);
        mds->settings->setValue(QString("securityalarm_secretly"), isSecretly);
        mds->io->widgetReciveIndexSignal(7, &data, this);
        break;
    }

    stateUpdate();
    return true;
}

void QsnWebPageSecurityAlarm::stateUpdate()
{
    if (isPenetration) {
        mds->alarm_status = 2;
    }
    if (!isPenetration) {
        if (isProtection) mds->alarm_status = 1;
        else mds->alarm_status = 0;
    }
    mds->db->registryPublicData("security", stateData(), this);
    mds->db->registryPublicData("security_state", stateArmedData(), this);
}

void QsnWebPageSecurityAlarm::setCancel()
{
    QByteArray data;
    QSNNotification notification;
    notification.notificationType = 13;
    notification.idp = mds->interface->getDeviceAddress();
    notification.notificationSource = 0;
    if (!stateSet(notification)) return;
    QSNByteToRAW(&data, 0, 31);
    QSNNotificationToRAW(&data, 1, notification);
    mds->io->widgetReciveIndexSignal(0, &data, this);
}

void QsnWebPageSecurityAlarm::setArming()
{
    if (isProtection) return;
    QByteArray data;
    QSNNotification notification;
    notification.notificationType = 10;
    notification.notificationSource = 0;
    notification.idp = mds->interface->getDeviceAddress();
    if (!stateSet(notification)) return;
    QSNByteToRAW(&data, 0, 31);
    QSNNotificationToRAW(&data, 1, notification);
    mds->io->widgetReciveIndexSignal(0, &data, this);
    isItActive = true;
}

void QsnWebPageSecurityAlarm::setArmingSecretly()
{
    if (isProtection) return;
    QByteArray data;
    QSNNotification notification;
    notification.notificationType = 14;
    notification.idp = mds->interface->getDeviceAddress();
    notification.notificationSource = 0;
    if (!stateSet(notification)) return;
    QSNByteToRAW(&data, 0, 31);
    QSNNotificationToRAW(&data, 1, notification);
    mds->io->widgetReciveIndexSignal(0, &data, this);
    isItActive = true;
}

void QsnWebPageSecurityAlarm::setDisarming()
{
    if (!isProtection) return;
    QByteArray data;
    QSNNotification notification;
    notification.notificationType = 11;
    notification.idp = mds->interface->getDeviceAddress();
    notification.notificationSource = 0;
    if (!stateSet(notification)) return;
    QSNByteToRAW(&data, 0, 31);
    QSNNotificationToRAW(&data, 1, notification);
    mds->io->widgetReciveIndexSignal(0, &data, this);
    isItActive = false;
}

void QsnWebPageSecurityAlarm::setPenetration()
{
    cancelTime = 60;
    QByteArray data;
    QSNNotification notification;
    notification.notificationType = 12;
    notification.idp = mds->interface->getDeviceAddress();
    notification.notificationSource = 0;
    if (!stateSet(notification)) return;
    QSNByteToRAW(&data, 0, 31);
    QSNNotificationToRAW(&data, 1, notification);
    mds->io->widgetReciveIndexSignal(0, &data, this);
}

void QsnWebPageSecurityAlarm::addEvent(QSNNotification alarm)
{
    intrusionAlarm ev;
    ev.time = QDateTime::currentDateTime();
    ev.alarm = alarm;
    lastLocation = mds->locations->location(alarm.idp & 0x0FFF);
    if (ev.alarm.notificationType == 12) ev.eventLocale = lastLocation;
    events.insert(0, ev);
    if (events.count() > 10) events.removeLast();
}

void QsnWebPageSecurityAlarm::sendMessage(QString msg)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 3;
    container.Signal = 1;
    if (msg.isEmpty()) container.info = msg;
    else container.info = QString("%1{%2}").arg(msg, lastLocation);
    container.Sender = 0;
    mds->interface->snBUSInput(container, this);
}

QString QsnWebPageSecurityAlarm::eventsToJSON()
{
    QString ret = "[";
    for (int i = 0; i < events.count(); i ++) {
        if (ret.count() > 1) ret += ",";
        ret += "{";
        ret += QString("\"locale\": \"%1\",").arg(events[i].eventLocale);
        ret += QString("\"msg\": \"%1\",").arg(textStateNotif(&events[i].alarm));
        ret += QString("\"time\": \"%1\",").arg(events[i].time.toString("dd.MM.yy hh:mm:ss"));
        ret += QString("\"level\": %1").arg(3/*events[i].alarm.notificationType*/);
        ret += "}";

    }
    ret += "]";
    return ret;
}

QByteArray QsnWebPageSecurityAlarm::stateArmedData()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, isProtection);
    QSNByteToRAW(&data, 2, 13);
    return data;
}

QByteArray QsnWebPageSecurityAlarm::stateData()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 31);
    if (isPenetration) QSNByteToRAW(&data, 1, 12);
    if (isProtection) QSNByteToRAW(&data, 1, 10);
    else QSNByteToRAW(&data, 1, 11);
    return data;
}

