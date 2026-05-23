#include "qsnwebpageyeelight.h"

QsnWebPageYeelight::QsnWebPageYeelight(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Yeelight"));
    widgetUrl = "/yeelight";
    widgetIcon = "subicon-yeelight";
    timerCount = 10;
    foundIteration = 0;
    timeBetweenRequests = YeelightDefaultTimeBetweenRequests;
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageYeelight::~QsnWebPageYeelight()
{
    //    for(auto &socket:udpSockets) {
    //        socket->close();
    //        delete socket;
    //    }
    //    udpSockets.clear();
}

void QsnWebPageYeelight::endConfiguration()
{
    initUDP();
}

void QsnWebPageYeelight::getFunctions(QStringList *functions, int, QString )
{
    *functions << QString("$(\"#ylupdate\").click(function(){"
                          "sendState('action', 'update', true);"
                          "}); ");

    *functions << QString("function tabledev(data) {"
                          "var table = $(\"#devtable\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "var nv = \"\";"
                          "if (item.nv == 2) nv = \"  class='caution'\";"
                          "if (item.nv == 1) nv = \"  class='warning'\";"
                          "table.append(\"<tr\" + nv + \"><td>\" + item.nm + \"</td>\" +"
                          "\"<td>\" + item.st + \"</td></tr>\");"
                          "});}");
}

void QsnWebPageYeelight::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#state').text(data.state);");
    *fjson << QString(" tabledev(data.devtable);");
}

void QsnWebPageYeelight::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("action") && options->value("id", "") == "update") {
            mds->interface->snBUSInput(QSNLogToContainer(
                                           BUSSERV_LOG_LEVEL_information,
                                           tr("Begin update"),
                                           BUSSERV_LOG_CATEGORY_module,
                                           QString(),
                                           true), this);
            findDevices();
        }
    }
    *returnItems << QString("\"state\": \"%1\"").arg(widgetState());
    *returnItems << QString("\"devtable\": %1").arg(devicesToJSON());
}

void QsnWebPageYeelight::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("status", tr("STATUS"), !devices.count());
    *contents << QsnBsTapsTabAdd("devices", tr("DEVICES"), devices.count());
    *contents << QsnBsTapsPanesBegin();

    *contents << QsnBsTapsPanelBegin("status", !devices.count());
    *contents << QsnBsFormLabel(tr("State"), widgetState(), "state");
    *contents << QsnBsFormButton(tr("Update"), QLatin1String("ylupdate"), QString("btn-info"));
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanelBegin("devices", devices.count());
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Name"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Sate"), QString());
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("devtable");

    for (int j = 0; j < devices.count(); j ++) {
        *contents << QString("<tr %1><td>%2</td><td>%3</td></tr>")
                     .arg(devices[j]->isConnected()?"":" class='caution'\"" ,devices[j]->objectName(), devices[j]->widgetState());
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
    *contents << QsnBsTapsPanesEnd();

}

void QsnWebPageYeelight::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO == 0) for( int i = 0; i < devices.count(); i ++) devices.at(i)->switchOff();
    if (indexIO == 1) for( int i = 0; i < devices.count(); i ++) devices[i]->actionRMCode(QSNRAWtoRMcode(data, 1));
}

void QsnWebPageYeelight::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("YL"))) {
            QsnWebItemYeelightDevice *dev = new QsnWebItemYeelightDevice(static_cast<quint32>(id), modules(), this);
            dev->fromStream(stream);
            devices.append(dev);
        }
    }
    // set options
    timeBetweenRequests = optionsMap.value("TO", YeelightDefaultTimeBetweenRequests).toInt();
}

void QsnWebPageYeelight::urlChanged(int )
{

}

QString QsnWebPageYeelight::widgetState()
{
    if (!UDPerror.isEmpty()) return UDPerror;
    if (!devices.count()) {
        if (foundDevices.count()) return QString("%1: %2").arg(tr("new devices found")).arg(foundDevices.count());
        return tr("no devices configured");
    }
    return QString("%1: %2").arg(tr("conn:")).arg(getConectedDevices());
}

void QsnWebPageYeelight::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 2) {
        foreach(QsnWebItemYeelightDevice *dev, devices) dev->checkTime();
        if (timerCount) timerCount --;
        if (timerCount == 0) {
            findDevices();
            timerCount = timeBetweenRequests;
        }
    }
}

void QsnWebPageYeelight::processPendingDatagrams()
{
    QUdpSocket *socket = static_cast<QUdpSocket*>(sender());
    while (socket->hasPendingDatagrams())   {
        QByteArray data;
        QHostAddress addr;
        data.resize(socket->pendingDatagramSize());
        socket->readDatagram(data.data(), data.size(), &addr);
        if (LastReceivedDeviceIP == addr) continue;
        LastReceivedDeviceIP = addr;
        addDevice(QString(data));
    }
}

void QsnWebPageYeelight::initUDP()
{
    multiCastAddress.setAddress("239.255.255.250");
    for(auto &socket:udpSockets) {
        socket->close();
        delete socket;
    }
    udpSockets.clear();

    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, addresses) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) { /*localIPAddress = address.toString();*/
            udpSockets.append(new QUdpSocket(this));
            if (!udpSockets.last()->bind(address, 0, QUdpSocket::ShareAddress)) {
                mds->interface->snBUSInput(QSNLogToContainer(
                                               BUSSERV_LOG_LEVEL_caution,
                                               tr("UDP bind failed %1").arg(address.toString()),
                                               BUSSERV_LOG_CATEGORY_module,
                                               QString("yeelight"),
                                               false), this);

            } else {
                mds->interface->snBUSInput(QSNLogToContainer(
                                               BUSSERV_LOG_LEVEL_information,
                                               tr("UDP bind success %1").arg(address.toString()),
                                               BUSSERV_LOG_CATEGORY_module,
                                               QString("yeelight"),
                                               false), this);

                udpSockets.last()->joinMulticastGroup(multiCastAddress);
                connect(udpSockets.last(), SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));// Поток данных идет, чтобы вызвать readyRead() сигнал

            }
        }
    }
}

