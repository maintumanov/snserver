#include "qsnbsshapes.h"


QString QsnBsItalic(QString text, bool conditional)
{
    if (conditional) return "<i>" + text + "</i>";
    return text;
}

QString QsnBsAccessRightsTextAddNote(QString text, quint8 accessRights)
{
    switch (accessRights) {
    case 0: return text + " (" + QObject::tr("Guest") + ")";
    case 2: return text + " (" + QObject::tr("Admin") + ")";
    }
    return text;
}

QString QsnBsJsonItem(QString type, QString id, QString varname, QString value)
{
    return QString("\"%1%2%3\": %4").arg(type, id, varname, value);
}

QString QsnBsJsonItemText(QString type, QString id, QString varname, QString value)
{
    return QString("\"%1%2%3\": \"%4\"").arg(type, id, varname, value);
}

QString QsnBsNavItem(QString name, QString icon, QString link, bool select)
{
    return QString("<li class=\"nav-item\">"
                   "<a class=\"nav-link  %4\" href=\"%1\">"
                   "<svg class=\"icon-sprite\">"
                   "<use xlink:href=\"assets/images/icons-sprite.svg#%3\"/>"
                   "</svg> %2</a></li>").arg(link, name, icon, select?"active":"");
}


QString QsnBsFormHiddenBlockBegin(QString id, bool hidden)
{
    return QString("<div id=\"%1\" class=\"%2\">").arg(id, hidden?" d-none":"");
}

QString QsnBsFormHiddenBlockEnd()
{
    return QString("</div>");
}

QString QsnBsFormTextEdit(QString name, QString value, QString id, QString type, bool isInvalid)
{
    QString item;
    item += QString("<div class=\"form-group row\">");
    item += QString("<label for=\"%1\" class=\"col-12 col-sm-3 col-xl-2 col-form-label\">%2</label>").arg(id, name);
    item += QString("<div class=\"col-12 col-sm-9 col-lg-5 col-xl-4\">");
    item += QString("<input class=\"form-control custom-focus%4\" type=\"%3\" value=\"%2\" id=\"%1\">")
            .arg(id, value, type, isInvalid?" is-invalid":"");
    item += QString("</div></div>");
    return item;
}


QString QsnBsFormTextArea(QString name, QString value, QString id)
{
    QString item;
    item += QString("<div class=\"form-group row\">");
    item += QString("<label for=\"%1\" class=\"col-12 col-sm-3 col-xl-2 col-form-label\">%2</label>").arg(id, name);
    item += QString("<div class=\"col-12 col-sm-9 col-lg-5 col-xl-4\">");
    item += QString("<textarea class=\"form-control custom-focus\" id=\"%1\">%2</textarea>")
            .arg(id, value);
    item += QString("</div></div>");
    return item;
}

QString QsnBsFormCheckboxL(QString name, bool checked, QString id)
{
    return QString("<div class=\"form-check\">"
                   "<input type=\"checkbox\" class=\"form-check-input\" id=\"%3\" %2>"
                   "<label class=\"form-check-label\" for=\"%3\">%1</label>"
                   "</div>").arg(name, checked?"checked":"", id);
}


QString QsnBsFormLabel(QString name, QString value, QString id)
{
    QString item;
    item += QString("<div class=\"form-group row\">");
    item += QString("<label for=\"%1\" class=\"col-12 col-sm-3 col-xl-2 col-form-label\">%2</label>").arg(id, name);
    item += QString("<div class=\"col-12 col-sm-9 col-lg-5 col-xl-4\">");
    item += QString("<label  class=\"col-form-value\"  id=\"%2\">%1</label>").arg(value, id);
    item += QString("</div></div>");
    return item;
}

QString QsnBsFormCheckbox(QString name, bool checked, QString id)
{
    return QString("<div class=\"form-group row\">"
                   "<label for=\"%3\" class=\"col-6 col-sm-3 col-xl-2 col-form-label\">%1</label>"
                   "<div class=\"col-6 col-sm-9 col-xl-10\">"
                   "<input type=\"checkbox\" class=\"float-end float-sm-left form-control\" id=\"%3\" %2>"
                   "</div></div>").arg(name, checked?"checked":"", id);
}

