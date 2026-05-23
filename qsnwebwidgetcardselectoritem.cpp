#include "qsnwebwidgetcardselectoritem.h"

QsnWebWidgetCardSelectorItem::QsnWebWidgetCardSelectorItem(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Item"));
    mds = modules;
}

void QsnWebWidgetCardSelectorItem::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    getOptionsFromStream(stream);
    //mds->io->loadIOFromStream(stream, this);
    *stream >> count; //io
    *stream >> count; //items
}







