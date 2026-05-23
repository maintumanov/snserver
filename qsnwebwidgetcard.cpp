#include "qsnwebwidgetcard.h"

QsnWebWidgetCard::QsnWebWidgetCard(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Card"));
    mds = modules;
    titleWidget = Q_NULLPTR;
    rm = Q_NULLPTR;
}

void QsnWebWidgetCard::getJavaScript(QStringList *script, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, widgets[i]->widgetAccessRights)) widgets[i]->getJavaScript(script, accountIndex);
}

void QsnWebWidgetCard::getJSDepending(QStringList *depending, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, widgets[i]->widgetAccessRights)) widgets[i]->getJSDepending(depending, accountIndex);
}

void QsnWebWidgetCard::getFunctions(QStringList *functions, int accountIndex, QString pageURL)
{
    if (titleWidget != Q_NULLPTR) titleWidget->getFunctions(functions, accountIndex, pageURL);

    for(int i = 0; i < widgets.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, widgets[i]->widgetAccessRights)) widgets[i]->getFunctions(functions, accountIndex, pageURL);
}

void QsnWebWidgetCard::getFunctionsJSON(QStringList *fjson, int accountIndex)
{
    if (titleWidget != Q_NULLPTR) titleWidget->getFunctionsJSON(fjson, accountIndex);
    for(int i = 0; i < widgets.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, widgets[i]->widgetAccessRights)) widgets[i]->getFunctionsJSON(fjson, accountIndex);
}

void QsnWebWidgetCard::getDialogs(QStringList *dialogs, int accountIndex)
{
    for(int i = 0; i < widgets.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, widgets[i]->widgetAccessRights)) widgets[i]->getDialogs(dialogs, accountIndex);
}

void QsnWebWidgetCard::getContents(QStringList *contents, int accountIndex)
{
    *contents << QString("<div class=\"card pb-2 pt-2\" data-unit-group=\"card-%1\">")
                 .arg(titleWidget != Q_NULLPTR?titleWidget->itemID():itemID());
    if (titleWidget != Q_NULLPTR) {
       QString data;
       for(int i = 0; i < widgets.count(); i ++) data += widgets[i]->widgetState() + ";";
       titleWidget->widgetData = data;
       titleWidget->getContents(contents, accountIndex);
    }
    *contents << QString("<ul class=\"list-group list-group-flush\">");
    for(int i = 0; i < widgets.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, widgets[i]->widgetAccessRights)) widgets[i]->getContents(contents, accountIndex);
    *contents << QString("</ul></div>");
}

void QsnWebWidgetCard::actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid)
{
    for(int i = 0; i < widgets.count(); i ++)
        widgets[i]->actionItem(URL, options, returnItems, uid);
}

void QsnWebWidgetCard::actionRMCode(QByteArray code)
{
    for(int i = 0; i < widgets.count(); i ++)
        widgets[i]->actionRMCode(code);
}

void QsnWebWidgetCard::fromStream(QDataStream *stream, QString path)
{
    int Count;
    QString sig;
    int id;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> Count;
    for (int i = 0; i < Count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("SW"))) {
            QsnWebWidgetCardSwitch *iswitch = new QsnWebWidgetCardSwitch(static_cast<quint32>(id), modules(), this);
            widgets.append(iswitch);
            iswitch->fromStream(stream, getPath());
        }

        if (sig == QString(QLatin1String("IN"))) {
            QsnWebWidgetCardIndicator *iindicator = new QsnWebWidgetCardIndicator(static_cast<quint32>(id), modules(), this);
            widgets.append(iindicator);
            iindicator->fromStream(stream, getPath());
        }

        if (sig == QString(QLatin1String("CTL"))) {
            QsnWebWidgetCardTitle *title = new QsnWebWidgetCardTitle(static_cast<quint32>(id), modules(), this);
            if (titleWidget != Q_NULLPTR) delete(titleWidget);
            titleWidget = title;
            title->fromStream(stream, getPath());
            title->setObjectName(objectName());
        }

        if (sig == QString(QLatin1String("CTS"))) {
            QsnWebWidgetCardTitleSwitch *titleswitch = new QsnWebWidgetCardTitleSwitch(static_cast<quint32>(id), modules(), this);
            if (titleWidget != Q_NULLPTR) delete(titleWidget);
            titleWidget = titleswitch;
            titleswitch->fromStream(stream, getPath());
            titleswitch->setObjectName(objectName());
        }

        if (sig == QString(QLatin1String("RCI"))) {
            if (rm != Q_NULLPTR) delete(rm);
            QsnWebWidgetRMCodeInput *rm = new QsnWebWidgetRMCodeInput(static_cast<quint32>(id), modules(), this);
            rm->fromStream(stream, getPath());
            connect(rm, SIGNAL(onRM(QByteArray)), this, SLOT(readRM(QByteArray)));
        }

        if (sig == QString(QLatin1String("THC"))) {
            QsnWebWidgetCardThermostatControl *ithemcon = new QsnWebWidgetCardThermostatControl(static_cast<quint32>(id), modules(), this);
            widgets.append(ithemcon);
            ithemcon->fromStream(stream, getPath());
        }

        if (sig == QString(QLatin1String("TB"))) {
            QsnWebWidgetCardTwoButtons *itb = new QsnWebWidgetCardTwoButtons(static_cast<quint32>(id), modules(), this);
            widgets.append(itb);
            itb->fromStream(stream, getPath());
        }

        if (sig == QString(QLatin1String("OB"))) {
            QsnWebWidgetCardButton *iob = new QsnWebWidgetCardButton(static_cast<quint32>(id), modules(), this);
            widgets.append(iob);
            iob->fromStream(stream);
        }

        if (sig == QString(QLatin1String("SL"))) {
            QsnWebWidgetCardSelector *isl = new QsnWebWidgetCardSelector(static_cast<quint32>(id), modules(), this);
            widgets.append(isl);
            isl->fromStream(stream, getPath());
        }

    }

    if (titleWidget != Q_NULLPTR) for(int i = 0; i < widgets.count(); ++i) widgets[i]->widgetData = titleWidget->widgetData;
}

void QsnWebWidgetCard::getItemJSON(QStringList *jsonItems)
{
    for(int i = 0; i < widgets.count(); i ++)
        widgets[i]->getItemJSON(jsonItems);
}

void QsnWebWidgetCard::readRM(QByteArray code)
{
    actionRMCode(code);
}


