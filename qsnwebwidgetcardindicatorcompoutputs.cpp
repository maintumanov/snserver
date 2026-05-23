#include "qsnwebwidgetcardindicatorcompoutputs.h"

QsnWebWidgetCardIndicatorCompOutputs::QsnWebWidgetCardIndicatorCompOutputs(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("output"));
    mds = modules;
    lastsend = -1; // Ensures first state change always triggers output
}

void QsnWebWidgetCardIndicatorCompOutputs::fromStream(QDataStream *stream)
{
    if (!stream || stream->status() != QDataStream::Ok) return;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    // Load UI-configurable options (currently none used in this component)
    getOptionsFromStream(stream);

    // Load I/O signal bindings (e.g., which hardware pin maps to which state)
    mds->io->loadIOFromStream(stream, this);
    *stream >> count;  // Child items (none expected, but preserved for compatibility)
}

void QsnWebWidgetCardIndicatorCompOutputs::outputLower()
{
    // Avoid redundant signaling: only emit if state changed
    if (lastsend == 2) return;
    QByteArray data;

     // Load I/O signal bindings (e.g., which hardware pin maps to which state)
    mds->io->widgetReciveIndexSignal(2, &data, this);
    lastsend = 2;
}

void QsnWebWidgetCardIndicatorCompOutputs::outputUpper()
{
    if (lastsend == 0) return;
    QByteArray data;
    mds->io->widgetReciveIndexSignal(0, &data, this); // Index 0 = upper state
    lastsend = 0;
}

void QsnWebWidgetCardIndicatorCompOutputs::outputNormal()
{
    if (lastsend == 1) return;
    QByteArray data;
    mds->io->widgetReciveIndexSignal(1, &data, this); // Index 1 = normal state
    lastsend = 1;
}

