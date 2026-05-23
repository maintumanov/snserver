#include "qsnwebitemscheduleaction.h"

QsnWebItemScheduleAction::QsnWebItemScheduleAction(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Action"));
    mds = modules;

}

void QsnWebItemScheduleAction::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    mds->io->registrationAction(this, objectName(), 0, 87);
}

void QsnWebItemScheduleAction::widgetRunAction(int )
{
    QByteArray data;
    data[0] = 0;
    mds->io->widgetReciveIndexSignal(0, &data, this);
}





