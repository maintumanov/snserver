#include "qsnwebpagecontrolpanel.h"

QsnWebPageControlPanel::QsnWebPageControlPanel(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    widgetUrl = "/settings";
    widgetIcon = "settings";
    setObjectName(tr("Control Panel"));
    widgetMenu = true;
    widgetColPage = true;
}


void QsnWebPageControlPanel::getFunctionsJSON(QStringList *fjson, int accountIndex)
{
    for (int i = 0; i < sitems.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, sitems[i].level)) {
            *fjson <<  QString("$('#card-%1').text(data.state%1vol);").arg(sitems[i].id);
            *fjson << QString("iot.setClassID('card-%1', data.state%1dng, 'text-danger');").arg(sitems[i].id);
            *fjson << QString("iot.setClassID('carda-%1', data.state%1bgs, 'bgcard-success');").arg(sitems[i].id);
            *fjson << QString("iot.setClassID('carda-%1', data.state%1bgc, 'bgcard-danger');").arg(sitems[i].id);
            *fjson << QString("iot.setClassID('carda-%1', data.state%1bgw, 'bgcard-warning');").arg(sitems[i].id);
        }
}

void QsnWebPageControlPanel::actionItem(QString , QMap<QString, QString> *, QStringList *returnItems, qint64 )
{
    for (int i = 0; i < sitems.count(); i ++) {
        *returnItems << QsnBsJsonItemText("state", sitems[i].id, "vol", sitems[i].wgt->widgetState());
        *returnItems << QsnBsJsonItem("state", sitems[i].id, "bgs", QSNBoolToText(sitems[i].wgt->widgetNotifState() == 's'));
        *returnItems << QsnBsJsonItem("state", sitems[i].id, "bgc", QSNBoolToText(sitems[i].wgt->widgetNotifState() == 'c'));
        *returnItems << QsnBsJsonItem("state", sitems[i].id, "bgw", QSNBoolToText(sitems[i].wgt->widgetNotifState() == 'w'));
        *returnItems << QsnBsJsonItem("state", sitems[i].id, "dng", QSNBoolToText(sitems[i].wgt->widgetState().endsWith("!")));
    }
}

void QsnWebPageControlPanel::getContents(QStringList *contents, int accountIndex)
{
//    *contents << QsnBsPanelTitle(objectName());
//    *contents << QString("<div class=\"col-12 pb-2 pt-0\">"
//                         "<h3>%1</h3>"
//                         "</div>").arg(objectName());

    for (int i = 0; i < sitems.count(); i ++)
        if (mds->auth->checkAcces(accountIndex, sitems[i].level)) {
            *contents << QString("<div class=\"col-12 px-0 px-sm-1\">");
            *contents << QString("<a id=\"carda-%3\" href=\"%1\" class=\"card card-link my-1%2 %4\" data-unit=\"card-%3\">")
                         .arg(sitems[i].url, true?" active":"", sitems[i].id, wStateToClass(sitems[i].wgt->widgetNotifState()));
            *contents << QString("<div class=\"card-body d-flex flex-row justify-content-start\">");
            *contents << QString("<div class=\"pull-left my-auto\">");
            *contents << QString("<svg class=\"icon-sprite  icon-1x\">");
            *contents << QString("<use xlink:href=\"assets/images/icons-sprite.svg#%1\"/>").arg(sitems[i].icon);
            *contents << QString("</svg></div>");
            *contents << QString("<div class=\"pull-right\">");
            *contents << QString("<h5>%1</h5><p id=\"card-%3\" class=\"status %4\">%2</p>")
                         .arg(sitems[i].name, sitems[i].wgt->widgetState(), sitems[i].id, sitems[i].wgt->widgetState().endsWith("!")?"text-danger":"");
            *contents << QString("</div></div></a></div>");
        }
}

void QsnWebPageControlPanel::addSetting(QsnWeb *wgt)
{
    settingItem item;
    item.name = wgt->objectName();
    item.url = wgt->widgetUrl;
    item.icon = wgt->widgetIcon;
    item.level = wgt->widgetAccessRights;
    item.id = item.url;
    item.id.remove(0, 1);
    item.wgt = wgt;
    sitems.append(item);
    versionID = QDateTime::currentMSecsSinceEpoch();
}

void QsnWebPageControlPanel::clearSettings()
{
    sitems.clear();
    versionID = QDateTime::currentMSecsSinceEpoch();
}

void QsnWebPageControlPanel::urlChanged(int )
{

}

QString QsnWebPageControlPanel::bool2text(bool state)
{
    if (state) return QLatin1String("true");
    return QLatin1String("false");
}

QString QsnWebPageControlPanel::wStateToClass(char wstate)
{
    if (wstate == 'c') return "bgcard-danger";
    if (wstate == 'w') return "bgcard-warning";
    if (wstate == 's') return "bgcard-success";
    return QString();
}
