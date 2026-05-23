#ifndef QSNWEBPAGEKEENETIC_H
#define QSNWEBPAGEKEENETIC_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QTcpSocket>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

#define DefaultAbsenceTimer 30
#define DefaultTimeBetweenRequests 45

class QsnWebPageKeenetic : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString keenetic READ widgetState)
public:
    QsnWebPageKeenetic(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageKeenetic();
    void endConfiguration();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    QString widgetState();
    char widgetNotifState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    void terminalRead();
    void socketError(QAbstractSocket::SocketError error);
    void socketConnected();


signals:
    void snBUSOutput(QSNContainer container, QObject *sender);

private:
    struct macItem {
        QString MAC;
        QString ip;
        QString name;
        QString user;
        QDateTime date;
    };

    QTcpSocket terminal;
    QString login;
    QString password;
    QString routerAddress;
    QString buffer;
    bool isAddMACmode;

    QsnGlobalModules *mds;
    QList<macItem> macs;

    int timerCount;
    int terminalState;
    QString clierror;
    quint64 timeAbsenceSec;
    int timeBetweenRequestsSec;
    bool isEnable;
    bool isWork;
    bool isPresence;

    QString usersToJSON();
    QString macToJSON();
    int countUsers();
    void check();
    void cleanLine(QByteArray *data);
    void fillMAC(QString *buff);
    void userMAC(macItem *item);
    void checkPresence();
    void cmdprocessed(QString cmd);
    bool checkMAC(QString mac);
    QString textState();
    void setAbsence();
    void setPresence();
    QByteArray stateData();
    void configUpdateBegin();
    void configUpdateEnd();

};

#endif // QSNWEBPAGEKEENETIC_H
