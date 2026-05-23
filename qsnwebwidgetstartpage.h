#ifndef QSNWEBWIDGETSTARTPAGE_H
#define QSNWEBWIDGETSTARTPAGE_H

#include <QObject>
#include "qsnweb.h"
#include "qsnwebadaptertcpserver.h"

class QsnWebWidgetStartPage : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetStartPage(quint32 iID, QsnGlobalModules *modules, QsnWebAdapterTCPserver **tcpServer, QObject *parent = Q_NULLPTR);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);

private:
    QsnGlobalModules *mds;
    QsnWebAdapterTCPserver **tcpSer;
};

#endif // QSNWEBWIDGETSTARTPAGE_H