QString QsnBsFormRadio(QString name, QString group, bool checked, QString id)
{
    return QString("<div class=\"form-group row\">"
                   "<label for=\"%3\" class=\"col-6 col-sm-3 col-xl-2 col-form-label\">%1</label>"
                   "<div class=\"col-6 col-sm-9 col-xl-10\">"
                   "<input type=\"radio\" name=\"%4\" class=\"float-end float-sm-left form-control\" id=\"%3\" %2>"
                   "</div></div>").arg(name, checked?"checked":"", id, group);
}

QString QsnBsFormSwitch(QString name, bool checked, QString id)
{
    return QString("<div class=\"form-group row\">"
                   "<label for=\"%3\" class=\"col-6 col-sm-3 col-xl-2 col-form-label\">%1</label>"
                   "<div class=\"col-6 col-sm-9 col-xl-10\">"
                   "<input type=\"checkbox\" class=\"form-control float-end float-sm-left form-control-switch\" id=\"%3\" %2>"
                   "</div></div>").arg(name, checked?"checked":"", id);
}

QString QsnBsFormButton(QString name, QString id, QString type)
{
    QString item;
    item += QString("<div class=\"form-group row\">");
    item += QString("<label for=\"%2\" class=\"col-12 col-sm-3 col-xl-2 col-form-label\"></label>").arg(id);
    item += QString("<div class=\"col-12 col-sm-9\">");
    item += QString("<button id=\"%2\" type=\"button\" "
                    "class=\"btn %3 d-lg-inline-block ms-auto \">"
                    "%1</button>").arg(name, id, type);
    item += QString("</div></div>");
    return item;
}


QString QsnBsFormWeeks(QString name, quint8 days, QString id)
{
    QString item;
    item += QString("<div class=\"form-group row\">");
    item += QString("<label class=\"col-6 col-sm-3 col-xl-2 col-form-label\">%1</label>").arg(name);
    item += QString("<div class=\"col-12 col-sm-9 col-lg-5 col-xl-4\">");
    item += QString("<div class=\"btn-group\" role=\"group\">");

    item += QString("<label class=\"btn btn-label btn-sm px-2 py-2 py-2\">");
    item += QString("<input type=\"checkbox\" name=\"%3\" data-unit=\"toggle-item\" autocomplete=\"off\" %2>%1</label>").arg(QObject::tr("MO"), (days & 0x01)?" checked":"", id);

    item += QString("<label class=\"btn btn-label btn-sm px-2 py-2 py-2\">");
    item += QString("<input type=\"checkbox\" name=\"%3\" data-unit=\"toggle-item\" autocomplete=\"off\" %2>%1</label>").arg(QObject::tr("TU"), (days & 0x02)?" checked":"", id);

    item += QString("<label class=\"btn btn-label btn-sm px-2 py-2 py-2\">");
    item += QString("<input type=\"checkbox\" name=\"%3\" data-unit=\"toggle-item\" autocomplete=\"off\" %2>%1</label>").arg(QObject::tr("WE"), (days & 0x04)?" checked":"", id);

    item += QString("<label class=\"btn btn-label btn-sm px-2 py-2 py-2\">");
    item += QString("<input type=\"checkbox\" name=\"%3\" data-unit=\"toggle-item\" autocomplete=\"off\" %2>%1</label>").arg(QObject::tr("TH"), (days & 0x08)?" checked":"", id);

    item += QString("<label class=\"btn btn-label btn-sm px-2 py-2 py-2\">");
    item += QString("<input type=\"checkbox\" name=\"%3\" data-unit=\"toggle-item\" autocomplete=\"off\" %2>%1</label>").arg(QObject::tr("FR"), (days & 0x10)?" checked":"", id);

    item += QString("<label class=\"btn btn-label btn-sm px-2 py-2 py-2\">");
    item += QString("<input type=\"checkbox\" name="%3" data-unit="toggle-item" autocomplete="off" %2>%1</label>").arg(QObject::tr("SA"), (days & 0x20)?" checked":"", id);

    item += QString("<label class=\"btn btn-label btn-sm px-2 py-2 py-2\">");
    item += QString("<input type=\"checkbox\" name="%3" data-unit="toggle-item" autocomplete="off" %2>%1</label>").arg(QObject::tr("SU"), (days & 0x40)?" checked":"", id);

    item += QString("</div></div></div>");

    return item;
}

