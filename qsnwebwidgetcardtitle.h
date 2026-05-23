#ifndef QSNWEBWIDGETCARDTITLE_H
#define QSNWEBWIDGETCARDTITLE_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

class QsnWebWidgetCardTitle : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardTitle(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream, QString path = QString());

public slots:
    //----BM


private:

    QsnGlobalModules *mds;
};

#endif // QSNWEBWIDGETCARDTITLE_H
