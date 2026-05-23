#include "qsnwebpageenergy.h"

QsnWebPageEnergy::QsnWebPageEnergy(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Electricity"));
    widgetUrl = "/energy";
    widgetIcon = "subicon-energy";
    isminmax = false;
    iscolumns = false;
    issmoothing = true;
    maxColor = "#F78773";
    minColor = "#73B87A";
    defaultColor = "#455187";
    dbName = "energy";
    period = 0;
    cost = 0;
    requestQueue = 0;
    isEOM = false;
    lastDay = tr("Waiting for data");
    lastWeek = lastDay;
    lastMonth = lastDay;
    sanitationTimeout = EnergyPageTimeSanitation;
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageEnergy::~QsnWebPageEnergy() {}

void QsnWebPageEnergy::getJSDepending(QStringList *depending, int ) {
    *depending << "assets/js/apexcharts.min.js";
}

void QsnWebPageEnergy::getCSSDepending(QStringList *depending, int ) {
    *depending << "assets/css/apexcharts.css";
}

void QsnWebPageEnergy::getHTMLOnLoad(QStringList *functions, int ) {
    *functions << QString("initEnergyDashboard();");
}

void QsnWebPageEnergy::getJavaScript(QStringList *, int ) {}

void QsnWebPageEnergy::getFunctions(QStringList *functions, int , QString ) {
    *functions << QString("function initEnergyDashboard(){loadEnergyData();setInterval(loadEnergyData,60000);}");
    *functions << QString("function loadEnergyData(){$.ajax({url:'/energy',type:'POST',dataType:'json',contentType:'application/json; charset=utf-8',data:JSON.stringify({action:'refresh'}),success:function(data){updateKPICards(data);updateChart(data);updateDeviceTable(data.devices);}});}");
    *functions << QString("function updateKPICards(data){if(data.kpi){$('#kpi-current-power').text(data.kpi.currentPower||'0 W');$('#kpi-hour').text(data.kpi.hour||'0 kWh');$('#kpi-day').text(data.kpi.day||'0 kWh');$('#kpi-month').text(data.kpi.month||'0 kWh');$('#kpi-cost').text(data.kpi.cost||'0 Rub');}}");
    *functions << QString("var energyChart;function updateChart(data){if(!energyChart){var options={series:[{name:'Consumption',data:[]}],chart:{type:'area',height:350,toolbar:{show:true}},colors:['#455187'],dataLabels:{enabled:false},stroke:{curve:'smooth',width:2},fill:{type:'gradient',gradient:{opacityFrom:0.6,opacityTo:0.1}},xaxis:{type:'datetime',labels:{format:'dd MMM HH:mm'}},yaxis:{title:{text:'kWh'},decimalsInFloat:2},grid:{borderColor:'#f0f0f0',strokeDashArray:4},tooltip:{theme:'light',x:{format:'dd MMM yyyy HH:mm'}}};energyChart=new ApexCharts(document.querySelector('#energy-chart'),options);energyChart.render();}if(data.chartSeries){energyChart.updateSeries([{data:data.chartSeries}]);}}");
    *functions << QString("function updateDeviceTable(devices){var tbody=$('#device-table-body');tbody.empty();if(devices&&devices.length>0){devices.forEach(function(dev){tbody.append('<tr><td class=\"fw-medium\">'+dev.name+'</td><td>'+dev.consumption+'</td><td>'+dev.cost+'</td></tr>');});}else{tbody.append('<tr><td colspan=\"3\" class=\"text-center text-muted py-4\">No devices</td></tr>');}}");
    *functions << QString("function setPeriod(period){$.ajax({url:'/energy',type:'POST',dataType:'json',contentType:'application/json; charset=utf-8',data:JSON.stringify({action:'setPeriod',period:period}),success:function(data){loadEnergyData();$('.period-btn').removeClass('active');$('#period-'+period).addClass('active');}});}");
    *functions << QString("function tabletotal(data){var table=$(\"#tabletotal\");table.empty();$.each(data,function(i,item){table.append(\"<tr><td><a href='/\"+item.link+\"'>\"+item.per+\"</a></td><td>\"+item.data+\"</td></tr>\");});}");
    *functions << QString("function tableudevies(data){var table=$(\"#tabledevs\");table.empty();$.each(data,function(i,item){table.append(\"<tr><td><a href='/\"+item.link+\"'>\"+item.name+\"</td><td>\"+item.data+\"</td></tr>\");});}");
}

void QsnWebPageEnergy::getFunctionsJSON(QStringList *fjson, int ) {
    *fjson << QString(" tabletotal(data.total);");
    *fjson << QString(" tableudevies(data.devices);");
}

void QsnWebPageEnergy::getContents(QStringList *contents, int ) {
    *contents << QString("<div class=\"col-12 mb-4\"><div class=\"d-flex justify-content-between align-items-center\"><h2 class=\"mb-0\"><svg class=\"bi me-2\" width=\"32\" height=\"32\"><use xlink:href=\"#subicon-energy\"/></svg>%1</h2>").arg(objectName());
    *contents << QString("<div class=\"btn-group\" role=\"group\"><button type=\"button\" class=\"btn btn-outline-primary btn-sm period-btn active\" id=\"period-hour\" onclick=\"setPeriod('hour')\">%1</button>").arg(tr("Hour"));
    *contents << QString("<button type=\"button\" class=\"btn btn-outline-primary btn-sm period-btn\" id=\"period-day\" onclick=\"setPeriod('day')\">%1</button>").arg(tr("Day"));
    *contents << QString("<button type=\"button\" class=\"btn btn-outline-primary btn-sm period-btn\" id=\"period-week\" onclick=\"setPeriod('week')\">%1</button>").arg(tr("Week"));
    *contents << QString("<button type=\"button\" class=\"btn btn-outline-primary btn-sm period-btn\" id=\"period-month\" onclick=\"setPeriod('month')\">%1</button>").arg(tr("Month"));
    *contents << QString("</div></div></div>");
    
    *contents << QString("<div class=\"row g-4 mb-4\">");
    *contents << QString("<div class=\"col-12 col-sm-6 col-xl-3\"><div class=\"card bg-white shadow-sm border-0 h-100\"><div class=\"card-body d-flex align-items-center\"><div class=\"flex-shrink-0 bg-primary bg-opacity-10 rounded-3 p-3 me-3\"><svg class=\"bi text-primary\" width=\"32\" height=\"32\"><use xlink:href=\"#bi-lightning-charge\"/></svg></div><div><h6 class=\"text-muted mb-1\">%1</h6>").arg(tr("Current Power"));
    *contents << QString("<h3 class=\"mb-0 fw-bold\" id=\"kpi-current-power\">--</h3></div></div></div></div>");
    
    *contents << QString("<div class=\"col-12 col-sm-6 col-xl-3\"><div class=\"card bg-white shadow-sm border-0 h-100\"><div class=\"card-body d-flex align-items-center\"><div class=\"flex-shrink-0 bg-success bg-opacity-10 rounded-3 p-3 me-3\"><svg class=\"bi text-success\" width=\"32\" height=\"32\"><use xlink:href=\"#bi-clock-history\"/></svg></div><div><h6 class=\"text-muted mb-1\">%1</h6>").arg(tr("Last Hour"));
    *contents << QString("<h3 class=\"mb-0 fw-bold\" id=\"kpi-hour\">--</h3></div></div></div></div>");
    
    *contents << QString("<div class=\"col-12 col-sm-6 col-xl-3\"><div class=\"card bg-white shadow-sm border-0 h-100\"><div class=\"card-body d-flex align-items-center\"><div class=\"flex-shrink-0 bg-info bg-opacity-10 rounded-3 p-3 me-3\"><svg class=\"bi text-info\" width=\"32\" height=\"32\"><use xlink:href=\"#bi-calendar-day\"/></svg></div><div><h6 class=\"text-muted mb-1\">%1</h6>").arg(tr("Today"));
    *contents << QString("<h3 class=\"mb-0 fw-bold\" id=\"kpi-day\">--</h3></div></div></div></div>");
    
    *contents << QString("<div class=\"col-12 col-sm-6 col-xl-3\"><div class=\"card bg-white shadow-sm border-0 h-100\"><div class=\"card-body d-flex align-items-center\"><div class=\"flex-shrink-0 bg-warning bg-opacity-10 rounded-3 p-3 me-3\"><svg class=\"bi text-warning\" width=\"32\" height=\"32\"><use xlink:href=\"#bi-calendar-month\"/></svg></div><div><h6 class=\"text-muted mb-1\">%1</h6>").arg(tr("This Month"));
    *contents << QString("<h3 class=\"mb-0 fw-bold\" id=\"kpi-month\">--</h3></div></div></div></div></div>");
    
    *contents << QString("<div class=\"col-12 mb-4\"><div class=\"card bg-white shadow-sm border-0\"><div class=\"card-body d-flex justify-content-between align-items-center\"><div><h5 class=\"card-title mb-1\">%1</h5>").arg(tr("Estimated Cost"));
    *contents << QString("<p class=\"text-muted mb-0\">%1</p></div>").arg(tr("Based on current tariff"));
    *contents << QString("<div class=\"text-end\"><h2 class=\"mb-0 fw-bold text-success\" id=\"kpi-cost\">--</h2></div></div></div></div>");
    
    *contents << QString("<div class=\"col-12 mb-4\"><div class=\"card bg-white shadow-sm border-0\"><div class=\"card-header bg-transparent border-0 pt-4 px-4 pb-0\"><h5 class=\"mb-0 fw-bold\">%1</h5>").arg(tr("Consumption Trend"));
    *contents << QString("</div><div class=\"card-body p-4\"><div id=\"energy-chart\"></div></div></div></div>");
    
    *contents << QString("<div class=\"col-12 mb-4\"><div class=\"card bg-white shadow-sm border-0\"><div class=\"card-header bg-transparent border-0 pt-4 px-4 pb-0\"><h5 class=\"mb-0 fw-bold\">%1</h5>").arg(tr("Devices Breakdown"));
    *contents << QString("</div><div class=\"card-body p-0\"><div class=\"table-responsive\"><table class=\"table table-hover align-middle mb-0\"><thead class=\"bg-light\"><tr>");
    *contents << QString("<th class=\"border-0 py-3 px-4\">%1</th>").arg(tr("Device Name"));
    *contents << QString("<th class=\"border-0 py-3\">%1</th>").arg(tr("Consumption"));
    *contents << QString("<th class=\"border-0 py-3\">%1</th>").arg(tr("Cost"));
    *contents << QString("</tr></thead><tbody id=\"device-table-body\"><tr><td colspan=\"3\" class=\"text-center text-muted py-4\">Loading...</td></tr></tbody></table></div></div></div></div>");
    
    *contents << QString("<div class=\"d-none\">");
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("total", tr("TOTAL"), true);
    *contents << QsnBsTapsTabAdd("dev", tr("DEVICES"), false);
    *contents << QsnBsTapsPanesBegin();
    *contents << QsnBsTapsPanelBegin("total", true);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Period"));
    *contents << QsnBsFormTableTheadAdd(tr("Consumption"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tabletotal");
    *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/graph?name=%2&minutes=%3%4'>%1</a>").arg(tr("Last hour"), dbName).arg(60).arg(linkOptions()), lastValue);
    *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/graph?name=%2&minutes=%3%4'>%1</a>").arg(tr("Per day"), dbName).arg(1440).arg(linkOptions()), lastDay);
    *contents << QsnBsFormTableBodyRowAdd(QString("<a href='/graph?name=%2&minutes=%3%4'>%1</a>").arg(tr("Per month"), dbName).arg(43200).arg(linkOptions()), lastMonth);
    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
    *contents << QsnBsTapsPanelBegin("dev", false);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Name"));
    *contents << QsnBsFormTableTheadAdd(tr("Consumption"));
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("tabledevs");
    QStringList l;
    for (int i = 0; i < devices.count(); i ++) if (!l.contains(devices[i]->location())) l << devices[i]->location();
    for (int h = 0; h < l.count(); h ++) {
        for (int i = 0; i < devices.count(); i ++)
            if (devices[i]->location() == l.at(h)) { devices[i]->getContents(contents, -1); }
    }
    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
    *contents << QsnBsTapsPanesEnd();
    *contents << QString("</div>");
}

QString QsnWebPageEnergy::linkOptions() {
    return QString();
}

void QsnWebPageEnergy::fromStream(QDataStream *stream) {
    QsnWeb::fromStream(stream);
    *stream >> dbName >> cost >> CU;
    *stream >> isminmax >> iscolumns >> issmoothing;
    *stream >> maxColor >> minColor >> defaultColor;
}

void QsnWebPageEnergy::snBUSInput(QSNContainer container, QObject *) {
    if (container.command == QSN_CMD_READ_INPUT || container.command == QSN_CMD_READ_OUTPUT) {
        addPowerData(&container.data);
    }
}

void QsnWebPageEnergy::devicePower(QByteArray *data) {
    addPowerData(data);
}

QString QsnWebPageEnergy::widgetState() {
    return lastValue;
}

void QsnWebPageEnergy::actionItem(QString URL, QMap<QString, QString> *options, QStringList *, qint64) {
    if (options->contains("action")) {
        QString action = options->value("action");
        if (action == "refresh" || action == "setPeriod") {
            // Handle AJAX requests
        }
    }
}

void QsnWebPageEnergy::receiveSignalIOIndex(int, QByteArray *) {
    // Handle IO signals
}

void QsnWebPageEnergy::endConfiguration() {
    // Finalize configuration
}

void QsnWebPageEnergy::urlChanged(int) {
    // Handle URL changes
}

void QsnWebPageEnergy::addPowerData(QByteArray *data) {
    if (!data || data->isEmpty()) return;
    // Process power data
    bool ok;
    double value = QString(*data).toDouble(&ok);
    if (ok) {
        currentPower = QString::number(value, 'f', 2);
        lastValue = formatEnergy(value / 1000.0);
    }
}

void QsnWebPageEnergy::addMeterData(QByteArray *data) {
    if (!data || data->isEmpty()) return;
    // Process meter data
}

QString QsnWebPageEnergy::totalToJSON() {
    return QString("{\"hour\":\"%1\",\"day\":\"%2\",\"month\":\"%3\"}")
        .arg(lastValue).arg(lastDay).arg(lastMonth);
}

QString QsnWebPageEnergy::devicesToJSON() {
    return QString("[]");
}

QString QsnWebPageEnergy::getChartSeries(QString, QDateTime, QDateTime, bool) {
    return QString("[]");
}

QString QsnWebPageEnergy::getDeviceBreakdownJSON() {
    return QString("[]");
}

QString QsnWebPageEnergy::getSummaryCardsJSON() {
    return QString("{}");
}

void QsnWebPageEnergy::spanGraph(QsnDB::dbSeries *) {
    // Span graph implementation
}

void QsnWebPageEnergy::getMaxMin(bool, QsnDB::dbSeries *, qreal *, qreal *, bool) {
    // Get max/min implementation
}

QString QsnWebPageEnergy::getLabelData(int) {
    return QString();
}

QString QsnWebPageEnergy::getLabelDataWithCost(int) {
    return QString();
}

int QsnWebPageEnergy::getMinutes(const QString &periodName) {
    if (periodName == "hour") return 60;
    if (periodName == "day") return 1440;
    if (periodName == "week") return 10080;
    if (periodName == "month") return 43200;
    if (periodName == "year") return 525600;
    return 60;
}

QString QsnWebPageEnergy::getPeriodLabel(const QString &periodName) {
    if (periodName == "hour") return tr("Last Hour");
    if (periodName == "day") return tr("Today");
    if (periodName == "week") return tr("This Week");
    if (periodName == "month") return tr("This Month");
    if (periodName == "year") return tr("This Year");
    return periodName;
}

quint16 QsnWebPageEnergy::nameToAddress(QString) {
    return 0;
}

void QsnWebPageEnergy::deviceSanitation() {
    // Device sanitation logic
}

QString QsnWebPageEnergy::formatPower(double watts) {
    if (watts >= 1000.0) {
        return QString::number(watts / 1000.0, 'f', 2) + " kW";
    }
    return QString::number(watts, 'f', 1) + " W";
}

QString QsnWebPageEnergy::formatEnergy(double kwh) {
    return QString::number(kwh, 'f', 3) + " kWh";
}

QString QsnWebPageEnergy::formatCost(double amount) {
    return QString::number(amount, 'f', 2) + " " + CU;
}

QString QsnWebPageEnergy::getCurrentPowerFromData() {
    return currentPower;
}
