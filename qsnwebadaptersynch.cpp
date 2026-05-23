#include "qsnwebadaptersynch.h"

QsnWebAdapterSynch::QsnWebAdapterSynch(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    widgetUrl = "/synch";
    setObjectName(tr("Synch server"));
    widgetIcon = "subicon-synch";
    widgetAccessRights = 2;
    Server = new QTcpServer(this);
    Address = QHostAddress::Any;
    Port = 8889;
    login = QString();
    password = QString();
    lastConnect = QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0));

    connect(Server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

QsnWebAdapterSynch::~QsnWebAdapterSynch()
{
    delete Server;
}

void QsnWebAdapterSynch::fromStream(QDataStream *stream)
{
    int Count;
    QString sig;
    *stream >> sig;
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> Count; //items
    // set options

    Port = optionsMap.value("PR", 8889).toUInt();
    login = optionsMap.value("LG", "administrator").toString();
    password = optionsMap.value("PS", "signalnet").toString();
}

void QsnWebAdapterSynch::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#state').text(data.state);");
    *fjson << QString(" $('#lcon').text(data.lcon);");
}

void QsnWebAdapterSynch::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"state\": \"%1\"").arg(widgetState());
            *returnItems << QString("\"lcon\": \"%1\"").arg(lastConnectText());
        }
    }
}

void QsnWebAdapterSynch::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormLabel(tr("State"), widgetState(), "state");
    *contents << QsnBsFormLabel(tr("Last connect"), lastConnectText(), "lcon");
    *contents << QsnBsFormEnd();
}

void QsnWebAdapterSynch::setDefault()
{
    Port = 8889;
}

quint16 QsnWebAdapterSynch::getPort()
{
    return Port;
}

void QsnWebAdapterSynch::adapterDisconnect()
{
    if (!Server->isListening()) return;
    for (int i = TCPClients.count() - 1; i >= 0; i --) TCPClients[i]->slotDisconnect();
    Server->close();
    log(WL_INFORMATION, tr("Closing port"), QLatin1String("Synch"), tr("Synch server"));
}

void QsnWebAdapterSynch::adapterConnect()
{
    if (Server->isListening()) return;
    if (Server->isListening()) Server->close();

    if (!Server->listen(QHostAddress::Any, Port)) {
        log(WL_CAUTION, tr("No open port %1").arg(Port), QLatin1String("Synch"), tr("Synch server"));
        return;
    }
    log(WL_INFORMATION, tr("Open port %1").arg(Port), QLatin1String("Synch"), tr("Synch server"));
}

QString QsnWebAdapterSynch::widgetState()
{
    if (!Server->errorString().isEmpty()) return Server->errorString();
    if (Server->isListening()) return tr("active");
    else return tr("not active");
}

void QsnWebAdapterSynch::onLog(quint8 level, QString msg)
{
    log(level + WL_DEBUG, msg, QLatin1String("Synch"), tr("Synch server"));
}

void QsnWebAdapterSynch::newConnection()
{
    QsnPSocket *tcpSocket = new QsnPSocket(Server->nextPendingConnection(),  this);
    TCPClients.append(tcpSocket);
    tcpSocket->setLoginPassword(login, password);
    lastConnect = QDateTime::currentDateTime();
    connect(tcpSocket, SIGNAL(closeConnected(QsnPSocket*)), this, SLOT(closeConnection(QsnPSocket*)));
}

void QsnWebAdapterSynch::closeConnection(QsnPSocket *socket)
{
    for (int i = TCPClients.count() - 1; i >= 0; i --) if (TCPClients[i] == socket) TCPClients.removeAt(i);
}

QString QsnWebAdapterSynch::lastConnectText()
{
    if(lastConnect == QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0))) return tr("There were no connections");
    return lastConnect.toString("dd.MM.yyyy hh:mm:ss");
}

