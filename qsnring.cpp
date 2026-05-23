#include "qsnring.h"


QSnRing::QSnRing(QObject *parent) :
    QObject(parent)
{
    newMsg.analysisState = 0;
    serial = new QSerialPort(this);
    serialEnable = false;
    isLog = false;
    lastError = QString();
    setObjectName(QString(tr("RING")));
    connect(serial, SIGNAL(readyRead()), this, SLOT(analysisCicle()));
}

void QSnRing::analysisCicle()
{
    QByteArray data = serial->readAll();
    for (int i =0; i < data.count(); i ++)
        analysis(static_cast<quint8>(data[i]));
}

void QSnRing::portStateChange(bool state)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 1;
    if (state) container.Signal = 0;
    else container.Signal = 1;
    {emit snBUSOutput(container, this);}
}

void QSnRing::analysis(quint8 data)
{
    if (data == 253)
    {
        newMsg.analysisState = 1;
        newMsg.isSignal  = false;
        newMsg.cacs = 0;
        return ;
    }

    if (data == 254)
    {
        newMsg.analysisState = 3;
        newMsg.isSignal  = true;
        newMsg.cacs = 0;
        return ;
    }

    if (data == 255)
        newMsg.analysisState += 50;
    else if (newMsg.analysisState >= 50)
    {
        newMsg.analysisState -= 50;
        data += 128;
    }

    switch (newMsg.analysisState)
    {
    case 1:
        newMsg.address = static_cast<quint16>(data << 8);
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.analysisState = 2;
        break;

    case 2:
        newMsg.address += data;
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.analysisState = 5;
        break;

        // === signal ===
    case 3:
        newMsg.signal = static_cast<quint16>(data << 8);
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.analysisState = 4;
        break;

    case 4:
        newMsg.signal += data;
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.analysisState = 5;
        break;

        // === sender ===
    case 5:
        newMsg.sender = static_cast<quint16>(data << 8);
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.analysisState = 6;
        break;

    case 6:
        newMsg.sender += data;
        newMsg.cacs = newMsg.cacs ^ data;
        if (newMsg.isSignal) newMsg.analysisState = 8;
        else newMsg.analysisState = 7;
        break;

        // === cmd ===
    case 7:
        newMsg.cmd = data;
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.analysisState = 8;
        break;

        // ===data ===
    case 8://datasize
        newMsg.dlc = data;
        newMsg.data.resize(newMsg.dlc);
        newMsg.dps = 0;
        newMsg.cacs = newMsg.cacs ^ data;
        if (newMsg.dlc == 0) newMsg.analysisState = 10;
        else newMsg.analysisState = 9;
        break;

    case 9://data

        newMsg.data[newMsg.dps] = static_cast<char>(data);
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.dps ++;
        if (newMsg.dps == newMsg.dlc) newMsg.analysisState = 10;
        break;

    case 10://ACS
        newMsg.acs = data;
        if (newMsg.acs == newMsg.cacs) recivePack(newMsg);
        newMsg.analysisState = 0;
        break;
    }
}

void QSnRing::recivePack(QRMsg Msg)
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_caution,
                         tr("Get raw: %1").arg(QString(Msg.data.toHex())),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString(),
                         true), this);
    if (Msg.sender == thisAddress) return;


    QSNContainer container = newContainer();
    if (!Msg.isSignal) {
        container.role = QSNContainer::message;
        container.Command = Msg.cmd;
        container.Address = Msg.address;
        container.Sender = Msg.sender;
        container.Data = Msg.data;
        emit snBUSOutput(container, this);
        if (Msg.address == 0) Transmit(container);
    }
    if (newMsg.isSignal) {
        container.role = QSNContainer::signal;
        container.Signal = Msg.signal;
        container.Sender = Msg.sender;
        container.Data = Msg.data;
        emit snBUSOutput(container, this);
        Transmit(container);
    }
}

void QSnRing::serviceProcessing(QSNContainer container)
{
    switch (container.Command)
    {
    case 0:
        if (container.Signal == 0) openPort(container.Address);
        if (container.Signal == 1) closePort();
        break;
    case 6:
        thisAddress = container.Sender;
        break;
    }
}

