#include "qsnwebwidgetrmcodeinput.h"

QsnWebWidgetRMCodeInput::QsnWebWidgetRMCodeInput(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Remote code input"));
    mds = modules;
}

void QsnWebWidgetRMCodeInput::fromStream(QDataStream *stream, QString path)
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

void QsnWebWidgetRMCodeInput::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO != 0 || (data->at(0) != 39 && data->at(0) != 30)) return;
    QByteArray rm = QSNRAWtoRMcode(data, 1);
    emit onRM(rm);
}

