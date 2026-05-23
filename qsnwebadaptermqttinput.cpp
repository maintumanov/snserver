#include "qsnwebadaptermqttinput.h"

QsnWebAdapterMQTTInput::QsnWebAdapterMQTTInput(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName("snMQTT Input");
    mds = modules;
    subscriptionTopic = QString();
}

void QsnWebAdapterMQTTInput::fromStream(QDataStream *stream)
{
    int Count;
    QString t;
    *stream >> t;
    setObjectName(t);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> Count; //items
    subscriptionTopic = optionsMap.value("TC", QString()).toString();
    valueName = optionsMap.value("VN", QString()).toString();
    valueType = optionsMap.value("VT", 0).toInt();

    // data init
    QSNByteToRAW(&valueData, 0, valueType);
    QString s = QSNRAWToValueString(&valueData, 1);
    QSNStringToRAW(&valueData, valueType, 1, s, QString(), QString());
}

QString QsnWebAdapterMQTTInput::getJsonValueItem()
{
    if (!valueName.isEmpty()) return QString("\"%1\":%2").arg(valueName, QSNRAWToValueString(&valueData, 1));
    else return QSNRAWToJSONString(&valueData);
    return QString();
}

QString QsnWebAdapterMQTTInput::getLabelLastData()
{
    if (!valueName.isEmpty()) return QSNEscapingHTMLCharacters(QString("%1").arg(QSNRAWToValueString(&valueData, 1)));
    else return QSNEscapingHTMLCharacters(QSNRAWToValueString(&valueData, 1));
    return tr("empty");
}

void QsnWebAdapterMQTTInput::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO != 0) return;
    if (valueName.isEmpty()) {
        emit onWritePublishJson(subscriptionTopic, QString("%2").arg(QSNRAWToValueString(data, 1)));
        valueData = *data;
        return;
    }

    if (QSNRAWtoByte(data, 0) != valueType) return;
    emit onWritePublishJson(subscriptionTopic, QString("{\"%1\":%2}").arg(valueName, QSNRAWToValueString(data, 1)));
    valueData = *data;
}

QString QsnWebAdapterMQTTInput::getTopic()
{
    return subscriptionTopic;
}







