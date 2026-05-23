#ifndef QSNWEBPAGESNIRF_H
#define QSNWEBPAGESNIRF_H

#include <QObject>
#include <QDebug>
#include <QColor>
#include "qsnweb.h"
#include "qsnbsshapes.h"
#include "qsnwebpagesnirfmqtt.h"

/**
 * @brief Web interface page for managing SNIRF-compatible devices and MQTT-tracked entities.
 *
 * This class handles:
 * - Registration and display of SNIRF (custom RF) devices
 * - Integration with MQTT-based location/status sources (e.g., OwnTracks)
 * - Device renaming, logging, timeout/battery monitoring
 * - Dynamic HTML/JS generation for web UI
*/

class QsnWebPageSNIRF : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString owntracks READ widgetState)
public:
    /**
     * @brief Constructor.
     * @param iID Unique widget ID.
     * @param modules Global application modules (settings, I/O, DB, etc.).
     * @param parent QObject parent.
     */
    QsnWebPageSNIRF(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageSNIRF();

    // Lifecycle & configuration
    void endConfiguration();

    // Web UI generation hooks
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);

    // Handle user actions (rename, remove)
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);

    // Stream serialization (load/save state)
    void fromStream(QDataStream *stream);

    // Handle incoming raw device data via I/O index
    void receiveSignalIOIndex(int indexIO, QByteArray *data);

    // React to URL changes (e.g., ?log=2 to view logs)
    void urlChanged(int accountIndex);

    // Public accessors for widget status
    QString widgetState();      ///< Human-readable summary (e.g., "disch.:1, off.:2")
    char widgetNotifState();     ///< Notification severity: 'c'=critical, 'w'=warning, 'n'=none

public slots:
    // Handle internal bus messages (time ticks, config updates)
    void snBUSInput(QSNContainer container, QObject *sender = 0);

    // Add MQTT-reported device (e.g., from OwnTracks)
    void addDeviceMQTT(QString name, QString value, quint8 batt, quint16 timeout, QString source);

private slots:
     // Rename a device by address
    void renameDevice(int addr, QString newName);

signals:
    // Emit processed SNIRF data onto internal bus
    void snBUSOutput(QSNContainer container, QObject *sender);

private:
    /**
     * @brief Log entry for a device event.
     */
    struct dev_log {
        QDateTime timestamp;  ///< When the event occurred
        QString data;         ///< Human-readable payload (e.g., "[5] TEMP 23.4°C")
        quint8 parametr;      ///< Original parameter ID from device
    };

    /**
     * @brief Represents a tracked device (SNIRF hardware or MQTT virtual).
     */
    struct device {
        bool isTimeout;           ///< True if last update > timeout threshold
        bool isSNIRF;             ///< True = real SNIRF device; false = MQTT-emulated
        bool isLowBattery;        ///< Hysteresis state for battery alerts
        quint8 address;           ///< Unique ID for SNIRF devices (0 for MQTT)
        quint8 parametr;          ///< Last reported parameter type
        quint8 batt;              ///< Battery level (%)
        quint8 retry;             /// 2 бита (0-3)
        quint8 errorlog[24];      ///< Error receive log 24 item (24 hour)
        quint8 countlog[24];      ///< Error receive log 24 item (24 hour)
        QDateTime battChangeDate; ///< When battery was last significantly changed
        QDateTime lastTimeUpdate;           ///< Timestamp of last received data
        QString lastData;         ///< Last human-readable payload
        QList<dev_log> log;       ///< Circular buffer (max 50 entries)
        QString name;             ///< Base name ("SNIRF" or MQTT topic)
        QString customName;       ///< User-defined alias (saved in settings)
        quint16 timeout;          ///< Inactivity timeout (seconds); used only for MQTT
        quint32 minInterval;   ///< Минимальный интервал между heartbeat (в секундах)
        quint32 maxInterval;   ///< Максимальный интервал между heartbeat (в секундах)
        qint64 estimatedRemainingHours;///< Прогноз остаточного времени в часах
        quint8 lastBattPercent;   // последний зафиксированный процент заряда
        QDateTime lastBattTime;   // время последнего замера процента

    };

    static constexpr quint8 LOW_BATT_THRESHOLD = 10;
    static constexpr quint8 RECOVERY_THRESHOLD = 30;

    // State flags
    bool isLog;                   ///< Whether current view is a log detail view

    // Dependencies
    QsnGlobalModules *mds;        ///< Access to settings, I/O, DB, messaging

    // Device registries
    QList<device> devices;                    ///< All known devices
    QList<QsnWebPageSNIRFMQTT *> devicesMQTT; ///< MQTT listeners (sub-components)

    // Configuration
    int SNIRF_TimeOut_global;     ///< Default timeout for SNIRF devices (in seconds)
    int battLowCount = 0;         ///< Count of devices with batt <= 10%
    int timeoutCount = 0;         ///< Count of offline devices
    int lastMinute = 0;            ///< Minutes since last full check (for periodic scan)
    int lastHour = 0;            ///< Minutes since last full check (for periodic scan)
    int itemLog = -1;             ///< Index of device whose log is currently shown

    // Utility methods
    QString devicesToJSON();                          ///< Serialize device list to JSON
    QString logToJSON(int dev);                       ///< Serialize one device's log to JSON
    QString deviceItemClassState(bool offline, quint8 batt); ///< CSS class for row styling

    // Internal processing
    void busPuplic(QSNSNIR snir);                 ///< Broadcast parsed SNIRF data on bus
    void addDeviceSNIRF(QByteArray *data);        ///< Parse raw SNIRF packet and register/update
    void addDevice(device dev);                   ///< Insert or update device in registry
    void removeDevice(int index);                 ///< Remove device + clean up settings
    void checkDevices();                          ///< Recompute timeout/batt counters
    void addDeviceLog(device *dev);               ///< Append to device log (capped at 50)
    void sendMessage(quint8 importance, QString message); ///< Send system notification
    void timeProcessing();                        ///< Called every second; triggers periodic checks

    // Helpers
    QString getDeviceCustomName(int addr, QString defaultName); ///< Load custom name from settings
    void configUpdateBegin();                     ///< Save current state before config reload
    void configUpdateEnd();                       ///< Restore state after config reload
    QString getDevName(device *dev);              ///< Return custom or default device name
    QString formatBatteryLife(const QDateTime &since); ///< Format "X hrs/days ago" for UI
    QString getSNIRFSignalStatus(const device &dev) const;
    QString buildIndicator(const QString &icon, const QString &label, const QString &suffix, const QString &id);
    int getSignalQualityCategory(const quint8 errorlog[24]) const; // возвращает 0=хороший, 1=плохой, 2=очень плохой
    int getSignalQualityPercent(const quint8 errorlog[24]) const;      // процент часов с retry==0
    QString getSignalGraph(const quint8 errorlog[]) const;
    QString getSignalGraphHTML(const device &dev) const;
    void updateTimes(device *dev);  ///< Обновить статистику интервалов heartbeat
    QString formatInterval(quint32 seconds) const;  ///< Форматировать интервал в читаемый вид
    QString formatRemainingTime(qint64 hours) const;
    void updateBatteryPrediction(device &dev, quint8 newBatt);
};

#endif // QSNWEBPAGESNIRF_H
