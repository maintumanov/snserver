#ifndef QSNWEBWIDGETCCARDINDICATORMQTT_H
#define QSNWEBWIDGETCCARDINDICATORMQTT_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

/**
 * @brief MQTT data source for QsnWebWidgetCardIndicator.
 *
 * Subscribes to a specific MQTT topic and extracts numeric values from JSON or raw payloads.
 * Converts incoming messages into standardized SNIRF-like raw packets (`QByteArray`) and emits them
 * via the `onMQTT()` signal for processing by the parent indicator.
 *
 * Supports:
 * - JSON path extraction (e.g., `"temperature"` from `{"temperature": 23.5}`)
 * - Type coercion (default: type 1 = scaled real)
 * - Value multiplier (e.g., for unit conversion)
 * - Min/Max value filtering (out-of-range values are discarded)
 *
 * This component does **not** publish data — only receives.
 */
class QsnWebWidgetCardIndicatorMQTT : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardIndicatorMQTT(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream, QString path = QString());
    void writeToMQTTPSN(QByteArray data);

public slots:
    /**
     * @brief Handles incoming messages from the global system bus (snBUS).
     *
     * Listens for `BUSSERV_MGTT` with signal `BUSSERV_MGTT_reception_raw`.
     * If topic matches, parses payload, applies filters, and emits `onMQTT()`.
     *
     * @param container Message payload.
     * @param sender Ignored.
     */
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

signals:
    /**
     * @brief Emitted when valid MQTT data is received and converted.
     *
     * @param data Standardized raw packet (SNIRF format).
     * @param source Identifier ("BUSSERV_MGTT").
     */
    void onMQTT(QByteArray data, QString source);

private:
    QsnGlobalModules *mds;              ///< Global modules (MQTT, settings, etc.)

    QString mqttTopic;                  ///< MQTT topic to subscribe (e.g., "sensors/temp")
    QString mqttItemJson;               ///< JSON key to extract (empty = use full payload)
    quint8 mqttItemType;                ///< SNIRF data type (default: 1 = scaled real)
    qreal mqttMultiplier;               ///< Multiplier applied after parsing (default: 1.0)
    QString mqttMinValue;               ///< Optional minimum allowed value (as string)
    QString mqttMaxValue;               ///< Optional maximum allowed value (as string)
};

#endif // QSNWEBWIDGETCCARDINDICATORMQTT_H
