#ifndef QSNWEBWIDGETCARDSELECTORITEM_H
#define QSNWEBWIDGETCARDSELECTORITEM_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"

class QsnWebWidgetCardSelectorItem : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardSelectorItem(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);

private:
    QsnGlobalModules *mds;
};


#endif // QSNWEBWIDGETCARDSELECTORITEM_H