QString QsnBsFormSelectorBegin(QString name, QString id)
{
    return QString("<div id=\"%2\" class=\"form-group row\">"
                   "<label for=\"date\" class=\"col-12 col-sm-3 col-xl-2 col-form-label\">%1</label>"
                   "<div class=\"col-12 col-sm-9 col-lg-5 col-xl-4\">"
                   "<select name=\"%2\" class=\"form-control custom-focus\">").arg(name, id);
}


QString QsnBsFormSelectorOption(QString name, QString value, bool selected)
{
    return QString("<option value=\"%2\"%3>%1</option>").arg(name, value, selected?" selected":"");
}

QString QsnBsFormSelectorEnd()
{
    return QString("</select></div></div>");
}


QString QsnBsFormGroupButtonBegin()
{
    QString item;
    item += QString("<div class=\"form-group row\">");
    item += QString("<label  class=\"col-12 col-sm-3 col-xl-2 col-form-label\"></label>");
    item += QString("<div class=\"col-12 col-sm-9 text-end text-sm-start\">");
    return item;
}

QString QsnBsFormGroupButton(QString name, QString id, QString type)
{
    return QString("<button id=\"%2\" type=\"button\" "
                   "class=\"btn %3 me-1 mb-1 \">"
                   "%1</button>").arg(name, id, type);
}

QString QsnBsFormGroupButtonLink(QString name, QString id, QString link, QString type)
{
    return QString("<button id=\"%2\" type=\"button\" "
                   "class=\"btn %3 me-1 mb-1\" onclick=\"%4\">"
                   "%1</button>").arg(name, id, type, link);
}

QString QsnBsFormGroupButtonEnd()
{
    return QString("</div></div>");
}

QString QsnBsAlertDanger(QString text)
{
    return QString("<div class=\"alert alert-danger\" role=\"alert\">%1</div>").arg(text);
}

QString QsnBsAlertWarning(QString text)
{
    return QString("<div class=\"alert alert-warning\" role=\"alert\">%1</div>").arg(text);
}

QString QsnBsCategory(QString title)
{
    return QString("<div class=\"col-12\"><h3>%1</h3></div>").arg(title);
}

//QString QsnBslCardUsersListAdd(QString link, QString text)
//{
//    QString item;
//    item += QString("<div class=\"col-12 col-sm-6 col-md-6 col-xl-3 col-lg-4 px-1 px-sm-1\">");
//    item += QString("<a href=\"%1\" class=\"mvh-150 card card-link text-white bg-primary my-1\" >").arg(link);
//    item += QString("<div class=\"card-body text-center\">");
//    item += QString("<svg class=\"icon-sprite icon-3x\">"
//                    "<use xlink:href=\"assets/images/icons-sprite.svg#subicon-add\" />"
//                    "</svg>");
//    item += QString("<h6 class=\"card-text\">%1</h6>").arg(text);
//    item += QString("</div></a></div>");
//    return item;
//}


QString QsnBsPanelWidgetsBegin(QString title)
{
    return QString("<div class=\"col-sm-12 col-md-6 col-xl-4 px-1 px-sm-2\">"
                   "<h4 class=\"card-title my-1\">%1</h4>").arg(title);
}

QString QsnBsPanelWidgetsEnd()
{
    return QString("</div>");
}

QString QsnBsPanelCardBegin(quint32 id, QString title)
{
    QString item;
    item += QString("<div class=\"col-sm-12 col-md-6 col-xl-4 px-1 px-sm-2\">");
    item += QString("<div class=\"card my-1 my-sm-3\" data-unit-group=\"panel-%1\">").arg(id);
    item += QString("<div class=\"card-body\">");
    item += QString("<h4 class=\"card-title ms-2\">%1</h4>").arg(title);
    item += QString("</div>");
    return item;
}

QString QsnBsPanelCardEnd()
{
    return QString("</div></div>");
}

