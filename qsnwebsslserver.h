#ifndef QSNWEBSSLSERVER_H
#define QSNWEBSSLSERVER_H

#include <QObject>
#include <QSslSocket>
#include <QCoreApplication>
#include <QTcpServer>
#include <QFile>
#include <QDir>
#include <QSslConfiguration>
#include <QSslKey>
#include "qsnwebsocketssl.h"
#include "qsnwebcontainer.h"

class QsnWebContainer;
class QsnWebSocketSSL;

class QsnWebSSLServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit QsnWebSSLServer(QsnWebContainer *container, QObject *parent = Q_NULLPTR);
    ~QsnWebSSLServer();
    void incomingConnection(qintptr socketDescriptor);

signals:

protected:

public slots:


private slots:
    void socketSslErrors(const QList<QSslError> &errors);
    void socketDisconnect();

private:
    QsnWebContainer *wcontainer;
    QList<QsnWebSocketSSL*> clients;
    QSslConfiguration sslConfiguration;

};

#endif // QSNWEBSSLSERVER_H
