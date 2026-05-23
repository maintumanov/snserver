#ifndef QSNWEBWIDGETCCARDINDICATORCOMP_H
#define QSNWEBWIDGETCCARDINDICATORCOMP_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"
#include "qsnwebwidgetcardindicatorcompoutputs.h"

/**
 * @brief Comparator module for QsnWebWidgetCardIndicator that evaluates data against thresholds.
 *
 * This component monitors incoming raw data and classifies it into one of three states:
 * - **Upper bound exceeded** → importance = `upperImportance`
 * - **Lower bound exceeded** → importance = `lowerImportance`
 * - **Within normal range** → importance = `normalImportance`
 *
 * Hysteresis prevents rapid state oscillation near boundaries.
 *
 * Configurable via UI options:
 * - Upper/Lower bounds (as strings, parsed to qreal)
 * - Importance levels (0=caution, 1=warning, 2=normal, 3=successful)
 * - Hysteresis margin (qreal)
 *
 * Emits `onStateChange()` when importance level changes, triggering alerts or UI updates.
 */
class QsnWebWidgetCardIndicatorComp : public QsnWeb
{
    Q_OBJECT
public:
    /**
     * @brief Constructor.
     *
     * Initializes comparator with default "normal" state.
     *
     * @param iID Unique identifier.
     * @param modules Global application context.
     * @param parent Parent QObject.
     */
    QsnWebWidgetCardIndicatorComp(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    /**
     * @brief Loads configuration from binary stream.
     *
     * Reads bounds, importance levels, hysteresis, and child output module.
     *
     * @param stream Valid QDataStream.
     * @param path Base path (unused in this component).
     */
    void fromStream(QDataStream *stream, QString path = QString());

    /**
     * @brief Evaluates current data against configured thresholds.
     *
     * Updates internal state (`isUpper`, `isLower`, `isNormal`) and `currentImportance`.
     * Triggers output signals and emits `onStateChange()` if state changed.
     *
     * @param data Raw SNIRF-like packet (must be ≥3 bytes; first byte = type).
     * @return Current importance level (0–3).
     */
    quint8 importance(QByteArray data);

public slots:
    /**
     * @brief Handles global bus messages (e.g., config save).
     *
     * Persists current importance to temporary settings before reload.
     *
     * @param container Message payload.
     * @param sender Ignored.
     */
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

signals:
    /**
     * @brief Emitted when importance level changes.
     *
     * @param newState New importance: 0=caution, 1=warning, 2=normal, 3=successful.
     */
    void onStateChange(quint8 newState);

private:
    // === State Flags ===
    bool isUpper;       ///< True if value ≥ upperBound
    bool isLower;       ///< True if value ≤ lowerBound
    bool isNormal;      ///< True if within [lowerBound, upperBound] (with hysteresis)

    // === Importance Levels (from UI options) ===
    quint8 currentImportance;   ///< Current classification (0–3)
    quint8 lastImportance;      ///< Previous classification (to detect changes)
    quint8 upperImportance;     ///< Importance when above upper bound (default: 1 = warning)
    quint8 lowerImportance;     ///< Importance when below lower bound (default: 1 = warning)
    quint8 normalImportance;    ///< Importance in normal range (default: 2 = normal)

    // === Thresholds ===
    QString upperBound;         ///< String representation of upper threshold (parsed to qreal)
    QString lowerBound;         ///< String representation of lower threshold (parsed to qreal)
    qreal boundHysteresis;      ///< Hysteresis margin to prevent flickering near bounds

    // === Dependencies ===
    QsnGlobalModules *mds;              ///< Global modules (settings, DB, etc.)
    QsnWebWidgetCardIndicatorCompOutputs *outputs; ///< Optional output triggers (e.g., relays)

    // === Private Helpers ===
    void compareUpper(QByteArray data);   ///< Checks if data exceeds upper bound
    void compareLower(QByteArray data);   ///< Checks if data falls below lower bound
    void compareNormal();                 ///< Sets normal state if neither bound is violated
    void checkChange();                   ///< Emits signal if importance changed

};

#endif // QSNWEBWIDGETCCARDINDICATORCOMP_H
