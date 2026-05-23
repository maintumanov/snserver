#include "qsnwebpageabout.h"

QsnWebPageAbout::QsnWebPageAbout(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    widgetUrl = "/about";
    widgetIcon = "info";
    setObjectName(tr("About"));
}

void QsnWebPageAbout::getFunctions(QStringList *functions, int , QString )
{
    //    *functions << QString(
    //                      "actionUpdate = function() {"
    //                      "dojo.xhrPost({"
    //                      "url: \"%1\", "
    //                      "postData: {action : 'action' , id : 'about' , state : 'reload'}, "
    //                      "handleAs: \"json\", "
    //                      "timeout: %2, "
    //                      "load: function(response, ioArgs) {},"
    //                      "error: function(response, ioArgs) {"
    //                      "showERRconnect();"
    //                      "return response; }}); };"
    //                      )
    //                  .arg(widgetUrl())
    //                  .arg(mds->timeOut);

    *functions << QString("function tablestat(data) {"
                          "var table = $(\"#tablestat\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td>\" + item.pr + \"</td>\" +"
                          "\"<td>\" + item.vl + \"</td></tr>\");"
                          "});}");
}

void QsnWebPageAbout::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" tablestat(data.stat);");
}

void QsnWebPageAbout::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("request"))  {
            *returnItems << QString("\"stat\": %1").arg(statToJSON());
        }
        if (state == QLatin1String("action"))   {

            QString id;
            if (options->contains("id")) id = options->value("id");
            if (id == "about")
                if (options->contains("state")) {
                    QString state = options->value("state");
                    if (state == QLatin1String("reload")) {
                        mds->interface->snBUSInput(QSNLogToContainer(
                                                       BUSSERV_LOG_LEVEL_caution,
                                                       tr("Begin reload configuration"),
                                                       BUSSERV_LOG_CATEGORY_core,
                                                       QString("server"),
                                                       false), this);
                        emit reload();
                    }
                }
        }
    }
}

void QsnWebPageAbout::getContents(QStringList *contents, int )
{
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("about", tr("ABOUT"), true);
    *contents << QsnBsTapsTabAdd("stat", tr("SATISTIC"), false);
    *contents << QsnBsTapsPanesBegin();

    *contents << QsnBsTapsPanelBegin("about", true);

    *contents << QsnBsAlertBegin();
    *contents << QString("<h4>%1 %2</h4>"
                         "%3<br>"
                         "email: <a transition='none' href=\"mailto:maintumanov@mail.ru\">maintumanov@mail.ru</a><br>"
                         "© 2014-%4 The SignalNet company.<br>%5<br>")
                 .arg(QCoreApplication::applicationName())
                 .arg(QCoreApplication::applicationVersion())
                 .arg(tr("Author: Tumanov Stanislav Aleksandrovich"))
                 .arg(QDate::currentDate().year())
                 .arg(tr("The program is provided AS IS with NO WARRANTY OF ANY KIND, "
                         "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
                         "FOR A PARTICULAR PURPOSE."));
    *contents << QsnBsAlertEnd();

    *contents << QsnBsTapsPanelEnd();


    *contents << QsnBsTapsPanelBegin("stat", false);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Parameters"));
    *contents << QsnBsFormTableTheadAdd(tr("Meaning"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tablestat");
    *contents << QsnBsFormTableBodyRowAdd(tr("The web request, minimum time"),
                                          QString::number(static_cast<double>(mds->S_PTWR_MIN) / 1000, 'f', 3));
    *contents << QsnBsFormTableBodyRowAdd(tr("The web request, maximum"),
                                          QString::number(static_cast<double>(mds->S_PTWR_MAX) / 1000, 'f', 3));
    *contents << QsnBsFormTableBodyRowAdd(tr("The web number of requests"), QString::number(mds->S_PTWR_COUNT));
    *contents << QsnBsFormTableBodyRowAdd(tr("The current time"), QTime::currentTime().toString("hh:mm:ss"));
    *contents << QsnBsFormTableBodyRowAdd(tr("The current date"), QDate::currentDate().toString("dd.MM.yyyy"));
    *contents << QsnBsFormTableBodyRowAdd(tr("Operating time"), workTime());

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
}

void QsnWebPageAbout::fromStream(QDataStream *stream)
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

QString QsnWebPageAbout::widgetState()
{
    return QString("%1 %2").arg(tr("version")).arg(QCoreApplication::applicationVersion());
}

QString QsnWebPageAbout::workTime()
{
    quint64 s = static_cast<quint64>(mds->S_TM_BEGIN.secsTo(QDateTime::currentDateTime()));
    quint64 day = s / 86400;
    s = s - day * 3600;
    quint64 hour = s / 3600;
    s = s - hour * 3600;
    quint64 min = s / 60;
    s = s - min * 60;
    return QString("%1 %2, %3").arg(day).arg(tr("days"), QTime(static_cast<int>(hour), static_cast<int>(min), static_cast<int>(s)).toString("hh:mm:ss"));
}

QString QsnWebPageAbout::statToJSON()
{
    QString ret = "[";
    ret += "{";
    ret += QString("\"pr\": \"%1\",").arg(tr("The web request, minimum time"));
    ret += QString("\"vl\": \"%1\"").arg(static_cast<double>(mds->S_PTWR_MIN) / 1000, 0, 'f', 3);
    ret += "},";

    ret += "{";
    ret += QString("\"pr\": \"%1\",").arg(tr("The web request, maximum"));
    ret += QString("\"vl\": \"%1\"").arg(static_cast<double>(mds->S_PTWR_MAX) / 1000, 0, 'f', 3);
    ret += "},";

    ret += "{";
    ret += QString("\"pr\": \"%1\",").arg(tr("The web number of requests"));
    ret += QString("\"vl\": \"%1\"").arg(mds->S_PTWR_COUNT);
    ret += "},";

    ret += "{";
    ret += QString("\"pr\": \"%1\",").arg(tr("The current time"));
    ret += QString("\"vl\": \"%1\"").arg( QTime::currentTime().toString("hh:mm:ss"));
    ret += "},";

    ret += "{";
    ret += QString("\"pr\": \"%1\",").arg(tr("The current date"));
    ret += QString("\"vl\": \"%1\"").arg(QDate::currentDate().toString("dd.MM.yyyy"));
    ret += "},";

    ret += "{";
    ret += QString("\"pr\": \"%1\",").arg(tr("Operating time"));
    ret += QString("\"vl\": \"%1\"").arg(workTime());
    ret += "}";

    ret += "]";
    return ret;
}

