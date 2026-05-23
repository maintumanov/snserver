#include "qsnwebpagegraph.h"

QsnWebPageGraph::QsnWebPageGraph(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Charts"));
    widgetUrl = "/graph";
    widgetIcon = "subicon-chart";
    dbName = QString();
    isminmax = false;
    iscolumns = false;
    issmoothing = true;
    iswithoutBreaks = false;
    isupperBound = false;
    islowerBound = false;
    upperBound = 0;
    lowerBound = 0;
    upperBoundPrecision = 0;
    lowerBoundPrecision = 0;
    defaultMinuts = 1440;// 24 hours
    maxColor = "#F78773";
    minColor = "#73B87A";
    ubColor = "#964b00";
    lbColor = "#8b00ff";
    defaultColor = "#455187";
    precision = 0;
}

QsnWebPageGraph::~QsnWebPageGraph()
{

}

void QsnWebPageGraph::getJSDepending(QStringList *depending, int )
{
    *depending << "assets/js/apexcharts.min.js";
}

void QsnWebPageGraph::getCSSDepending(QStringList *depending, int )
{
    *depending << "assets/css/apexcharts.css";
}

void QsnWebPageGraph::getHTMLOnLoad(QStringList *functions, int )
{
    *functions << QString("chart.render();");
}

void QsnWebPageGraph::getFunctions(QStringList *functions, int , QString)
{

    if (dbName.isEmpty()) return;
    // Chart configuration object
    *functions << QString("var options={chart:{"
                          "animations:{},"
                          "toolbar:{show: true,offsetX: 0,offsetY: 0,"
                          "tools: {download:true,selection:true,zoom:true,zoomin:true,zoomout:true,pan:false,reset: true,customIcons: []},"
                          "export: {csv: {filename: undefined,columnDelimiter: ';',headerCategory: '%1',headerValue: 'data',"
                          "dateFormatter(timestamp) {return new Date(timestamp).toDateString()}},"
                          "svg: {filename: undefined,},png: {filename: undefined,}},"
                          "autoSelected: 'zoom'},},").arg(tr("Time"));
    //    *functions << QString("title: {text: '%1',align: 'left',offsetX: 14},").arg(dbLabel);
    *functions << QString("series: ");
    *functions << getChartSeries(dbName, beginDT, endDT, isminmax, uBound, lBound, iswithoutBreaks);
    *functions << QString(",");

    *functions << QString("xaxis: {"
                          "type: 'datetime',"
                          "tickAmount: 6,"
                          "labels: {rotate: -15,rotateAlways: false,"
                          "formatter: function (value) {var d = new Date(value);"
                          "return "
                          "d.toISOString().slice(0,10).replace(/-/g,\".\")"
                          " + \" \" + (\"0\" + d.getHours()).slice(-2)"
                          " + \":\" + (\"0\" + d.getMinutes()).slice(-2);}"
                          "}},");
    *functions << QString("stroke: {curve: 'smooth',width: 3,},"
                          "dataLabels: {enabled: false},"
                          //                          "fill: {type: 'gradient',"
                          //                          "gradient: {type: \"vertical\",inverseColors: false,opacityFrom: 0.4,opacityTo: 0.05,"
                          //                          "stops: [20, 100, 100, 100]},},"
                          "");
    *functions << QString("grid: {row: {colors: ['#f3f3f3', 'transparent'],opacity: 0.5}, column: {colors: ['#f8f8f8', 'transparent'],opacity: 0.5}, "
                          "xaxis: {lines: {show: true}}},");
    *functions << QString("legend: {position: 'top'},");

    *functions << QString("}; ");
    *functions << QString("var chart = new ApexCharts(document.querySelector(\"#chart\"), options);");

    *functions << QString("$(\"#update\").click(function(){chartupdate();});");

    // Manual update button handler
    *functions << QString("function chartupdate(){"
                          " $.ajax({"
                          "url: \"%1\","
                          "type: 'POST',"
                          "dataType: 'json', "
                          "contentType: 'application/json; charset=utf-8',"
                          "data: {"
                          "action : 'updateGraph', ").arg(widgetUrl);
    *functions << QString("name : '%1', ").arg(dbName);
    *functions << QString("ubound : '%1', ").arg(uBound);
    *functions << QString("lbound : '%1', ").arg(lBound);
    *functions << QString("minmax : '%1', ").arg(isminmax);
    *functions << QString("wbr : '%1', ").arg(iswithoutBreaks);
    *functions << QString("timebegin: $('#timebegin').val(),"
                          "datebegin: $('#datebegin').val(), "
                          "timeend: $('#timeend').val(), "
                          "dateend: $('#dateend').val(), ");
    *functions << QString("},"
                          "success: function(data) {"
                          "if (data.hasOwnProperty('series') == true) {"
                          //                          "console.log(\"chart update\");"
                          "chart.updateSeries(data.series);"
                          "}"
                          " }, "
                          "error: function (x, t, e) { } }); "
                          "};");

    // Quick period selector (today, week, month, etc.)
    *functions << QString(
                      "function setPeriod(period) {"
                      "  const now = new Date();"
                      "  let begin, end;"
                      "  switch (period) {"
                      "    case 'today':"
                      "      begin = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 0, 0, 0);"
                      "      end = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 23, 59, 59);"
                      "      break;"
                      "    case 'yesterday':"
                      "      begin = new Date(now.getFullYear(), now.getMonth(), now.getDate() - 1, 0, 0, 0);"
                      "      end = new Date(now.getFullYear(), now.getMonth(), now.getDate() - 1, 23, 59, 59);"
                      "      break;"
                      "    case 'week':"
                      "      const day = now.getDay();"
                      "      const diff = now.getDate() - day + (day === 0 ? -6 : 1);"
                      "      const monday = new Date(now.getFullYear(), now.getMonth(), diff);"
                      "      begin = new Date(monday.getFullYear(), monday.getMonth(), monday.getDate(), 0, 0, 0);"
                      "      end = new Date(monday.getFullYear(), monday.getMonth(), monday.getDate() + 6, 23, 59, 59);"
                      "      break;"
                      "    case 'month':"
                      "      begin = new Date(now.getFullYear(), now.getMonth(), 1, 0, 0, 0);"
                      "      end = new Date(now.getFullYear(), now.getMonth() + 1, 0, 23, 59, 59);"
                      "      break;"
                      "    case 'year':"
                      "      begin = new Date(now.getFullYear(), 0, 1, 0, 0, 0);"
                      "      end = new Date(now.getFullYear(), 11, 31, 23, 59, 59);"
                      "      break;"
                      "    default:"
                      "      return;"
                      "  }"
                      "  const pad = n => (n < 10 ? '0' : '') + n;"
                      "  const formatDate = d => d.getFullYear() + '-' + pad(d.getMonth() + 1) + '-' + pad(d.getDate());"
                      "  const formatTime = d => pad(d.getHours()) + ':' + pad(d.getMinutes());"
                      "  const updateInput = (id, value) => {"
                      "    const el = document.getElementById(id);"
                      "    if (el) el.value = value;"
                      "  };"
                      "  updateInput('datebegin', formatDate(begin));"
                      "  updateInput('timebegin', formatTime(begin));"
                      "  updateInput('dateend', formatDate(end));"
                      "  updateInput('timeend', formatTime(end));"
                      "if (typeof chartupdate === 'function') chartupdate();"
                      "}");
}