void QSnRing::addStart(bool isSignal, QByteArray *sd)
{
    if (isSignal) sd->append(static_cast<char>(254));
    else sd->append(static_cast<char>(253));
}

void QSnRing::addByte(quint8 data, QByteArray *sd, quint8 *acs)
{
    if (data < 253) sd->append(static_cast<char>(data));
    else {
        sd->append(static_cast<char>(255));
        sd->append(static_cast<char>(data) - 128);
    }
    *acs = *acs ^ data;
}

void QSnRing::addWord(quint16 data, QByteArray *sd, quint8 *acs)
{
    addByte((data >> 8) & 0xFF, sd, acs);
    addByte(data & 0xFF, sd, acs);
}

void QSnRing::addData(QByteArray *data, QByteArray *sd, quint8 *acs)
{
    int i = 0;
    addByte(static_cast<quint8>(data->count()), sd, acs);
    for (i = 0; i < data->count(); i++) addByte(static_cast<quint8>(data->at(i)), sd, acs);
    addByte(*acs, sd, acs);
}

void QSnRing::SendPacket(quint16 address, quint16 sender, quint8 cmd, QByteArray *data)
{
    quint8 acs = 0;
    QByteArray frame;
    addStart(false, &frame);
    addWord(address, &frame, &acs);
    addWord(sender, &frame, &acs);
    addByte(cmd, &frame, &acs);
    addData(data, &frame, &acs);
    serial->write(frame);

}

void QSnRing::SendSignal(quint16 signal, QByteArray *data)
{
    quint8 acs = 0;
    QByteArray frame;
    addStart(true, &frame);
    addWord(signal, &frame, &acs);
    addWord(thisAddress, &frame, &acs);
    addData(data, &frame, &acs);
    serial->write(frame);
}

void QSnRing::Transmit(QSNContainer container)
{
    if (isConnected())
    {
        if (container.role == QSNContainer::message)
            SendPacket(container.Address, container.Sender, container.Command, &container.Data);

        if (container.role == QSNContainer::signal)
            SendSignal(container.Signal, &container.Data);
    }
}

void QSnRing::openPort(quint16 address)
{
    thisAddress = address;
    if (serial->isOpen()) return;
    if (!serialEnable) return;

    serial->setBaudRate(static_cast<int>(115200));
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        portStateChange(true);
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Opening the port successfully"),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("adapterring"),
                             false), this);
    } else {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             serial->errorString(),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("adapterring"),
                             false), this);
    }
}

void QSnRing::closePort()
{
    if (!serial->isOpen()) return;
    serial->close();
    portStateChange(false);
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Closing the port"),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("adapterring"),
                         false), this);
}

QString QSnRing::error()
{
    return lastError;
}

void QSnRing::setLogEnable(bool enable)
{
    isLog = enable;
}

void QSnRing::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("Ring_Enabled"), isEnabled());
    settings->setValue(QLatin1String("Ring_Port"), getPortName());
}

void QSnRing::loadSettings(QSettings *settings)
{
    setEnabled(settings->value(QLatin1String("Ring_Enabled"), isEnabled()).toBool());
    setPortName(settings->value(QLatin1String("Ring_Port"), getPortName()).toString());
}

void QSnRing::saveSettingsStream(QDataStream *stream)
{
    *stream << isEnabled();
    *stream << getPortName();
}

void QSnRing::loadSettingsStream(QDataStream *stream)
{
    bool enable;
    QString port;
    *stream >> enable;
    *stream >> port;
    setEnabled(enable);
    setPortName(port);
}

bool QSnRing::isEnabled()
{
    return serialEnable;
}

void QSnRing::setEnabled(bool enable)
{
    serialEnable = enable;
}

QString QSnRing::getPortName()
{
    return serial->portName();
}

void QSnRing::setPortName(QString portName)
{
    serial->setPortName(portName);
}

bool QSnRing::isConnected()
{
    return serial->isOpen();
}

void QSnRing::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role == QSNContainer::service)
    {
        serviceProcessing(container);
        return;
    }
    if (isConnected())
    {
        if (container.role == QSNContainer::message) {
            if (container.Address != 161) {
            SendPacket(container.Address, container.Sender, container.Command, &container.Data);
            }
        }

        if (container.role == QSNContainer::signal)
            SendSignal(container.Signal, &container.Data);
    }
}
