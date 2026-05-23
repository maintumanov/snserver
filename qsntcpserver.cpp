#include "qsntcpserver.h"

QsnTCPserver::QsnTCPserver(QsnInterface *interface, QObject *parent) : QObject(parent)
{
    Server = new QTcpServer(this);
    setObjectName(QString(tr("TCP Server")));
    sninterface = interface;
    connect(Server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    Address = QHostAddress::Any;
    Port = 8888;
    serverState = false;
    serverEnable = false;
    authEnable = true;
    lastError = QString();
}

QsnTCPserver::~QsnTCPserver()
{
    actionDisconnect();
    delete Server;
}

void QsnTCPserver::setAuthEnable(bool enable)
{
    authEnable = enable;
}

bool QsnTCPserver::isAuthEnable()
{
    return authEnable;
}

quint16 QsnTCPserver::getPort()
{
    return Port;
}

void QsnTCPserver::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role == QSNContainer::service) {
        serviceProcessing(container);
        return;
    }
}

void QsnTCPserver::responseAuthentication(bool access, QObject *sender)
{
    for (int i = 0; i < TCPClients.count();  i ++)
        if (TCPClients[i] == sender) TCPClients[i]->responseAuthentication(access);
}

void QsnTCPserver::actionConnect()
{
    if (serverState == true) return;
    if (!Server->listen(Address, Port)) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Not listen port %1").arg(Port),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("tcpserver"),
                             false), this);
        return;
    }

    serverState = true;
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Listen port %1").arg(Port),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("tcpserver"),
                         true), this);
    connectedChange(true);
}

void QsnTCPserver::actionDisconnect()
{
    if (serverState == false) return;

    for (int i = TCPClients.count() - 1; i >= 0; i --)
        TCPClients[i]->slotDisconnect();

    Server->close();
    serverState = false;
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_warning,
                         tr("Closing port"),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("tcpserver"),
                         false), this);
    connectedChange(false);
}

bool QsnTCPserver::isEnabled()
{
    return serverEnable;
}

QString QsnTCPserver::error()
{
    return lastError;
}

void QsnTCPserver::setEnabled(bool enable)
{
    serverEnable = enable;
}

void QsnTCPserver::setPort(quint16 port)
{
    quint16 oldPort = Port;
    Port = port;
    if (serverState && Port != oldPort) {
        actionDisconnect();
        actionConnect();
    }
}

bool QsnTCPserver::isConnected()
{
    return serverState;
}

void QsnTCPserver::newConnection()
{
    QsnTCPSocket *tcpSocket = new QsnTCPSocket(Server->nextPendingConnection(), sninterface, authEnable, this);
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("New connection from %1").arg(QSNCleanIP(tcpSocket->clientIPAddress().toString())),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString(),
                         true), this);

    TCPClients.append(tcpSocket);
    connect(tcpSocket, SIGNAL(closeConnected(QsnTCPSocket*)), this, SLOT(closeConnection(QsnTCPSocket*)));
    connect(tcpSocket, SIGNAL(snBUSOutput(QSNContainer,QObject*)), sninterface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(sninterface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), tcpSocket, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(tcpSocket, SIGNAL(authorizationRequest(QString,QString,QString,QObject*)), this, SIGNAL(authorizationRequest(QString,QString,QString,QObject*)));
    emit stateChanged();
}

void QsnTCPserver::closeConnection(QsnTCPSocket *socket)
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Close connection from %1").arg(QSNCleanIP(socket->clientIPAddress().toString())),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("tcpserver"),
                         true), this);
    for (int i = TCPClients.count() - 1; i >= 0; i --)
        if (TCPClients[i] == socket) TCPClients.removeAt(i);
    emit stateChanged();
}

void QsnTCPserver::serviceProcessing(QSNContainer container)
{
    switch (container.Command)
    {
    case 0:
        if (container.Signal == 0) actionConnect();
        if (container.Signal == 1) actionDisconnect();
        break;
    }
}

void QsnTCPserver::connectedChange(bool state)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 1;
    if (state) container.Signal = 0;
    else container.Signal = 1;
    snBUSOutput(container, this);
}

//==================================================================================================

