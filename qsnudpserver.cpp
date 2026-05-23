#include "qsnudpserver.h"

QsnUDPserver::QsnUDPserver(QObject *parent) :
    QObject(parent)
{
    setObjectName(QString(tr("UDP Server")));
    serverSocket = new QUdpSocket(this);
    serverPort = 29550;
    deviceAddress = 4090;
    isLog = false;
    serverEnable = false;
    encryptKey = QLatin1String("signalnet");
    lastError = QString();
    loadClients();
    QObject::connect(serverSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));
}

QsnUDPserver::~QsnUDPserver()
{
    unregisteredClients();
    QObject::disconnect(serverSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));
    delete serverSocket;
}

void QsnUDPserver::serviceProcessing(QSNContainer container)
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

void QsnUDPserver::changeSNAddress(QHostAddress address, quint16 port, QSNContainer *container)
{
    QString name = deviceName(container->Sender);
    unregistredClient(address, port);
    registredClient(address, port, QSNRAWtoUInt16(&container->Data, 0), name);
}

void QsnUDPserver::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("UDPServer_Enabled"), isEnabled());
    settings->setValue(QLatin1String("UDPServer_Port"), getPort());
    settings->setValue(QLatin1String("UDPServer_Key"), getKey());
}

void QsnUDPserver::loadSettings(QSettings *settings)
{
    setEnabled(settings->value(QLatin1String("UDPServer_Enabled"), isEnabled()).toBool());
    setPort(static_cast<quint16>(settings->value(QLatin1String("UDPServer_Port"), getPort()).toUInt()));
    setKey(settings->value(QLatin1String("UDPServer_Key"), getKey()).toString());
}

void QsnUDPserver::actionConnect(quint16 address)
{
    if (!serverEnable) return;
    if (isConnected()) return;
    deviceAddress = address;
    serverSocket->abort();
    bool connected = serverSocket->bind(QHostAddress::Any, serverPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!connected) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Bind server failed"),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("udpserver"),
                             false), this);
        lastError = tr("Bind server failed");
        return;
    }
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Bind server port %1").arg(serverPort),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("udpserver"),
                         false), this);
    connectedChange(true);
}

bool QsnUDPserver::isEnabled()
{
    return serverEnable;
}

void QsnUDPserver::setEnabled(bool enable)
{
    serverEnable = enable;
}

quint16 QsnUDPserver::getPort()
{
    return serverPort;
}

void QsnUDPserver::setPort(quint16 port)
{
    serverPort = port;
}

QString QsnUDPserver::getKey()
{
    return encryptKey;
}

void QsnUDPserver::setKey(QString key)
{
    encryptKey = key;
}

bool QsnUDPserver::isConnected()
{
    if (serverSocket->state() == QAbstractSocket::BoundState) return true;
    return false;
}

QString QsnUDPserver::error()
{
    return lastError;
}

void QsnUDPserver::setLogEnable(bool enable)
{
    isLog = enable;
}

bool QsnUDPserver::logEnable()
{
    return isLog;
}

void QsnUDPserver::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    //if (container.role == QSNContainer::message && container.Address == deviceAddress) return;
    if (container.role == QSNContainer::service)  {
        serviceProcessing(container);
        return;
    }

    if (!isConnected()) return;

    if (container.role == QSNContainer::authorization) return;
    if (container.role == QSNContainer::registration) return;

    if (container.role == QSNContainer::information) {
        if (container.Command != 0 && container.Command != 5) return;
        QStringList options = QString::fromUtf8(container.Data).split("/");
        if (options.count() > 1) container.Data = QString("%1/%2").arg(objectName(), options[1]).toUtf8();
    }

    if (!checkingContainerForRestrictions(&container)) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Exceeding container limits %1").arg(QString::fromUtf8(container.Data)),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("udpserver"),
                             false), this);
        return;
    }

    QByteArray block;
    QDataStream stream(&block, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << static_cast<quint8>(0x73);
    stream << static_cast<quint8>(0x6e);
    stream << deviceAddress;
    containerToStream(&container, &stream);
    dataEncript(&block, 4, encryptKey);

    // Send signals
    if (container.role == QSNContainer::signal || container.role == QSNContainer::information) {
        for (int i = 0; i < clients.count(); i ++)
            serverSocket->writeDatagram(block, clients[i].ipAddress, clients[i].port);
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             "[SIGNAL]" + QSNContainerToLogText(container, false),
                             BUSSERV_LOG_CATEGORY_network,
                             QString(),
                             true), this);

        return;
    }

    if (container.role == QSNContainer::message && container.Command != 0) {
        for (int i = 0; i < clients.count(); i ++)
            if (clients[i].devAddress == container.Address)
                serverSocket->writeDatagram(block, clients[i].ipAddress, clients[i].port);

        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             "[SIGNAL]" + QSNContainerToLogText(container, false),
                             BUSSERV_LOG_CATEGORY_network,
                             QString(),
                             true), this);

        return;
    }

    // Sending all known connections
    for (int j = 0; j < clients.count(); j ++)
        serverSocket->writeDatagram(block, clients[j].ipAddress, clients[j].port);
    if (container.role != QSNContainer::information)
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             "[ALL]" + QSNContainerToLogText(container, false),
                             BUSSERV_LOG_CATEGORY_network,
                             QString(),
                             true), this);
}

