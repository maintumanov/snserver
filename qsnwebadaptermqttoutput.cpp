#include "qsnwebadaptermqttoutput.h"

QsnWebAdapterMQTTOutput::QsnWebAdapterMQTTOutput(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName("snMQTT Output");
    mds = modules;
    subscriptionTopic = QString();
    repeatTimeoutSet = 0;
    repeatTimeout = 0;
    repeatTimeoutMinute = 0;
}

void QsnWebAdapterMQTTOutput::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items
    subscriptionTopic = optionsMap.value("TC", QString()).toString();
    valueJson = optionsMap.value("VN", QString()).toString();
    valueType = optionsMap.value("VT", 0).toInt();
    repeatTimeoutSet = optionsMap.value("RT", 0).toUInt();

    // data init
    QSNByteToRAW(&valueData, 0, valueType);
    QString s = QSNRAWToValueString(&valueData, 1);
    QSNStringToRAW(&valueData, valueType, 1, s, QString(), QString());
}

void QsnWebAdapterMQTTOutput::reciveTopic(QString topic, QByteArray msg)
{
    if (topic != subscriptionTopic) return;
    if (valueJson.isEmpty()) {
        QSNStringToRAW(&valueData, valueType, 1, msg, QString(), QString::number(outputIDP));
        mds->io->widgetReciveIndexSignal(0, &valueData, this);
        repeatTimeoutMinute = 60;
        repeatTimeout = repeatTimeoutSet;
        return;
    }

    QMap<QString, QString> options;
    QSNParseJSON(QString::fromUtf8(msg), &options);

    if (!options.contains(valueJson)) return;
    valueData.clear();
    QSNStringToRAW(&valueData, valueType, 1, options.value(valueJson), QString(), QString::number(outputIDP));
    mds->io->widgetReciveIndexSignal(0, &valueData, this);
    repeatTimeoutMinute = 60;
    repeatTimeout = repeatTimeoutSet;
}

QString QsnWebAdapterMQTTOutput::getTopic()
{
    return subscriptionTopic;
}

QString QsnWebAdapterMQTTOutput::getJsonValueItem()
{
    if (!valueJson.isEmpty()) return QString("\"%1\":%2").arg(valueJson, QSNRAWToValueString(&valueData, 1));
    else return QSNRAWToJSONString(&valueData);
    return QString();
}

void QsnWebAdapterMQTTOutput::setIDP(quint16 idp)
{
    outputIDP = idp;
}

QString QsnWebAdapterMQTTOutput::getLabelLastData()
{
    if (!valueJson.isEmpty()) return QSNEscapingHTMLCharacters(QString("%1").arg(QSNRAWToValueString(&valueData, 1)));
    else return QSNEscapingHTMLCharacters(QSNRAWToValueString(&valueData, 1));
    return tr("empty");
}

void QsnWebAdapterMQTTOutput::timeCheck()
{
    //qDebug() << repeatTimeout << repeatTimeoutMinute;
    if (!repeatTimeout) return;
    repeatTimeout --;
    if (repeatTimeoutMinute) repeatTimeoutMinute --;
    if (repeatTimeoutMinute) return;
    mds->io->widgetReciveIndexSignal(0, &valueData, this);
    repeatTimeoutMinute = 60;
}






