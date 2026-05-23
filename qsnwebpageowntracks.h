#ifndef QSNWEBPAGEOWNTRACKS_H
#define QSNWEBPAGEOWNTRACKS_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

class QsnWebPageOwnTracks : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString owntracks READ widgetState)
public:
    QsnWebPageOwnTracks(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageOwnTracks();
    void endConfiguration();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void actionJSON(QByteArray *INjson, QByteArray *outJSON);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    QString widgetState();

public slots:
    //----BM
        void snBUSInput(QSNContainer container, QObject *sender = 0);

private slots:

signals:
    void snBUSOutput(QSNContainer container, QObject *sender);

private:
    struct location {
        QString tid;
        QString lat;
        QString lon;
        QString tst;
        QString conn;
        QString batt;
        QString label;
        double distance;
        double distance_last;
        quint8 distance_area;
        qint64 fsize;
    };

    struct waypoint {
        QString tid;
        QString desc;
        QString lat;
        QString lon;
        QString tst;
        QString rad;
    };

    bool isLog;
    bool enableSignal;
    double area_home;
    double area_approach;
    double cons_dist_last;
    QString wState;
    QsnGlobalModules *mds;
    QList<location> locations;
    QList<waypoint> waypoints;
    QString jsonCards();
    QString usersToJSON();
    void parseJSON(QString ljson);
    void locationAdd(location l);
    void waypointAdd(waypoint w);
    int countUsers();
    location *getLocation(QString tid);
    QString connectType(QString conn);
    double getDistance(double lat1, double lon1, double lat2, double lon2);
    void checkDistance(location *l);
    void writeTrackPosition(location *l);
    void loadLastTrackPosition(QString tid);
    QString createGPXFile(QString tid);
    QString areaLabel(quint8 index);
    void checkUsersArea();
    void toRAbsent();
    void toRPresent();
    void toRApproaching();
    bool isEnableRec(QString tid);
    QByteArray stateData(bool state);
    void configUpdateBegin();
    void configUpdateEnd();

};

#endif // QSNWEBPAGEOWNTRACKS_H
