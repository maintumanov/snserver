#include "qsnwebpageschedule.h"

QsnWebPageSchedule::QsnWebPageSchedule(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Schedule"));
    widgetUrl = "/schedule";
    widgetIcon = "subicon-schedule";
    nextTime = QTime::currentTime();

    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageSchedule::~QsnWebPageSchedule()
{

}

void QsnWebPageSchedule::getFunctions(QStringList *functions, int , QString)
{
    if (itemEventEdit >= -1) {
        *functions << QString(
                          "$('#search').keyup(function () {"
                          "var search = $(this).val();"
                          //                          "console.log(search);"
                          "$('table tbody tr').hide();"
                          "var len = $('table tbody tr:not(.notfound) td:contains(\"' + search + '\")').length;"
                          "if (len > 0) {"
                          "$('table tbody tr:not(.notfound) td:contains(\"' + search + '\")').each(function () {"
                          "$(this).closest('tr').show();"
                          "});} else {$('.notfound').show();}});");

        *functions << QString(
                          "$('#typeselect').change(function() {"
                          "var sl = $(this).find(\":selected\").val();"
                          "if (sl == \"date\") $('#dateblock').removeClass(\"d-none\"); else $('#dateblock').addClass(\"d-none\");"
                          "if (sl == \"week\") $('#weekblock').removeClass(\"d-none\"); else $('#weekblock').addClass(\"d-none\");"
                          "});");

        *functions << QString(
                          "$('#savebtn').click(function() {"
                          "if ($('#nameedit').val() == '') {"
                          "$('#nameedit').addClass(\"is-invalid\");"
                          "return; } else $('#nameedit').removeClass(\"is-invalid\");"
                          "var days = 0;"
                          "$.each($(\"input[name='weekedit']:checked\"), function() {days += Number($(this).attr('data-bs-toggle'));});"
                          "var actions = \"\";"
                          "$.each($(\"input[name='action']:checked\"), function() {"
                          "actions += ($(this).attr('data-bs-toggle') + ';'); });"
                          "console.log(actions);"
                          "console.log(days);"
                          " $.ajax({"
                          "url: \"%1\","
                          "type: 'POST',"
                          "data: {"
                          "action : 'save', "
                          "item: \"%2\","
                          "name: $('#nameedit').val(),"
                          "type: $('#typeselect').find(\":selected\").val(),"
                          "date: $('#dateedit').val(),"
                          "time: $('#timeedit').val(), "
                          "weekdays: days, "
                          "act: actions "
                          "},"
                          "dataType: 'json',"
                          "contentType: \"application/json; charset=utf-8\","
                          "success: function(data) {"
                          "if (data.save == true) {document.location.href=\"%1\"};"
                          " }, "
                          "error: function (x, t, e) { } }); "
                          "});").arg(widgetUrl).arg(itemEventEdit);

        *functions << QString("$(\"#removebtn\").click(function(){"
                              "sendState('remove', '%1', true);"
                              "document.location='%2';"
                              "}); ").arg(itemEventEdit).arg(widgetUrl);
        return;
    }

    *functions << QString("function tableschedules(data) {"
                          "var table = $(\"#scheduletable\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "var wr = \"\"; "
                          "var lnk = '\"%1?event=' + item.link + '\"'; "
                          "if (item.warning == \"w\") wr = \" class='warning'\";"
                          "table.append(\"<tr\" + wr + \"><td>\" + item.time + \"</td>\" +"
                          "\"<td>\" + item.date + \"</td>\" +"
                          "\"<td>\" + item.events + \"</td>\" +"
                          "\"<td class='d-none d-md-table-cell' scope='col' >\" + item.lastrun + \"</td>\" +"
                          "\"<td class='td-actions text-right p-0 pe-2 align-middle' >"
                          "<button type='button' class='btn btn-secondary btn-ms p-0' "
                          "onclick='window.location.href=\" + lnk + \";'>"
                          "<svg class='icon-sprite icon-05x'>"
                          "<use xlink:href='assets/images/icons-sprite.svg#subicon-edit'/>"
                          "</svg></button>"
                          "</td></tr>\");});}").arg(widgetUrl);
}

void QsnWebPageSchedule::getFunctionsJSON(QStringList *fjson, int )
{
    if (itemEventEdit >= -1) return;
    *fjson << QString(" tableschedules(data.schedules);");
}


void QsnWebPageSchedule::getContents(QStringList *contents, int )
{

    if (itemEventEdit >= -1) {
        //==================================Event setting =======================================
        QsnScheduleItem eventItem;
        if (itemEventEdit != -1 && itemEventEdit < events.count()) eventItem = events[itemEventEdit];
        else {
            eventItem.time = QTime::currentTime();
            eventItem.date = QDate::currentDate();
            eventItem.actions = "";
            eventItem.isdate = true;
            eventItem.daysweek = 31;
        }

        *contents << QsnBsPanelTitle(objectName() + " - " + ((itemEventEdit == -1)?tr("New task"):eventItem.name));
        *contents << QsnBsTapsBegin();
        *contents << QsnBsTapsTabAdd("trigger", tr("TIME"), true);
        *contents << QsnBsTapsTabAdd("actions", tr("ACTIONS"), false);
        *contents << QsnBsTapsPanesBegin();

        *contents << QsnBsTapsPanelBegin("trigger", true);

        *contents << QsnBsFormTextEdit(tr("Name"), eventItem.name, "nameedit", "text", false);

        *contents << QsnBsFormSelectorBegin(tr("Period"), "typeselect");
        *contents << QsnBsFormSelectorOption(tr("Week days"), "week", !eventItem.isdate);
        *contents << QsnBsFormSelectorOption(tr("Date"), "date", eventItem.isdate);
        *contents << QsnBsFormSelectorEnd();

        *contents << QsnBsFormTextEdit(tr("Time"), eventItem.time.toString("hh:mm"), "timeedit", "time", false);

        *contents << QsnBsFormHiddenBlockBegin("dateblock", !eventItem.isdate);
        *contents << QsnBsFormTextEdit(tr("Date"), eventItem.date.toString("yyyy-MM-dd"), "dateedit", "date", false);
        *contents << QsnBsFormHiddenBlockEnd();
        *contents << QsnBsFormHiddenBlockBegin("weekblock", eventItem.isdate);
        *contents << QsnBsFormWeeks(tr("Days"), eventItem.daysweek, "weekedit");
        *contents << QsnBsFormHiddenBlockEnd();


        *contents << QsnBsFormGroupButtonBegin();
        *contents << QsnBsFormGroupButtonLink(tr("Cancel"), "personal-cancel", QString("document.location.href='%1'").arg(widgetUrl) , "btn-secondary");
        if (itemEventEdit >= 0) *contents << QsnBsFormGroupButton(tr("Remove"), "removebtn", "btn-danger");
        *contents << QsnBsFormGroupButton(itemEventEdit >= 0?tr("Apply"):tr("Add"), "savebtn", "btn-primary");
        *contents << QsnBsFormGroupButtonEnd();


        *contents << QsnBsTapsPanelEnd();

        *contents << QsnBsTapsPanelBegin("actions", false);
        *contents << QString("<div class=\"form-group row\">"
                             "<div class=\"col-12 col-sm-9 col-xl-5\">"
                             "<input class=\"form-control custom-focus mb-3\" type=\"text\" value=\"\""
                             "placeholder='Enter search text' id=\"search\"></div></div>");
        *contents << QsnBsFormTableBegin();
        *contents << QsnBsFormTableBodyBegin("tableactions");
        for (int i = 0; i < mds->io->actions.count(); i ++) {
            *contents << QString("<tr class=\"noeven\">"
                                 "<td width=1% class=\"py-0 pe-0 \"><input type=\"checkbox\" name=\"action\" class=\"form-control\""
                                 "data-bs-toggle=\"%3\" %2></td>"
                                 "<td>%1</td></tr>")
                         .arg(getActionTitle(i), isContains(eventItem.actions, i)?" checked":"").arg(i);
        }

        *contents << QsnBsFormTableBodyEnd();
        *contents << QsnBsFormTableEnd();

        *contents << QsnBsTapsPanelEnd();
        return;
    }

    //================================== Event task panel =======================================

    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Time"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Date/Days"));
    *contents << QsnBsFormTableTheadAdd(tr("Actions"), QString());
    *contents << QsnBsFormTableTheadAddMD(tr("Run"));
    *contents << QString("<th class=\"text-right p-1 pe-2 align-middle col-1\" >");
    *contents << QString("<button type=\"button\" class=\"btn btn-primary btn-sm p-0\" "
                         "onclick=\"window.location.href='%1?event=-1';\">").arg(widgetUrl);
    *contents << QString("<svg class=\"icon-sprite icon-05x \"><use xlink:href=\"assets/images/icons-sprite.svg#subicon-add\" /></svg>");
    *contents << QString("</button>");
    *contents << QString("</th>");
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("scheduletable");

    for (int i = 0; i < events.count(); i ++) {
        *contents << QString("<tr%1>").arg(false?" class=\"success\"":"");
        *contents << QsnBsFormTableBodyRowCustomCell(events[i].time.toString("hh:mm"));
        *contents << QsnBsFormTableBodyRowCustomCell(events[i].isdate?events[i].date.toString("dd.MM.yyyy"):QSNDaysWeekToLine(events[i].daysweek));
        *contents << QsnBsFormTableBodyRowCustomCell(cardLabel(&events[i]));
        *contents << QsnBsFormTableBodyRowCustomCellMD((events[i].lastRun == QDateTime())?tr("It did not start"):tr("last") + ": " + events[i].lastRun.toString("dd.MM.yyyy hh:mm:ss"));
        *contents << QString("<td class=\"td-actions text-right p-0 pe-2 align-middle\">");
        *contents << QString("<button type=\"button\" class=\"btn btn-secondary btn-ms p-0\""
                             "onclick=\"window.location.href='%1?event=%2';\">").arg(widgetUrl).arg(i);
        *contents << QString("<svg class=\"icon-sprite icon-05x\">"
                             "<use xlink:href=\"assets/images/icons-sprite.svg#subicon-edit\" />"
                             "</svg>");
        *contents << QString("</button></td>");
        *contents << QsnBsFormTableBodyRowCustomEnd();
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsFormEnd();
}

