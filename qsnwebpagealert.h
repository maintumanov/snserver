#ifndef QSNWEBPAGEALERT_H
#define QSNWEBPAGEALERT_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

struct recentEvent {
    QDateTime time;
    QSNNotification alarm;
    QString eventLocale;
};

class QsnWebPageAlert : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageAlert(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
//    void getChartFunctions(QStringList *functions, QStringList *depending, QStringList *argument, int accountIndex);
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
//    void getDialogs(QStringList *dialogs, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void addWidget(QsnWeb *widget);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void fromStream(QDataStream *stream);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender =Q_NULLPTR);

protected:

private:
    QList<QsnWeb *> widgets;
    QList<recentEvent> events;
    QString lastLocation;
    QDateTime lastChange;
    quint8 lastEventType;
    QsnGlobalModules *mds;

    QString iconStateNotif(QSNNotification *event);
    QString textStateNotif(QSNNotification *event);

    bool stateSet(QSNNotification alert, bool loged = true);
    void statusSet(quint8 status);
    void setCancel();
    void sendToOutput(QSNNotification alert);
    void setClear();
    void sendMessage(QString message);
    void addEvent(QSNNotification event);
    QString eventsToJSON();

};

#endif // QSNWEBPAGEALERT_H
