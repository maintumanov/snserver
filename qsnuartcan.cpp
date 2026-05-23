#include "qsnuartcan.h"


QSnUartCan::QSnUartCan(QObject *parent) :
    QObject(parent)
{
    newMsg.analysisState = 0;
    serial = new QSerialPort(this);
    serialEnable = false;
    isLog = false;
    lastError = QString();
    sentMsg = 0;
    reciveMsg = 0;
    setObjectName(QString(tr("UartCan")));
    connect(serial, SIGNAL(readyRead()), this, SLOT(analysisCicle()));
}

void QSnUartCan::analysisCicle()
{
    QByteArray data = serial->readAll();
    for (int i =0; i < data.count(); i ++)
        analysis(static_cast<quint8>(data[i]));
}

void QSnUartCan::portStateChange(bool state)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 1;
    if (state) container.Signal = 0;
    else container.Signal = 1;
    {emit snBUSOutput(container, this);}
}

QByteArray QSnUartCan::sizeLimit(QByteArray *data)
{
    QByteArray d = *data;
    if (d.count() > 8) {
        d.resize(8);
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             QString(tr("The data being sent was truncated [%1]").arg(QSNRAWtoScaledVolume(data, QString()))),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("adapteruartcan"),
                             false), this);
    }
    return d;
}

void QSnUartCan::analysis(quint8 data)
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
        newMsg.analysisState = 8;
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
        newMsg.analysisState = 7;
        break;

        // === cmd ===
    case 7:
        newMsg.cmd = data;
        newMsg.cacs = newMsg.cacs ^ data;
        newMsg.analysisState = 8;
        break;

        // ===data ===
    case 8:   //datasize
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

void QSnUartCan::recivePack(QRMsg Msg)
{
    QSNContainer container = newContainer();
    if (!Msg.isSignal) {
        container.role = QSNContainer::message;
        container.Command = Msg.cmd;
        container.Address = Msg.address;
        container.Sender =  Msg.sender;
        container.Data = Msg.data;
        emit snBUSOutput(container, this);
        reciveMsg ++;
    }
    if (Msg.sender == thisAddress) return;
    if (Msg.isSignal) {
        container.role = QSNContainer::signal;
        container.Signal = Msg.signal;
        container.Data = Msg.data;
        emit snBUSOutput(container, this);
        reciveMsg ++;

    }
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         QSNContainerToLogText(container, true),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString(),
                         true), this);
}

void QSnUartCan::serviceProcessing(QSNContainer container)
{
    switch (container.Command) {
    case 0:
        if (container.Signal == 0) openPort(container.Address);
        if (container.Signal == 1) closePort();
        break;
    case 6:
        thisAddress = container.Sender;
        break;
    }
}

void QSnUartCan::addStart(bool isSignal, QByteArray *sd)
{
    if (isSignal) sd->append(static_cast<char>(254));
    else sd->append(static_cast<char>(253));
}

void QSnUartCan::addByte(quint8 data, QByteArray *sd, quint8 *acs)
{
    if (data < 253) sd->append(static_cast<char>(data));
    else {
        sd->append(static_cast<char>(255));
        sd->append(static_cast<char>(data) - 128);
    }
    *acs = *acs ^ data;
}

void QSnUartCan::addWord(quint16 data, QByteArray *sd, quint8 *acs)
{
    addByte((data >> 8) & 0xFF, sd, acs);
    addByte(data & 0xFF, sd, acs);
}

void QSnUartCan::addData(QByteArray *data, QByteArray *sd, quint8 *acs)
{
    int i = 0;
    addByte(static_cast<quint8>(data->count()), sd, acs);
    for (i = 0; i < data->count(); i++) addByte(static_cast<quint8>(data->at(i)), sd, acs);
    addByte(*acs, sd, acs);
}

void QSnUartCan::sendPacket(quint16 address, quint16 sender, quint8 cmd, QByteArray *data)
{
    quint8 acs = 0;
    QByteArray frame;
    QByteArray d = sizeLimit(data);
    addStart(false, &frame);
    addWord(address, &frame, &acs);
    addWord(sender, &frame, &acs);
    addByte(cmd, &frame, &acs);
    addData(&d, &frame, &acs);
    serial->write(frame);
    sentMsg ++;
}

void QSnUartCan::sendSignal(quint16 signal, QByteArray *data)
{
    quint8 acs = 0;
    QByteArray frame;
    QByteArray d = sizeLimit(data);
    addStart(true, &frame);
    addWord(signal, &frame, &acs);
    addData(&d, &frame, &acs);
    serial->write(frame);
    sentMsg ++;
}

void QSnUartCan::openPort(quint16 address)
{
    thisAddress = address;
    if (serial->isOpen()) serial->close();
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
                             QString("adapteruartcan"),
                             false), this);
        lastError.clear();
    } else {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             serial->errorString(),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("adapteruartcan"),
                             false), this);
        lastError = serial->errorString();
    }
}

void QSnUartCan::closePort()
{
    if (!serial->isOpen()) return;
    serial->close();
    portStateChange(false);
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Closing the port"),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("adapteruartcan"),
                         false), this);
}

QString QSnUartCan::error()
{
    return lastError;
}

void QSnUartCan::setLogEnable(bool enable)
{
    isLog = enable;
}

int QSnUartCan::statMsgSent()
{
    return sentMsg;
}

int QSnUartCan::statMsgRecived()
{
    return reciveMsg;
}

void QSnUartCan::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("UartCan_Enabled"), isEnabled());
    settings->setValue(QLatin1String("UartCan_Port"), getPortName());
}

void QSnUartCan::loadSettings(QSettings *settings)
{
    setEnabled(settings->value(QLatin1String("UartCan_Enabled"), isEnabled()).toBool());
    setPortName(settings->value(QLatin1String("UartCan_Port"), getPortName()).toString());
}

void QSnUartCan::saveSettingsStream(QDataStream *stream)
{
    *stream << isEnabled();
    *stream << getPortName();
}

void QSnUartCan::loadSettingsStream(QDataStream *stream)
{
    bool enable;
    QString port;
    *stream >> enable;
    *stream >> port;
    setEnabled(enable);
    setPortName(port);
}

bool QSnUartCan::isEnabled()
{
    return serialEnable;
}

void QSnUartCan::setEnabled(bool enable)
{
    serialEnable = enable;
}

QString QSnUartCan::getPortName()
{
    return serial->portName();
}

void QSnUartCan::setPortName(QString portName)
{
    serial->setPortName(portName);
}

bool QSnUartCan::isConnected()
{
    return serial->isOpen();
}

void QSnUartCan::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role == QSNContainer::service) {
        serviceProcessing(container);
        return;
    }
    if (isConnected()) {
        if (container.role == QSNContainer::message)
            sendPacket(container.Address, container.Sender, container.Command, &container.Data);


        if (container.role == QSNContainer::signal)
            sendSignal(container.Signal, &container.Data);

        if (container.role != QSNContainer::information)
            emit snBUSOutput(QSNLogToContainer(
                                 BUSSERV_LOG_LEVEL_information,
                                 QSNContainerToLogText(container, false),
                                 BUSSERV_LOG_CATEGORY_adapter,
                                 QString(),
                                 true), this);
    }
}