void QsnWebPageSchedule::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("save"))  {
            QsnScheduleItem clockItem;
            QString vol = options->value("act");
            clockItem.actions = vol.replace("%3B", ";");
            clockItem.daysweek = static_cast<quint8>(options->value("weekdays").toUInt());
            clockItem.isdate = (options->value("type") == "date");
            clockItem.date = QDate::fromString(options->value("date"), "yyyy-MM-dd");
            vol = options->value("time");
            clockItem.time = QTime::fromString(vol.replace("%3A", ":"), "hh:mm");
            clockItem.name = QSNDecodeText(options->value("name"));
            int item = options->value("item").toInt();
            if (item != -1 && item < events.count()) events[item] = clockItem;
            else events.append(clockItem);
            saveSetting();
            *returnItems << QString("\"save\":true");
        }

        if (state == QLatin1String("remove"))  {
            int item = options->value("id").toInt();
            if (item != -1 && item < events.count()) events.removeAt(item);
            saveSetting();
        }
    }

    *returnItems << QString("\"schedules\": %1").arg(schedulesToJSON());
}

void QsnWebPageSchedule::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    // set options
    if (optionsMap.contains("Url"))  widgetUrl = optionsMap.value("Url").toString();

    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("ACT"))) {
            QsnWebItemScheduleAction *action = new QsnWebItemScheduleAction(static_cast<quint32>(id), modules(), this);
            action->fromStream(stream);
        }
    }
}

