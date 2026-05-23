#ifndef QSNWEBADAPTER_H
#define QSNWEBADAPTER_H

#include <QObject>
#include "qsninterface.h"

class QsnWebAdapter : public QObject
{
    Q_OBJECT
public:
    explicit QsnWebAdapter(QsnInterface *interface, QObject *parent = Q_NULLPTR);
    virtual void fromStream(QDataStream *stream);
    virtual void adapterDisconnect();
    virtual void adapterConnect();

    QsnInterface *snInterface;

signals:

public slots:

private:


};


#endif // QSNWEBADAPTER_H