QString QsnBsPanelCardItemSwitchBegin(quint8 integated, quint32 id, bool active)
{
    switch (integated){
    case 0:
        return QString("<div class=\"card my-1 my-sm-3%2\" data-unit=\"switch-%1\">"
                       "<div class=\"card-body d-flex flex-row \">").arg(id).arg(active?" active":"");
    case 1:
        return QString("<li class=\"list-group-item d-flex%2\" data-unit=\"switch-%1\">").arg(id).arg((active?" active":""));
    case 2:
        return QString("<li class=\"list-group-item d-flex%2 pb-3\" data-unit=\"switch-%1\">").arg(id).arg((active?" active":""));
    }
    return QString();
}

QString QsnBsPanelCardItemSwitchEnd(quint8 integated)
{
    switch (integated) {
    case 0:
        return QString("</div></div>");
    case 1:
        return QString("</li>");
    }
    return QString();
}

QString QsnBsPanelCardItemSwitchSwitch(quint32 id, bool checked)
{
    return QString("<label class=\"switch_m ms-auto %1\">"
                   "<input type=\"checkbox\" id=\"switch-%2\" %1>"
                   "<span></span></label>").arg(checked?"checked":"").arg(id);
}

QString QsnBsPanelCardItemSwitchFunJSON(quint32 id)
{
    return QString("iot.setSwitchState('switch-%1', data.switch%1state);").arg(id);
}

QString QsnBsPanelCardItemSwitchFunClick(quint32 id)
{
    return QString("$(\"#switch-%1\").click(function(){"
                   "sendState('action', '%1', ($(this).prop('checked')));"
                   "}); ").arg(id);
}

QString QsnBsPanelCardItemLabelBegin(quint8 integated, quint32 id, bool active, QString link)
{
    QString item;
    switch (integated) {
    case 0:
        if (link.isEmpty()) item += QString("<div class=\"card my-1 my-sm-3");
        else item += QString("<a href=\"/%1\" class=\"card my-1 my-sm-3").arg(link);
        item += QString(" %1\" data-unit=\"label-%2\">").arg(active?"active":"").arg(id);
        item += QString("<div class=\"card-body d-flex px-2 flex-row \">");
        break;
    case 1:
        item += QString("<li class=\"list-group-item d-flex  px-2%2\" data-unit=\"label-%1\">").arg(id).arg((active?" active":""));
        break;
    case 2:
        item += QString("<li class=\"list-group-item d-flex  px-2 pb-3%2\" data-unit=\"label-%1\">").arg(id).arg((active?" active":""));
        break;
    }
    return item;
}

QString QsnBsPanelCardItemLabelEnd(quint8 integated, QString link)
{
    switch (integated) {
    case 0:
        if (link.isEmpty()) return QString("</div></div>");
        else return QString("</a></div>");
    case 1:
        return QString("</li>");
    }
    return QString();
}

QString QsnBsPanelCardItemLabelBadge(quint32 id, QString value)
{
    return QString("<h6 class=\"ms-auto text-primary\" id=\"label-%1\">%2</h6>").arg(id).arg(value);
}

QString QsnBsPanelCardItemLabelFunJSON(quint32 id)
{
    //return QString(" $('#label-%1').text(data.label%1data);").arg(id);
    return QString("iot.setLabelState('label-%1', data.label%1data, data.label%1active, data.label%1warning);").arg(id);
}

QString QsnBsPanelCardItemLinkBegin(quint32 id, bool active, QString link)
{
    QString item = QString("<a href=\"%1\" class=\"card my-1 my-sm-3 card-link").arg(link);
    item += QString(" %1\" data-unit=\"link-%2\">").arg(active?"active":"").arg(id);
    item += QString("<div class=\"card-body d-flex flex-row card-link-icon\">");
    return item;
}

QString QsnBsPanelCardItemLinkEnd()
{
    return QString("<svg class=\"icon-link\">"
                   "<use xlink:href=\"assets/images/icons-sprite.svg#link\" />"
                   "</svg></div></a>");
}

QString QsnBsPanelCardItemLinkFunJSON(quint32 id)
{
    return QString("iot.setLabelState('link-%1', data.link%1data, data.link%1active, data.link%1warning);").arg(id);
}

