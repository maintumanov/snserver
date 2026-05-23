#include "qsnwebpage404.h"

QsnWebPage404::QsnWebPage404(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    widgetIcon = "info";
    setObjectName(tr("Info"));
}

void QsnWebPage404::getContents(QStringList *contents, int )
{
    *contents << QsnBsAlertBegin();
    *contents << tr("The requested page could not be found, check the address. <br>");
    *contents << QsnBsLink(tr("Click to return home."), "/");
    *contents << QsnBsAlertEnd();
}

void QsnWebPage404::fromStream(QDataStream *stream)
{
    int Count;
    QString title;
    *stream >> title;

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> Count;
    // set options
    setObjectName(title);
    if (optionsMap.contains("Url")) widgetUrl = optionsMap.value("Url").toString();
}


