#ifndef QSNWEBWIDGETCARDINDICATOROUTPUT_H
#define QSNWEBWIDGETCARDINDICATOROUTPUT_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

/**
 * @brief Output controller for QsnWebWidgetCardIndicator.
 *
 * Emits I/O signal index 0 whenever new data arrives, with optional debouncing:
 * - If the same data is received within 1 second, it is **ignored**.
 *
 * Used to trigger external actions (e.g., relays, logs, or other widgets) based on
 * indicator value changes.
 */
class QsnWebWidgetCardIndicatorOutput : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardIndicatorOutput(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);

public slots:
    /**
     * @brief Emits I/O signal if data changed or debounce timeout expired.
     *
     * Compares incoming `data` with `lastdata`. If identical and received within
     * the last 1000 ms, the signal is suppressed.
     *
     * @param data Raw sensor packet to forward.
     */
    void outputSignal(QByteArray data);

private:
    QsnGlobalModules *mds;              ///< Global modules context
    QByteArray lastdata;                ///< Last emitted data (for deduplication)
    quint64 lastsend;                   ///< Timestamp of last emission (ms since epoch)
};

#endif // QSNWEBWIDGETCARDINDICATOROUTPUT_H