QString QsnBsIconNum(int num, QString par)
{
    return QString("<svg class=\"icon-sprite\" %2>"
                   "<use xlink:href=\"assets/images/icons-items.svg#item%1\"/>"
                   "</svg>").arg(num).arg(par);
}

QString QsnBsIconSys(QString name)
{
    return QString("<svg class=\"icon-sprite\">"
                   "<use xlink:href=\"assets/images/icons-sprite.svg#%1\"/>"
                   "</svg>").arg(name);
}

QString QsnBsIconLink()
{
    return QString("<svg class=\"icon-link\">"
                   "<use xlink:href=\"assets/images/icons-sprite.svg#link\"/>"
                   "</svg>");
}

QString QsnBsPanelCardItemName(quint8 integated, QString name)
{
    switch (integated) {
    case 0:
        //        return QString("<h5>%1</h5>").arg(name);
    case 1:
    case 2:
        return QString("<p class=\"specs\">%1</p>").arg(name);
    }
    return QString();
}

QString QsnBsPanelCardItemEnd()
{
    return QString("</div></div>");
}

QString QsnBsTapsBegin()
{
    return QString("<div class=\"px-0 px-sm-4 col-12 col-sm-12\">"
                   "<ul class=\"nav nav-tabs nav-fill\" role=\"tablist\">");
}

QString QsnBsTapsTabAdd(QString id, QString name, bool active)
{
    return QString("<li class=\"nav-item\">"
                   "<a class=\"nav-link %1\" data-bs-toggle=\"tab\" data-bs-target=\"#%2\" "
                   "href=\"#%2\" role=\"tab\">%3</a></li>").arg((active==true?"active":" "), id, name);
}

QString QsnBsTapsPanesBegin()
{
    return QString("</ul><div class=\"tab-content px-0 py-3 px-sm-4 py-sm-4\">");
}

QString QsnBsTapsPanelBegin(QString id, bool active)
{
    return QString("<div class=\"tab-pane fade show %1\" id=\"%2\" role=\"tabpanel\"><form>").arg((active==true?"active":" "), id);
}

QString QsnBsTapsPanelEnd()
{
    return QString("</form></div>");
}

QString QsnBsTapsPanesEnd()
{
    return QString("</div>");
}

QString QsnBsTapsEnd()
{
    return QString("</div>");
}

QString QsnBsFormBegin()
{
    return QString("<div class=\"col-12\">");
}

QString QsnBsFormEnd()
{
    return QString("</div>");
}

QString QsnBsFormTableBegin()
{
    return QString("<table class=\"table\">");
}

QString QsnBsFormTableTheadBegin()
{
    return QString("<thead><tr>");
}

QString QsnBsFormTableTheadAdd(QString title, QString width)
{
    if (width.isEmpty()) return QString("<th>%1</th>").arg(title);
    return QString("<th style=\"width: %2;\">%1</th>").arg(title, width);
}

QString QsnBsFormTableTheadAddMD(QString title, QString width)
{
    if (width.isEmpty()) return QString("<th class=\"d-none d-md-table-cell\" scope=\"col\">%1</th>").arg(title);
    return QString("<th class=\"d-none d-md-table-cell\" scope=\"col\" style=\"width: %2;\">%1</th>").arg(title, width);
}

QString QsnBsFormTableTheadAddCl(QString title, QString addclass)
{
    if (addclass.isEmpty()) return QString("<th>%1</th>").arg(title);
    return QString("<th style=\"%2;\">%1</th>").arg(title, addclass);
}

QString QsnBsFormTableTheadEnd()
{
    return QString("</tr></thead>");
}

QString QsnBsFormTableBodyBegin(QString id)
{
    return QString("<tbody id=\"%1\">").arg(id);
}

QString QsnBsFormTableBodyRowLogAdd(int level, QString time, QString text)
{
    QString cl;
    if (level == 0) cl = " class=\"caution\"";
    if (level == 1) cl = " class=\"warning\"";
    return QString("<tr%1><td>%2</td><td colspan=\"2\">%3</td></tr>").arg(cl, time, text);
}