QString QsnWebPageGraph::getChartSeries(QString name, QDateTime begin, QDateTime end, bool maxmin, QString upbound, QString lwbound, bool wbreaks)
{
    if (!mds || !mds->db) {
        qWarning() << "QsnWebPageGraph: mds or db is null";
        return "[{\"name\":\"Error\",\"data\":[]}]";
    }

    QString series = "[";

    QsnDB::dbSeries data;
    QsnDB::dbSeries dataUB;
    QsnDB::dbSeries dataLB;

    bool oisUpperBound = !upbound.isEmpty();
    double oLevelUpperBound = 0;
    QString oDbUpperBound;
    bool oisLowerBound = !lwbound.isEmpty();
    double oLevelLowerBound = 0;
    QString oDbLowerBound;

    QString vol;

    // Parse bound definitions
    if (oisUpperBound) {
        bool ok = false;
        oLevelUpperBound = upbound.toDouble(&ok);
        if (!ok) oDbUpperBound = upbound;
    }

    if (oisLowerBound) {
        bool ok = false;
        oLevelLowerBound = lwbound.toDouble(&ok);
        if (!ok) oDbLowerBound = lwbound;
    }

    // Load primary data
    data.begin = begin;
    data.end = end;
    data.name = name;
    data.withoutBreaks = wbreaks;

    spanGraph(&data);
    mds->db->typeYBD(&data);
    mds->db->requestDataFromYBD(&data);
    dbLabel = data.label;

    quint8 scale = QSNScalingIndex(data.type, data.max);

    // Load dynamic upper bound if needed
    if (oisUpperBound && !oDbUpperBound.isEmpty()) {
        dataUB.begin = data.begin;
        dataUB.end = data.end;
        dataUB.name = oDbUpperBound;
        dataUB.withoutBreaks = data.withoutBreaks;
        dataUB.seriesCount = data.seriesCount;
        mds->db->typeYBD(&dataUB);
        mds->db->requestDataFromYBD(&dataUB);
    }

    // Load dynamic lower bound if needed
    if (oisLowerBound && !oDbLowerBound.isEmpty()) {
        dataLB.begin = data.begin;
        dataLB.end = data.end;
        dataLB.name = oDbLowerBound;
        dataLB.withoutBreaks = data.withoutBreaks;
        dataLB.seriesCount = data.seriesCount;
        mds->db->typeYBD(&dataLB);
        mds->db->requestDataFromYBD(&dataLB);
    }

    // Main data series
    vol = "\"\"";
    series += QString("{\"name\":\"%1\",\"type\":\"line\",").arg(data.label);
    series += QLatin1String("\"data\":[");
    for (int v = 0; v < data.series.count(); v ++) {
        if (data.series[v].used) vol = QSNVariantToScaledNumberString(data.series[v].vol, scale, data.type);
        series += QString("[%1,%2]")
                .arg(data.series[v].dateTime.toMSecsSinceEpoch())
                .arg(data.series[v].used?vol:(wbreaks?vol:"\"\""));
        if (v != data.series.count() - 1) series += QString(",");
    }
    series += QLatin1String("]}");

    // Lower bound series
    if (oisLowerBound) {
        series += QString(", {\"name\":\"%1\",\"type\":\"line\",").arg(oDbLowerBound.isEmpty()?tr("Lower"):dataLB.label);

        //data
        series += QLatin1String("\"data\":[");
        if (oDbLowerBound.isEmpty()){
            QString SUB = QString("%1").arg(oLevelLowerBound, 0, 'f', data.precision);
            for (int v = 0; v < data.series.count(); v ++) {
                series += QString("[%1,%2]").arg(data.series[v].dateTime.toMSecsSinceEpoch()).arg(SUB);
                if (v != data.series.count() - 1) series += QString(",");
            }
        } else {
            vol = "\"\"";
            for (int v = 0; v < dataLB.series.count(); v ++) {
                if (dataLB.series[v].used) vol = QSNVariantToScaledNumberString(dataLB.series[v].vol, scale, data.type);
                series += QString("[%1,%2]")
                        .arg(dataLB.series[v].dateTime.toMSecsSinceEpoch())
                        .arg(dataLB.series[v].used?vol:(wbreaks?vol:"\"\""));
                if (v != dataLB.series.count() - 1) series += QString(",");
            }
        }
        series += QLatin1String("]}");
    }

    // Upper bound series
    if (oisUpperBound) {
        series += QString(", {\"name\":\"%1\",\"type\":\"line\",").arg(oDbUpperBound.isEmpty()?tr("Upper"):dataUB.label);

        //data
        series += QLatin1String("\"data\":[");
        if (oDbUpperBound.isEmpty()){
            QString SUB = QString("%1").arg(oLevelUpperBound, 0, 'f', data.precision);
            for (int v = 0; v < data.series.count(); v ++) {
                series += QString("[%1,%2]").arg(data.series[v].dateTime.toMSecsSinceEpoch()).arg(SUB);
                if (v != data.series.count() - 1) series += QString(",");
            }
        } else {
            vol = "\"\"";
            for (int v = 0; v < dataUB.series.count(); v ++) {
                if (dataUB.series[v].used) vol = QSNVariantToScaledNumberString(dataUB.series[v].vol, scale, data.type);
                series += QString("[%1,%2]")
                        .arg(dataUB.series[v].dateTime.toMSecsSinceEpoch())
                        .arg(dataUB.series[v].used?vol:(wbreaks?vol:"\"\""));
                if (v != dataUB.series.count() - 1) series += QString(",");
            }
        }
        series += QLatin1String("]}");
    }

    // Min/Max envelope series
    if (maxmin) {
        series += QString(",{\"name\":\"%1\",\"type\":\"area\",").arg(tr("max"));
        //data
        vol = "\"\"";
        series += QLatin1String("\"data\":[");
        for (int v = 0; v < data.series.count(); v ++) {
            if (data.series[v].used) vol = QSNVariantToScaledNumberString(data.series[v].max, scale, data.type);
            series += QString("[%1,%2]")
                    .arg(data.series[v].dateTime.toMSecsSinceEpoch())
                    .arg(data.series[v].used ? vol : (wbreaks ? vol : "null"));
            if (v != data.series.count() - 1) series += QString(",");
        }
        series += QLatin1String("]}");

        series += QString(",{\"name\":\"%1\",\"type\":\"area\",").arg(tr("min"));
        //data
        vol = "\"\"";
        series += QLatin1String("\"data\":[");
        for (int v = 0; v < data.series.count(); v ++) {
            if (data.series[v].used) vol = QSNVariantToScaledNumberString(data.series[v].min, scale, data.type);
            series += QString("[%1,%2]")
                    .arg(data.series[v].dateTime.toMSecsSinceEpoch())
                    .arg(data.series[v].used ? vol : (wbreaks ? vol : "null"));
            if (v != data.series.count() - 1) series += QString(",");
        }
        series += QLatin1String("]}");
    }

    series += QLatin1String("]");
    return series;

}

