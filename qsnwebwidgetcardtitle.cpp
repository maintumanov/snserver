#include "qsnwebwidgetcardtitle.h"

QsnWebWidgetCardTitle::QsnWebWidgetCardTitle(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Card title"));
    mds = modules;
}


void QsnWebWidgetCardTitle::getContents(QStringList *contents, int )
{
    *contents << QString("<div class=\"card-body ml-2\">"
                         "<h5 class=\"card-title\">%1</h5>"
                         "</div>").arg(objectName());
}

void QsnWebWidgetCardTitle::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
}