QString QsnBsFormTableBodyRowAlertAdd(bool alert, QString time, QString text, QString locales)
{
    QString cl;
    if (alert) cl = " class=\"caution\"";
    return QString("<tr%1><td>%2</td><td>%3</td><td>%4</td></tr>").arg(cl, text, locales, time);
}

QString QsnBsFormTableBodyRowAdd(QString c1)
{
    return QString("<tr><td>%1</td></tr>").arg(c1);
}

QString QsnBsFormTableBodyRowAdd(QString c1, QString c2)
{
    return QString("<tr><td>%1</td><td>%2</td></tr>").arg(c1, c2);
}

QString QsnBsFormTableBodyRowAdd(QString c1, QString c2, QString c3)
{
    return QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(c1, c2, c3);
}

QString QsnBsFormTableBodyRowAdd(QString c1, QString c2, QString c3, QString c4)
{
    return QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>").arg(c1, c2, c3, c4);
}

QString QsnBsFormTableBodyRowAdd(QString c1, QString c2, QString c3, QString c4, QString c5)
{
    return QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>").arg(c1, c2, c3, c4, c5);
}


QString QsnBsFormTableBodyRowCustomBegin()
{
    return QString("<tr>");
}

QString QsnBsFormTableBodyRowCustomCell(QString text)
{
    return QString("<td>%1</td>").arg(text);
}

QString QsnBsFormTableBodyRowCustomCellMD(QString text)
{
    return  QString("<td class=\"d-none d-md-table-cell\">%1</td>").arg(text);
}

QString QsnBsFormTableBodyRowCustomEnd()
{
    return QString("</tr>");
}


QString QsnBsFormTableBodyEnd()
{
    return QString("</tbody>");
}

QString QsnBsFormTableEnd()
{
    return QString("</table>");
}

QString QsnBsAlertBegin(QString level)
{
    QString alert;
    alert += QString("<div class=\"alert alert-");
    if (level.isEmpty()) alert += "primary"; else alert += level;
    alert += QString(" col-12 col-sm-10 ms-sm-auto\" role=\"alert\">");
    return alert;
}

QString QsnBsLink(QString text, QString link)
{
    return  QString("<a href=\"%1\" class=\"alert-link\">%2</a>").arg(link, text);
}

QString QsnBsAlertEnd()
{
    return QString("</div>");
}


QString QsnBsPageTitle(QString text)
{
    QString ret;
    ret = QString("<div class=\"col-12 col-sm-11 ms-sm-auto pb-3 pt-0\">");
    ret += QString("<h3>%1</h3>").arg(text);
    ret += QString("</div>");
    return ret;
}


QString QsnBsPageTitleBegin(QString text)
{
    QString ret;
    ret = QString("<div class=\"card-body d-flex flex-row \">");
    ret += QString("<h3>%1</h3>").arg(text);
    return ret;
}

QString QsnBsPageTitleEnd()
{
    return QString("</div>");
}

QString QsnBsPanelTitle(QString text)
{
    QString ret;
    ret = QString("<div class=\"col-12 pb-3 pt-0\">");
    ret += QString("<h3>%1</h3>").arg(text);
    ret += QString("</div>");
    return ret;
}

QString QsnBsPanelTitleButton(QString title, QString name, QString id, QString type, QString link)
{
    QString ret;
    ret = QString("<div class=\"col-12 mb-2\">");
    ret += QString("<div class=\"row px-0 px-sm-1\">");
    ret += QString("<h3 class=\"px-0 col-12 col-sm-8\">%1</h3>").arg(title);
    ret += QString("<button id=\"%2\" type=\"button\" "
                   "class=\"px-sm-0 col-12 col-sm-3 btn btn-info ms-sm-auto %3\" onclick=\"window.location.href='%4';\">"
                   "%1</button>").arg(name, id, type, link);

    ret += QString("</div></div>");
    return ret;
}

QString QsnBsButton(QString name, QString id, QString type)
{
    return QString("<button id=\"%2\" type=\"button\" "
                   "class=\"btn %3 d-lg-inline-block ms-auto \">"
                   "%1</button>").arg(name, id, type);
}



