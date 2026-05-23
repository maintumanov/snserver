#include "qsnwebpagelogconnector.h"

QsnWebPageLogConnector::QsnWebPageLogConnector(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Log connector"));
    widgetUrl = "/logconnector";
    widgetIcon = "subicon-logconnector";

    serverSocket = new QUdpSocket(this);
    serverSocket->bind(QHostAddress::Any, 29000, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);


    QObject::connect(serverSocket, SIGNAL(readyRead()),this, SLOT(readyRead()));

    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));

    nameDevice = "Signalnet";

    loger_level = static_cast<quint8>(mds->settings->value(QString("LogConnectorLevel"), false).toUInt());
    loger_address.setAddress(mds->settings->value(QString("LogConnectorAddress"), "127.0.0.1").toString());
}

QsnWebPageLogConnector::~QsnWebPageLogConnector()
{

}

void QsnWebPageLogConnector::endConfiguration()
{

}

void QsnWebPageLogConnector::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#level').text(data.level);");
}

void QsnWebPageLogConnector::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"level\": \"%1\"").arg(widgetState());
        }
    }
}

void QsnWebPageLogConnector::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormLabel(tr("Log level"), widgetState(), "level");
    *contents << QsnBsFormEnd();
}

void QsnWebPageLogConnector::fromStream(QDataStream *stream)
{
    int Count;
    QString sig;
    *stream >> sig;
    getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> Count; //items
    // set options
}

void QsnWebPageLogConnector::urlChanged(int )
{

}

QString QsnWebPageLogConnector::widgetState()
{
    if (!loger_level) return tr("disabled");
    QStringList list = getChannelsNames().split(';', QString::SkipEmptyParts);
    QString ret;
    for (int i = 0; i < 6; i++)
        if (loger_level & (static_cast<quint8>(1) << i)) {
            if (!ret.isEmpty()) ret += ", ";
            ret += list.at(i);
     }
    return ret;
}

char QsnWebPageLogConnector::widgetNotifState()
{
    //if (!serverSocket->isOpen()) return 'c';
    return 'n';
}

void QsnWebPageLogConnector::snBUSInput(QSNContainer container, QObject *sender)
{
    if (container.role == QSNContainer::information && loger_level & BUSSERV_LOG_CATEGORY_devices) {
        if (!(loger_level & static_cast<quint8>(container.Address))) return;
        quint8 wlevel = container.Command;
        if (wlevel > 4) wlevel = wlevel - 5;
        if (container.Sender) {
            QString dlabel = QString::fromUtf8(container.Data);
            if (dlabel.isEmpty()) dlabel = tr("Unknown device");
            sendToLog(QSNWarningLevelToText(wlevel) + " " + dlabel + " - " + container.info);
        } else {
            QStringList options = QString::fromUtf8(container.Data).split("/");
            if (options.count() > 1) sendToLog(QSNWarningLevelToText(wlevel) + " " + options[1] + ": " + container.info);
            else sendToLog(QSNWarningLevelToText(wlevel) + " " + (sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN")) + " - " + container.info);
        }
    }

    if (loger_level & BUSSERV_LOG_CATEGORY_network && (container.role == QSNContainer::signal || container.role == QSNContainer::message)) {
        sendToLog(QString("%1[3][%2] %3")
                  .arg(QSNWarningLevelToText(2),
                       sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN"),
                       QSNContainerToLogText(container, 2)));
    }

    if (container.role == QSNContainer::service) {
        if ((loger_level & BUSSERV_LOG_CATEGORY_core) && (container.Sender & BUSSERV_LOG_CATEGORY_core)) {
            sendToLog(QString("%1[0][%2] %3")
                      .arg(QSNWarningLevelToText(container.Signal),
                           sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN"),
                           QSNContainerToLogText(container, 2)));
        }

        if (loger_level & BUSSERV_LOG_CATEGORY_adapter && (container.Sender & BUSSERV_LOG_CATEGORY_adapter)) {
            sendToLog(QString("%1[1][%2] %3")
                      .arg(QSNWarningLevelToText(container.Signal),
                           sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN"),
                           QSNContainerToLogText(container, 2)));
        }

        if (loger_level & BUSSERV_LOG_CATEGORY_module && (container.Sender & BUSSERV_LOG_CATEGORY_module)) {
            sendToLog(QString("%1[2][%2] %3")
                      .arg(QSNWarningLevelToText(container.Signal),
                           sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN"),
                           QSNContainerToLogText(container, 2)));
        }


    }

    if (loger_level & BUSSERV_LOG_CATEGORY_bus) {
        if (container.Command != BUSSERV_TIME_SEC_EVENT || container.role != QSNContainer::service)
            sendToLog(QString("%1[4][%2] %3")
                      .arg(QSNWarningLevelToText(sender != Q_NULLPTR?2:1),
                           sender != Q_NULLPTR?sender->objectName():tr("UNKNOWN"),
                           QSNContainerToLogText(container, 2)));
    }
}

void QsnWebPageLogConnector::readyRead()
{
    while (serverSocket->isValid() && serverSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(serverSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        serverSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        loger_address = sender;
        if (static_cast<quint8>(datagram.at(0)) < 128) {
            loger_level = static_cast<quint8>(datagram[0]) & 0b01111111;
            mds->settings->setValue(QString("LogConnectorLevel"), loger_level);
            mds->settings->setValue(QString("LogConnectorAddress"), loger_address.toString());

        }
        answerNetworkMode();
    }
}

void QsnWebPageLogConnector::sendToLog(QString text)
{
    QByteArray datagram;
    QDataStream stream(&datagram, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << text;
    serverSocket->writeDatagram(datagram, loger_address, 28999);
    serverSocket->flush();
}

void QsnWebPageLogConnector::answerNetworkMode()
{
    QByteArray datagram;
    QDataStream stream(&datagram, QIODevice::ReadWrite);
    stream.setVersion(QDataStream::Qt_4_7);
    stream << QString("!!loglevel_%1_%2[%3]")
              .arg(loger_level)
              .arg("snServer", getChannelsNames());
    serverSocket->writeDatagram(datagram, loger_address, 28999);
    serverSocket->flush();
}

QString QsnWebPageLogConnector::msgName(quint8 type)
{
    switch (type)
    {
    case 0: return tr("get info");
    case 1: return tr("new device");
    case 2: return tr("get memory");
    case 3: return tr("read memory");
    case 4: return tr("set memory");
    case 5: return tr("write memory");
    case 6: return tr("set address");
    case 7: return tr("answer address");
    case 9: return tr("answer parametr");
    }
    return tr("not known");
}

QString QsnWebPageLogConnector::getChannelsNames()
{
    return QString("%1;%2;%3;%4;%5;%6")
            .arg(tr("Core"),
                 tr("Adapter"),
                 tr("Modules"),
                 tr("Network"),
                 tr("BUS"),
                 tr("DEVICES"));
}
