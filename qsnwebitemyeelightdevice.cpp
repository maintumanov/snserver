#include "qsnwebitemyeelightdevice.h"

QsnWebItemYeelightDevice::QsnWebItemYeelightDevice(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Yeelight lamp"));
    switchState = false;
    connectState = 0;
    connectAttempt = 0;
    mds = modules;
    devIP = QHostAddress::Null;
    lampbright = 100;
    lamptemp = 4700;
    rmOn = 0;
    rmOff = 0;
    rmSwitch = 0;
    timeCheckConnectet = 10;
    TCPSocket = new QTcpSocket(this);
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(TCPSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(TCPSocket, SIGNAL(connected()), this, SLOT(tcpconnected()));
    connect(TCPSocket, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
}

QsnWebItemYeelightDevice::~QsnWebItemYeelightDevice()
{
    TCPSocket->close();
}

void QsnWebItemYeelightDevice::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    rmOn = optionsMap.value("RMON", QByteArray()).toByteArray();
    rmOff = optionsMap.value("RMOFF", QByteArray()).toByteArray();
    rmSwitch = optionsMap.value("RMSW", QByteArray()).toByteArray();

    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("YSL"))) {
            QsnWebItemYeelightScenario *sc = new QsnWebItemYeelightScenario(static_cast<quint32>(id), modules(), this);
            sc->fromStream(stream);
            connect(sc, SIGNAL(applyScenarioLamp(quint8 , quint16)), this, SLOT(applyScenarioLamp(quint8 , quint16)));
            scenarios.append(sc);
        }
    }
    // set options
    settingIP = optionsMap.value("IP", QString()).toString();
    devIP = QHostAddress(settingIP);
    devID = optionsMap.value("ID", QString()).toString();

    if (!settingIP.isEmpty())  beginConnect(QHostAddress(settingIP), 55443);
}

void QsnWebItemYeelightDevice::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO == 3) turnOn();
    if (indexIO == 4) turnOff();
    if (indexIO == 5) QSNRAWtoBool(data, 1)?turnOn():turnOff();
}

void QsnWebItemYeelightDevice::actionRMCode(QByteArray code)
{
    if (rmOff == code) turnOff();
    else if (rmOn == code) turnOn();
    else if (rmSwitch == code) {if (switchState) turnOff(); else turnOn();}
}

void QsnWebItemYeelightDevice::setAddress(QHostAddress address, quint16 port)
{
    checkConnected();

    if (TCPSocket->peerAddress() == address && TCPSocket->peerPort() == port) {
        if (!connectState && !connectAttempt) beginConnect(address, port);
        return;
    }
    if (connectState) tcpdisconnected();
    beginConnect(address, port);
}

void QsnWebItemYeelightDevice::checkConnected()
{
    if (connectState) {
        getProperty();
        return;
    }

    if (settingIP.isEmpty()) return;
    if (connectState) return;
    beginConnect(QHostAddress(settingIP), 55443);
}

QHostAddress QsnWebItemYeelightDevice::deviceIP()
{
    return TCPSocket->peerAddress();
}

quint16 QsnWebItemYeelightDevice::devicePort()
{
    return TCPSocket->peerPort();
}

QString QsnWebItemYeelightDevice::deviceID()
{
    return devID;
}

void QsnWebItemYeelightDevice::switchOff()
{
    if (!TCPSocket->isOpen()) return;
    if (!switchState) return;
    turnOff();
}

void QsnWebItemYeelightDevice::setBright(quint8 b)
{
    if (!TCPSocket->isOpen()) return;
    lampbright = b;
    MSGAdd(QString("\"set_bright\",\"params\":[%1, \"smooth\",500]").arg(lampbright));
}

void QsnWebItemYeelightDevice::setTemperature(quint16 t)
{
    if (!TCPSocket->isOpen()) return;
    lamptemp = t;
    MSGAdd(QString("\"set_ct_abx\",\"params\":[%1, \"smooth\",500]").arg(lamptemp));
}

QString QsnWebItemYeelightDevice::widgetState()
{
    if (!connectState) return tr("OFFLINE");

    QString ret;
    ret += QString("%1: %2<br>").arg(tr("Power"), switchState?tr("ON"):tr("OFF"));
    ret += QString("%1: %2<br>").arg(tr("Bright")).arg(lampbright);
    ret += QString("%1: %2K").arg(tr("Color temp.")).arg(lamptemp);
    return ret;
}

