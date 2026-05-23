#include "qsnwebitempresencetime.h"

QsnWebItemPresenceTime::QsnWebItemPresenceTime(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Motion sensor"));
    mds = modules;
}

void QsnWebItemPresenceTime::fromStream(QDataStream *stream)
{
    int count;
    QString Name;
    *stream >> Name;
    if (!Name.isEmpty()) setObjectName(Name);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items
    presencetime = optionsMap.value("PT", 3600).toInt();
    presencetimealt = optionsMap.value("PTA", 60).toInt();
}

void QsnWebItemPresenceTime::receiveSignalIOIndex(int indexIO, QByteArray *)
{
    if (indexIO == 0) {
        emit presenceAction(objectName(), presencetime, presencetimealt);
    }
}






