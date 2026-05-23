#ifndef QSNWEBSOCKET_H
#define QSNWEBSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFile>
#include <QVector>
#include "qsnwebcontainer.h"


class QsnWebContainer;

class QsnWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit QsnWebSocket(QTcpSocket *Socket, QsnWebContainer *container, QObject *parent = Q_NULLPTR);
    ~QsnWebSocket();
    QTcpSocket* clientSocket;
signals:
    void closeConnected(QsnWebSocket *socket);

public slots:
    void slotDisconnect();
    void checkData();
    void writeData(QByteArray data);

private slots:
    void slotReadClient();
    void slotDisconnected();

private:
    QsnWebContainer *webContainer;
    int inputState;
    int inputDataSize;
    QString xRequestedWith;
    QString URL;
    QByteArray inputStream;
    QByteArray inputHeader;
    QByteArray inputData;
    QByteArray outputContent;
    QMap<QString, QString> dataMap;
    QMap<QString, QString> cookiesMap;
    QTime wtimer;

    void readHeader();
    void readPostData();
    QString findHeaderOption(QString opionName);
    void findDataOption(QString *option, QString *value);
    bool isPOST();
    bool isGET();
    bool isJSON(QString *url);
    void getInputDataSize();
    void getXrequestedWith();
    void getURL();
    bool checkAuthorized();
    void parseXHRdata();
    void parseCookies();
    bool isUrlFileName(QString name);
    QString getContentTypeFromName(QString name);
    void parseHeader();
    void replyFile(QString fileName);
    void replyHTTP();
    void replyJSON();
    void requestXHR();
};

#endif // QSNWEBSOCKET_H
