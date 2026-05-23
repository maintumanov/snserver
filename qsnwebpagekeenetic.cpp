#include "qsnwebpagekeenetic.h"

QsnWebPageKeenetic::QsnWebPageKeenetic(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Keenetic CLI"));
    widgetUrl = "/keenetic";
    widgetIcon = "subicon-router";

    isWork = false;
    isEnable = true;
    timeAbsenceSec = 0;
    timeBetweenRequestsSec = DefaultTimeBetweenRequests;
    timerCount = 10;
    isAddMACmode = false;
    terminalState = 0;

    mds->auth->addParametr("MAC", tr("Device MAC"), QString());

    connect(&terminal, SIGNAL(readyRead()), this, SLOT(terminalRead()));
    connect(&terminal, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(&terminal, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));

    mds->db->registryPublicData("keenetic", stateData(), this);
}

QsnWebPageKeenetic::~QsnWebPageKeenetic()
{

}

void QsnWebPageKeenetic::endConfiguration()
{

}

void QsnWebPageKeenetic::getFunctions(QStringList *functions, int, QString )
{
    if (isAddMACmode) return;

    *functions << QString("function tablemacs(data) {"
                          "var table = $(\"#tablemacs\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td><a href='keenetic?MAC=\" + item.mac + \"'>\" + item.mac + \"</a></td>\" +"
                          "\"<td>\" + item.name + \"</td>\" +"
                          "\"<td>\" + item.ip + \"</td>\" +"
                          "\"<td>\" + item.user + \"</td></tr>\");"
                          "});}");

    *functions << QString("function tableusers(data) {"
                          "var table = $(\"#tableusers\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td>\" + item.user + \"</td>\" +"
                          "\"<td>\" + item.state + \"</td>\" +"
                          "\"<td>\" + item.date + \"</td></tr>\");"
                          "});}");


    *functions << QString("$(\"#swenable\").click(function(){"
                          "sendState('action', 'swenable', ($(this).prop('checked')));"
                          "}); ");

}

void QsnWebPageKeenetic::getFunctionsJSON(QStringList *fjson, int )
{
    if (isAddMACmode) return;
    *fjson << QString(" $('#swenable').prop('checked', data.swenable);");
    *fjson << QString(" $('#status').text(data.status);");
    *fjson << QString(" tableusers(data.users);");
    *fjson << QString(" tablemacs(data.macs);");
}

void QsnWebPageKeenetic::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");

        if (state == QLatin1String("action"))   {

            if (options->value("id", "") == "swenable") {
                QString state = options->value("state");
                if (state == QLatin1String("false"))  {
                    isEnable = false;
                    isWork = false;
                }
                if (state == QLatin1String("true")) isEnable = true;

                mds->settings->setValue(QString("KeeneticEnable"), isEnable);
                mds->interface->snBUSInput(QSNLogToContainer(
                                               BUSSERV_LOG_LEVEL_information,
                                               tr("Keenetic is %1").arg(isEnable?tr("enable"):tr("disable")),
                                               BUSSERV_LOG_CATEGORY_module,
                                               QString("keenetic"),
                                               false), this);
            }
        }

        *returnItems << QString("\"swenable\": %1").arg(QSNBoolToText(isEnable));
        *returnItems << QString("\"status\": \"%1\"").arg(textState());
        *returnItems << QString("\"macs\": %1").arg(macToJSON());
        *returnItems << QString("\"users\": %1").arg(usersToJSON());
    }
}

