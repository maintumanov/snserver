#ifndef QSNWEBWIDGETCCARDINDICATORMSG_H
#define QSNWEBWIDGETCCARDINDICATORMSG_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

/**
 * @brief Message sender for QsnWebWidgetCardIndicator.
 *
 * Sends human-readable notifications based on importance level (0–4):
 * - 0 = caution
 * - 1 = warning
 * - 2 = normal
 * - 3 = successful
 * - 4 = confidence timeout (inactive)
 *
 * Messages support placeholder `[d]`, which is replaced with formatted sensor value.
 *
 * Configurable via UI options:
 * - MC: Caution message
 * - MW: Warning message
 * - MN: Normal message
 * - MS: Successful message
 * - MI: Invalid/confidence-loss message
 */
class QsnWebWidgetCardIndicatorMsg : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardIndicatorMsg(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream, QString path = QString());

public slots:
    /**
     * @brief Sends a message if importance level changed.
     *
     * Skips sending if `imp == lastImp` (debounce).
     * Emits message via global snBUS (`BUSSERV_MESSAGE`).
     *
     * @param imp Importance level (0–4).
     * @param data Raw sensor data (used to replace `[d]` placeholder).
     */
    void sendMSG(quint8 imp, QByteArray data);

private:
    QsnGlobalModules *mds;              ///< Global modules context

    QString cautionMsg;                 ///< Template for importance=0 (caution)
    QString warningMsg;                 ///< Template for importance=1 (warning)
    QString normalMsg;                  ///< Template for importance=2 (normal)
    QString successfulMsg;              ///< Template for importance=3 (successful)
    QString confidenceMsg;              ///< Template for importance=4 (inactive/timeout)

    quint8 lastImp;                     ///< Last sent importance (initialized to 10 = invalid)

    /**
     * @brief Replaces `[d]` placeholder with formatted sensor value.
     *
     * If data is empty, removes placeholder entirely.
     */
    QString dataImplementation(QString msg, QByteArray data);

    /**
     * @brief Dispatches message via global snBUS.
     *
     * Uses `BUSSERV_MESSAGE` with signal=3 and formatted info string.
     */
    void sendMessage(quint8 importance, QString message);

};

#endif // QSNWEBWIDGETCCARDINDICATORMSG_H
