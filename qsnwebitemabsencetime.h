#ifndef QSNWEBITEMABSENCETIME_H
#define QSNWEBITEMABSENCETIME_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebItemAbsenceTime : public QsnWeb
{
    Q_OBJECT
public:   
    QsnWebItemAbsenceTime(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getContents(QStringList *contents, int accountIndex);
    void thisContents(QStringList *contents);
    void fromStream(QDataStream *stream);

signals:
    void absanceState(bool state);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = 0);
    void setAbsenceState(bool state);
    void timeUpdate();

private:
    bool stateAbsence;
    QsnGlobalModules *mds;
    QTime beginTime;
    QTime endTime;
    bool isMonday;
    bool isTuesday;
    bool isWednesday;
    bool isThursday;
    bool isFriday;
    bool isSaturday;
    bool isSunday;

    QString days();
    void sendMessage(QString text);

};

#endif // QSNWEBITEMABSENCETIME_H
