#ifndef QSNWEBPAGENOTIFICATION_H
#define QSNWEBPAGENOTIFICATION_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnglobalmodules.h"
#include "qsnimageconvert.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "qsnbsshapes.h"

class QsnWebPageNotification : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageNotification(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageNotification();
    void fromStream(QDataStream *stream);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void urlChanged(int accountIndex);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void onFinished(QNetworkReply* reply);

private slots:
    void sendmessage(QString text, bool filter = true);



private:
    // NOTIF QUEUE
    struct queueItem {
        QString name;
        QString note;
        QString phone;
        QString text;
        quint16 type;
        quint16 user;
    };

    bool isLog;
    bool isUsePhone;
    int rMethod; //0 - GET, 1 - POST
    int timeout;
    QString widgetNID;
    QString rURL;
    QString rPOSTdata;
    QString lastError;

    QString notifState;
    QString notifStateLabel;
    QString notifStateWidget;

    QList<queueItem> notifqueue;
    QsnGlobalModules *mds;

    QNetworkAccessManager manager;

    void checkTime();
    void checkQueue();
    void sendTest();
};

#endif // QSNWEBPAGENOTIFICATION_H