void QsnWebPageKeenetic::getContents(QStringList *contents, int )
{
    if (isAddMACmode > 0) {
        *contents << QsnBsFormBegin();
        *contents << QsnBsPanelTitle(objectName() + " " + tr("Users"));
        *contents << QsnBsFormTableBegin();
        *contents << QsnBsFormTableTheadBegin();
        *contents << QsnBsFormTableTheadAdd(tr("Name"));
        *contents << QsnBsFormTableTheadAdd(tr("MAC"));
        *contents << QsnBsFormTableTheadEnd();
        *contents << QsnBsFormTableBodyBegin("tableaccaunts");

        QsnWebAuthorization::accountItem *aitem;
        for (int i = 0; i < mds->auth->countAccounts(); i ++) {//modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC")
            aitem = mds->auth->atAccount(i);
            *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/users?AID=%1&MAC=%2'>%3</a>").arg(aitem->ID).arg(getValue("MAC"), aitem->login),
                                                  modules()->auth->parametrValue(aitem, "MAC"));
        }
        *contents << QsnBsFormTableBodyEnd();
        *contents << QsnBsFormTableEnd();
        *contents << QsnBsFormEnd();
        return;
    }

    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("info", tr("INFO"), !macs.count());
    *contents << QsnBsTapsTabAdd("dev", tr("DEVICES"), macs.count());
    *contents << QsnBsTapsTabAdd("users", tr("USERS"));
    *contents << QsnBsTapsPanesBegin();

    *contents << QsnBsTapsPanelBegin("info", !macs.count());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormSwitch(tr("Enable"), isEnable, "swenable");
    *contents << QsnBsFormLabel(tr("Status"), textState(), "status");
    //    *contents << QsnBsFormFieldlabel(tr("Timer"), QTime::fromMSecsSinceStartOfDay(timerAbsence * 1000).toString("mm:ss"), "timer");
    *contents << QsnBsFormEnd();
    *contents << QsnBsTapsPanelEnd();


    *contents << QsnBsTapsPanelBegin("dev", macs.count());
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("MAC"));
    *contents << QsnBsFormTableTheadAdd(tr("Name"));
    *contents << QsnBsFormTableTheadAdd(tr("IP address"));
    *contents << QsnBsFormTableTheadAdd(tr("User"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tablemacs");
    for (int j = 0; j < macs.count(); j ++) {
        *contents << QsnBsFormTableBodyRowAdd(QString("<a href='keenetic?MAC=%1'>%1</a>").arg(macs[j].MAC), macs[j].name, macs[j].ip, macs[j].user);
    }
    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanelBegin("users", false);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("User"));
    *contents << QsnBsFormTableTheadAdd(tr("State"));
    *contents << QsnBsFormTableTheadAdd(tr("Last detection"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tableusers");

    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").isEmpty()) {
            *contents << QsnBsFormTableBodyRowAdd(modules()->auth->atAccount(i)->login,
                                                  mds->auth->getAccountPresenceState(mds->auth->atAccount(i), "keenetic")?tr("Presence"):tr("Absence"),
                                                  mds->auth->getAccountPresenceLastDetect(mds->auth->atAccount(i), "keenetic").toString("hh:mm:ss-dd.MM.yyyy"));
        }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanesEnd();
}

void QsnWebPageKeenetic::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    // set options
    login = optionsMap.value("LG", tr("admin")).toString();
    password = optionsMap.value("PS", QLatin1String("admin")).toString();
    routerAddress = optionsMap.value("AD", QLatin1String("my.keenetic.net")).toString();
    timeBetweenRequestsSec = optionsMap.value("TO", DefaultTimeBetweenRequests).toInt();
    timeAbsenceSec = optionsMap.value("TA", DefaultAbsenceTimer).toInt() * 60;

    isEnable = mds->settings->value(QString("KeeneticEnable"), true).toBool();
    configUpdateEnd();
}

void QsnWebPageKeenetic::urlChanged(int )
{
    isAddMACmode = isKey("MAC");
}

QString QsnWebPageKeenetic::widgetState()
{
    return textState();
}

char QsnWebPageKeenetic::widgetNotifState()
{
    if (!clierror.isEmpty()) return 'c';
    return 'n';
}

void QsnWebPageKeenetic::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT && isEnable) {
        if (timerCount) timerCount --;
        if (timerCount == 0) {
            check();
            timerCount = timeBetweenRequestsSec;
        }
    }
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        configUpdateBegin();
    }
}

void QsnWebPageKeenetic::terminalRead()
{
    QByteArray in;
    in = terminal.readAll();
    cleanLine(&in);
    QString line = QString(in);
    buffer += line;
    cmdprocessed(line);
}

void QsnWebPageKeenetic::socketError(QAbstractSocket::SocketError )
{
    clierror = tr("Connection error!"); //Ошибка подключения
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_caution,
                                   tr("Connection error!"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("keenetic"),
                                   true), this);
    terminalState = 0;
//    if (terminal.isOpen()) terminal.disconnectFromHost();
}

void QsnWebPageKeenetic::socketConnected()
{
//    qDebug() << "connected";
}

