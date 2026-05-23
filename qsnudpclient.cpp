#include "qsnudpclient.h"

QsnUDPclient::QsnUDPclient(QObject *parent) :
    QObject(parent)
{
    setObjectName(QString(tr("UDP Client")));
    deviceName = QCoreApplication::applicationName();
    clientSocket = new QUdpSocket();
    serverport = 29550;
    sourceport = 29549;
    clientaddress = QLatin1String("192.168.1.1");
    clientkey = QLatin1String("signalnet");
    deviceAddress = 4090;
    isLog = false;
    clientEnable = false;
    autoRepeatMode = false;
    lastError = QString();
    registredTimeOut = 0;
    connect(clientSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));
}

QsnUDPclient::~QsnUDPclient()
{
    disconnect(clientSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));
    delete clientSocket;
}

void QsnUDPclient::serviceProcessing(QSNContainer container)
{
    switch (container.Command) {
    case 0:
        if (container.Signal == 0) actionConnect(container.Address);
        if (container.Signal == 1) actionDisconnect();
        break;
    case 2:
        checkDeviceExist();
        break;
    }
}

void QsnUDPclient::checkDeviceExist()
{
    if (registredTimeOut) {
        registredTimeOut --;
        if (registredTimeOut == 5) registration(true);
        if (registredTimeOut == 2) registration(true);
        if (registredTimeOut == 0) timeout();
    }
}

bool QsnUDPclient::checkingContainerForRestrictions(QSNContainer *container)
{
    if (container->Data.count() > 120) return false;
    if (container->info > 120) return false;
    return true;
}

void QsnUDPclient::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("UDP_Enabled"), isEnabled());
    settings->setValue(QLatin1String("UDP_ServerPort"), getServerPort());
    settings->setValue(QLatin1String("UDP_SourcePort"), getSourcePort());
    settings->setValue(QLatin1String("UDP_Address"), getAddress());
    settings->setValue(QLatin1String("UDP_Key"), getKey());
}

void QsnUDPclient::loadSettings(QSettings *settings)
{
    setEnabled(settings->value(QLatin1String("UDP_Enabled"), isEnabled()).toBool());
    setServerPort(static_cast<quint16>(settings->value(QLatin1String("UDP_ServerPort"), getServerPort()).toUInt()));
    setSourcePort(static_cast<quint16>(settings->value(QLatin1String("UDP_SourcePort"), getSourcePort()).toUInt()));
    setAddress(settings->value(QLatin1String("UDP_Address"), getAddress()).toString());
    setKey(settings->value(QLatin1String("UDP_Key"), getKey()).toString());
}

void QsnUDPclient::actionConnect(quint16 address)
{
    if (!clientEnable) return;

    registredTimeOut = 0;
    deviceAddress = address;
    clientSocket->abort();
    clientSocket->close();
    bool connected = clientSocket->bind(QHostAddress::Any, sourceport, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!connected) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Bind failed"),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("udpclient"),
                             false), this);
        lastError = tr("Bind failed");
        return;
    }
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Bind port %1").arg(sourceport),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("udpclient"),
                         false), this);
    lastError = QString();
    registration();
}

bool QsnUDPclient::isEnabled()
{
    return clientEnable;
}

void QsnUDPclient::setEnabled(bool enable)
{
    clientEnable = enable;
}

quint16 QsnUDPclient::getServerPort()
{
    return serverport;
}

void QsnUDPclient::setServerPort(quint16 port)
{
    serverport = port;
}

quint16 QsnUDPclient::getSourcePort()
{
    return sourceport;
}

void QsnUDPclient::setSourcePort(quint16 port)
{
    sourceport = port;
}

QString QsnUDPclient::getAddress()
{
    return clientaddress;
}

void QsnUDPclient::setAddress(QString address)
{
    clientaddress = address;
}

QString QsnUDPclient::getKey()
{
    return clientkey;
}

void QsnUDPclient::setKey(QString key)
{
    clientkey = key;
}

bool QsnUDPclient::isConnected()
{
    return (registredTimeOut != 0);
}

void QsnUDPclient::setDeviceName(QString name)
{
    deviceName = name;
}

void QsnUDPclient::saveSettingsStream(QDataStream *stream)
{
    *stream << isEnabled();
    *stream << getServerPort();
    *stream << getSourcePort();
    *stream << getAddress();
    *stream << getKey();
}

