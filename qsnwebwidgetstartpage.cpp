#include "qsnwebwidgetstartpage.h"

QsnWebWidgetStartPage::QsnWebWidgetStartPage(quint32 iID, QsnGlobalModules *modules, QsnWebAdapterTCPserver **tcpServer, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    tcpSer = tcpServer;
}

void QsnWebWidgetStartPage::getContents(QStringList *contents, int )
{
    *contents << QsnBsAlertBegin("warning");
    *contents << QString(tr("The server is running, but is not configured.") + "<br>");
    *contents << QString(tr("Download the configuration can be one of the following ways:") + "<br>");
    *contents << QString(tr("1. Place the configuration file \"webappconf.wac\"  in the folder \"signalnet\" to the server and restart server.")  + "<br>");
    if ((*tcpSer) != Q_NULLPTR) *contents << QString(tr("2. Using the configuration tool, by connecting to the TCP server, port %1")).arg((*tcpSer)->getPort());
    *contents << QsnBsAlertEnd();
}

void QsnWebWidgetStartPage::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count;
    // set options
    if (optionsMap.contains("Url")) widgetUrl = optionsMap.value("Url").toString();
}