void QsnUDPserver::readyRead()
{
    while (serverSocket->isValid() && serverSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(serverSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        serverSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        if (sender == serverSocket->localAddress()) continue;
        dataDecript(&datagram, 4, encryptKey);
        QDataStream stream(&datagram, QIODevice::ReadWrite);
        stream.setVersion(QDataStream::Qt_4_7);
        quint8 p;
        quint16 addr;
        stream >> p;
        if (p != 0x73) continue;
        stream >> p;
        if (p != 0x6e) continue;
        stream >> addr;
        if (addr == deviceAddress) continue;

        QSNContainer container = containerFromStream(&stream);
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             QSNContainerToLogText(container, true),
                             BUSSERV_LOG_CATEGORY_network,
                             QString(),
                             true), this);
        dataEncript(&datagram, 4, encryptKey);

        if (container.role == QSNContainer::information) {
            if (container.Signal == 0) setDeviceError(addr, container.info);
        }

        if (container.role == QSNContainer::registration) {
            if (container.Signal == 1) registredClient(sender, senderPort, addr, container.info);
            else unregistredClient(sender, senderPort);
            registredClientAnswer(sender, senderPort);
            continue;
        }

        if (container.role == QSNContainer::signal) {
            for (int i = 0; i < clients.count(); i ++) {
                if (clients[i].ipAddress != sender || clients[i].port != senderPort)
                    serverSocket->writeDatagram(datagram, clients[i].ipAddress, clients[i].port);
            }
            emit snBUSOutput(container, this);
            continue;
        }

        if (container.role == QSNContainer::message && container.Address != 0) {
            if (container.Command == 7) changeSNAddress(sender, senderPort, &container);
            // Sending known connections
            for (int j = 0; j < clients.count(); j ++)
                if (clients[j].devAddress == container.Address) serverSocket->writeDatagram(datagram, clients[j].ipAddress, clients[j].port);
            emit snBUSOutput(container, this);
            continue;
        }

        // Sending all known connections
        for (int j = 0; j < clients.count(); j ++)
            if (clients[j].ipAddress != sender || clients[j].port != senderPort) serverSocket->writeDatagram(datagram, clients[j].ipAddress, clients[j].port);
        emit snBUSOutput(container, this);

    }
}

void QsnUDPserver::actionDisconnect()
{
    if (!isConnected()) return;
    unregisteredClients();
    serverSocket->abort();
    connectedChange(false);
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Bind server stop"),
                         BUSSERV_LOG_CATEGORY_network,
                         QString("udpserver"),
                         false), this);
}

void QsnUDPserver::connectedChange(bool state)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 1;
    if (state) container.Signal = 0;
    else container.Signal = 1;
    {emit snBUSOutput(container, this);}
    emit stateChanged();
}

void QsnUDPserver::loadClients()
{
    //    QFile file(QSNHomeSubPath("server", "connections").absoluteFilePath(QString("udpserver.txt")));
    //    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    //    QTextStream in(&file);
    //    QString line;
    //    int pos;
    //    clients.clear();
    //    clientRec rec;
    //    while (!in.atEnd()) {
    //        line = in.readLine();
    //        pos = line.indexOf(' ');
    //        rec.ipAddress = QHostAddress(line.left(pos));
    //        line = line.remove(0, pos + 1);
    //        pos = line.indexOf(' ');
    //        rec.port = static_cast<quint16>(line.left(pos).toUInt());
    //        line = line.remove(0, pos + 1);
    //        pos = line.indexOf(' ');
    //        rec.devAddress = static_cast<quint16>(line.left(pos).toInt());
    //        line = line.remove(0, pos + 1);
    //        rec.name = line;
    //        rec.checkTime = QDateTime::currentDateTime();
    //        rec.connectTime = QDateTime::currentDateTime();
    //        clients.append(rec);
    //    }
    //    file.close();
}

void QsnUDPserver::unregisteredClients()
{
    QList<clientRec> oldcl;
    oldcl.append(clients);

    clients.clear();
    //    QFile file(QSNHomeSubPath("server", "connections").absoluteFilePath(QString("udpserver.txt")));
    //    file.remove();

    for (int i = oldcl.count() - 1; i >= 0; i --)
        registredClientAnswer(oldcl[i].ipAddress, oldcl[i].port, false);
}

