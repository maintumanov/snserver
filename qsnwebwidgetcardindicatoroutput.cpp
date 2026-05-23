#include "qsnwebwidgetcardindicatoroutput.h"

QsnWebWidgetCardIndicatorOutput::QsnWebWidgetCardIndicatorOutput(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("output"));
    mds = modules;

    // Initialize to current time to allow immediate first signal
    lastsend = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

void QsnWebWidgetCardIndicatorOutput::fromStream(QDataStream *stream)
{
    if (!stream || stream->status() != QDataStream::Ok) return;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    // Load general options (none used, but preserved for compatibility)
    getOptionsFromStream(stream);

    // Load I/O signal bindings (e.g., which hardware pin maps to output index 0)
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; // child items (none expected)
}

void QsnWebWidgetCardIndicatorOutput::outputSignal(QByteArray data)
{
    quint64 stamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // Debounce: suppress if same data received within last 1000 ms
    if ((lastdata == data) && (lastsend + 1000 > stamp)) return;
    lastdata = data;
    lastsend = stamp;

    // Emit I/O signal on index 0 (configured in constructor model)
    mds->io->widgetReciveIndexSignal(0, &data, this);
}
