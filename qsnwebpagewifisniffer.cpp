#include "qsnwebpagewifisniffer.h"

QsnWebPageWiFiSniffer::QsnWebPageWiFiSniffer(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("WiFi Sniffer"));
    widgetUrl = "/wifisniffer";
    widgetIcon = "subicon-wifisniffer";

    detectedCount = 0;
    maxDetectTimeSec = 0;
    isEnable = true;
    mds->auth->addParametr("MAC", tr("Device MAC"), QString());

    serial = new QSerialPort(this);
    serialAnalysisState = 0;

    connect(serial, SIGNAL(readyRead()), this, SLOT(analysisCicle()));
    connect(serial, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(serialError(QSerialPort::SerialPortError)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageWiFiSniffer::~QsnWebPageWiFiSniffer()
{

}

void QsnWebPageWiFiSniffer::endConfiguration()
{
    mds->db->registryPublicData("wifisniffer", stateData(false), this);
}

void QsnWebPageWiFiSniffer::getFunctions(QStringList *functions, int, QString )
{
    if (isAddMACmode) return;
    //    *functions << QsnDojoListItemSwitch_getFun("swenable", "actionenable", widgetUrl(), mds->timeOut);
    //    *functions << QsnDojoDataList_getFun("usersList", listToJSON());
    //    *functions << QsnDojoDataList_getFun("macList", macToJSON());

    *functions << QString("function tablelive(data) {"
                          "var table = $(\"#tablebody\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td>\" + item.mac + \"</td>\" +"
                          "\"<td>\" + item.rssi + \"</td></tr>\");"
                          "});}");

    *functions << QString("$(\"#swenable\").click(function(){"
                          "sendState('action', 'swenable', ($(this).prop('checked')));"
                          "}); ");

    *functions << QString("$(\"#wsnflog\").click(function(){"
                          "sendState('action', 'wsnflog', ($(this).prop('checked')));"
                          "}); ");
}

void QsnWebPageWiFiSniffer::getFunctionsJSON(QStringList *fjson, int )
{
    if (isAddMACmode) return;

    *fjson << QString(" $('#swenable').prop('checked', data.swenable);");
    *fjson << QString(" $('#status').text(data.status);");
    *fjson << QString(" $('#maxtime').text(data.maxtime);");
    *fjson << QString(" $('#wsnflog').prop('checked', data.wsnflog);");
    *fjson << QString(" tablelive(data.mac);");
}

void QsnWebPageWiFiSniffer::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (isAddMACmode) return;
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"status\": \"%1\"").arg(detectionState());
            *returnItems << QString("\"maxtime\": \"%1\"").arg(mdtState());
            *returnItems << QString("\"swenable\": %1").arg(QSNBoolToText(isEnable));
            *returnItems << QString("\"wsnflog\": %1").arg(QSNBoolToText(false));
            *returnItems << QString("\"mac\": %1").arg(macToJSON());
        }

        if (state == QLatin1String("action"))  {
            //if (options->value("id", "") == "wsnflog") tcpServer->setLogEnable(options->value("state", "false") == "true");
            if (options->value("id", "") == "swenable") {
                if (options->value("state", "false") == "true") {
                    isEnable = true;
                } else {
                    isEnable = false;
                    checkDeviceDetected();
                }
                mds->settings->setValue(QString("wifisnifferEnable"), isEnable);
                emit snBUSOutput(QSNLogToContainer(
                                     BUSSERV_LOG_LEVEL_information,
                                     tr("wifisniffer is %1").arg(isEnable?tr("enable"):tr("disable")),
                                     BUSSERV_LOG_CATEGORY_module,
                                     QString("wifisniffer"),
                                     false), this);
            }
        }
    }
}

void QsnWebPageWiFiSniffer::getContents(QStringList *contents, int )
{
    if (!isAddMACmode) {
        *contents << QsnBsTapsBegin();
        *contents << QsnBsTapsTabAdd("info", tr("INFO"), !items.count());
        *contents << QsnBsTapsTabAdd("devices", tr("DEVICES"), items.count());
        *contents << QsnBsTapsPanesBegin();

        *contents << QsnBsTapsPanelBegin("info", !items.count());
        *contents << QsnBsFormCheckbox(tr("Enable"), isEnable, "swenable");
        *contents << QsnBsFormLabel(tr("Port"), serial->portName(), "port");
        *contents << QsnBsFormLabel(tr("Status"), detectionState(), "status");
        *contents << QsnBsFormLabel(tr("Max. detection time"), mdtState(), "maxtime");
        //*contents << QsnBsFormCheckbox(tr("Allow logging"), false, "tcplog");
        *contents << QsnBsTapsPanelEnd();


        *contents << QsnBsTapsPanelBegin("devices", items.count());
        *contents << QsnBsFormTableBegin();
        *contents << QsnBsFormTableTheadBegin();
        *contents << QsnBsFormTableTheadAdd(tr("MAC"));
        *contents << QsnBsFormTableTheadAdd(tr("RSSI"));
        *contents << QsnBsFormTableTheadEnd();
        *contents << QsnBsFormTableBodyBegin("tablebody");

        for (int i = 0; i < items.count(); i ++) {
            *contents << QsnBsFormTableBodyRowAdd(items[i].MAC, QString::number(items[i].RSSI));
        }

        *contents << QsnBsFormTableBodyEnd();
        *contents << QsnBsFormTableEnd();


        *contents << QsnBsTapsPanelEnd();



    } else {
//        *contents << QsnDojoCantegory(tr("Registered users"), mds->panelsStyle);
//        *contents << QsnDojoListBegin(mds->panelsStyle);
//        QsnWebAuthorization::accountItem *aitem;
//        for (int i = 0; i < mds->auth->countAccounts(); i ++) {
//            aitem = mds->auth->atAccount(i);
//            *contents << QsnDojoListItem(QString(), aitem->name, QString(), QString("/users?AID=%1&MAC=%2").arg(aitem->ID).arg(getValue("MAC")));
//        }
//        *contents << QsnDojoListEnd();
    }
}