QString QsnWebPageKeenetic::usersToJSON()
{
    QString ret = "[";

    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").isEmpty()) {
            if (ret.count() > 1) ret += ",";
            ret += "{";
            ret += QString("\"user\": \"%1\",").arg(modules()->auth->atAccount(i)->login);
            ret += QString("\"state\": \"%1\",").arg(mds->auth->getAccountPresenceState(mds->auth->atAccount(i), "keenetic")?tr("Presence"):tr("Absence"));
            ret += QString("\"date\": \"%1\"").arg(mds->auth->getAccountPresenceLastDetect(mds->auth->atAccount(i), "keenetic").toString("hh:mm:ss-dd.MM.yyyy"));
            ret += "}";
        }

    ret += "]";
    return ret;

}

QString QsnWebPageKeenetic::macToJSON()
{
    QString ret = "[";
    for (int j = 0; j < macs.count(); j ++) {
        if (ret.count() > 1) ret += ",";
        ret += "{";
        ret += QString("\"mac\":\"%1\",").arg(macs[j].MAC);
        ret += QString("\"name\":\"%1\",").arg(macs[j].name);
        ret += QString("\"user\":\"%1\",").arg(macs[j].user);
        ret += QString("\"ip\":\"%1\"").arg(macs[j].ip);
        ret += "}";

    }
    ret += "]";
    return ret;
}

int QsnWebPageKeenetic::countUsers()
{
    int u = 0;
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").isEmpty()) u ++;
    return u;
}

void QsnWebPageKeenetic::check()
{
    switch (terminalState) {
    case 0:
        if (terminal.isOpen()) terminal.disconnectFromHost();
        terminal.connectToHost(routerAddress, 23);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        terminal.disconnectFromHost();
        terminalState = 0;
        break;
    case 5:
        terminal.write(QString("show ip arp\r\n").toLocal8Bit());
        terminalState = 4;
        break;

    }
}

void QsnWebPageKeenetic::cleanLine(QByteArray *data)
{
    QByteArray r;
    r.append(0x1B);
    r.append(0x5B);
    r.append(0x4B);
    int i = data->indexOf(r);
    while (i > -1) {
        data->remove(i, 3);
        i = data->indexOf(r);
    }
}

void QsnWebPageKeenetic::fillMAC(QString *buff)
{
    QStringList list = buff->split("@@", QString::SkipEmptyParts);
    macs.clear();
    macItem m;
    for (int i = 0; i < list.count(); i ++)
        if (list[i].count(':') >= 5) {
            m.name = list[i].mid(0, 19);
            m.name = m.name.trimmed();
            m.ip = list[i].mid(20, 17);
            m.ip = m.ip.trimmed();
            m.MAC = list[i].mid(38, 17);
            m.MAC = m.MAC.trimmed();
            userMAC(&m);
            macs.append(m);
        }
    isWork = true;
    checkPresence();
}

void QsnWebPageKeenetic::userMAC(macItem *item)
{
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").isEmpty()) {
            if (item->MAC.toLower() == mds->auth->parametrValue(mds->auth->atAccount(i), "MAC").toLower()) {
                item->user = modules()->auth->atAccount(i)->login;
                modules()->auth->setAccountPresenceItem(modules()->auth->atAccount(i), "keenetic", true, timeAbsenceSec);
            } else {
                item->user = QString();
            }
        }
}

void QsnWebPageKeenetic::checkPresence()
{
    int acount = 0;
    int hcount = 0;
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").isEmpty()) {
            acount ++;
            QDateTime dt = modules()->auth->getAccountPresenceLastDetect(modules()->auth->atAccount(i), "keenetic");
            quint64 r = dt.secsTo(QDateTime::currentDateTime());
            bool state = r < timeAbsenceSec;
            if (!state) modules()->auth->setAccountPresenceItem(modules()->auth->atAccount(i), "keenetic", false, timeAbsenceSec);
            if (state) hcount ++;
        }
    if (!acount) return;
    if (hcount) setPresence();
    else setAbsence();
}