void QsnWebPageGraph::getContents(QStringList *contents, int )
{
    if (!dbName.isEmpty()) {
        // === Detailed Chart View ===
        *contents << QString("<div class=\"col-12 mb-4\">");
        *contents << QString("  <div class=\"card bg-white shadow-sm\">");

        *contents << QString("    <div class=\"card-header bg-light fw-bold\">%1</div>")
                     .arg(objectName() + " - " + dbLabel);
        *contents << QString("    <div class=\"card-body p-0\">");
        *contents << QString("      <div class=\"col-12\" id=\"chart\" style=\"min-height: 400px;\"></div>");
        *contents << QString("    </div>");
        *contents << QString("  </div>");
        *contents << QString("</div>");


        // === Control Panel ===
        *contents << QString("<div class=\"col-12 mb-4\">");
        *contents << QString("  <div class=\"card bg-white shadow-sm\">");
        *contents << QString("    <div class=\"card-header bg-light fw-bold\">%1</div>").arg(tr("Time Range"));
        *contents << QString("    <div class=\"card-body py-4 px-4\">"); // ← увеличенные отступы: py-4, px-4

        // Кнопки быстрого выбора — добавим больше отступа снизу
        *contents << QString("      <div class=\"row mb-4\">");
        *contents << QString("        <div class=\"col text-center\">");
        *contents << QString("          <div class=\"btn-group\" role=\"group\">");
        *contents << QString("            <button type=\"button\" class=\"btn btn-secondary btn-sm\" onclick=\"setPeriod('today')\">%1</button>").arg(tr("Today"));
        *contents << QString("            <button type=\"button\" class=\"btn btn-secondary btn-sm\" onclick=\"setPeriod('yesterday')\">%1</button>").arg(tr("Yesterday"));
        *contents << QString("            <button type=\"button\" class=\"btn btn-secondary btn-sm\" onclick=\"setPeriod('week')\">%1</button>").arg(tr("Week"));
        *contents << QString("            <button type=\"button\" class=\"btn btn-secondary btn-sm\" onclick=\"setPeriod('month')\">%1</button>").arg(tr("Month"));
        *contents << QString("            <button type=\"button\" class=\"btn btn-secondary btn-sm\" onclick=\"setPeriod('year')\">%1</button>").arg(tr("Year"));
        *contents << QString("          </div>");
        *contents << QString("        </div>");
        *contents << QString("      </div>");
        // Begin/End inputs
        *contents << QString("      <div class=\"row mb-4 align-items-center\">");
        *contents << QString("        <label class=\"col-sm-2 col-form-label text-sm-end\">%1</label>").arg(tr("Begin"));
        *contents << QString("        <div class=\"col-sm-5\">");
        *contents << QString("          <input type=\"date\" class=\"form-control\" id=\"datebegin\" value=\"%1\">").arg(beginDT.toString("yyyy-MM-dd"));
        *contents << QString("        </div>");
        *contents << QString("        <div class=\"col-sm-5\">");
        *contents << QString("          <input type=\"time\" class=\"form-control\" id=\"timebegin\" value=\"%1\">").arg(beginDT.toString("hh:mm"));
        *contents << QString("        </div>");
        *contents << QString("      </div>");

        // End group — mb-4
        *contents << QString("      <div class=\"row mb-4 align-items-center\">");
        *contents << QString("        <label class=\"col-sm-2 col-form-label text-sm-end\">%1</label>").arg(tr("End"));
        *contents << QString("        <div class=\"col-sm-5\">");
        *contents << QString("          <input type=\"date\" class=\"form-control\" id=\"dateend\" value=\"%1\">").arg(endDT.toString("yyyy-MM-dd"));
        *contents << QString("        </div>");
        *contents << QString("        <div class=\"col-sm-5\">");
        *contents << QString("          <input type=\"time\" class=\"form-control\" id=\"timeend\" value=\"%1\">").arg(endDT.toString("hh:mm"));
        *contents << QString("        </div>");
        *contents << QString("      </div>");

        // Кнопка Update — добавим отступ сверху
        *contents << QString("      <div class=\"row mt-3\">"); // ← mt-3 для отступа сверху
        *contents << QString("        <div class=\"col text-center\">");
        *contents << QString("          <button id=\"update\" class=\"btn btn-primary px-5 py-2\">%1</button>").arg(tr("Update")); // ← px-5 для ширины кнопки
        *contents << QString("        </div>");
        *contents << QString("      </div>");

        *contents << QString("    </div>"); // card-body
        *contents << QString("  </div>");   // card
        *contents << QString("</div>");     // col
    } else {
        // === Graph List View ===
        QsnDB::dbFileList list;
        mds->db->listYBD(&list);

        *contents << QString("<div class=\"col-12 mb-4\">");
        *contents << QString("  <div class=\"card bg-white shadow-sm\">");
        *contents << QString("    <div class=\"card-header bg-light fw-bold\">%1</div>").arg(tr("Available Graphs"));
        *contents << QString("    <div class=\"card-body p-0\">");

        if (list.items.isEmpty()) {
            *contents << QString("      <div class=\"text-center py-4 text-muted\">%1</div>").arg(tr("No graphs available"));
        } else {
            *contents << QString("      <div class=\"list-group list-group-flush\">");
            for (int i = 0; i < list.items.count(); ++i) {
                if (list.items[i].name.isEmpty()) continue;

                QString label = list.items[i].label;
                QString range = QString("%1–%2").arg(list.items[i].beginYear).arg(list.items[i].endYear);
                QString typeLabel = QSNTypeLabel(static_cast<quint8>(list.items[i].type));
                QString url = QString("graph?name=%1&minutes=43200").arg(list.items[i].name);

                *contents << QString("        <a href=\"%1\" class=\"list-group-item list-group-item-action px-4 py-3\">")
                             .arg(url);
                *contents << QString("          <div class=\"d-flex justify-content-between align-items-start\">");
                *contents << QString("            <div><strong>%1</strong><br><small class=\"text-muted\">%2 • %3</small></div>")
                             .arg(label, range, typeLabel);
                *contents << QString("          </div>");
                *contents << QString("        </a>");
            }
            *contents << QString("      </div>"); // list-group
        }

        *contents << QString("    </div>"); // card-body
        *contents << QString("  </div>");   // card
        *contents << QString("</div>");     // col
    }
}

