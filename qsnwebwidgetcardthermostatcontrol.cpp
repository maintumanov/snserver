#include "qsnwebwidgetcardthermostatcontrol.h"

QsnWebWidgetCardThermostatControl::QsnWebWidgetCardThermostatControl(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Thermostat"));
    mds = modules;
    numIcon = 0;
    temperature = 22;
    isActual = 0;
    ind = Q_NULLPTR;

    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardThermostatControl::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString("iot.setThermostatState('thrmcon-%1', data.thrmcon%1value, data.thrmcon%1active, data.thrmcon%1state);").arg(itemID());
}

void QsnWebWidgetCardThermostatControl::getContents(QStringList *contents, int )
{
    // CARD ITEM
    *contents << QString("<li class=\"list-group-item d-flex %2\" data-unit=\"thrmcon-%1\" >").arg(itemID()).arg((isActual?" active":""));
    *contents << QsnBsIconNum(numIcon, getLink());
    *contents << QString("<p class=\"specs\">%1</p>").arg(getLabel());
    *contents << QString("<div class=\"ml-auto d-flex pb-2 mt-1\">");
    *contents << QString("<button class=\"ml-auto btn btn-outline-primary btn-mk mr-1 \" "
                         "data-unit=\"thrmcon-%1btn\" onclick=\"sendState('action', '%1', 'inc')\" "
                         "type=\"button\"%2>+</button>").arg(itemID()).arg((isActual?"":" disabled"));
    *contents << QString("<h6 class=\"badge my-auto %3 \" id=\"thrmcon-%1\">%2</h6>").arg(itemID()).arg(getTextState(), getIndState());
    *contents << QString("<button class=\"btn btn-outline-primary btn-mk ml-1\" "
                         "data-unit=\"thrmcon-%1btn\" onclick=\"sendState('action', '%1', 'dec')\" "
                         "type=\"button\"%2>-</button>").arg(itemID()).arg((isActual?"":" disabled"));
    *contents << QString("</div></li>");
}

void QsnWebWidgetCardThermostatControl::actionItem(QString, QMap<QString, QString> *options, QStringList *, qint64 )
{
    quint32 id = 0;

    if (options->contains("id")) id = options->value("id").toUInt();
    if (id != this->itemID()) return;
    if (options->contains("state")) {
        QString state = options->value("state");
        if (state == QLatin1String("inc")) {
            double newtemp = temperature + 0.5;
            if (newtemp > tMax) newtemp = tMax;
            updateTemperature(newtemp, "WEB");
        }

        if (state == QLatin1String("dec")) {
            double newtemp = temperature - 0.5;
            if (newtemp < tMin) newtemp = tMin;
            updateTemperature(newtemp, "WEB");
        }
    }
}

void QsnWebWidgetCardThermostatControl::getItemJSON(QStringList *jsonItems)
{
    *jsonItems << QsnBsJsonItemText("thrmcon", QString::number(itemID()), "value", getTextState());
    *jsonItems << QsnBsJsonItem("thrmcon", QString::number(itemID()), "active", isActual == 0?"false":"true");
    *jsonItems << QsnBsJsonItemText("thrmcon", QString::number(itemID()), "state", getIndStateJson());
}

void QsnWebWidgetCardThermostatControl::fromStream(QDataStream *stream, QString path)
{
    int id;
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;

        if (sig == QString(QLatin1String("THI"))) {
            if (ind != Q_NULLPTR) delete(ind);
            ind = new QsnWebWidgetCardThermostatIndicator(static_cast<quint32>(id), modules(), this);
            ind->fromStream(stream);
            ind->setPath(getPath());
        }
    }

    numIcon = optionsMap.value("IC", 0).toInt();
    dbName = optionsMap.value("YDB", QString()).toString();
    tMax = optionsMap.value("MX", 32).toDouble();
    tMin = optionsMap.value("MN", 18).toDouble();

    QString tsname = dbName.isEmpty()?objectName()+QString::number(itemID()):dbName;
    temperature = mds->db->tempSettings.value(tsname + "temperature", temperature).toReal();
    isActual = mds->db->tempSettings.value(tsname + "actual", 0).toInt();
}

void QsnWebWidgetCardThermostatControl::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO == 0 && data->at(0) == 9) {
        qreal t = QSNRAWtoTemperature(data, 1);
        if (t > -100) updateTemperature(t, "SIGNAL");
    }
}