void QsnWebPageKeenetic::cmdprocessed(QString cmd)
{
    if (cmd.indexOf("Login:", 0, Qt::CaseInsensitive) > -1) {
        if (terminalState == 2) {
            clierror = QString(tr("Incorrect login or password!")); //Не правельный логин или пароль
            mds->interface->snBUSInput(QSNLogToContainer(
                                           BUSSERV_LOG_LEVEL_caution,
                                           tr("Incorrect login or password!"),
                                           BUSSERV_LOG_CATEGORY_module,
                                           QString("keenetic"),
                                           true), this);
            isWork = false;
            terminal.disconnectFromHost();
            return;
        }
        terminal.write(QString("%1\r\n").arg(login).toLocal8Bit());
        terminalState = 2;
    } else if (cmd.indexOf("Password:", 0, Qt::CaseInsensitive) > -1) {
        terminal.write(QString("%1\r\n").arg(password).toLocal8Bit());
        buffer.clear();
        terminalState = 3;
    } else if (cmd.indexOf("(config)>", 0, Qt::CaseInsensitive) > -1) {
        if (terminalState == 3) {
            terminal.write(QString("show ip arp\r\n").toLocal8Bit());
            terminalState = 4;
        } else {
            //            terminal.disconnectFromHost();
            buffer.replace(QChar(10), "@@", Qt::CaseInsensitive);
            buffer.replace(QChar(13), "@@", Qt::CaseInsensitive);
            fillMAC(&buffer);
            buffer.clear();
            terminalState = 5;
            clierror.clear();
        }
    }
}

bool QsnWebPageKeenetic::checkMAC(QString mac)
{
    for (int j = 0; j < macs.count(); j ++)
        if (macs[j].MAC.toLower() == mac.toLower()) return true;
    return false;
}

QString QsnWebPageKeenetic::textState()
{
    if (!clierror.isEmpty()) return clierror;
    if (!isWork) return tr("unknown");
    if (isPresence) return tr("presence");
    return QString(tr("absence"));
}

void QsnWebPageKeenetic::setAbsence()
{
    if (!isEnable) return;
    QByteArray data;
    data[0] = 0;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Absence"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("keenetic"),
                                   true), this);
    mds->io->widgetReciveIndexSignal(0, &data, this);
    mds->db->registryPublicData("keenetic", stateData(), this);
    isPresence = false;
}

void QsnWebPageKeenetic::setPresence()
{
    QByteArray data;
    data[0] = 0;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Presence"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("keenetic"),
                                   true), this);
    mds->io->widgetReciveIndexSignal(1, &data, this);
    mds->db->registryPublicData("keenetic", stateData(), this);
    isPresence = true;
}

QByteArray QsnWebPageKeenetic::stateData()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, isPresence);
    QSNByteToRAW(&data, 2, 7);
    return data;
}

void QsnWebPageKeenetic::configUpdateBegin()
{
    mds->db->tempSettings.insert("keeneticcliCount", macs.count());
    for (int i = 0; i < macs.count(); i ++) {
        mds->db->tempSettings.insert(QString("keeneticcli%1MAC").arg(i), macs.at(i).MAC);
        mds->db->tempSettings.insert(QString("keeneticcli%1date").arg(i), macs.at(i).date);
        mds->db->tempSettings.insert(QString("keeneticcli%1ip").arg(i), macs.at(i).ip);
        mds->db->tempSettings.insert(QString("keeneticcli%1name").arg(i), macs.at(i).name);
        mds->db->tempSettings.insert(QString("keeneticcli%1user").arg(i), macs.at(i).user);
    }
    mds->db->tempSettings.insert("keeneticcliPresence", isPresence);
    mds->db->tempSettings.insert("keeneticcliEnable", isEnable);
}

void QsnWebPageKeenetic::configUpdateEnd()
{
    int count = mds->db->tempSettings.value(QString("keeneticcliCount"), 0).toInt();
    macItem mi;
    for (int i = 0; i < count; i ++) {
        mi.MAC = mds->db->tempSettings.value(QString("keeneticcli%1MAC").arg(i), QString()).toString();
        mi.date = mds->db->tempSettings.value(QString("keeneticcli%1date").arg(i), QDateTime::currentDateTime()).toDateTime();
        mi.ip = mds->db->tempSettings.value(QString("keeneticcli%1ip").arg(i), QString()).toString();
        mi.name = mds->db->tempSettings.value(QString("keeneticcli%1name").arg(i), QString()).toString();
        mi.user = mds->db->tempSettings.value(QString("keeneticcli%1user").arg(i), QString()).toString();
        macs.append(mi);
    }

    isPresence = mds->db->tempSettings.value(QString("keeneticcliPresence"), isPresence).toBool();
    isEnable = mds->db->tempSettings.value(QString("keeneticcliEnable"), isEnable).toBool();
}

