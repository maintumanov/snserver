#include "qsnwebwidgetcardtitleswitch.h"

QsnWebWidgetCardTitleSwitch::QsnWebWidgetCardTitleSwitch(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Card title switch"));
    mds = modules;
    widgetData = QString("iot.checkSwitchGroup(\"%1\");").arg(itemID());
}

void QsnWebWidgetCardTitleSwitch::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$('#switch-%1').click(function() {").arg(itemID());
    *functions << QString("var action = \"all-off\";");
    *functions << QString("if ($(this).prop('checked')) action = \"all-on\";");
    *functions << QString("iot.switchGroup(\"card-%1\", action); });").arg(itemID());
}

void QsnWebWidgetCardTitleSwitch::getItemJSON(QStringList *jsonItems)
{
    *jsonItems << QString("iot.checkSwitchGroup(\"%1\");").arg(itemID());
}

void QsnWebWidgetCardTitleSwitch::getContents(QStringList *contents, int )
{  
    bool state = widgetData.toString().contains("true");
    *contents << QString("<div class=\"d-flex me-2 ms-2 mb-3\">"
                         "<h5 class=\"card-title\">%1</h5>"
                         "<label class=\"switch_m ms-auto\"><input type=\"checkbox\""
                         "id=\"switch-%3\" %2><span></span></label>"
                         "</div>").arg(objectName(), state?"checked":"").arg(itemID());

}

void QsnWebWidgetCardTitleSwitch::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);
    getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items

}


