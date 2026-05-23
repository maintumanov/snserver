#include "qsnwebwidgetcardindicatormsg.h"


QsnWebWidgetCardIndicatorMsg::QsnWebWidgetCardIndicatorMsg(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Messages"));
    mds = modules;
    lastImp = 10; // Invalid initial state to force first message
}

void QsnWebWidgetCardIndicatorMsg::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    setPath(path);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; // child items (none expected)

    cautionMsg = optionsMap.value("MC", tr("Alert [d]")).toString();
    warningMsg = optionsMap.value("MW", tr("Warning [d]")).toString();
    normalMsg = optionsMap.value("MN", tr("Normal [d]")).toString();
    successfulMsg = optionsMap.value("MS", tr("Successful [d]")).toString();
    confidenceMsg = optionsMap.value("MI", tr("The data is invalid")).toString();
}

void QsnWebWidgetCardIndicatorMsg::sendMSG(quint8 imp, QByteArray data)
{
    // Debounce: skip if no change
    if (imp == lastImp) return;
    switch (imp) {
    case 0: // Caution
        if (cautionMsg.isEmpty()) break;
        sendMessage(0, dataImplementation(cautionMsg, data));
        break;
    case 1: // Warning
        if (warningMsg.isEmpty()) break;
        sendMessage(1, dataImplementation(warningMsg, data));
        break;
    case 2: // Normal
        if (normalMsg.isEmpty()) break;
        sendMessage(2, dataImplementation(normalMsg, data));
        break;
    case 3: // Successful
        if (successfulMsg.isEmpty()) break;
        sendMessage(2, dataImplementation(successfulMsg, data));
        break;
    case 4:// Treated as warning
        if (confidenceMsg.isEmpty()) break;
        sendMessage(1, confidenceMsg);
        break;
    }
    lastImp = imp;
}

QString QsnWebWidgetCardIndicatorMsg::dataImplementation(QString msg, QByteArray data)
{
    if (data.isEmpty()) return msg.replace("[d]", "");
    return msg.replace("[d]", QSNRAWtoScaledVolume(&data));
}

void QsnWebWidgetCardIndicatorMsg::sendMessage(quint8 importance, QString message)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MESSAGE;
    container.Signal = 3; // Standard message signal
    container.Sender = 0;
    // Format: "LevelText Message {path}"
    container.info = QSNWarningLevelToText(importance) + message + "{" + getPath(true).toUtf8() + "}";
    mds->interface->snBUSInput(container, this);
}


