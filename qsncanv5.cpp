#include "qsncanv5.h"


QSnCanV5::QSnCanV5(QObject *parent) :
    QObject(parent)
{
    NewMsg.AnalysisState = 0;
    serial = new QSerialPort(this);
    isLog = false;
    serialEnable = false;
    lastError = QString();
    setObjectName(QString(tr("CANv5")));
    connect(serial, SIGNAL(readyRead()), this, SLOT(analisisCicle()));
}

void QSnCanV5::analisisCicle()
{
    QByteArray data = serial->readAll();
    for (int i =0; i < data.count(); i ++)
        analysis(static_cast<quint8>(data[i]));
}

void QSnCanV5::portStateChange(bool state)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 1;
    container.Address = 0;
    container.Sender = 0;
    if (state) container.Signal = 0;
    else container.Signal = 1;
    {emit snBUSOutput(container, this);}
}

QByteArray QSnCanV5::sizeLimit(QByteArray *data)
{
    QByteArray d = *data;
    if (d.count() > 8) {
        d.resize(8);
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_warning,
                             tr("The data being sent was truncated [%1]").arg(QSNRAWToValueString(data, 1)),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("adaptercanv5"),
                             false), this);
    }
    return d;
}

void QSnCanV5::analysis(quint8 D)
{

    switch (NewMsg.AnalysisState)
    {
    case 0:
        if (D == 170) NewMsg.AnalysisState = 1;
        break;
    case 1:
        NewMsg.check = 0;
        if (D == 85) NewMsg.AnalysisState = 2;
        else NewMsg.AnalysisState = 0;
        break;
    case 2:
        NewMsg.check += D;
        if (D == 1) NewMsg.AnalysisState = 3;
        else NewMsg.AnalysisState = 0;
        break;
    case 3:
        NewMsg.check += D;
        if (D == 2) NewMsg.AnalysisState = 4;
        else NewMsg.AnalysisState = 0;
        break;
    case 4:
        NewMsg.check += D;
        if (D == 1) NewMsg.AnalysisState = 5;
        else NewMsg.AnalysisState = 0;
        break;
    case 5: // read id
        NewMsg.check += D;
        NewMsg.ID =  D;
        NewMsg.AnalysisState = 6;
        break;
    case 6:
        NewMsg.check += D;
        NewMsg.ID +=  (static_cast<quint32>(D << 8));
        NewMsg.AnalysisState = 7;
        break;
    case 7:
        NewMsg.check += D;
        NewMsg.ID +=  (static_cast<quint32>(D << 16));
        NewMsg.AnalysisState = 8;
        break;
    case 8:
        NewMsg.check += D;
        NewMsg.ID +=  (static_cast<quint32>(D << 24));
        NewMsg.AnalysisState = 9;
        break;
    case 9: // read data size
        NewMsg.check += D;
        NewMsg.DataSize = D;
        NewMsg.Data.clear();
        NewMsg.AnalysisState = 10;
        break;
    case 10: // read data
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 11;
        break;
    case 11:
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 12;
        break;
    case 12:
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 13;
        break;
    case 13:
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 14;
        break;
    case 14:
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 15;
        break;
    case 15:
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 16;
        break;
    case 16:
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 17;
        break;
    case 17:
        NewMsg.check += D;
        NewMsg.Data.append(static_cast<char>(D));
        NewMsg.AnalysisState = 18;
        break;
    case 18:
        NewMsg.check += D;
        if (D == 0) NewMsg.AnalysisState = 19;
        else NewMsg.AnalysisState = 0;
        break;
    case 19:
        NewMsg.AnalysisState = 0;
        if (D == NewMsg.check)
            recivePack(NewMsg);
        break;
    }
}

void QSnCanV5::recivePack(QRMsg Msg)
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Get raw: %1").arg(QString(NewMsg.Data.toHex())),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("adaptercanv5"),
                         false), this);
    QSNContainer container = newContainer();
    Msg.Data.resize(Msg.DataSize);

    if ((Msg.ID >> 28) == 0)
    {
        container.role = QSNContainer::message;
        container.Command = (Msg.ID >> 24) & 0xF;
        container.Address = Msg.ID & 0xFFF;
        container.Sender = (Msg.ID >> 12) & 0xFFF;
        container.Data = Msg.Data;
        emit snBUSOutput(container, this);
    }
    if ((Msg.ID >> 28) == 1)
    {
        container.role = QSNContainer::signal;
        container.Signal = (Msg.ID >> 12) & 0xFFFF;
        container.Data = Msg.Data;
        emit snBUSOutput(container, this);
    }
}

