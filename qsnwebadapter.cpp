#include "qsnwebadapter.h"

QsnWebAdapter::QsnWebAdapter(QsnInterface *interface, QObject *parent) : QObject(parent)
{
    snInterface = interface;
}

void QsnWebAdapter::fromStream(QDataStream *)
{

}

void QsnWebAdapter::adapterDisconnect()
{

}

void QsnWebAdapter::adapterConnect()
{

}
