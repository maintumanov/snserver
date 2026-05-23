#include "qsniotable.h"
#include "qsnweb.h"

QsnIOTable::QsnIOTable(QsnInterface *snInterface, QObject *parent) : QObject(parent)
{
    setObjectName(tr("snIOTable"));
    interface = snInterface;
    connect(interface, SIGNAL(eventReciveSignal(QSNSignal)), this, SLOT(interfaceReciveSignalData(QSNSignal)));
    connect(this, SIGNAL(interfaceSendSignal(quint16,QByteArray*)), interface, SLOT(actionSendSignal(quint16,QByteArray*)));
}

void QsnIOTable::registrationInput(int index, QsnWeb *widget)
{
    ioItem io;
    io.index = index;
    io.widget = widget;
    inputs.append(io);
}

void QsnIOTable::registrationOutput(int index, QsnWeb *widget)
{
    ioItem io;
    io.index = index;
    io.widget = widget;
    outputs.append(io);
}

void QsnIOTable::registrationSignal(int index, quint16 signal)
{
    signalItem sitem;
    sitem.index = index;
    sitem.signal = signal;
    signalsTable.append(sitem);
}

void QsnIOTable::clearIOS()
{
    inputs.clear();
    outputs.clear();
    signalsTable.clear();
    ulinks.clear();
    actions.clear();
}

void QsnIOTable::loadIOFromStream(QDataStream *stream, QsnWeb *widget)
{
    ioItem io;
    bool isInput;

    int ioCount;
    *stream >> ioCount;
    for (int j = 0; j < ioCount; j ++) {
        *stream >> isInput;
        *stream >> io.index;
        io.widget = widget;
        io.indexItemIO = j;
        if (isInput) inputs.append(io);
        outputs.append(io);
    }
}

void QsnIOTable::loadSignalsStream(QDataStream *stream)
{
    signalItem sitem;

    int ioCount;
    *stream >> ioCount;
    for (int j = 0; j < ioCount; j ++) {
        *stream >> sitem.index;
        *stream >> sitem.signal;
        signalsTable.append(sitem);
    }
}

void QsnIOTable::registrationAction(QsnWeb *widget, QString name, int ioIndex, int icon)
{
    if (widget == Q_NULLPTR) return;
    actionItem it;
    it.webwidget = widget;
    it.name = name;
    it.iconIndex = icon;
    it.ioIndex = ioIndex;
    it.id = widget->itemID();
    actions.append(it);

    interface->snBUSInput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         QString("%1 - %2").arg(tr("Registration action"), it.name),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString(),
                         true), this);
}

void QsnIOTable::runAction(int actionIndex)
{
    if (actions.count() <= actionIndex) return;
    interface->snBUSInput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         QString("%1 - %2").arg(tr("Run action"), actions[actionIndex].name),
                         BUSSERV_LOG_CATEGORY_adapter,
                         QString("sniotable"),
                         false), this);
    actions[actionIndex].webwidget->widgetRunAction(actions[actionIndex].ioIndex);
}

void QsnIOTable::interfaceReciveSignalData(QSNSignal signal)
{
    interfaceReciveSignalData(signal.Signal, &signal.Data);
}

void QsnIOTable::interfaceReciveSignalData(quint16 signal, QByteArray *data)
{
    for (int i = 0; i < signalsTable.count(); i ++)
        if (signalsTable[i].signal == signal)
            reciveSignalFromIndex(signalsTable[i].index, data);
}

void QsnIOTable::widgetReciveIndexSignal(int indexItemIO, QByteArray *data, QsnWeb *widget)
{
    for (int i = 0; i < outputs.count(); i ++)
        if (outputs[i].widget == widget && outputs[i].indexItemIO == indexItemIO)
        {
            sendSignalFromIndex(outputs[i].index, data);
            break;
        }
}

void QsnIOTable::sendSignalFromIndex(int index, QByteArray *data)
{
    for (int i = 0; i < signalsTable.count(); i ++)
        if (signalsTable[i].index == index) {
            if (signalsTable[i].signal > 255) {emit interfaceSendSignal(signalsTable[i].signal, data);}
            interfaceReciveSignalData(signalsTable[i].signal, data);
        }
}

void QsnIOTable::reciveSignalFromIndex(int index, QByteArray *data)
{
    for (int i = 0; i < inputs.count(); i ++)
        if (inputs[i].index == index)
        {
            inputs[i].widget->receiveSignalIOIndex(inputs[i].indexItemIO, data);
            break;
        }
}


