#include "qsnwebpage.h"

QsnWebPage::QsnWebPage(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Page"));
    iconNum = 0;
    wEnable = true;
    widgetColPage = true;
    rm = Q_NULLPTR;
}

void QsnWebPage::getJavaScript(QStringList *script, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)  widgets[i]->getJavaScript(script, accountIndex);
}

void QsnWebPage::getJSDepending(QStringList *depending, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)  widgets[i]->getJSDepending(depending, accountIndex);
}

void QsnWebPage::getFunctions(QStringList *functions, int accountIndex, QString pageURL)
{
    for(int i = 0; i < widgets.count(); i ++)  if (widgets[i]->widgetAccessRights <= wEnable) widgets[i]->getFunctions(functions, accountIndex, pageURL);
}

void QsnWebPage::getChartFunctions(QStringList *functions, QStringList *depending, QStringList *argument, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)  if (widgets[i]->widgetAccessRights <= wEnable) widgets[i]->getChartFunctions(functions, depending, argument, accountIndex);
}

void QsnWebPage::getFunctionsJSON(QStringList *fjson, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)  if (widgets[i]->widgetAccessRights <= wEnable) widgets[i]->getFunctionsJSON(fjson, accountIndex);
}

void QsnWebPage::getDialogs(QStringList *dialogs, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)  if (widgets[i]->widgetAccessRights <= wEnable) widgets[i]->getDialogs(dialogs, accountIndex);
}

void QsnWebPage::getContents(QStringList *contents, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)  if (widgets[i]->widgetAccessRights <= wEnable) widgets[i]->getContents(contents, accountIndex);
}

void QsnWebPage::actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid)
{
    if (options->contains("action")) {
    QString state = options->value("action");

        if (state == QLatin1String("action"))
            for(int i = 0; i < widgets.count(); i ++)
                widgets[i]->actionItem(URL, options, returnItems, uid);

        if (state == QLatin1String("request") || state == QLatin1String("action")) {
            for(int i = 0; i < widgets.count(); i ++) widgets[i]->getItemJSON(returnItems);
        }
    }
}

void QsnWebPage::addWidget(QsnWeb *widget)
{
    widgets.append(widget);
}

void QsnWebPage::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    int id;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count;
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;

        if (sig == QString(QLatin1String("CR"))) {
            QsnWebWidgetCard *card = new QsnWebWidgetCard(static_cast<quint32>(id), modules(), this);
            addWidget(card);
            card->fromStream(stream, getPath());
        }

        if (sig == QString(QLatin1String("RCI"))) {
            if (rm != Q_NULLPTR) delete(rm);
            QsnWebWidgetRMCodeInput *rm = new QsnWebWidgetRMCodeInput(static_cast<quint32>(id), modules(), this);
            rm->fromStream(stream, getPath());
            connect(rm, SIGNAL(onRM(QByteArray)), this, SLOT(readRM(QByteArray)));
        }

    }
    // set options
    widgetUrl = optionsMap.value("URL", QLatin1String("/")).toString();
    iconNum = static_cast<quint8>(optionsMap.value("IC", 0).toUInt());
    widgetMenu = optionsMap.value("PB", true).toBool();
    widgetIcon = QString("pageIcon%1").arg(iconNum);
}


void QsnWebPage::urlChanged(int accountIndex)
{
    wEnable = mds->auth->checkAcces(accountIndex, widgetAccessRights);
}

void QsnWebPage::readRM(QByteArray code)
{
    for(int i = 0; i < widgets.count(); i ++)  widgets[i]->actionRMCode(code);
}