void QSnCanV5::serviceProcessing(QSNContainer container)
{
    switch (container.Command)
    {
    case 0:
        if (container.Signal == 0) openPort();
        if (container.Signal == 1) closePort();
        break;
    }
}

void QSnCanV5::sendFrame(quint32 ID, QByteArray *data)
{
    QByteArray frame;
    frame.resize(20);
    frame[0] = static_cast<char>(0xAA);
    frame[1] = static_cast<char>(0x55);
    frame[2] = static_cast<char>(0x01);
    frame[3] = static_cast<char>(0x02);
    frame[4] = static_cast<char>(0x01);
    frame[5] = static_cast<char>(ID & 0xFF);
    frame[6] = static_cast<char>((ID >> 8) & 0xFF);
    frame[7] = static_cast<char>((ID >> 16) & 0xFF);
    frame[8] = static_cast<char>((ID >> 24) & 0xFF);
    frame[9] = static_cast<char>(data->count());
    int i = 0;
    for (i = 0; i < data->count(); i ++)
        frame[10 + i] = data->at(i);
    frame[18] = 0;
    quint8 check = 0;
    for (i = 2; i < 19; i ++)
        check += frame[i];
    frame[19] = static_cast<char>(check);
    serial->write(frame);
    serial->flush();
}

void QSnCanV5::CANv5Initialization()
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Initialization"),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("adaptercanv5"),
                         false), this);
    QByteArray data = QByteArray::fromHex("AA5502080100000000000000000001000000000C");
    serial->write(data);
    serial->flush();
}

void QSnCanV5::openPort()
{
    if (serial->isOpen()) return;
    if (!serialEnable) return;

    serial->setBaudRate(static_cast<int>(1228800));
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        CANv5Initialization();
        portStateChange(true);
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             tr("Opening the port successfully"),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("adaptercanv5"),
                             false), this);
    } else {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             serial->errorString(),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("adaptercanv5"),
                             false), this);
    }
}

void QSnCanV5::closePort()
{
    if (!serial->isOpen()) return;
    serial->close();
    portStateChange(false);
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Closing the port"),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("adaptercanv5"),
                         true), this);
}

QString QSnCanV5::error()
{
    return lastError;
}

void QSnCanV5::setLogEnable(bool enable)
{
    isLog = enable;
}

void QSnCanV5::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("CanV5_Enabled"), isEnabled());
    settings->setValue(QLatin1String("CanV5_Port"), getPortName());
}

void QSnCanV5::loadSettings(QSettings *settings)
{
    setEnabled(settings->value(QLatin1String("CanV5_Enabled"), isEnabled()).toBool());
    setPortName(settings->value(QLatin1String("CanV5_Port"), getPortName()).toString());
}


void QSnCanV5::saveSettingsStream(QDataStream *stream)
{
    *stream << isEnabled();
    *stream << getPortName();
}

void QSnCanV5::loadSettingsStream(QDataStream *stream)
{
    bool enable;
    QString port;
    *stream >> enable;
    *stream >> port;
    setEnabled(enable);
    setPortName(port);
}

bool QSnCanV5::isEnabled()
{
    return serialEnable;
}

void QSnCanV5::setEnabled(bool enable)
{
    serialEnable = enable;
}

QString QSnCanV5::getPortName()
{
    return serial->portName();
}

void QSnCanV5::setPortName(QString portName)
{
    serial->setPortName(portName);
}

bool QSnCanV5::isConnected()
{
    return serial->isOpen();
}

void QSnCanV5::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role == QSNContainer::service)
    {
        serviceProcessing(container);
        return;
    }
    if (isConnected())
    {
        QByteArray data = sizeLimit(&container.Data);
        if (container.role == QSNContainer::signal)
        {
            quint32 id = 1;
            id = (((id << 16) | container.Signal) << 12);
            sendFrame(id, &data);
        }
        if (container.role == QSNContainer::message)
        {
            quint32 id = (container.Command & 0xF);
            id = ((((id << 12) | container.Sender) << 12) | container.Address);
            sendFrame(id, &data);
        }
    }
}