//==================================================================================================
QsnPSocket::QsnPSocket(QTcpSocket *Socket, QObject *)
{
    setObjectName(QString(tr("TCP Server")));
    clientSocket = Socket;
    caseMode = 1;
    login = QString();
    password = QString();
    authState = false;
    connectTime = QDateTime::currentDateTime();
    connect(Socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(Socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
}

QsnPSocket::~QsnPSocket()
{
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
}

QHostAddress QsnPSocket::clientIPAddress()
{
    return clientSocket->peerAddress();
}

QString QsnPSocket::clientLastError()
{
    return lastError;
}

void QsnPSocket::setLoginPassword(QString l, QString p)
{
    login = l;
    password = p;
}

void QsnPSocket::slotDisconnect()
{
    clientSocket->close();
}

void QsnPSocket::readyRead()
{
    switch (caseMode)
    {
    case 1:state_1(); break;
    case 2:state_2(); break;
    case 3:readSize(); break;
    case 4:readData(); break;
    }
}

void QsnPSocket::slotDisconnected()
{
    emit closeConnected(this);
    this->deleteLater();
}

void QsnPSocket::writeSize(quint32 size)
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

void QsnPSocket::readSize()
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

void QsnPSocket::readData()
{
    if (clientSocket->bytesAvailable() < static_cast<qint64>(blockSize)) return;
    QByteArray block;
    caseMode = 1;
    block = clientSocket->read(static_cast<qint64>(blockSize));
    QDataStream stream(&block, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_7);
    quint8 command;
    stream >> command;
    if (!authState && command != 100) {
        accessAnswer();
        return;
    }
    switch (command) {
    case 10: requestPatternsList(); break;
    case 20: requestUpload(&stream); break;
    case 30: requestDownload(&stream); break;
    case 100: requestAuth(&stream); break;
    }
    readyRead();
}

void QsnPSocket::sendData(QByteArray *block)
{
    writeSize(static_cast<quint32>(block->size()));
    clientSocket->write(*block);
    clientSocket->flush();
}

void QsnPSocket::state_1()
{
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    clientSocket->read(&p, 1);
    if (p == char(115)) caseMode ++;
    readyRead();
}

void QsnPSocket::state_2()
{
    if (clientSocket->bytesAvailable() < 1) return;
    char p;
    clientSocket->read(&p, 1);
    if (p == char(110)) caseMode ++;
    else caseMode = 1;
    readyRead();
}

void QsnPSocket::requestPatternsList()
{
    QString items;
    QString item;
    QDir Dir(QSNHomeSubPath("server", "patterns"));
    QStringList Filtres;
    Filtres << QLatin1String("*.xml");
    QStringList FileList;
    FileList = Dir.entryList(FileList,QDir::Files);
    for (int i = 0; i < FileList.count(); i++) {
        requestPatternsListRead(Dir.absoluteFilePath(FileList[i]), &item);
        if (!item.isEmpty()) {
            if (!items.isEmpty()) items.append(";");
            items.append(item);
        }
    }

    QByteArray ba;
    QDataStream stream(&ba, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << (quint8)11;
    stream << items;
    sendData(&ba);
}

void QsnPSocket::requestPatternsListRead(QString path, QString *item)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QXmlStreamReader xmlDoc(&file);
    QXmlStreamAttributes attrib;

    while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
        if (xmlDoc.readNext() == QXmlStreamReader::StartElement && xmlDoc.name() == QLatin1String("PatternContainer")) {
            while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
                if (xmlDoc.readNext() == QXmlStreamReader::StartElement) {
                    if (xmlDoc.name() == QLatin1String("general")) {
                        attrib = xmlDoc.attributes();
                        if (!attrib.value("DeviceVersion").toString().isEmpty())
                            *item = QString("%1:%2:%3").arg(attrib.value("DeviceIndex").toUInt())
                                .arg(attrib.value("Name").toString(), attrib.value("DeviceVersion").toString());
                        else *item = QString();
                    }
                }
            }
        }
    }
    file.close();
}

void QsnPSocket::requestDownload(QDataStream *stream) //client to server
{
    quint16 devindex;
    QByteArray bfile;
    *stream >> devindex;
    *stream >> bfile;

    QByteArray ba;
    QDataStream streamf(&ba, QIODevice::ReadWrite);
    streamf.setVersion(QDataStream::Qt_4_7);

    QDir Dir(QSNHomeSubPath("server", "patterns"));
    QFile file(Dir.absoluteFilePath(QLatin1String("DevicePattern_") + QString::number(devindex)+QLatin1String(".xml")));
    if (!file.open(QIODevice::WriteOnly)) {
        streamf << (quint8)32;
        sendData(&ba);
        return;
    }

    file.write(bfile);
    file.close();

    streamf << (quint8)31;
    sendData(&ba);
}

void QsnPSocket::requestUpload(QDataStream *stream) //server to client
{
    quint16 devindex;
    *stream >> devindex;
    QByteArray ba;
    QDataStream streamf(&ba, QIODevice::ReadWrite);
    streamf.setVersion(QDataStream::Qt_4_7);

    QDir Dir(QSNHomeSubPath("server", "patterns"));
    QFile file(Dir.absoluteFilePath(QLatin1String("DevicePattern_") + QString::number(devindex)+QLatin1String(".xml")));
    if (!file.open(QIODevice::ReadOnly)) {
        streamf << (quint8)22;
        sendData(&ba);
        return;
    }

    QByteArray bf = file.readAll();
    file.close();

    streamf << (quint8)21;
    streamf << (quint16)devindex;
    streamf << bf;
    sendData(&ba);
}

void QsnPSocket::requestAuth(QDataStream *stream)
{
    QString l;
    QString p;
    *stream >> l;
    *stream >> p;
    if (login == l && password == p) authState = true;
    if (login.isEmpty()) authState = true;
    accessAnswer();
}

void QsnPSocket::accessAnswer()
{
    QByteArray ba;
    QDataStream streamf(&ba, QIODevice::ReadWrite);
    streamf.setVersion(QDataStream::Qt_4_7);

    if (authState) streamf << (quint8)101;
    else streamf << (quint8)102;
    sendData(&ba);
    if (!authState) clientSocket->close();
}

