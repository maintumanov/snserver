#include "qsnwebitemmessage.h"

QsnWebItemMessage::QsnWebItemMessage(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Message"));
    mds = modules;
}

void QsnWebItemMessage::getContents(QStringList *, int )
{

}

void QsnWebItemMessage::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items
    mtext = optionsMap.value("TX", tr("Text")).toString();
    mlevel = optionsMap.value("LV", 3).toUInt();
    lshashe = hashCalc(objectName() + mtext + QString::number(mlevel));
}

void QsnWebItemMessage::receiveSignalIOIndex(int indexIO, QByteArray *)
{
    if (indexIO != 0) return;
    emit sendMessage(QSNWarningLevelToText(mlevel) + mtext);
}

void QsnWebItemMessage::setText(QString stext)
{
    mtext = stext;
}

QString QsnWebItemMessage::getText()
{
    return mtext;
}

void QsnWebItemMessage::setLevel(quint8 level)
{
    mlevel = level;
}

quint8 QsnWebItemMessage::getLevel()
{
    return mlevel;
}

QString QsnWebItemMessage::key()
{
    return lshashe.toHex();
}

bool QsnWebItemMessage::isThisKey(QString key)
{
    return lshashe.toHex() == key;
}

void QsnWebItemMessage::writeSetting(QXmlStreamWriter *stream)
{
    stream->writeAttribute("key", key());
    stream->writeAttribute("text", mtext);
    stream->writeAttribute("level", QString::number(mlevel));
}

void QsnWebItemMessage::readSetting(QXmlStreamAttributes *attrib)
{
    mtext = attrib->value("text").toString();
    mlevel = attrib->value("level").toUInt();
}

QByteArray QsnWebItemMessage::hashCalc(QString text)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(text.toUtf8());
    return hash.result();
}