QsnTCPSocket::QsnTCPSocket(QTcpSocket *Socket, QsnInterface *interface, bool authEn, QObject *)
{
    setObjectName(QString(tr("TCP Server")));
    clientSocket = Socket;
    sninterface = interface;
    caseMode = 1;
    authState = false;
    authEnable = authEn;
    login = "anonymous";
    cConnectionTime = QDateTime::currentDateTime();
    connect(Socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(Socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
}

QsnTCPSocket::~QsnTCPSocket()
{
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
}

quint16 QsnTCPSocket::clientDevAddress()
{
    return cDeviceAddress;
}

QHostAddress QsnTCPSocket::clientIPAddress()
{
    return clientSocket->peerAddress();
}

QString QsnTCPSocket::clientName()
{
    return cName;
}

QDateTime QsnTCPSocket::connectionTime()
{
    return cConnectionTime;
}

QString QsnTCPSocket::clientLastError()
{
    return cLastError;
}

void QsnTCPSocket::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;

    if (container.role <= QSNContainer::message)  {
        QByteArray block;
        QDataStream stream(&block, QIODevice::ReadWrite);
        containerToStream(&container, &stream);
        sendData(&block);
    }

    if (container.role == QSNContainer::information)  {
        if (container.Command != 0 && container.Command != 5) return;
        QStringList options = QString::fromUtf8(container.Data).split("/");
        if (options.count() > 1) container.Data = QString("%1/%2").arg(objectName(), options[1]).toUtf8();
        if (checkingContainerForRestrictions(&container)) {
            QByteArray block;
            QDataStream stream(&block, QIODevice::ReadWrite);
            containerToStream(&container, &stream);
            sendData(&block);
        } else
            emit snBUSOutput(QSNLogToContainer(
                                 BUSSERV_LOG_LEVEL_caution,
                                 tr("Exceeding container limits %1").arg(QString::fromUtf8(container.Data)),
                                 BUSSERV_LOG_CATEGORY_adapter,
                                 QString("tcpserver"),
                                 true), this);
    }
}

void QsnTCPSocket::responseAuthentication(bool access)
{
    if (access) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             tr("Incoming connection - access is allowed"),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString(),
                             true), this);
    } else {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_warning,
                             tr("Incoming connection - access is denied!"),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("tcpserver"),
                             true), this);
    }
    authState = access;
    QByteArray block;
    QDataStream stream(&block, QIODevice::ReadWrite);
    QSNContainer container = newContainer();
    container.role = QSNContainer::authorization;
    if (authState)  container.Signal = 1;
    else container.Signal = 0;
    container.Sender = sninterface->getDeviceAddress();
    containerToStream(&container, &stream);
    sendData(&block);
    if (!authState) slotDisconnect();
}

void QsnTCPSocket::slotDisconnect()
{
    clientSocket->close();
}

void QsnTCPSocket::readyRead()
{
    switch (caseMode)
    {
    case 1:state_1(); break;
    case 2:state_2(); break;
    case 3:readSize(); break;
    case 4:readData(); break;
    }
}

void QsnTCPSocket::slotDisconnected()
{
    emit closeConnected(this);
    this->deleteLater();
}

void QsnTCPSocket::writeSize(quint32 size)
{
    QByteArray block;
    QDataStream stream(&block, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << quint8(115);
    stream << quint8(110);
    stream << size;
    clientSocket->write(block);
    clientSocket->flush();
}

void QsnTCPSocket::readSize()
{
    if (clientSocket->bytesAvailable() < 4) return;
    QByteArray ba;
    ba = clientSocket->read(4);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_7);
    stream >> blockSize;
    caseMode ++;
    readyRead();
}

void QsnTCPSocket::readData()
{
    if (clientSocket->bytesAvailable() < static_cast<qint64>(blockSize)) return;
    QByteArray block;
    block = clientSocket->read(static_cast<qint64>(blockSize));
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Read") + " " + tr("data") + " " + block.toHex(),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString(),
                         true), this);
    QDataStream stream(&block, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_7);
    if (!authState) readAuth(&stream);
    else {
        QSNContainer container = containerFromStream(&stream);
        if (container.role == QSNContainer::information) {
            if (container.Signal == 0) cLastError = container.info;
        }
        emit snBUSOutput(container, this);
    }
    caseMode = 1;
    readyRead();
}

void QsnTCPSocket::sendData(QByteArray *block)
{
    writeSize(static_cast<quint32>(block->size()));
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Send") + " " + tr("data") + " " + block->toHex(),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString(),
                         true), this);
    clientSocket->write(*block);
    clientSocket->flush();
}

void QsnTCPSocket::readAuth(QDataStream *stream)
{
    QSNContainer container = containerFromStream(stream);
    cDeviceAddress = container.Sender;
    cName = tr("Unknown device");
    if (!authEnable) {
        cName = container.info;
        responseAuthentication(true);
        return;
    }
    login = container.info;
    if (container.role == QSNContainer::authorization) {
        QString user = container.info;
        QString pwd = QString(container.Data);
        int index = pwd.indexOf(":::");
        if (index != -1) {
            user = pwd.left(index);
            pwd.remove(0, index + 3);
            cName = container.info;
        }
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             QString("authorizationRequest user:%1 ip:%2").arg(user, QSNCleanIP(clientIPAddress().toString())),
                             BUSSERV_LOG_CATEGORY_adapter,
                             QString("tcpserver"),
                             false), this);
        emit authorizationRequest(user, pwd, QSNCleanIP(clientIPAddress().toString()), this);
    }
}

void QsnTCPSocket::state_1()
{
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    clientSocket->read(&p, 1);
    if (p == char(115)) caseMode ++;
    readyRead();
}

void QsnTCPSocket::state_2()
{
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    clientSocket->read(&p, 1);
    if (p == char(110)) caseMode ++;
    else caseMode = 1;
    readyRead();
}

