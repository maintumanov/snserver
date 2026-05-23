#include "qsnwebpagelog.h"

QsnWebPageLog::QsnWebPageLog(quint32 iID, QsnGlobalModules *modules, QObject *parent)
    : QsnWeb(iID, modules, parent)
{
    mds = modules;
    mds->isEnableLog = true;
    setObjectName(tr("Logs"));
    widgetUrl = "/logs";
    widgetIcon = "subicon-log";
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebPageLog::getFunctions(QStringList *functions, int , QString pageURL)
{

    if (logname == QString()) return;

    *functions << QString("var logfilesize = %1;").arg(logFileSize());

    *functions << QString("function tableadd(data) {"
                          "var table = $(\"#tablelog\");"
                          "$.each(data, function (i, item) {"
                          "var levstyle = \"\";"
                          "if (item.lv == 0) levstyle = \"  class='caution'\";"
                          "if (item.lv == 1) levstyle = \"  class='warning'\";"
                          "table.prepend(\"<tr\" + levstyle + \">\" +"
                          "\"<td>\" + item.tm + \"</td>\" +"
                          "\"<td colspan='2'>\" + item.msg + \"</td></tr>\");"
                          "});"
                          "var rowCount = $(\"#logtable tr\").length;"
                          "while (rowCount > 200) {"
                          "$('#logtable tr:last').remove();"
                          "rowCount --; }"
                          "}");

    *functions << QString("function getlog(){"
                          " $.ajax({"
                          "url: \"%1\","
                          "type: 'POST',"
                          "data: {\"action\": \"action\", \"id\": \"getlog\", \"logfilesize\": logfilesize},"
                          "contentType: \"application/json; charset=utf-8\","
                          "dataType: 'json',"
                          "success: function(data) {"
                          "logfilesize = data.logfilesize;"
                          "tableadd(data.diff);"
                          "}, "
                          "error: function (x, t, e) { } }); "
                          "};").arg(pageURL);

    *functions << QString("$(\"#logclear\").click(function(){"
                          "sendState('action', 'logclear', 'clear');"
                          "window.location.replace('%1');"
                          "}); ").arg(widgetUrl);
}

void QsnWebPageLog::getFunctionsJSON(QStringList *fjson, int )
{
    if (logname == QString()) return;
    *fjson << QString("if (logfilesize > data.logfilesize) location.reload();");
    *fjson << QString("if (logfilesize < data.logfilesize) getlog();");
    *fjson << QString("logfilesize = data.logfilesize;");
}

void QsnWebPageLog::getContents(QStringList *contents, int )
{

    if (logname != QString()) {
        QFile file(QSNHomeSubPath("server", "logs").absoluteFilePath(QString("%1.log").arg(logname)));
        if (file.exists()) {
            QString line;
            int level = 0;
            int icount = 0;
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream log(&file);
                QString l;
                *contents << QsnBsPanelTitle(QString("%1 - %2").arg(tr("Log"), log.readLine()));
                *contents << QString("<div class=\"col-12\">");
                *contents << QString("<table id=\"logtable\" class=\"table\">");
                *contents << QsnBsFormTableTheadBegin();
                *contents << QsnBsFormTableTheadAdd(tr("Time"), "12rem");
                *contents << QsnBsFormTableTheadAdd(tr("Message"));
                *contents << QString("<th class=\"text-right p-1 pe-2 align-middle col-1\" >");
                *contents << QString("<button id=\"%1\" type=\"button\" class=\"btn btn-danger btn-sm p-0 ms-auto\" >").arg("logclear");
                *contents << QString("<svg class=\"icon-sprite icon-05x \"><use xlink:href=\"assets/images/icons-sprite.svg#subicon-delete\" /></svg>");
                *contents << QString("</button>");
                *contents << QString("</th>");

                *contents << QsnBsFormTableTheadEnd();
                *contents << QsnBsFormTableBodyBegin("tablelog");

                QString timeLine;
                int i;
                while (!log.atEnd() && icount < 200) {
                    line = log.readLine();
                    if (line.count() < 24) continue;
                    icount ++;
                    level = QSNWarningLevelFromText(line);
                    i = line.indexOf("]") + 1;
                    timeLine = line.left(i - 1);
                    line.remove(0, i);
                    i = timeLine.indexOf("[") + 1;
                    if (i != -1) timeLine.remove(0, i);

                    l.prepend(QsnBsFormTableBodyRowLogAdd(level,
                                                          timeLine,
                                                          QSNEscapingHTMLCharacters(QSNGetWarninMSGBody(line))));
                }
                contents->append(l);
                *contents << QsnBsFormTableBodyEnd();
                *contents << QsnBsFormTableEnd();
                *contents << QString("</div>");
            }
        } else {
            *contents << QsnBsPanelTitle(tr("The log is missing!"));
        }
    } else {
        *contents << QsnBsPanelTitle(objectName());
        QStringList listFiles = QSNHomeSubPath("server", "logs").entryList(QString("*.log").split(","), QDir::Files);
        if (listFiles.count() == 0)  *contents << QsnBsPanelTitle(tr("Logs not found!"));
        else {
            *contents << QString("<div class=\"col-12 \">");
            *contents << QString("<table class=\"table clickable\">");
            *contents << QsnBsFormTableTheadBegin();
            *contents << QsnBsFormTableTheadAddCl(tr("Name"));
            *contents << QsnBsFormTableTheadAddCl(tr("Last change"));
            *contents << QsnBsFormTableTheadEnd();
            *contents << QsnBsFormTableBodyBegin(QString("table_logs"));

            QList<QPair<QString,QDateTime>> lf;
            QStringListIterator i(listFiles);
            QDateTime ldt;
            QString content;

            while (i.hasNext()) {
                content = getListItem(i.next(), &ldt);
                lf.append(qMakePair(content,ldt));
            }

            qSort(lf.begin(), lf.end(), QPairSecondComparer());

            for (int i = 0; i < lf.count(); i ++ ) *contents << lf[i].first;

            *contents << QsnBsFormTableBodyEnd();
            *contents << QsnBsFormTableEnd();
            *contents << QString("</div>");
        }
    }
}

