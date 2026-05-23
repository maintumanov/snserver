#ifndef QSNWEBPAGEDAYTIME_H
#define QSNWEBPAGEDAYTIME_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebPageDaytime : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(bool state_daytime READ daytimeSatus)
    Q_PROPERTY(QString daytime READ widgetState)
public:
    QsnWebPageDaytime(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    void timeUpdate();

private:
    bool isManual;
    quint8 daytimeState;
    QTime beginDay;
    QTime endDay;
    QDate lastUpdateDate;
    QsnGlobalModules *mds;
    QString YDBname;

    QString textState();
    QString textTime();
    bool daytimeSatus();
    void toDay(bool local);
    void toNotDay(bool local);

    QTime getSunTime(QDate dt, double latitude, double longitude, double zenith, qint8 localOffset, bool sunset);
    void sendDB(bool state);
    QByteArray stateData();

};

#endif // QSNWEBPAGEDAYTIME_H
