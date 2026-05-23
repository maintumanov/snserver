#ifndef QSNWEBPAGESECURITYALARM_H
#define QSNWEBPAGESECURITYALARM_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

struct intrusionAlarm {
    QDateTime time;
    QSNNotification alarm;
    QString eventLocale;
};

class QsnWebPageSecurityAlarm : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString security READ widgetState)
public:
    QsnWebPageSecurityAlarm(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);

    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void addWidget(QsnWeb *widget);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void fromStream(QDataStream *stream);
    QString widgetState();
    char widgetNotifState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:


private:
    QList<QsnWeb *> widgets;
    QList<intrusionAlarm> events;
    QsnGlobalModules *mds;
    QString lastLocation;
    QDateTime lastChange;

    bool isLog;
    bool isSecretly;
    bool isProtection;
    bool isPenetration;
    bool isItActive;
    quint8 Mask;
    int cancelTime;

    QString textState();
    QString bool2text(bool state);
    QString bool2visable(bool visable);
    QString iconState();
    QString iconStateNotif(QSNNotification *alarm);
    QString textStateNotif(QSNNotification *alarm);
    QString cancelVisable();

    bool stateSet(QSNNotification alarm, bool loged = true);
    void stateUpdate();
    void setCancel();
    void setArming();
    void setArmingSecretly();
    void setDisarming();
    void setPenetration();
    void addEvent(QSNNotification alarm);
    void sendMessage(QString msg);
    QString eventsToJSON();
    QByteArray stateArmedData();
    QByteArray stateData();

};

#endif // QSNWEBPAGESECURITYALARM_H