QString QsnWebPageLog::getListItem(QString fname, QDateTime *date)
{
    QFile file(QSNHomeSubPath("server", "logs").absoluteFilePath(QString("%1").arg(fname)));
    QString name;
    QString ret;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream log(&file);
        QFileInfo info(file);
        if (!log.atEnd()) name = log.readLine();
        if (!name.isEmpty()) {
            *date = info.lastModified();
            ret = QString("<tr class=\"clickable\" "
                          "onclick=\"window.location='%1?name=%2'\">").arg(widgetUrl, fname.remove(fname.lastIndexOf("."), 4));
            ret += QString("<td>%1</td>").arg(name);
            ret += QString("<td>%1</td>").arg(info.lastModified().toString("yyyy.MM.dd-hh:mm:ss"));
            ret += QString("</tr>");
        }
    } else return QString();
    file.close();
    return ret;
}

void QsnWebPageLog::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid)
{
    if (!mds->auth->checkAccesFromUID(uid, widgetAccessRights)) return;

    *returnItems << QString("\"logfilesize\": %1").arg(logFileSize());
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("action"))   {
            if (options->value("id", "") == "logclear") {
                QFile file(QSNHomeSubPath("server", "logs").absoluteFilePath(QString("%1.log").arg(logname)));
                if (file.exists()) {
                    file.remove();
                }

            }

            if (options->value("id", "") == "getlog") {
                *returnItems << QString("\"diff\": ") + logDifferenceJson(options->value("logfilesize", "").toLong());
            }
        }

    }
}

void QsnWebPageLog::urlChanged(int )
{
    logname = QString();
    QString logreturnurl;
    if (isKey("name")) {
        logname = getValue("name");
    } else {
        if (isKey("logenable")) {
            if (getValue("logenable") == "enable") mds->isEnableLog = true;
            else  mds->isEnableLog = false;
            mds->settings->setValue(QString("logEnable"), mds->isEnableLog);
        }

    }
    if (isKey("return")) logreturnurl = getValue("return");
}

qint64 QsnWebPageLog::logFileSize()
{
    if (logname == QString()) return 0;
    QFile file(QSNHomeSubPath("server", "logs").absoluteFilePath(QString("%1.log").arg(logname)));
    if (file.exists()) return file.size();
    return 0;
}

QString QsnWebPageLog::logDifferenceJson(quint64 start)
{
    QString ret = "[";
    QFile file(QSNHomeSubPath("server", "logs").absoluteFilePath(QString("%1.log").arg(logname)));
    if (file.exists()) {
        QString line;
        int level = 0;
        int icount = 0;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream log(&file);
            log.seek(start);
            QString timeLine;
            int i;
            while (!log.atEnd() && icount < 200) {
                line = log.readLine();
                if (line.count() < 24) continue;
                icount ++;
                level = QSNWarningLevelFromText(line);
                i = line.indexOf("]") + 1;
                timeLine = line.left(i - 1);
                line.remove(0, i);
                i = timeLine.indexOf("[") + 1;
                if (i != -1) timeLine.remove(0, i);
                ret += "{";
                ret += QString("\"lv\":\"%1\",").arg(level);
                ret += QString("\"tm\":\"%1\",").arg(timeLine);
                ret += QString("\"msg\":\"%1\"").arg(QSNEscapingHTMLCharacters(QSNGetWarninMSGBody(line)));
                ret += "},";

            }
        }
    }
    if (ret.right(1) == ',') ret.remove(ret.count() - 1, 1);
    ret += "]";
    return ret;
}

void QsnWebPageLog::snBUSInput(QSNContainer container, QObject *sender)
{
    if (container.role == QSNContainer::service && container.Command == BUSSERV_LOG && !container.Address) {
        if (!container.Data.isEmpty())
            mds->db->writeLog(QSNWarningLevelToText(container.Signal) + container.info,
                              container.Data, sender->objectName());
    }

    if (container.role == QSNContainer::information && container.Command) {
        if (container.Sender == 0) {
            QStringList options = QString::fromUtf8(container.Data).split("/");
            if (options.count() == 1)
                mds->db->writeLog(QSNWarningLevelToText(container.Command) + container.info,
                                  options[0], sender->objectName());
            else
                mds->db->writeLog(QSNWarningLevelToText(container.Command) + container.info,
                                  options[0], options[1]);
        } else {
            QString dlabel = QString::fromUtf8(container.Data);
            if (dlabel.isEmpty()) dlabel = tr("Unknown device");
            mds->db->writeLog(QSNWarningLevelToText(container.Command) + container.info,
                              QString("devicelog_a%1").arg(container.Sender), dlabel);
        }
    }
}




