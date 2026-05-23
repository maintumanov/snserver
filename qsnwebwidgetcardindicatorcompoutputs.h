#ifndef QSNWEBWIDGETCARDINDICATORCOMPOUTPUTS_H
#define QSNWEBWIDGETCARDINDICATORCOMPOUTPUTS_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

/**
 * @brief Output controller for QsnWebWidgetCardIndicatorComp.
 *
 * This component emits I/O signals when the comparator enters one of three states:
 * - **Upper threshold exceeded** → triggers output index 0
 * - **Normal range** → triggers output index 1
 * - **Lower threshold exceeded** → triggers output index 2
 *
 * It prevents redundant signal emission by tracking the last sent state (`lastsend`).
 *
 * Used to drive relays, LEDs, or other actuators based on sensor value ranges.
 */
class QsnWebWidgetCardIndicatorCompOutputs : public QsnWeb
{
    Q_OBJECT
public:

    QsnWebWidgetCardIndicatorCompOutputs(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);

public slots:
    /**
     * @brief Triggers output for "lower bound exceeded" state.
     *
     * Sends I/O signal on index **2** (if not already active).
     */
    void outputLower();
    /**
     * @brief Triggers output for "upper bound exceeded" state.
     *
     * Sends I/O signal on index **0** (if not already active).
     */
    void outputUpper();
    /**
     * @brief Triggers output for "normal range" state.
     *
     * Sends I/O signal on index **1** (if not already active).
     */
    void outputNormal();

private:
    QsnGlobalModules *mds;      ///< Global modules (I/O subsystem access)
    qint8 lastsend;             ///< Last emitted state: 0=upper, 1=normal, 2=lower, -1=none

};

#endif // QSNWEBWIDGETCARDINDICATORCOMPOUTPUTS_H
