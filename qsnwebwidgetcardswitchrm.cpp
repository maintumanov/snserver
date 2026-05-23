#include "qsnwebwidgetcardswitchrm.h"

QsnWebWidgetCardSwitchRM::QsnWebWidgetCardSwitchRM(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Remote code"));
    mds = modules;
}

void QsnWebWidgetCardSwitchRM::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    rmOn = optionsMap.value("ON", QByteArray()).toByteArray();
    rmOff = optionsMap.value("OFF", QByteArray()).toByteArray();
    rmSwitch = optionsMap.value("SW", QByteArray()).toByteArray();
}

void QsnWebWidgetCardSwitchRM::onRM(QByteArray code)
{
    if (rmOff == code) {emit onStateChange(false, "RM");}
    else if (rmOn == code) {emit onStateChange(true, "RM");}
    else if (rmSwitch == code) {emit onToggle("RM");}
}