void QsnWebPageSchedule::urlChanged(int )
{  
    itemEventEdit = getValue("event", "-2").toInt();
}

void QsnWebPageSchedule::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role == QSNContainer::service && container.Command == 2) timeUpdate();
}

void QsnWebPageSchedule::endConfiguration()
{
    loadSetting();
}

void QsnWebPageSchedule::timeUpdate()
{
    if (QTime::currentTime() < nextTime) return;
    QTime currentTime = clearSec(QTime::currentTime());
    nextTime = currentTime.addSecs(60);

    for (int i = 0; i < events.count(); i ++)
        if (events[i].isdate) {
            if (clearSec(events[i].time) == currentTime && events[i].date == QDate::currentDate()) actionsBegin(&events[i]);
        } else {
            quint8 week = static_cast<quint8>(1 << (QDate::currentDate().dayOfWeek() - 1));
            if ((events[i].daysweek & week) && clearSec(events[i].time) == currentTime) actionsBegin(&events[i]);
        }
}

void QsnWebPageSchedule::saveSetting()
{
    QFile file(QSNHomeSubPath("server", "settings").absoluteFilePath(QString("schedules.xml")));
    if (!file.open(QIODevice::WriteOnly)) return;
    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement(QLatin1String("SchedulesSettings"));
    for (int i = 0; i < events.count(); i ++) {
        stream.writeStartElement("event");
        stream.writeAttribute("time", events[i].time.toString("hh:mm"));
        stream.writeAttribute("date", events[i].date.toString("dd:MM:yyyy"));
        stream.writeAttribute("isdate", events[i].isdate?"true":"false");
        stream.writeAttribute("week", QString::number(events[i].daysweek));
        stream.writeAttribute("name", events[i].name);
        stream.writeAttribute("actions", actionsIDsToNames(events[i].actions));
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
}

void QsnWebPageSchedule::loadSetting()
{
    QFile file(QSNHomeSubPath("server", "settings").absoluteFilePath(QString("schedules.xml")));
    if (!file.open(QIODevice::ReadOnly)) return;
    QXmlStreamReader::TokenType token;
    QXmlStreamReader xmlDoc(&file);
    QXmlStreamAttributes attrib;
    xmlDoc.readNext();
    events.clear();
    QsnScheduleItem eventItem;
    while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
        token = xmlDoc.readNext();
        if (token == QXmlStreamReader::StartElement && xmlDoc.name() == QLatin1String("SchedulesSettings")) {
            while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
                xmlDoc.readNext();
                if (xmlDoc.isStartElement() && xmlDoc.name() == QLatin1String("event")) {
                    attrib = xmlDoc.attributes();
                    eventItem.daysweek = static_cast<quint8>(attrib.value("week").toUInt());
                    eventItem.time = QTime::fromString(attrib.value("time").toString(), "hh:mm");
                    eventItem.date = QDate::fromString(attrib.value("date").toString(), "dd:MM:yyyy");
                    eventItem.isdate = ((attrib.value("isdate").toString() == "true"));
                    eventItem.name = attrib.value("name").toString();
                    eventItem.actions = actionsNamesToIDs(attrib.value("actions").toString());
                    eventItem.lastRun = QDateTime();
                    if (!eventItem.actions.isEmpty()) events.append(eventItem);
                    else  mds->interface->snBUSInput(QSNLogToContainer(
                                                         BUSSERV_LOG_LEVEL_warning,
                                                         tr("The task %1 was deleted due to lack of actions").arg(eventItem.name),
                                                         BUSSERV_LOG_CATEGORY_module,
                                                         QString("schedule"),
                                                         false), this);
                    while (xmlDoc.isEndElement()) xmlDoc.readNext();
                }
            }

        }
    }
    file.close();
}