QString QsnWebPageYeelight::devicesToJSON()
{
    QString ret = "[";

    for (int j = 0; j < devices.count(); j ++) {
        if (ret.count() > 1) ret += ',';
        ret += "{";
        ret += QString("\"nm\": \"%1\",").arg(devices[j]->objectName());
        ret += QString("\"nv\": %1,").arg(devices[j]->isConnected()?0:2);
        ret += QString("\"st\": \"%1\"").arg(devices[j]->widgetState());
        ret += "}";
    }

    for (int j = 0; j < foundDevices.count(); j ++) {
        if (ret.count() > 1) ret += ',';
        ret += "{";
        ret += QString("\"nm\": \"%1\",").arg(foundDevices[j].model);
        ret += QString("\"nv\": %1,").arg(1);
        ret += QString("\"st\": \"%1\"").arg(devicesItemToString(&foundDevices[j]));
        ret += "}";
    }

    ret += "]";
    return ret;
}

QString QsnWebPageYeelight::devicesItemToString(FItem *item)
{
    QString ret;
    ret += QString("%1: %2<br>").arg(tr("ID"), item->id);
    ret += QString("%1: %2<br>").arg(tr("IP"), item->ip);
    ret += QString("%1: %2").arg(tr("Power"), item->power);
    return ret;
}

void QsnWebPageYeelight::findDevices()
{
    //    foreach(QsnWebItemYeelightDevice *dev, devices) dev->checkConnected();

    for (int m = foundDevices.count() - 1; m >= 0; m--) //AI edit
        if (foundDevices.at(m).iteration != foundIteration) {
            mds->interface->snBUSInput(QSNLogToContainer(
                                           BUSSERV_LOG_LEVEL_warning,
                                           QString("%1: %2 %3").arg(tr("The device is lost"), foundDevices.at(m).model, foundDevices.at(m).id),
                                           BUSSERV_LOG_CATEGORY_module,
                                           QString("yeelight"),
                                           false), this);
            foundDevices.removeAt(m);
        }

    foundIteration ++;
    LastReceivedDeviceIP = QHostAddress::Null;
    QByteArray datagram = "M-SEARCH * HTTP/1.1\r\n HOST: 239.255.255.250:1982\r\n MAN: \"ssdp:discover\"\r\n ST: wifi_bulb";
    for(auto &socket:udpSockets){
        socket->writeDatagram(datagram.data(), datagram.size(), multiCastAddress, 1982);
    }
}

void QsnWebPageYeelight::addDevice(QString data)
{

    QMap<QString, QString> options;
    answerToMap(data, &options);
    extractIPandPort("Location", &options);

    FItem item;
    item.id = options.value("id");
    item.ip = options.value("ipaddress");
    item.port = options.value("ipport");
    item.model = options.value("model");
    item.power = options.value("power");
    item.iteration = foundIteration;

    foreach(QsnWebItemYeelightDevice *dev, devices) {
        if (dev->deviceID() == item.id) {
            dev->setAddress(QHostAddress(item.ip), item.port.toUInt());
            return;
        }

        if (dev->deviceIP() == QHostAddress(item.ip) && dev->devicePort() == item.port.toUInt()) {
            return;
        }
    }

    for (int m = 0; m < foundDevices.count(); m ++)
        if (foundDevices.at(m).id == item.id) {
            foundDevices[m] = item;
            return;
        }

    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1: %2 %3").arg(tr("Add new device"), item.model, item.id),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("yeelight"),
                                   false), this);
    foundDevices.append(item);
}

void QsnWebPageYeelight::answerToMap(QString answer, QMap<QString, QString> *options)
{
    QStringList list = answer.split("\r\n", QString::SkipEmptyParts);
    foreach (QString line, list) {
        int sp = line.indexOf(':');
        if (sp == -1) continue;
        options->insert(line.left(sp).trimmed(), line.right(line.count() - sp - 1).trimmed());
    }
}

void QsnWebPageYeelight::extractIPandPort(QString name, QMap<QString, QString> *options)
{
    if (!options->contains(name)) return;
    QString line = options->value(name);
    int i = line.indexOf("//");
    if (i != -1) line.remove(0, i + 2);
    i = line.indexOf(':');
    if (i == -1) {
        options->insert("ipaddress", line.trimmed());
        return;
    }
    options->insert("ipaddress", line.left(i).trimmed());
    options->insert("ipport", line.right(line.count() - i - 1).trimmed());
}

//HTTP/1.1 200 OK
//Cache-Control: max-age=3600
//Date:
//Ext:
//Location: yeelight://192.168.1.65:55443
//Server: POSIX UPnP/1.0 YGLC/1
//id: 0x000000000e135868
//model: desklamp
//fw_ver: 62
//support: get_prop set_default set_power toggle set_ct_abx set_bright start_cf stop_cf set_scene cron_add cron_get cron_del set_adjust adjust_bright adjust_ct set_name
//power: on
//bright: 70
//color_mode: 2
//ct: 4000
//rgb: 0
//hue: 0
//sat: 0
//name:

int QsnWebPageYeelight::getConectedDevices()
{
    int count = 0;
    for (int i = 0; i < devices.count(); i ++)
        if (devices.at(i)->isConnected()) count ++;
    return count;
}
