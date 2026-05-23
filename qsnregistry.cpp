#include "qsnregistry.h"

QsnRegistry::QsnRegistry(QObject *parent) : QObject(parent)
{

}

QVariant QsnRegistry::value(QString key)
{
    if (!reg.contains(key)) return QVariant();
    return reg[key];
}

void QsnRegistry::clear()
{
    reg.clear();
}

void QsnRegistry::setValue(QString key, QVariant value)
{
    reg[key] = value;
    changes(key, value);
}

void QsnRegistry::log(quint8 warningLevel, QString text, QString fname, QString title)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::information;
    container.Command = warningLevel;
    container.Address = 64;
    container.Sender = 0;
    container.info = text;
    container.Data = QString("%1/%2").arg(fname, title).toUtf8();
    container.Signal = 0;
    emit snBUSOutput(container, this);
}

void QsnRegistry::changes(QString key, QVariant vol)
{
    log(7, tr("Change value: %1 = %2").arg(key, vol.toString()), QString("registry"), tr("Registry"));

    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 5;
    container.Address = 0;
    container.Sender = 0;
    container.info = key;
    container.Data = vol.toByteArray();
    container.Signal = 0;
    emit snBUSOutput(container, this);
}
