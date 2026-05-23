#ifndef QSNWEBWIDGETRMCODEINPUT_H
#define QSNWEBWIDGETRMCODEINPUT_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebWidgetRMCodeInput : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetRMCodeInput(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream, QString path = QString());
    void receiveSignalIOIndex(int indexIO, QByteArray *data);

signals:
    void onRM(QByteArray data);

private:
    QsnGlobalModules *mds;
};

#endif // QSNWEBWIDGETRMCODEINPUT_H
