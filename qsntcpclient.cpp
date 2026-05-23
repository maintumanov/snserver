#include "qsntcpclient.h"


QsnTCPclient::QsnTCPclient(QObject *parent) :
    QObject(parent)
{
    setObjectName(QString(tr("TCPclient")));
    deviceName = QCoreApplication::applicationName();
    clientSocket = new QTcpSocket();
    clientaddress = QLatin1String("localhost");
    clientport = 8888;
    clientEnable = false;
    caseMode = 1;
    authorized = false;
    QObject::connect(clientSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));

}

QsnTCPclient::~QsnTCPclient()
{
    QObject::disconnect(clientSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));
    delete clientSocket;
}

void QsnTCPclient::serviceProcessing(QSNContainer container)
{
    switch (container.Command)
    {
    case 0:
        if (container.Signal == 0) connectToServer(container.Address);
        if (container.Signal == 1) disconnectFromServer();
        break;
    }
}

void QsnTCPclient::info(quint8 warningLevel, QString text)
{
    QSNContainer container;
    container.role = QSNContainer::information;
    container.Command = warningLevel;
    container.info = text;
    container.Address = 0;
    container.Sender = 0;
    snBUSOutput(container, this);
}

void QsnTCPclient::authorizationRequest()
{
    QByteArray block;
    QDataStream stream(&block, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    QSNContainer container;
    container.role = QSNContainer::authorization;
    container.Address = 0;
    container.info = deviceName;
    QString lp = clientlogin + ":::" + clientpassword;
    container.Data = lp.toUtf8();
    container.Sender = deviceAddress;
    stream.setVersion(QDataStream::Qt_4_7);
    containerToStream(&container, &stream);
    sendBlock(&block);
}

void QsnTCPclient::readAuth(QDataStream *stream)
{
    QSNContainer container = containerFromStream(stream);
    if (container.role != QSNContainer::authorization) return;
    if (container.Signal) {
        authorized = true;
        connectedChange(true);
        info(2, QString(tr("Connect to server successfully")));
    } else {
        info(0, QString(tr("Authorization failed")));
        disconnectFromServer();
    }
}

void QsnTCPclient::state_1()
{
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    clientSocket->read(&p, 1);
    if (p == char(115)) caseMode ++;
    readyRead();
}

void QsnTCPclient::state_2()
{
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    clientSocket->read(&p, 1);
    if (p == char(110)) caseMode ++;
    else caseMode = 1;
    readyRead();
}

void QsnTCPclient::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("TCP_Enabled"), isEnabled());
    settings->setValue(QLatin1String("TCP_Port"), getPort());
    settings->setValue(QLatin1String("TCP_Address"), getAddress());
    settings->setValue(QLatin1String("TCP_Login"), getLogin());
    settings->setValue(QLatin1String("TCP_Password"), getPassword());
}

void QsnTCPclient::loadSettings(QSettings *settings)
{
    setEnabled(settings->value(QLatin1String("TCP_Enabled"), isEnabled()).toBool());
    setPort(settings->value(QLatin1String("TCP_Port"), getPort()).toUInt());
    setAddress(settings->value(QLatin1String("TCP_Address"), getAddress()).toString());
    clientlogin = settings->value(QLatin1String("TCP_Login"), getLogin()).toString();
    clientpassword = settings->value(QLatin1String("TCP_Password"), getPassword()).toString();
}

void QsnTCPclient::saveSettingsStream(QDataStream *stream)
{
    *stream << isEnabled();
    *stream << getPort();
    *stream << getAddress();
    *stream << getLogin();
    *stream << getPassword();
}

void QsnTCPclient::loadSettingsStream(QDataStream *stream)
{
    bool enable;
    QString s;
    quint16 i;
    *stream >> enable;
    setEnabled(enable);
    *stream >> i;
    setPort(i);
    *stream >> s;
    setAddress(s);
    *stream >> clientlogin;
    *stream >> clientpassword;
}