QString QsnWebPageSchedule::getActionTitle(int index)
{
    if (index >= mds->io->actions.count()) return tr("no action");
    if (mds->io->actions[index].name.isEmpty()) return mds->io->actions[index].webwidget->getPath();
    return QString("%1-%2")
            .arg(mds->io->actions[index].webwidget->getPath(),
                 mds->io->actions[index].name);
}

QTime QsnWebPageSchedule::clearSec(QTime time)
{
    return QTime(time.hour(), time.minute(), 0, 0);
}

QString QsnWebPageSchedule::actionsIDsToNames(QString actions)
{
    QString ret = QString();
    QStringList list = actions.split(";");
    bool ok;
    quint16 index;
    for (int i = 0; i < list.count(); i++) {
        index = list.at(i).toUInt(&ok);
        if (!ok) continue;
        if (index >= mds->io->actions.count()) continue;
        ret += QString("%1-%2;")
                .arg(mds->io->actions[index].webwidget->objectName(),
                     mds->io->actions[index].name);
    }
    return ret;
}

QString QsnWebPageSchedule::actionsNamesToIDs(QString names)
{
    QString ret = QString();
    QStringList list = names.split(";");
    QString name;

    for (int a = 0; a < mds->io->actions.count(); a ++) {
        name = QString("%1-%2")
                .arg(mds->io->actions[a].webwidget->objectName()
                     ,mds->io->actions[a].name);
        for (int i = 0; i < list.count(); i++) {
            if (list.at(i) == name) {
                ret += QString::number(a) + ";";
                break;
            }
        }
    }
    return ret;
}

