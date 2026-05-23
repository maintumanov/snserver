#ifndef QsnWebSocketSSL_H
#define QsnWebSocketSSL_H

#include <QObject>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFile>
#include <QVector>
#include "qsnwebcontainer.h"
//#include "qsnwebsocketutil.h"


class QsnWebContainer;

class QsnWebSocketSSL : public QSslSocket
{
    Q_OBJECT
public:
    explicit QsnWebSocketSSL(QsnWebContainer *container, QObject *parent = Q_NULLPTR);

signals:
    void closeConnected(QsnWebSocketSSL *socket);


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
//    steady_clock::time_point start_time;
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

#endif // QsnWebSocketSSL_H
