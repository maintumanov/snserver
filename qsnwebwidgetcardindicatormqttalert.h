#ifndef QSNWEBWIDGETCCARDINDICATORMQTTALERT_H
#define QSNWEBWIDGETCCARDINDICATORMQTTALERT_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

/**
 * @brief MQTT-based alert detector for QsnWebWidgetCardIndicator.
 *
 * Listens to an MQTT topic, parses a JSON or plain-text value, and compares it
 * against configured "true"/"false" strings. When the value changes state,
 * it:
 * - Emits `onActive()` to refresh the parent indicator's activity timer,
 * - Emits `onMQTT()` with a synthetic SNIRF-like notification packet,
 * - Sends a system-wide alert via `BUSSERV_ALERT`.
 *
 * Supports 5 predefined alert types (Fire, Water leakage, etc.) mapped to
 * notification codes.
 */
class QsnWebWidgetCardIndicatorMQTTAlert : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardIndicatorMQTTAlert(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream, QString path = QString());

public slots:
    /**
     * @brief Handles incoming messages from the global system bus (snBUS).
     *
     * Listens for `BUSSERV_MGTT` with signal `BUSSERV_MGTT_reception_raw`.
     * If topic matches, delegates to `MsgProcess()`.
     *
     * @param container Message payload.
     * @param sender Ignored.
     */
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

signals:
    /**
     * @brief Emitted when valid MQTT data is received (even if unchanged).
     *
     * Payload is a synthetic 14-byte SNIRF notification packet.
     */
    void onMQTT(QByteArray data, QString source);

    /**
     * @brief Emitted whenever a message is received (used to refresh activity timer).
     */
    void onActive(QString source);

private:
    QsnGlobalModules *mds;              ///< Global modules context

    QString mqttTopic;                  ///< MQTT topic to subscribe
    QString mqttItemJson;               ///< Optional JSON field to extract
    QString mqttValueTrue;              ///< String representing "true" state (default: "1")
    QString mqttValueFalse;             ///< String representing "false" state (default: "0")
    //bool mqttOnlyChange;                ///< ⚠️ **Declared but never used!**

    quint8 mqttAlert;                   ///< Alert type index (0–4): Fire, Water, etc.
    quint8 lastState;                   ///< Last known state: 0=false, 1=true, 2=unknown
    quint16 mqttAddr;                   ///< Notification address (0–4095)

    /**
     * @brief Parses and evaluates incoming MQTT message.
     *
     * Extracts value (from JSON or full payload), compares to true/false strings,
     * and triggers state change logic.
     */
    void MsgProcess(QString msg);

    /**
     * @brief Broadcasts alert via global snBUS (`BUSSERV_ALERT`).
     */
    void AlertSend(const QSNNotification &nitification);

};

#endif // QSNWEBWIDGETCCARDINDICATORMQTTALERT_H
