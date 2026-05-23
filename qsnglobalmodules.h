#ifndef QSNGLOBALMODULES
#define QSNGLOBALMODULES
#include "qsnwebauthorization.h"
#include "qsninterface.h"
#include "qsniotable.h"
#include "qsndb.h"
#include "qsnlocations.h"
#include <QSettings>

class QsnIOTable;

struct QsnGlobalModules {
    QsnWebAuthorization *auth;
    QsnInterface *interface;
    QsnIOTable *io;
    QsnDB *db;
    QSettings *settings;
    QsnLocations *locations;

    int iconTheme;
    int interfaceTheme;
    int timeOut;
    bool supportSSL;
    double latitude;
    double longitude;
    qint8 timezone;
    bool supportMQTT;
    bool isEnableLog;

    //ALERT
    int alert_status;
    //ALARM
    int alarm_status;

    //STATISTIC
    int S_PTWR_MIN;
    int S_PTWR_MAX;
    quint32 S_PTWR_COUNT;
    QDateTime S_TM_BEGIN;

};
#endif // QSNGLOBALMODULES

