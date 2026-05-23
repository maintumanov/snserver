#ifndef QSNWEBPAGEPRESENCE_H
#define QSNWEBPAGEPRESENCE_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnwebitempresencetime.h"
#include "qsnbsshapes.h"

class QsnWebPagePresence : public QsnWeb
{
    Q_OBJECT
   // Q_PROPERTY(bool absence READ propertyAbsence WRITE setPropertyAbsence)
//    Q_PROPERTY(bool state_absence READ presenceState)
//    Q_PROPERTY(QString absence READ widgetState)
public:
    QsnWebPagePresence(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPagePresence();
    void getFunctions(QStringList *funct, int userID, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *content, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private:
    QsnGlobalModules *mds;
    QList<QsnWebItemPresenceTime *> presences;
    quint8 oldMinutes;
    qint64 presencetime;
    qint64 presencetimealt;
    QString userspToJSON();
    void presenceAddDB();
};

#endif // QSNWEBPAGEPRESENCE_H
