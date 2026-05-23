#ifndef QSNWEBWIDGETCCARDINDICATORSNIRF_H
#define QSNWEBWIDGETCCARDINDICATORSNIRF_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

/**
 * @brief SNIRF protocol data source for QsnWebWidgetCardIndicator.
 *
 * Listens to global `BUSSERV_SNIRF` messages and filters by:
 * - Address (`snirfAddress`)
 * - Parameter (`snirfParametr`)
 *
 * When a matching packet arrives:
 * - Emits `onSNIRFactive()` to refresh parent indicator's confidence timer,
 * - Emits `onSNIRF()` with raw SNIRF payload (or synthetic packet if empty).
 *
 * Supports configurable "empty signal" behavior via `snirfEmptyType`:
 * - 0 = ignore empty packets
 * - 1–6 = generate predefined 3-byte synthetic states (e.g., on/off, open/close)
 */
class QsnWebWidgetCardIndicatorSNIRF : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardIndicatorSNIRF(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream, QString path = QString());
    void writeToMQTTPSN(QByteArray data);

public slots:
public slots:
    /**
     * @brief Handles incoming messages from the global system bus (snBUS).
     *
     * Listens for `BUSSERV_SNIRF`. If address and parameter match,
     * processes payload or generates synthetic state.
     *
     * @param container Message payload.
     * @param sender Ignored.
     */
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

signals:
    /**
     * @brief Emitted when valid SNIRF data (real or synthetic) is available.
     *
     * @param data Raw SNIRF-like packet (≥3 bytes: [type][value][subtype]).
     * @param source Identifier ("BUSSERV_SNIRF").
     */
    void onSNIRF(QByteArray data, QString source);
    /**
     * @brief Emitted whenever a matching SNIRF message is received.
     *
     * Used to refresh parent indicator's activity timer.
     */
    void onSNIRFactive(QString source);

private:
    QsnGlobalModules *mds;              ///< Global modules context

    quint8 snirfAddress;                ///< SNIRF device address (0–127)
    quint8 snirfParametr;               ///< SNIRF parameter ID (0–63)
    quint8 snirfEmptyType;              ///< Empty-packet handling: 0=ignore, 1–6=synthetic states
};

#endif // QSNWEBWIDGETCCARDINDICATORSNIRF_H