void QsnWebPageWiFiSniffer::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    // set options

    serialState = QString();
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->setPortName(optionsMap.value("PR", "COM1").toString());
    serialConnect();

    timeAbsenceSec = optionsMap.value("TA", DefaultAbsenceTimer).toInt() * 60;

    isEnable = mds->settings->value(QString("wifisnifferEnable"), false).toBool();
}

void QsnWebPageWiFiSniffer::urlChanged(int )
{
    isAddMACmode = isKey("MAC");
}

QString QsnWebPageWiFiSniffer::widgetState()
{
    return detectionState();
}

char QsnWebPageWiFiSniffer::widgetNotifState()
{
    if (!serial->isOpen()) return 'c';
    return 'n';
}

void QsnWebPageWiFiSniffer::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 2/* && isEnable*/) {
        if (serialReconnectTimer) serialReconnectTimer --;
        else if (!serial->isOpen()) serialConnect();
        checkTimeItems();
    }
}

void QsnWebPageWiFiSniffer::analysisCicle()
{
    QByteArray data = serial->readAll();
    for (int i = 0; i < data.size(); i ++) {
        switch (serialAnalysisState) {
        case 0:
        case 1:
        case 2:
        case 3: if (data.at(i) == static_cast<char>(254)) serialAnalysisState ++;  else serialAnalysisState = 0;  break;
        case 4: serialAnalysisItem.MAC = macNum(data.at(i));
            serialAnalysisCountCRS = static_cast<quint8>(data.at(i));
            serialAnalysisState ++;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9: serialAnalysisItem.MAC += ':' +  macNum(data.at(i));
            serialAnalysisCountCRS ^= static_cast<quint8>(data.at(i));
            serialAnalysisState ++;
            break;
        case 10: serialAnalysisItem.RSSI = static_cast<quint8>(data.at(i));
            serialAnalysisCountCRS ^= static_cast<quint8>(data.at(i));
            serialAnalysisState ++;
            break;
        case 11:
            if (serialAnalysisCountCRS == static_cast<quint8>(data.at(i))) {
                serialAnalysisItem.timer = timeAbsenceSec;
                serialAnalysisItem.isUser = isUserMAC(serialAnalysisItem.MAC);
                if (serialAnalysisItem.isUser) serialAnalysisItem.timeReg = QTime::currentTime();
                addMACitem(&serialAnalysisItem);
            }
            serialAnalysisState = 0;
            break;
        default: serialAnalysisState = 0;
        }
    }
}

void QsnWebPageWiFiSniffer::serialError(QSerialPort::SerialPortError error)
{
    if (error == 0) {
        serialState = tr("Port open");
    } else {
        serialState = QSNGetSerialErrorName(error) + "!";
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             serialState,
                             BUSSERV_LOG_CATEGORY_module,
                             QString("wifisniffer"),
                             false), this);
        serialDisconnect();
        serialReconnectTimer = 3600;
    }
}

void QsnWebPageWiFiSniffer::serialDisconnect()
{
    if (!serial->isOpen()) return;
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_warning,
                          tr("Closing the port"),
                         BUSSERV_LOG_CATEGORY_module,
                         QString("wifisniffer"),
                         false), this);
    serial->close();
}

void QsnWebPageWiFiSniffer::serialConnect()
{
    if (serial->isOpen()) return;
    log(WL_INFORMATION, tr("Opening the port"), QLatin1String("wifisniffer"), objectName());
    if (serial->open(QIODevice::ReadWrite)) serialState.clear();
}

QString QsnWebPageWiFiSniffer::listToJSON()
{
    //    QString ret = "{\"items\": [";

    //    if (countUsers() == 0) {
    //        ret += QString("{\"label\": \"%1\", \"transition\": \"none\"}")
    //                .arg(tr("There are no users with the appropriate settings")); //Пользователи с соответствующими настройками отсутствуют
    //    } else {
    //        for (int i = 0; i < modules()->auth->countAccounts(); i ++)
    //            if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").isEmpty()) {
    //                QString d;
    //                if (isMAC(modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC"))) d = QString("%1").arg(tr("detected"));
    //                else d = QString("%1").arg(tr("not detected"));

    //                if (ret.at(ret.count() - 1) == '}') ret += ',';
    //                ret += QString("{\"label\": \"%1\", \"rightText\":\"%2\", \"variableHeight\": \"true\", \"transition\": \"none\"}")
    //                        .arg(modules()->auth->getTypeAccountName(modules()->auth->atAccount(i), true), d);
    //            }
    //    }
    //    ret += "]}";
    //    return ret;
    return QString();
}