bool QsnWebPageSchedule::isContains(QString actions, int index)
{
    QString act = ";" + actions;
    return act.contains(";" + QString::number(index) + ";");
}

void QsnWebPageSchedule::actionsBegin(QsnScheduleItem *action)
{
    QStringList list = action->actions.split(";");
    bool ok;
    quint16 index;
    action->lastRun = QDateTime::currentDateTime();
    for (int i = 0; i < list.count(); i++) {
        index = list.at(i).toUInt(&ok);
        if (!ok) continue;
        if (index >= mds->io->actions.count()) continue;
        mds->io->runAction(index);
    }
}

QString QsnWebPageSchedule::cardLabel(QsnScheduleItem *action)
{
    QString ret;
    QStringList list = action->actions.split(";");
    bool ok;
    quint16 index;
    for (int i = 0; i < list.count(); i++) {
        index = list.at(i).toUInt(&ok);
        if (!ok) continue;
        if (index >= mds->io->actions.count()) continue;
        if (!ret.isEmpty()) {
            return ret + ",...";
        }
        ret += getActionTitle(index);
    }
    return ret;
}

QString QsnWebPageSchedule::schedulesToJSON()
{
    QString ret = "[";
    for (int i = 0; i < events.count(); i ++) {
        if (ret.count() > 1)ret += ",";
        ret += "{";
        ret += QString("\"time\": \"%1\",").arg(events[i].time.toString("hh:mm"));
        ret += QString("\"date\": \"%1\",").arg(events[i].isdate?events[i].date.toString("dd.MM.yyyy"):QSNDaysWeekToLine(events[i].daysweek));
        ret += QString("\"events\": \"%1\",").arg(cardLabel(&events[i]));
        ret += QString("\"lastrun\": \"%1\",").arg((events[i].lastRun == QDateTime())?tr("It did not start"):tr("last") + ": " + events[i].lastRun.toString("dd.MM.yyyy hh:mm:ss"));
        ret += QString("\"warning\": \"%1\",").arg(false?"w":"n");
        ret += QString("\"link\": \"%1\"").arg(i);
        ret += "}";
    }
    ret += "]";
    return ret;
}




