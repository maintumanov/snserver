#include "qsnwebwidgetcardswitch.h"

QsnWebWidgetCardSwitch::QsnWebWidgetCardSwitch(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Switch"));
    switchState = false;
    mds = modules;
    numIcon = 0;
    noticeLabel = QString();
    confirmedWait = 0;
    confirmedEnable = false;
    mqtt = Q_NULLPTR;
    rm = Q_NULLPTR;
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardSwitch::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$(\"#switch-%1\").click(function(){"
                          "sendState('action', '%1', ($(this).prop('checked')));").arg(itemID());
    *functions << QString("}); ");

}

void QsnWebWidgetCardSwitch::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString("iot.setSwitchState('switch-%1', data.switch%1state);").arg(itemID());
    if (!widgetData.isNull()) *fjson << widgetData.toString();
}

void QsnWebWidgetCardSwitch::getContents(QStringList *contents, int )
{
    // CARD ITEM
    *contents << QString("<li class=\"list-group-item d-flex %2\" data-unit=\"switch-%1\" >").arg(itemID()).arg((switchState?" active":""));
    *contents << QsnBsIconNum(numIcon, getLink());
    *contents << QString("<p class=\"specs\">%1</p>").arg(textLabel());
    *contents << QString("<label class=\"switch_m ms-auto %1\">"
                         "<input type=\"checkbox\" id=\"switch-%2\" %1>"
                         "<span></span></label>").arg(switchState?"checked":"").arg(itemID());
    *contents << QString("</li>");
}

void QsnWebWidgetCardSwitch::actionItem(QString, QMap<QString, QString> *options, QStringList *, qint64 )
{
    quint32 id = 0;

    if (options->contains("id")) id = options->value("id").toUInt();
    if (id != this->itemID()) return;
    if (options->contains("state")) {
        QString state = options->value("state");
        if ((state == QLatin1String("off") || state == QLatin1String("false")) && switchState) {
            turnOff("WEB");
            if (confirmedEnable) confirmedWait = 3;
        }
        if ((state == QLatin1String("on") || state == QLatin1String("true")) && !switchState) {
            turnOn("WEB");
            if (confirmedEnable) confirmedWait = 3;
        }
    }
}

void QsnWebWidgetCardSwitch::actionRMCode(QByteArray code)
{
    emit onRM(code);
}

void QsnWebWidgetCardSwitch::getItemJSON(QStringList *jsonItems)
{
    *jsonItems << QsnBsJsonItem("switch", QString::number(itemID()), "state", textState());
    *jsonItems << QsnBsJsonItemText("switch", QString::number(itemID()), "label", textLabel());
}

void QsnWebWidgetCardSwitch::setState(bool state)
{
    switchState = state;
}

void QsnWebWidgetCardSwitch::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    int id;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;

        if (sig == QString(QLatin1String("MQS"))) {
            if (mqtt != Q_NULLPTR) delete(mqtt);
            mqtt = new QsnWebWidgetCardSwitchMQTT(static_cast<quint32>(id), modules(), this);
            mqtt->fromStream(stream);
            mqtt->setPath(getPath());
            connect(mqtt, SIGNAL(onMQTT(bool,QString)), this, SLOT(setStateSwitch(bool,QString)));
            connect(this, SIGNAL(changeStateSwitch(bool,QString)), mqtt, SLOT(onStateChange(bool,QString)));

        }

        if (sig == QString(QLatin1String("RMS"))) {
            if (rm != Q_NULLPTR) delete(rm);
            rm = new QsnWebWidgetCardSwitchRM(static_cast<quint32>(id), modules(), this);
            rm->fromStream(stream);
            rm->setPath(getPath());
            connect(this, SIGNAL(onRM(QByteArray)), rm, SLOT(onRM(QByteArray)));
            connect(rm, SIGNAL(onStateChange(bool,QString)), this, SLOT(setStateSwitch(bool,QString)));
            connect(rm, SIGNAL(onToggle(QString)), this, SLOT(setToggle(QString)));

        }
    }
    alias = optionsMap.value("AL", QString()).toString();
    numIcon = optionsMap.value("IC", 0).toInt();
    confirmedEnable = optionsMap.value("CE", false).toBool();
    dbName = optionsMap.value("YDB", QString()).toString();
    filldb = optionsMap.value("FDB", false).toBool();

    switchState = mds->settings->value(QString("%1_switch").arg(itemID()), false).toBool();
    QString tsname = dbName.isEmpty()?objectName()+QString::number(itemID()):dbName;
    noticeLabel = mds->db->tempSettings.value(tsname + "notice", noticeLabel).toString();
    switchState = mds->db->tempSettings.value(tsname + "state", switchState).toBool();
    fdbTimer = mds->db->tempSettings.value(tsname + "fdbtimer", fdbTimer).toInt();

    mds->io->registrationAction(this, tr("On"), 0, numIcon?numIcon:77);
    mds->io->registrationAction(this, tr("Off"), 1, numIcon?numIcon:77);
}

void QsnWebWidgetCardSwitch::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    noticeLabel = QString();
    if (indexIO == 2) switchState = true;
    if (indexIO == 3) switchState = false;
    if (indexIO == 5) {
        setLabelOnOff(data);
        noticeLabel = QSNRAWtoNotice(data);
        switchState = QSNRAWtoBool(data, 1);
    }
    confirmedWait = 0;
}

void QsnWebWidgetCardSwitch::widgetRunAction(int ioIndex)
{
    switch(ioIndex) {
    case 0: turnOn("ACTION"); break;
    case 1: turnOff("ACTION"); break;
    }
}

QString QsnWebWidgetCardSwitch::widgetState()
{
    if (switchState) return QLatin1String("true");
    else return QLatin1String("false");
}

