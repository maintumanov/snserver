#include "qsnwebwidgetcardthermostatindicator.h"


QsnWebWidgetCardThermostatIndicator::QsnWebWidgetCardThermostatIndicator(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Indicator input"));
    mds = modules;
    mode = 0;
    istate = false;
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardThermostatIndicator::snBUSInput(QSNContainer container, QObject *)
{
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        mds->db->tempSettings.insert(objectName() + QString::number(itemID()) + "state", istate);
    }
}


void QsnWebWidgetCardThermostatIndicator::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    mode = optionsMap.value("MD", 0).toUInt();
    istate = mds->db->tempSettings.value(objectName() + QString::number(itemID()) + "state", istate).toBool();
}

void QsnWebWidgetCardThermostatIndicator::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO == 0) istate = true;
    if (indexIO == 1) istate = false;
    if (indexIO == 2) istate = QSNRAWtoBool(data, 1);
}

quint8 QsnWebWidgetCardThermostatIndicator::state()
{
    if (!istate) return 0;
    if (mode == 0) return 1;
    if (mode == 1) return 2;
    return 0;
}


