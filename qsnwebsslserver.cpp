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
//    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);

    // Qt 5.11.3 не поддерживает TlsV1_2OrLater, используем TlsV1_2
    // TLS 1.2 доступен начиная с Qt 5.4+
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);
#else
    // Для Qt 5.11.3 явно указываем TLS 1.2
    sslConfiguration.setProtocol(QSsl::TlsV1_2);
#endif

    // Настраиваем современные безопасные шифры
    // Приоритет: ECDHE с AES-GCM (Perfect Forward Secrecy), затем DHE, затем AES-CBC
    QList<QSslCipher> secureCiphers;
    secureCiphers << QSslCipher("ECDHE-RSA-AES256-GCM-SHA384")
                  << QSslCipher("ECDHE-RSA-AES128-GCM-SHA256")
                  << QSslCipher("DHE-RSA-AES256-GCM-SHA384")
                  << QSslCipher("DHE-RSA-AES128-GCM-SHA256")
                  << QSslCipher("AES256-GCM-SHA384")
                  << QSslCipher("AES128-GCM-SHA256")
                  << QSslCipher("ECDHE-RSA-AES256-SHA384")
                  << QSslCipher("ECDHE-RSA-AES128-SHA256")
                  << QSslCipher("AES256-SHA256")
                  << QSslCipher("AES128-SHA256");

    // Фильтруем только поддерживаемые шифры
    QList<QSslCipher> availableCiphers;
    for (const QSslCipher &cipher : secureCiphers) {
        if (!cipher.name().isEmpty()) {
            availableCiphers.append(cipher);
        }
    }

    // Если ни один шифр не подошёл, используем шифры по умолчанию
    if (availableCiphers.isEmpty()) {
        sslConfiguration.setCiphers(QSslConfiguration::defaultConfiguration().ciphers());
    } else {
        sslConfiguration.setCiphers(availableCiphers);
    }

    // Perfect Forward Secrecy (PFS) обеспечивается через шифры ECDHE/DHE
    // Примечание: setDiffieHellmanParameters и DefaultDhParams доступны только с Qt 5.12+
    // В Qt 5.11.3 шифры ECDHE из списка выше уже обеспечивают PFS автоматически

    // Добавляем цепочку сертификатов (системные CA сертификаты)
    sslConfiguration.setCaCertificates(QSslSocket::systemCaCertificates());

    // Логирование информации о конфигурации
//    qDebug() << "SSL/TLS Configuration:";
//    qDebug() << "  Protocol:" << sslConfiguration.protocol();
//    qDebug() << "  Ciphers count:" << sslConfiguration.ciphers().count();
//    if (!sslConfiguration.ciphers().isEmpty()) {
//        qDebug() << "  First cipher:" << sslConfiguration.ciphers().first().name();
//    }

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