void QsnUDPserver::registredClientAnswer(QHostAddress address, quint16 port, bool success)
{
    QByteArray block;
    QDataStream stream(&block, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << static_cast<quint8>(0x73);
    stream << static_cast<quint8>(0x6e);
    stream << deviceAddress;

    QSNContainer container = newContainer();
    container.role = QSNContainer::registration;
    if (success) container.Signal = 2;
    else container.Signal = 3;
    containerToStream(&container, &stream);
    dataEncript(&block, 4, encryptKey);
    serverSocket->writeDatagram(block, address, port);
}

void QsnUDPserver::registredClient(QHostAddress address, quint16 port, quint16 devAddr, QString name)
{
    clientRec rec;
    for (int i = 0; i < clients.count(); i ++) {
        if (clients[i].ipAddress == address && clients[i].port == port) {
            if (clients[i].devAddress == devAddr) {
                clients[i].name = name;
                clients[i].lastError = QString();
                clients[i].checkTime = QDateTime::currentDateTime();
                emit snBUSOutput(QSNLogToContainer(
                                     BUSSERV_LOG_LEVEL_information,
                                     tr("Registred udp client %1, time update").arg(address.toString()),
                                     BUSSERV_LOG_CATEGORY_network,
                                     QString(),
                                     true), this);
                return;
            }
            unregistredClient(clients[i].ipAddress, clients[i].port);
        }
    }

    rec.ipAddress = address;
    rec.port = port;
    rec.devAddress = devAddr;
    rec.name = name;
    rec.lastError = QString();
    rec.connectTime = QDateTime::currentDateTime();
    rec.checkTime = QDateTime::currentDateTime();
    clients.append(rec);
    emit stateChanged();
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Registred udp client %1").arg(address.toString()),
                         BUSSERV_LOG_CATEGORY_network,
                         QString(),
                         true), this);
    //    QFile file(QSNHomeSubPath("server", "connections").absoluteFilePath(QString("udpserver.txt")));
    //    if (!file.open(QIODevice::Append | QIODevice::Text)) return;
    //    QTextStream Out(&file);
    //    Out << address.toString() << QLatin1String(" ") << QString::number(port) << QLatin1String(" ") << devAddr
    //        << QLatin1String(" ") << name << endl;
    //    file.close();
}

void QsnUDPserver::unregistredClient(QHostAddress address, quint16 port)
{
    //    bool saveEnable = false;
    for (int i = clients.count() - 1; i >= 0; i --)
        if (clients[i].ipAddress == address && clients[i].port == port) {
            emit snBUSOutput(QSNLogToContainer(
                                 BUSSERV_LOG_LEVEL_information,
                                 tr("Unegistred udp client %1").arg(address.toString()),
                                 BUSSERV_LOG_CATEGORY_network,
                                 QString(),
                                 true), this);
            clients.removeAt(i);
            emit stateChanged();
            //            saveEnable = true;
        }
    //    if (!saveEnable) return;
    //    QFile file(QSNHomeSubPath("server", "connections").absoluteFilePath(QString("udpserver.txt")));
    //    if (clients.count()) {
    //        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    //        QTextStream Out(&file);
    //        for (int i = 0; i < clients.count(); i ++)
    //            Out << clients[i].ipAddress.toString() << QLatin1String(" ") << QString::number(clients[i].port) << QLatin1String(" ") << clients[i].devAddress
    //                << QLatin1String(" ") << clients[i].name << endl;
    //        file.close();
    //    } else file.remove();
}

void QsnUDPserver::setDeviceName(quint16 address, QString name)
{
    if (name.isEmpty()) return;
    for (int j = clients.count() - 1; j >= 0; j --)
        if (clients[j].devAddress == address) {
            clients[j].name = name;
            return;
        }
}

QString QsnUDPserver::deviceName(quint16 address)
{
    for (int j = clients.count() - 1; j >= 0; j --)
        if (clients[j].devAddress == address)
            return clients[j].name;
    return QString();
}

void QsnUDPserver::setDeviceError(quint16 address, QString error)
{
    if (error.isEmpty()) return;
    emit stateChanged();
    for (int j = clients.count() - 1; j >= 0; j --)
        if (clients[j].devAddress == address) {
            clients[j].lastError = error;
            return;
        }
}

void QsnUDPserver::checkDeviceExist()
{
    for (int j = clients.count() - 1; j >= 0; j --){
        if (clients[j].checkTime.secsTo(QDateTime::currentDateTime()) >= QSNUDPSERVER_TIMEOUT) {
            registredClientAnswer(clients[j].ipAddress, clients[j].port, false);
            unregistredClient(clients[j].ipAddress, clients[j].port);
        }
    }
}