void QsnWebWidgetCardSwitch::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) timeProcessing();
    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_sn && container.info == getPath() && !container.Signal) readFromMQTT(&container.Data);
        if (container.Signal == BUSSERV_MGTT_request_sn && QSNMQTTTopicFilterCompare(getPath(), container.info)  && container.Signal) writeToMQTTPSN(switchState);
    }
    if (container.Command == BUSSERV_DB) {
        if (!dbName.isEmpty() && container.info == dbName && !container.Data.isEmpty()) {
            if (container.Data.at(0) == 1) setStateSwitch(QSNRAWtoBool(&container.Data, 1), "BUSSERV_DB");
        }
    }
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        QString tsname = dbName.isEmpty()?objectName()+QString::number(itemID()):dbName;
        mds->db->tempSettings.insert(tsname + "notice", noticeLabel);
        mds->db->tempSettings.insert(tsname + "state", switchState);
        mds->db->tempSettings.insert(tsname + "fdbtimer", fdbTimer);
    }
}

void QsnWebWidgetCardSwitch::setStateSwitch(bool state, QString source)
{
    if (state == switchState) return;
    if (state) turnOn(source);
    else turnOff(source);
}

void QsnWebWidgetCardSwitch::setToggle(QString source)
{
    if (!switchState) turnOn(source);
    else turnOff(source);
}

QString QsnWebWidgetCardSwitch::textState()
{
    if (switchState) return QLatin1String("true");
    else return QLatin1String("false");
}

QString QsnWebWidgetCardSwitch::textLabel()
{
    QString rs;
    rs = getLabel();
    if (!noticeLabel.isEmpty()) rs += " " + noticeLabel;
    return rs;
}

void QsnWebWidgetCardSwitch::turnOn(QString source)
{
    if (!switchState) {emit changeStateSwitch(true, source);}
    switchState = true;
    turnSendSignal(switchState);
    noticeLabel = QString();
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1 [%2] - %3").arg(objectName(), tr("Server"), switchState?tr("on"):tr("off")),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("widgets"),
                                   false), this);
    mds->settings->setValue(QString("%1_switch").arg(itemID()), switchState);
    writeToMQTTPSN(switchState);
    if (source != "BUSSERV_DB") sendDB(switchState);
}

void QsnWebWidgetCardSwitch::turnOff(QString source)
{
    if (switchState) {emit changeStateSwitch(false, source);}
    switchState = false;
    turnSendSignal(switchState);
    noticeLabel = QString();
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1 [%2] - %3").arg(objectName(), tr("Server"), switchState?tr("on"):tr("off")),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("widgets"),
                                   false), this);
    mds->settings->setValue(QString("%1_switch").arg(itemID()), switchState);
    writeToMQTTPSN(switchState);
    if (source != "BUSSERV_DB") sendDB(switchState);
}

void QsnWebWidgetCardSwitch::turnSendSignal(bool state)
{
    QByteArray data;
    if (state) mds->io->widgetReciveIndexSignal(0, &data, this);
    else mds->io->widgetReciveIndexSignal(1, &data, this);
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, state);
    QSNByteToRAW(&data, 2, 1);
    mds->io->widgetReciveIndexSignal(4, &data, this);
}

void QsnWebWidgetCardSwitch::timeProcessing()
{
    if (confirmedEnable) {
        if (confirmedWait > 0) confirmedWait --;
        else {
            mds->interface->snBUSInput(QSNLogToContainer(
                                           BUSSERV_LOG_LEVEL_caution,
                                           QString("%1 - %2").arg(objectName(), "ERROR SWITCH"),
                                           BUSSERV_LOG_CATEGORY_module,
                                           QString("widgets"),
                                           false), this);
            noticeLabel = QString("<font color=\"red\">%1</font>").arg(tr("ERROR"));
        }
    }

    // update db
    if (!dbName.isEmpty() && filldb) {
        if (fdbTimer < 59) fdbTimer ++;
        else sendDB(switchState);

    }
}


void QsnWebWidgetCardSwitch::sendDB(bool state)
{
    QByteArray d;
    d.append(1);
    QSNBoolToRAW(&d, 1, state);
    mds->db->writeRAWtoYBD(dbName, &d, QDateTime::currentDateTime(), getLabel(), this);
    fdbTimer = 0;
}

void QsnWebWidgetCardSwitch::setLabelOnOff(QByteArray *data)
{
    if (data->size() < 3) return;
    if (data->at(0) != 1) return;
    if (static_cast<quint8>(data->at(2)) > 100) return;
}


void QsnWebWidgetCardSwitch::readFromMQTT(QByteArray *data)
{
    if (data->at(0) != 1) return;
    setLabelOnOff(data);
    noticeLabel = QSNRAWtoNotice(data);
    if (QSNRAWtoBool(data, 1)) turnOn("BUSSERV_MGTT_SN");
    else  turnOff("BUSSERV_MGTT_SN");
}

void QsnWebWidgetCardSwitch::writeToMQTTPSN(bool state)
{
    if (!mds->supportMQTT) return;
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, state);
    QSNByteToRAW(&data, 2, 1);

    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_publication_sn;
    container.Sender = 0;
    container.info = getPath();
    container.Data = data;
    mds->interface->snBUSInput(container, this);
}


QString QsnWebWidgetCardSwitch::getLabel()
{
    return (alias.isEmpty() ? objectName() : alias);
}

QString QsnWebWidgetCardSwitch::getLink()
{
    if (dbName.isEmpty()) QString();
    QString chOptions;
    if (!filldb) chOptions += "&withoutbreaks";
    return QString("onclick=\"location.href='/graph?name=%1%2';\"").arg(dbName, chOptions);
}