QString QsnWebWidgetCardThermostatControl::widgetState()
{
    return getTextState();
}

void QsnWebWidgetCardThermostatControl::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) if (isActual > 0) isActual --;

    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_sn && container.info == getPath() && !container.Signal) readFromMQTT(&container.Data);
        if (container.Signal == BUSSERV_MGTT_request_sn && QSNMQTTTopicFilterCompare(getPath(), container.info)  && container.Signal) writeToMQTTPSN(temperature);
    }
    if (container.Command == BUSSERV_DB) {
        if (!dbName.isEmpty() && container.info == dbName && !container.Data.isEmpty()) {
            if (container.Data.at(0) == 9) updateTemperature(QSNRAWtoTemperature(&container.Data, 1), "BUSSERV_DB");
        }
    }

    if (container.Command == BUSSERV_ADAPTER_STATE && container.Signal == BUSSERV_ADAPTER_STATE_connected && isActual == 0) {
        QByteArray data;
        data[0] = 9;
        QSNTemperatureToRAW(&data, 1, static_cast<qreal>(-100));
        mds->io->widgetReciveIndexSignal(1, &data, this);
    }

    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
            QString tsname = dbName.isEmpty()?objectName()+QString::number(itemID()):dbName;
            mds->db->tempSettings.insert(tsname + "temperature", temperature);
            mds->db->tempSettings.insert(tsname + "actual", isActual);
    }
}

void QsnWebWidgetCardThermostatControl::updateTemperature(qreal newthemperature, QString source)
{
    if (source == "SIGNAL") isActual = 600;
    if (temperature <= -100) return;
    if (static_cast<int>(temperature * 100) == static_cast<int>(newthemperature * 100)) return;
    temperature = newthemperature;
    mds->settings->setValue(QString("%1_temperature").arg(itemID()), temperature);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1 [%2] - %3").arg(objectName(), tr("Server"), tr("set temperature") + QString(" %1").arg(temperature)),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("widgets"),
                                   false), this);
    QByteArray data;
    data[0] = 9;
    QSNTemperatureToRAW(&data, 1, temperature);
    mds->db->writeRAWtoYBD(dbName, &data, QDateTime::currentDateTime(), getLabel(), this);
    if (source != "BUSSERV_DB") sendDB(temperature);
    if (source != "SIGNAL") mds->io->widgetReciveIndexSignal(1, &data, this);
    writeToMQTTPSN(temperature);
}

QString QsnWebWidgetCardThermostatControl::getTextState()
{
    if (isActual == 0) return QString("---");
    return QString("%1").arg(temperature, 0, 'f', 1);
}

void QsnWebWidgetCardThermostatControl::sendDB(double temp)
{
    QByteArray d;
    d.append(9);
    QSNTemperatureToRAW(&d, 1, temp);
    mds->db->writeRAWtoYBD(dbName, &d, QDateTime::currentDateTime(), getLabel(), this);
}

void QsnWebWidgetCardThermostatControl::readFromMQTT(QByteArray *data)
{
    if (data->at(0) != 9) return;
    double temp = QSNRAWtoTemperature(data, 1);
    if (temp <= -100) return;
    updateTemperature(temp, "MQTT");
}

void QsnWebWidgetCardThermostatControl::writeToMQTTPSN(double temp)
{
    if (!mds->supportMQTT) return;
    QByteArray data;
    data.append(9);
    QSNTemperatureToRAW(&data, 1, temp);

    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_publication_sn;
    container.Sender = 0;
    container.info = getPath();
    container.Data = data;
    mds->interface->snBUSInput(container, this);
}

QString QsnWebWidgetCardThermostatControl::getLabel()
{
    return alias.isEmpty() ? objectName() : alias;
}

QString QsnWebWidgetCardThermostatControl::getLink()
{
    if (!dbName.isEmpty()) return QString("onclick=\"location.href='/graph?name=%1';\"").arg(dbName);
    return QString();
}

QString QsnWebWidgetCardThermostatControl::getIndState()
{
    if (ind != Q_NULLPTR) {
        switch (ind->state()) {
            case 1: return "bg-outline-warning";
            case 2: return "bg-outline-secondary";
        }
    }
    return "bg-outline-primary";
}

QString QsnWebWidgetCardThermostatControl::getIndStateJson()
{
    if (ind != Q_NULLPTR) {
        switch (ind->state()) {
            case 1: return "heat";
            case 2: return "cool";
        }
    }
    return "normal";
}

