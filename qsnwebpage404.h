#ifndef QSNWEBPAGE404_H
#define QSNWEBPAGE404_H

#include <QObject>
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebPage404 : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPage404(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);

private:
    QsnGlobalModules *mds;
};

#endif // QSNWEBPAGE404_H
