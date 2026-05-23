#ifndef QSNWEBPAGENIGHTTIME_H
#define QSNWEBPAGENIGHTTIME_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebPageNighttime : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(bool state_nighttime READ nighttimeStatus)
    Q_PROPERTY(QString nighttime READ widgetState)
public:
    QsnWebPageNighttime(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getItemJSON(QStringList *jsonItems);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    void timeUpdate();

private:
    bool isLog;
    bool isManualNight;
    bool isManualNotNight;
    quint8 stateNight;
    QTime beginNight;
    QTime endNight;
    QsnGlobalModules *mds;
    QString YDBname;

    bool nighttimeStatus();
    QString textState();
    QString textTime();
    void toNight(bool local, bool isManual = false);
    void toNotNight(bool local, bool isManual = false);
    void sendDB(QByteArray *data);
    QByteArray stateData();
    bool isBeginValid();
    bool isEndValid();

};

#endif // QSNWEBPAGENIGHTTIME_H