void QsnWebPageGraph::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (!mds || !mds->db) return;
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("updateGraph")) {
            bool mm = options->value("minmax", 0).toInt();
            bool wb = options->value("wbr", 0).toInt();
            QString ub = options->value("ubound", "");
            QString lb = options->value("lbound", "");
            QString name = options->value("name", "");
            QString item;
            QDateTime begin;
            QDateTime end;
            item = options->value("timebegin");
            begin.setTime(QTime::fromString(item.replace("%3A", ":"), "hh:mm"));
            begin.setDate(QDate::fromString(options->value("datebegin"), "yyyy-MM-dd"));
            item = options->value("timeend");
            end.setTime(QTime::fromString(item.replace("%3A", ":"), "hh:mm"));
            end.setDate(QDate::fromString(options->value("dateend"), "yyyy-MM-dd"));

            QString series = getChartSeries(name, begin, end, mm, ub, lb, wb);
            *returnItems << QString("\"series\":") + series;
        }
    }
}

void QsnWebPageGraph::fromStream(QDataStream *stream)
{
    int Count;
    QString title;
    *stream >> title;

    getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> Count;
    // set options
    setObjectName(title);
}

void QsnWebPageGraph::urlChanged(int )
{
    if (!mds || !mds->db) return;
    if (!isKey("minutes")) {
        if (isKey("begin")) beginDT = QSNStrToDateTime(getValue("begin"));
        else beginDT = QDateTime::currentDateTime().addSecs(-86400);
        if (isKey("end")) endDT = QSNStrToDateTime(getValue("end"));
        else endDT = QDateTime::currentDateTime();
    } else {
        defaultMinuts = getValue("minutes").toInt();
        endDT = QDateTime::currentDateTime().addSecs(-60);
        beginDT = endDT.addSecs(defaultMinuts * -60);
    }

    if (beginDT >= endDT) beginDT = endDT.addSecs(-3600);

    isminmax = isKey("minmax");
    dbName = getValue("name");
    iscolumns = isKey("columns");
    issmoothing = !isKey("nosmoothing");
    iswithoutBreaks = isKey("withoutbreaks");

    lBound = getValue("lbound", "");
    uBound = getValue("ubound", "");

    if (isKey("ubound")) {
        upperBound = getValue("ubound").toDouble(&isupperBound);
        if (!isupperBound) {dbNameUB = getValue("ubound"); isupperBound = true;}
        else dbNameUB.clear();
    } else isupperBound = false;

    if (isKey("lbound")) {
        lowerBound = getValue("lbound").toDouble(&islowerBound);
        if (!islowerBound) {dbNameLB = getValue("lbound"); islowerBound = true;}
        else dbNameLB.clear();
    } else islowerBound = false;
}