void QsnTCPclient::connectToServer(quint16 devaddress)
{
    if (!clientEnable) return;
    bool connected;
    deviceAddress = devaddress;
    clientSocket->connectToHost(clientaddress, clientport);
    connected = clientSocket->waitForConnected(3000);
    if (!connected) {
        info(0, QString(tr("Connect to server failed")));
        return;
    }
    authorized = false;
    caseMode = 1;
    timeDelay.start();
    QObject::connect(clientSocket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    readyRead();
    authorizationRequest();
}

bool QsnTCPclient::isEnabled()
{
    return clientEnable;
}

void QsnTCPclient::setEnabled(bool enable)
{
    clientEnable = enable;
}

quint16 QsnTCPclient::getPort()
{
    return clientport;
}

void QsnTCPclient::setPort(quint16 port)
{
    clientport = port;
}

bool QsnTCPclient::isConnected()
{
    return clientSocket->isOpen();
}

void QsnTCPclient::setLoginAndPassword(QString login, QString pass)
{
    clientlogin = login;
    clientpassword = pass;
}

QString QsnTCPclient::getLogin()
{
    return clientlogin;
}

QString QsnTCPclient::getPassword()
{
    return clientpassword;
}

void QsnTCPclient::setDeviceName(QString name)
{
    deviceName = name;
}

void QsnTCPclient::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role == QSNContainer::service)
    {
        serviceProcessing(container);
        return;
    }
    if (isConnected() && (container.role == QSNContainer::signal || container.role == QSNContainer::message))
    {
        QByteArray block;
        QDataStream stream(&block, QIODevice::ReadWrite);
        stream.setVersion(QDataStream::Qt_4_7);
        containerToStream(&container, &stream);
        sendBlock(&block);
    }
}

QString QsnTCPclient::getAddress()
{
    return clientaddress;
}

void QsnTCPclient::setAddress(QString address)
{
    clientaddress = address;
}

void QsnTCPclient::readSize()
{
    if (clientSocket->bytesAvailable() < 4) return;
    QByteArray ba;
    ba = clientSocket->read(4);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_7);
    stream >> dataBlockSize;
    caseMode ++;
    readyRead();
}

void QsnTCPclient::writeSize(quint32 size)
{
    QByteArray block;
    QDataStream stream(&block, QIODevice::ReadWrite);
    stream << quint8(115);
    stream << quint8(110);
    stream << size;
    clientSocket->write(block);
    clientSocket->flush();
}

void QsnTCPclient::readyRead()
{
    switch (caseMode)
    {
    case 1:state_1(); break;
    case 2:state_2(); break;
    case 3:readSize(); break;
    case 4:readAnswerPath(); break;
    }
}

void QsnTCPclient::disconnected()
{
    connectedChange(false);
    clientSocket->close();
    info(0, QString(tr("The connection terminated unexpectedly!")));
}

void QsnTCPclient::sendBlock(QByteArray *block)
{
    writeSize(block->size());
    clientSocket->write(*block);
    clientSocket->flush();
    caseMode = 1;
    readyRead();
}

void QsnTCPclient::disconnectFromServer()
{
    QObject::disconnect(clientSocket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    clientSocket->disconnectFromHost();
    clientSocket->close();
    connectedChange(false);
}

void QsnTCPclient::connectedChange(bool state)
{
    QSNContainer container;
    container.role = QSNContainer::service;
    container.Address = 0;
    container.Sender = 0;
    container.Command = 1;
    if (state) container.Signal = 0;
    else container.Signal = 1;
    {emit snBUSOutput(container, this);}
}

void QsnTCPclient::readAnswerPath()
{
    if (clientSocket->bytesAvailable() < (qint64)dataBlockSize) return;
    QByteArray block;
    block = clientSocket->read((qint64)dataBlockSize);
    readedBlock(&block);
    caseMode = 1;
    readyRead();
}

void QsnTCPclient::readedBlock(QByteArray *block)
{
    QDataStream stream(block, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    if (authorized) {emit snBUSOutput(containerFromStream(&stream), this);}
    else readAuth(&stream);
}
