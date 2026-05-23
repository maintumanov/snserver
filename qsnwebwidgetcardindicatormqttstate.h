#ifndef QSNWEBWIDGETCCARDINDICATORMQTTSTATE_H
#define QSNWEBWIDGETCCARDINDICATORMQTTSTATE_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

/**
 * @brief MQTT-based state detector for QsnWebWidgetCardIndicator.
 *
 * Listens to an MQTT topic, parses a JSON or plain-text value, and maps it
 * to a binary state (`true`/`false`). Emits:
 * - `onMQTT()` with a 3-byte SNIRF-like packet: [type=1][bool][subtype]
 * - `onActive()` to refresh the parent indicator's confidence timer.
 *
 * Supports configurable string values for "true" and "false", and 10 predefined
 * subtype labels (e.g., "on/off", "open/close").
 *
 * Unlike `MQTTAlert`, this component does **not** send system-wide alerts —
 * it only updates the visual indicator and activity state.
 */
class QsnWebWidgetCardIndicatorMQTTState : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardIndicatorMQTTState(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream, QString path = QString());
    void writeToMQTTPSN(QByteArray data);

public slots:
    /**
     * @brief Handles incoming messages from the global system bus (snBUS).
     *
     * Listens for `BUSSERV_MGTT` with signal `BUSSERV_MGTT_reception_raw`.
     * If topic matches, delegates to `QSNMQTTTopicMsgProcess()`.
     *
     * @param container Message payload.
     * @param sender Ignored.
     */
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

signals:
    /**
     * @brief Emitted when valid MQTT data is received and converted.
     *
     * Payload is a 3-byte packet: [1][bool][subtype].
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
    QString mqttValueTrue;              ///< String representing "true" (default: "1")
    QString mqttValueFalse;             ///< String representing "false" (default: "0")

    quint8 mqttSubType;                 ///< Subtype index (0–9): "on/off", "open/close", etc.
    bool mqttOnlyChange;                ///< If true, emit only on state change (debounce)

    quint8 lastState;                   ///< Last known state: 0=false, 1=true, 255=unknown

    /**
     * @brief Parses and evaluates incoming MQTT message.
     *
     * Extracts value, compares to true/false strings, and emits signals if needed.
     */
    void QSNMQTTTopicMsgProcess(QString msg);

};

#endif // QSNWEBWIDGETCCARDINDICATORMQTTSTATE_H
