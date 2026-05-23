#include "qsnwebwidgetcardindicatorsnirf.h"

QsnWebWidgetCardIndicatorSNIRF::QsnWebWidgetCardIndicatorSNIRF(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("SNIRF"));
    mds = modules;
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardIndicatorSNIRF::fromStream(QDataStream *stream, QString path)
{
    int Count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> Count; // child items (none expected)

    // Load SNIRF-specific configuration options from the stream
    snirfEmptyType = static_cast<quint8>(optionsMap.value("ET", 0).toInt());   // Empty signal handling: 0=ignore, 1–6=synthetic states (true/false/on/off/open/close)
    snirfAddress   = static_cast<quint8>(optionsMap.value("AD", 0).toInt());   // SNIRF device address (0–127)
    snirfParametr  = static_cast<quint8>(optionsMap.value("PR", 0).toInt());   // SNIRF parameter ID (0–63)
}

void QsnWebWidgetCardIndicatorSNIRF::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;

    // Only process SNIRF messages
    if (container.role != QSNContainer::service) return;

    if (container.Command == BUSSERV_SNIRF) {
        if (container.Signal != snirfAddress) return;
        emit onSNIRFactive("SNIRF");
        if (container.Sender != snirfParametr) return;
        QByteArray vol = container.Data;
        if (!vol.isEmpty() && vol.at(0) > 0) {
            // Case 1: Valid non-empty packet with positive type byte
            emit onSNIRF(vol, "BUSSERV_SNIRF");
        } else {
            // Case 2: Empty or invalid packet → generate synthetic state if configured
            vol.clear();
            switch (snirfEmptyType) {
            case 1: vol.append(char(1)); vol.append(char(0)); vol.append(char(0)); break; // false, subtype 0
            case 2: vol.append(char(1)); vol.append(char(1)); vol.append(char(0)); break; // true,  subtype 0
            case 3: vol.append(char(1)); vol.append(char(0)); vol.append(char(1)); break; // false, subtype 1
            case 4: vol.append(char(1)); vol.append(char(1)); vol.append(char(1)); break; // true,  subtype 1
            case 5: vol.append(char(1)); vol.append(char(0)); vol.append(char(3)); break; // false, subtype 3
            case 6: vol.append(char(1)); vol.append(char(1)); vol.append(char(3)); break; // true,  subtype 3
            default: return; // snirfEmptyType == 0 → ignore
            }

            if (!vol.isEmpty()) {
                emit onSNIRF(vol, "BUSSERV_SNIRF");
            }
        }
    }
}