void QsnUDPclient::loadSettingsStream(QDataStream *stream)
{
    bool enable;
    QString s;
    quint16 i;
    *stream >> enable;
    setEnabled(enable);
    *stream >> i;
    setServerPort(i);
    *stream >> i;
    setSourcePort(i);
    *stream >> s;
    setAddress(s);
    *stream >> s;
    setKey(s);
}

void QsnUDPclient::setLogEnable(bool enable)
{
    isLog = enable;
}

bool QsnUDPclient::logEnable()
{
    return isLog;
}

void QsnUDPclient::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role == QSNContainer::service)  {
        serviceProcessing(container);
        return;
    }

    if (!isConnected()) return;
    if (!checkingContainerForRestrictions(&container)) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Exceeding container limits %1").arg(QString::fromUtf8(container.Data)),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("udpclient"),
                             false), this);
        lastError = tr("Exceeding container limits %1").arg(QString::fromUtf8(container.Data));
        return;
    }

    if (container.role == QSNContainer::authorization) return;
    if (container.role == QSNContainer::registration) return;

    if (container.role == QSNContainer::information) {
        if (container.Command != 0 && container.Command != 5) return;
        QStringList options = QString::fromUtf8(container.Data).split("/");
        if (options.count() > 1) container.Data = QString("%1/%2").arg(objectName(), options[1]).toUtf8();
    }

    QByteArray block;
    QDataStream stream(&block, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << static_cast<quint8>(0x73);
    stream << static_cast<quint8>(0x6e);
    stream << deviceAddress;
    containerToStream(&container, &stream);
    dataEncript(&block, 4, clientkey);
    clientSocket->writeDatagram(block, QHostAddress(clientaddress), serverport);
}

void QsnUDPclient::readyRead()
{
    while (clientSocket->isValid() && clientSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(clientSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        clientSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        readedBlock(&datagram);
    }
}

void QsnUDPclient::actionDisconnect()
{
    registration(false);
    connectedChange(false);
    clientSocket->abort();
    clientSocket->close();
    registredTimeOut = 0;
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Close"),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("udpclient"),
                         false), this);
    lastError = QString();
}

void QsnUDPclient::setRepeatMode()
{
    autoRepeatMode = true;
}

QString QsnUDPclient::error()
{
    return lastError;
}

void QsnUDPclient::connectedChange(bool state)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 1;
    if (state) container.Signal = 0;
    else container.Signal = 1;
    {emit snBUSOutput(container, this);}
}

void QsnUDPclient::timeout()
{
    if (!autoRepeatMode) {
        registredTimeOut = 0;
        clientSocket->abort();
        clientSocket->close();
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Connection lost"),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("udpclient"),
                             false), this);
        lastError = tr("Connection lost");
        connectedChange(false);
        return;
    }
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_caution,
                         tr("Connection lost, reconnect"),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("udpclient"),
                         false), this);
    lastError = tr("Connection lost, reconnect");
    registredTimeOut = 60;
}

void QsnUDPclient::registrationAnswer(QSNContainer *container)
{

    if (container->Signal == 2) {
        if (registredTimeOut == 0) connectedChange(true);
        lastError = QString();
        registredTimeOut = 20;
        return;
    }
    if (container->Signal == 3) {
        clientSocket->abort();
        clientSocket->close();
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Registration is denied"),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("udpclient"),
                             false), this);
        lastError = tr("Registration is denied");
        registredTimeOut = 0;
        connectedChange(false);
        actionConnect(deviceAddress);
    }
}

void QsnUDPclient::registration(bool registration)
{
    QByteArray block;
    QDataStream stream(&block, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << static_cast<quint8>(0x73);
    stream << static_cast<quint8>(0x6e);
    stream << deviceAddress;
    QSNContainer container = newContainer();
    container.role = QSNContainer::registration;
    if (registration) container.Signal = 1;
    else container.Signal = 0;
    container.Sender = deviceAddress;
    container.info = objectName();
    containerToStream(&container, &stream);
    dataEncript(&block, 4, clientkey);
    clientSocket->writeDatagram(block, QHostAddress(clientaddress), serverport);
}

void QsnUDPclient::readedBlock(QByteArray *block)
{
    QDataStream stream(block, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    quint8 p;
    quint16 addr;
    stream >> p;
    if (p != 0x73) return;
    stream >> p;
    if (p != 0x6e) return;
    stream >> addr;
    if (addr == deviceAddress) return;
    if (!dataDecript(block, 4, clientkey)) return;
    QSNContainer container = containerFromStream(&stream);
    if (container.role == QSNContainer::registration) {
        registrationAnswer(&container);
        return;
    }
    emit snBUSOutput(container, this);
}