QString QsnWebPageWiFiSniffer::macToJSON()
{
    QString ret = "[";
    for (int i = 0; i < items.count(); i ++) {
        ret += "{";
        ret += QString("\"mac\": \"%1\",").arg(items[i].MAC);
        ret += QString("\"rssi\": \"%1\"").arg(items[i].RSSI);
        ret += "}";
        if (i != items.count() - 1) ret += ",";
    }
    ret += "]";

    return ret;
}

int QsnWebPageWiFiSniffer::countUsers()
{
    int u = 0;
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").isEmpty()) u ++;
    return u;
}

void QsnWebPageWiFiSniffer::addMACitem(macItem *item)
{
    QMutableListIterator <macItem> i(items);
    while (i.hasNext()) {
        i.next();
        if (i.value().MAC == item->MAC) {
            i.value().RSSI = item->RSSI;
            i.value().timer = timeAbsenceSec;
            if (!i.value().isUser) return;
            int t = i.value().timeReg.secsTo(QTime::currentTime());
            if (t > maxDetectTimeSec) maxDetectTimeSec = t;
            //  qDebug() << "Update " << item->MAC << item->RSSI;
            return;
        }
    }
    items.append(*item);
}

void QsnWebPageWiFiSniffer::checkTimeItems()
{
    QMutableListIterator<macItem> i(items);
    while (i.hasNext()) {
        i.next();
        if (i.value().timer) {
            i.value().timer --;
        } else {
            //qDebug() << "Remove " << i.value().MAC << i.value().RSSI << QTime::currentTime();
            i.remove();
        }
    }
    checkDeviceDetected();
}

bool QsnWebPageWiFiSniffer::isMAC(QString mac)
{
    QMutableListIterator<macItem> i(items);
    while (i.hasNext()) {
        if (i.next().MAC == mac) return true;
    }
    return false;
}

bool QsnWebPageWiFiSniffer::isUserMAC(QString mac)
{
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (modules()->auth->parametrValue(mds->auth->atAccount(i), "MAC").toLower() == mac.toLower()) return true;
    return false;
}

quint8 QsnWebPageWiFiSniffer::isDetectionCount()
{
    QMutableListIterator<macItem> i(items);
    quint8 ret = 0;
    while (i.hasNext()) {
        if (i.next().isUser) ret ++;
    }
    return ret;
}

QString QsnWebPageWiFiSniffer::detectionState()
{
    if (!serialState.isEmpty()) return serialState;
    if (!isEnable) return tr("disable");
    if (!items.count()) return QString(tr("unknown"));
    switch (isDetectionCount()) {
    case 0: return tr("not detected");
    case 1: return tr("detected");
    default:  return tr("several detected");
    }

    if (isDetectionCount()) return tr("presence");
    else return QString(tr("absence"));
}

QString QsnWebPageWiFiSniffer::mdtState()
{
    if (!maxDetectTimeSec) return tr("no data");
    QTime time(0, 0, 0);
    time = time.addSecs(maxDetectTimeSec);
    return time.toString("hh:mm:ss");
}

void QsnWebPageWiFiSniffer::checkDeviceDetected()
{
    quint8 state = isDetectionCount();
    if (isEnable) {
        if (state == 0 && detectedCount != 0) setNotDetected();
        //if (state == 1 && detectedState != 1)
        if (state > 0 && detectedCount < 1) setDetected();//several detected
    }
    detectedCount = state;
}

void QsnWebPageWiFiSniffer::setNotDetected()
{
    if (!isEnable) return;
    QByteArray data;
    data[0] = 0;
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_warning,
                          tr("Not detected"),
                         BUSSERV_LOG_CATEGORY_module,
                         QString("wifisniffer"),
                         false), this);
    mds->io->widgetReciveIndexSignal(0, &data, this);
    mds->db->registryPublicData("wifisniffer", stateData(true), this);
}

void QsnWebPageWiFiSniffer::setDetected()
{
    if (!isEnable) return;
    QByteArray data;
    data[0] = 0;
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                          tr("Detected"),
                         BUSSERV_LOG_CATEGORY_module,
                         QString("wifisniffer"),
                         false), this);
    mds->io->widgetReciveIndexSignal(1, &data, this);
    mds->db->registryPublicData("wifisniffer", stateData(false), this);
}

QString QsnWebPageWiFiSniffer::macNum(quint8 num)
{
    QString ret = QString("%1").arg(num, 2, 16, QLatin1Char('0'));
    return ret;
}

QByteArray QsnWebPageWiFiSniffer::stateData(bool state)
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, state);
    QSNByteToRAW(&data, 2, 10);
    return data;
}

