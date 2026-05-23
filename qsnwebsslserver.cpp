#include "qsnwebsslserver.h"

QsnWebSSLServer::QsnWebSSLServer(QsnWebContainer *container, QObject *parent) : QTcpServer(parent)
{
    wcontainer = container;
   // ":/defaultconf.wac"
    QFile keyFile(QSNHomeSubPath("server", "certificate").absoluteFilePath(QLatin1String("server.key")));
    QFile pemFile(QSNHomeSubPath("server", "certificate").absoluteFilePath(QLatin1String("server.pem")));
    if (!keyFile.exists()) keyFile.setFileName(QLatin1String(":/server.key"));
    if (!pemFile.exists()) pemFile.setFileName(QLatin1String(":/server.pem"));

    keyFile.open(QIODevice::ReadOnly);
    pemFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&pemFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    keyFile.close();
    pemFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);

}

QsnWebSSLServer::~QsnWebSSLServer()
{

}

void QsnWebSSLServer::incomingConnection(qintptr socketDescriptor)
{
    QsnWebSocketSSL *serverSocket = new QsnWebSocketSSL(wcontainer, this);
    if(serverSocket->setSocketDescriptor(socketDescriptor)) {
        clients << serverSocket;
        serverSocket->setSslConfiguration(sslConfiguration);
        connect(serverSocket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(socketSslErrors(const QList<QSslError> &)));
        connect(serverSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnect()));
        serverSocket->startServerEncryption();
    } else delete serverSocket;
}

void QsnWebSSLServer::socketSslErrors(const QList<QSslError> &)
{
    emit wcontainer->snBUSOutput(QSNLogToContainer(
                                  BUSSERV_LOG_LEVEL_information,
                                     QString(tr("WEB SSL ERROR %1 ")).arg(qobject_cast<QSslSocket*>(sender())->errorString()),
                                  BUSSERV_LOG_CATEGORY_core,
                                  QString(),
                                  true), this);
}

void QsnWebSSLServer::socketDisconnect()
{
    for (int i = clients.count() - 1; i >= 0; i --)
        if (clients[i] == qobject_cast<QsnWebSocketSSL*>(sender())) {
            clients[i]->deleteLater();
            clients.removeAt(i);
        }
}