bool QsnWebItemYeelightDevice::isConnected()
{
    return connectState;
}

void QsnWebItemYeelightDevice::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 2) MSGCheck();
}

void QsnWebItemYeelightDevice::checkTime()
{
    if (connectAttempt) connectAttempt --;
    if (timeCheckConnectet) {timeCheckConnectet --; if(!timeCheckConnectet) {checkConnected(); timeCheckConnectet = 30;}};
    if (connectState) {connectState --; if(!connectState) TCPSocket->close();};
}

void QsnWebItemYeelightDevice::MSGCheck()
{
    if (!TCPSocket->isOpen()) return;
    if (MSGqueue.count() == 0) return;
    QByteArray message;
    message.append("{\"id\":");
    message.append(devID.toUtf8());
    message.append(",\"method\":");
    message.append(MSGqueue.first().toUtf8());
    message.append("}\r\n");
    MSGqueue.removeFirst();
    TCPSocket->write(message);
}

void QsnWebItemYeelightDevice::readyRead()
{
    QMap<QString, QString> options;
    QString r = TCPSocket->readAll();
    QSNParseJSON(r, &options);
    connectState = 60;

    if (options.contains("result")) readProperty(options.value("result"));
    if (options.contains("params")) readParams(options.value("params"));
}

void QsnWebItemYeelightDevice::applyScenarioLamp(quint8 bright, quint16 colorTemp)
{
    setBright(bright);
    setTemperature(colorTemp);
}

void QsnWebItemYeelightDevice::tcpconnected()
{
    connectState = 30;
    connectAttempt = 0;
    MSGqueue.clear();
    getProperty();
}

void QsnWebItemYeelightDevice::tcpdisconnected()
{
    connectState = 0;
    connectAttempt = 0;
    TCPSocket->close();
}

void QsnWebItemYeelightDevice::turnOn()
{
    MSGAdd("\"set_power\",\"params\":[\"on\", \"smooth\",500]");
}

void QsnWebItemYeelightDevice::turnOff()
{
    MSGAdd("\"set_power\",\"params\":[\"off\", \"smooth\",500]");
}

void QsnWebItemYeelightDevice::setStateOn()
{
    if (switchState) return;
    switchState = true;
    QByteArray data;
    mds->io->widgetReciveIndexSignal(0, &data, this);
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, switchState);
    QSNByteToRAW(&data, 2, 1);
    mds->io->widgetReciveIndexSignal(2, &data, this);
}

void QsnWebItemYeelightDevice::setStateOff()
{
    if (!switchState) return;
    switchState = false;
    QByteArray data;
    mds->io->widgetReciveIndexSignal(1, &data, this);
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, switchState);
    QSNByteToRAW(&data, 2, 1);
    mds->io->widgetReciveIndexSignal(2, &data, this);
}

void QsnWebItemYeelightDevice::getProperty()
{
    MSGAdd("\"get_prop\",\"params\":[\"power\",\"bright\",\"ct\"]");
}

void QsnWebItemYeelightDevice::readProperty(QString property)
{
    QStringList p = property.remove('"').remove('[').remove(']').split(',');
    if (p.count() < 3) return;
    if (p.at(0) == "on") setStateOn();
    if (p.at(0) == "off") setStateOff();
    lampbright = p.at(1).toUInt();
    lamptemp = p.at(2).toUInt();
}

void QsnWebItemYeelightDevice::readParams(QString params)
{
    QMap<QString, QString> options;
    QSNParseJSON(params, &options);
    if (options.contains("bright")) lampbright = options.value("bright").toUInt();
    if (options.contains("ct")) lamptemp = options.value("ct").toUInt();
    if (options.contains("power")) {
        //            qDebug() << options.value("power");
        if (options.value("power") == "on") setStateOn();
        if (options.value("power") == "off") setStateOff();
    }
}

void QsnWebItemYeelightDevice::MSGAdd(QString msg)
{
    MSGqueue.append(msg);
    MSGCheck();
}

void QsnWebItemYeelightDevice::beginConnect(QHostAddress address, quint16 port)
{
    if (connectAttempt) return;
    if (connectState) return;
    TCPSocket->connectToHost(address, port);
    connectAttempt = 60;
}