void QsnWebPageGraph::spanGraph(QsnDB::dbSeries *data)
{
    if (data->begin > data->end) {
        QDateTime tdt = data->begin;
        data->begin = data->end;
        data->end = tdt;
    }

    if (data->begin.secsTo(data->end) < 1800) data->begin = data->end.addSecs(-1800);

    quint64 m = static_cast<quint64>(data->begin.secsTo(data->end) / 60);

    // ~5 лет (в минутах)
    if (m >= 2628000) {
        data->axisXlabel = tr("Date");
        data->seriesCount = static_cast<int>(m / 87600);
        data->indexRange = 5;
        return;
    }

    // ~2 месяца
    if (m >= 1209600) {
        data->axisXlabel = tr("Date");
        data->seriesCount = static_cast<int>(m / 7200);
        data->indexRange = 4;
        return;
    }

    // ~1.5 месяца (70560 мин = 49 дней)
    if (m >= 70560) {
        data->axisXlabel = tr("Date");
        data->seriesCount = static_cast<int>(m / 1680);
        data->indexRange = 3;
        return;
    }

    // 1 неделя = 10080 минут
    if (m >= 10080) {
        data->axisXlabel = tr("Date");
        data->seriesCount = static_cast<int>(m / 240);
        data->indexRange = 2;
        return;
    }

    // 1 день = 1440 минут
    if (m >= 1440) {
        data->axisXlabel = QString(tr("Time (%1)")).arg(data->end.toString("dd.MM.yyyy"));
        data->seriesCount = static_cast<int>(m / 10);
        data->indexRange = 1;
        return;
    }

    // 4 часа = 240 минут
    if (m >= 240) {
        data->axisXlabel = QString(tr("Time (%1)")).arg(data->end.toString("dd.MM.yyyy"));
        data->seriesCount = static_cast<int>(m / 2);
        data->indexRange = 1;
        return;
    }

    data->axisXlabel = QString(tr("Time (%1)")).arg(data->end.toString("dd.MM.yyyy"));
    data->seriesCount = static_cast<int>(m);
    data->indexRange = 0;
}


