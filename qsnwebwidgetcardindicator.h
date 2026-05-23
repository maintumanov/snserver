#ifndef QSNWEBWIDGETCARDINDICATOR_H
#define QSNWEBWIDGETCARDINDICATOR_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"
#include "qsnwebwidgetcardindicatormqtt.h"
#include "qsnwebwidgetcardindicatormqttstate.h"
#include "qsnwebwidgetcardindicatormqttalert.h"
#include "qsnwebwidgetcardindicatoroutput.h"
#include "qsnwebwidgetcardindicatorcomp.h"
#include "qsnwebwidgetcardindicatormsg.h"
#include "qsnwebwidgetcardindicatorsnirf.h"

/**
 * @brief A dynamic web-based indicator widget that displays real-time sensor or system data.
 *
 * This class integrates multiple input sources:
 * - MQTT topics (data, state, alerts)
 * - SNIRF protocol streams
 * - Local I/O signals
 * - Database entries (YBD)
 *
 * It supports:
 * - Visual state (active/inactive based on confidence timeout)
 * - Warning levels (via comparator module)
 * - Automatic persistence across restarts
 * - HTML/JSON rendering for web UI
 * - Optional periodic logging to database
 *
 * The widget is designed for use in monitoring dashboards where timely and reliable status
 * indication is critical (e.g., industrial IoT, smart home, telemetry).
 */
class QsnWebWidgetCardIndicator : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardIndicator(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void getItemJSON(QStringList *jsonItems);
    void fromStream(QDataStream *stream, QString path = QString());
    void receiveSignalIOIndex(int indexIO, QByteArray *data);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void setIndicatorValue(QByteArray data, QString source);
    void setActive(QString source);
    void onCompChange(quint8 newState);

private:
    // === Configuration & State ===
    bool filldb;                ///< Enable interval filling of the database (every minute while active)
    int isActive;               ///< Activity state in minutes: 0 = inactive, >0 = countdown, 100 = always active
    int activeMin;              ///< Internal seconds counter (0–59) for minute-based logic
    int fdbTimer;               ///< Seconds since last DB write (0–59)
    int iconNum;                ///< Icon override (0 = auto-select based on data type)
    int confidenceTime;         ///< Time of relevance in **minutes**; 0 = never expire
    int currentType;            ///< Last known data type (first byte of non-empty payload)
    int currentSubType;         ///< Subtype from type-1 SNIRF packets; -1 = unknown
    int defaultSubType;         ///< Forced subtype override (if > 0)
    quint8 emptysignal;         ///< Empty signal handling: 0=ignore, 1=timestamp packet, 2=increment counter


    // === Runtime Data ===
    QByteArray lastData;        ///< Last raw data (may be synthetic if emptysignal ≠ 0)
    QString labelString;        ///< Human-readable formatted value
    QString dbName;             ///< Associated YBD database name (for persistence and graph link)
    QString graphTitle;         ///< Reserved (not currently used)

    // === Dependencies ===
    QsnGlobalModules *mds;      ///< Global application modules (settings, DB, MQTT, I/O, etc.)

    // === Child Components ===
    QsnWebWidgetCardIndicatorMQTT *mqtt;        ///< Primary data source (MQTT)
    QsnWebWidgetCardIndicatorMQTTState *mqtts;  ///< State-triggered activation (MQTT)
    QsnWebWidgetCardIndicatorMQTTAlert *mqtta;  ///< Alert-triggered activation (MQTT)
    QsnWebWidgetCardIndicatorOutput *output;    ///< Output trigger (e.g., relay, LED)
    QsnWebWidgetCardIndicatorComp *comp;        ///< Comparator for warning levels
    QsnWebWidgetCardIndicatorMsg *msg;          ///< Message sender (SMS, push, etc.)
    QList<QsnWebWidgetCardIndicatorSNIRF *> snrfs; ///< List of SNIRF stream handlers


    void timeProcessing();
    void writeToMQTTPSN(QByteArray data);
    QByteArray addSubType(QByteArray data, bool issignal);
    QString warningState();
    QString getLink();

};

#endif // QSNWEBWIDGETCARDINDICATOR_H
