#ifndef QSNWEBAUTHORIZATION_H
#define QSNWEBAUTHORIZATION_H

#include <QObject>
#include <QDateTime>
#include <QTime>
#include <QVector>
#include <QDir>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QXmlStreamReader>
//#include "qsnwebaccount.h"
#include "qsnshapes.h"

//----- acces levels ----------

class QsnWebAuthorization : public QObject
{
    Q_OBJECT
public:
    struct _userId {
        qint64 cookeID;
        qint64 accountID;
        QDateTime expires;
    };

    enum accountMSGtype {alert = 0, alarm = 1, bell = 2, absence = 3, message = 4};

    struct tagItem {
        QString label;
        QString tag;
        QString tagState;
        QDateTime shtamp;
    };

    struct parameterItem {
       QString name;
       QString title;
       QString value;
       QString type;
    };

    struct permissionItem {
        QString name;
        QString title;
        bool value;
        int accessBit;
    };

    struct presenceItem {
        QString source;
        bool presence;
        QDateTime lastDetection;
        QDateTime endPresence;
        QDateTime endPresenceAlt;
    };

    struct accountItem {
        qint64 ID;
        QString name;
        QString login;
        QString password;
        QString email;
        bool alertNotification;
        bool alarmNotification;
        bool bellNotification;
        quint8 accessRights;
        QDateTime lastAccessDenied;
        QDateTime lastAccessAllowed;
        int countAccessDenied;
        QList<tagItem> tags;
        QList<parameterItem> parameters;
        QList<permissionItem> permissions;
        QList<presenceItem> presence;
    };

    QVector<accountItem> accounts;
    QVector<parameterItem> parameters;
    QVector<permissionItem> permissions;

    explicit QsnWebAuthorization(QObject *parent = Q_NULLPTR);
    virtual void fromStream(QDataStream *stream);

    int accountIndex(QString login, QString password);
    void authorizationAttempt(QString login, QString password, int *aindex, QString *error);
    QString getCooke(int accountIndex, bool isSession);
    int accountIndexFromCookeID(qint64 cookeID);
    qint64 accountUIDfromIndex(int index);
    int accountIndexfromAID(qint64 AID);
    bool isWEBAuthorization();
    bool isTCPAuthorization();

    void setAdminLogin(QString login);
    void setAdminPassword(QString pass);
    void setAdminEmail(QString email);
    void setEnableWEBAuthorization(bool enable);
    void setEnableTCPAuthorization(bool enable);
    void setEnableNotifyBlock(bool enable);
    void loadSetting();
    void saveSetting();
    int countAccounts();
    accountItem *atAccount(int index);
    bool isAdmin(int index);
    bool isUser(int index);
    void deleteAccount(qint64 AID);
    quint8 applyAccount(accountItem item);
    bool checkAccountMSGType(int index, int type);
    void addTag(int index, tagItem item);
    void setTagState(int accountIndex, QString tag, QString tagState, QString label);
    void accountChanged();
    bool checkAcces(int accountIndex, quint8 accessRights);
    bool checkAccesFromUID(qint64 uid,  quint8 accessRights);
    bool checkCookeID(qint64 cookeID);
    bool checkAID(qint64 AID);
    bool logout(qint64 cookeID);
    void addParametr(QString name, QString title, QString value, QString ptype = QString());
    QString parametrValue(accountItem *item, QString name);
    void addPermission(QString name, QString title, bool value);
    bool permissionValue(accountItem *item, QString name);
    void fillAccount(accountItem *item);
    void cleanAccounts();
    void setLogEnable(bool enable);
    QString getTypeAccountName(accountItem *item);
    bool getAccountPresenceState(accountItem *item, QString source);
    QStringList getAccountPresenceSources(accountItem *item);
    QDateTime getAccountPresenceLastDetect(accountItem *item, QString source);
    void getAccountPresenceLastConsolidated(accountItem *item, bool *presence, QDateTime *lastDetect, QString *sourceDetect, QDateTime *endAbsence);
    void setAccountPresenceItem(accountItem *item, QString source, bool state, quint64 presenceTime, quint64 altPresenceTime = 0);
    QString presenceLabel();
    int isPresence();
    void setAbsence();

signals:
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void isAllNotHome();
    void isHome();

public slots:
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void setPresenceSystem(QString source, qint64 presenceTime, qint64 altPresenceTime = 0);

private:
    int experienceDays;
    bool isLog;
    bool WEBAuthorization;
    bool TCPAuthorization;
    bool notifyBlock;
    int presenceState = -1;
    QDateTime presenceNextCheck;

    accountItem admin;

    QVector<_userId> usersID;

    _userId newUID(int accountIndex, bool isSession);
    void verificationAIDs();
    void checkUIDs();
    void saveUIDs();

    void loadAUid();
    QString getExpiresString(QDateTime expires);
    void checkLocationTag();
    void presenceItemsCheck();
    void presenceClearSystemSources();


};


#endif // QSNWEBAUTHORIZATION_H
