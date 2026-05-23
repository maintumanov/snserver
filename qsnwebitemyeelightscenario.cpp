#include "qsnwebitemyeelightscenario.h"

QsnWebItemYeelightScenario::QsnWebItemYeelightScenario(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Yeelight lamp scenario"));
    mds = modules;
    lampbright = 100;
    lamptemp = 4700;
}

void QsnWebItemYeelightScenario::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    lampbright = optionsMap.value("BR", 100).toUInt();
    lamptemp = optionsMap.value("TM", 4700).toUInt();
}

void QsnWebItemYeelightScenario::receiveSignalIOIndex(int indexIO, QByteArray *)
{
    if (indexIO == 0) {emit applyScenarioLamp(lampbright, lamptemp);}
}