void QsnWebPageGraph::getMaxMin(bool onlyValues, QsnDB::dbSeries *data, qreal *max, qreal *min, bool isNew)
{
    if (data->series.count() == 0) {
        if (isNew) { *max = 1; *min = 0;}
        return;
    }

    if (isNew) {
        *max = data->series.at(0).vol.toDouble();
        *min = data->series.at(0).vol.toDouble();
    }

    for (int i = 0; i < data->series.count(); i ++) {
        if (*max < data->series.at(i).vol.toDouble()) *max = data->series.at(i).vol.toDouble();
        if (*min > data->series.at(i).vol.toDouble()) *min = data->series.at(i).vol.toDouble();
        if (!onlyValues) {
            if (*max < data->series.at(i).max.toDouble()) *max = data->series.at(i).max.toDouble();
            if (*min > data->series.at(i).max.toDouble()) *min = data->series.at(i).max.toDouble();
            if (*max < data->series.at(i).min.toDouble()) *max = data->series.at(i).min.toDouble();
            if (*min > data->series.at(i).min.toDouble()) *min = data->series.at(i).min.toDouble();
        }
    }

    if (*max > 0) *max = *max + (*max * 0.1);
    else *max = *max - (*max * 0.1);
    if (*min > 0) *min = *min - (*min * 0.1);
    else *min = *min + (*min * 0.1);
}



