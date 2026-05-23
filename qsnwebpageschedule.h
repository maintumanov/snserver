#ifndef QSNWEBPAGESCHEDULE_H
#define QSNWEBPAGESCHEDULE_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "qsnweb.h"
#include "qsnwebitemscheduleaction.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

struct QsnScheduleItem {
    QString actions;
    QString name;
    bool isdate;
    quint8 daysweek;
    QTime time;
    QDate date;
    QDateTime lastRun;
};

class QsnWebPageSchedule : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageSchedule(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageSchedule();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void urlChanged(int index);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void endConfiguration();

private slots:
    void timeUpdate();

private:
    QList<QsnScheduleItem> events;
    QsnGlobalModules *mds;
    QTime nextTime;

    //html parametrs
    bool isEventsEdit;
    int itemEventEdit;

    void saveSetting();
    void loadSetting();
    QString getActionTitle(int index);
    QTime clearSec(QTime time);
    QString actionsIDsToNames(QString actions);
    QString actionsNamesToIDs(QString names);
    bool isContains(QString actions, int index);
    void actionsBegin(QsnScheduleItem *action);
    QString cardLabel(QsnScheduleItem *action);
    QString schedulesToJSON();


};

#endif // QSNWEBPAGESCHEDULE_H
