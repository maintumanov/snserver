#include "qsnwebpageowntracks.h"

QsnWebPageOwnTracks::QsnWebPageOwnTracks(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    isLog = false;
    area_home = 0.1;
    area_home = 1;
    cons_dist_last = 0;
    enableSignal = true;
    area_approach = 0;
    wState = tr("no connection");
    setObjectName(tr("OwnTracks"));
    widgetUrl = "/owntracks";
    widgetIcon = "subicon-owntracks";

    mds->auth->addParametr("trackID", tr("Track ID"), QString());
    mds->auth->addParametr("trackRec", tr("Track REC"), QString(), "bool");

    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageOwnTracks::~QsnWebPageOwnTracks()
{

}

void QsnWebPageOwnTracks::endConfiguration()
{
    enableSignal = false;
    for (int i = 0; i < mds->auth->countAccounts(); i ++)
        if (!mds->auth->parametrValue(mds->auth->atAccount(i), "trackID").isEmpty())
            loadLastTrackPosition(mds->auth->parametrValue(mds->auth->atAccount(i), "trackID"));
    enableSignal = true;
}

void QsnWebPageOwnTracks::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("function downloadGPX(TID){"
                          " $.ajax({"
                          "url: \"%1\","
                          "type: 'POST',"
                          "data: {action : 'download', tid: TID},"
                          "dataType: 'json',"
                          "contentType: \"application/json; charset=utf-8\","
                          "success: function(data) {"
                          "if (data.hasOwnProperty('gpx')) {"
                          "var element = document.createElement('a');"
                          "element.setAttribute('href', 'data:text/plain;charset=utf-8,' + data.gpx);"
                          "element.setAttribute('download', TID + '.gpx');"
                          "element.style.display = 'none';"
                          "document.body.appendChild(element);"
                          "element.click();"
                          "document.body.removeChild(element);}"
                          ""
                          " }, "
                          "error: function (x, t, e) { } }); "
                          "};").arg(widgetUrl);

    *functions << QString("function tableusers(data) {"
                          "var table = $(\"#userstable\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "var bt = \"<td></td>\";"
                          "var tid = '\"' + item.tid + '\"'; "
                          "var ih = \"\"; "
                          "if (item.ih == \"in\") ih = \" class='success'\";"
                          "if (item.tid != \"-\") bt = \"<td class='td-actions text-end p-0 pe-2 align-middle'>"
                          "<button type='button' class='btn btn-secondary btn-ms p-0' "
                          "onclick='downloadGPX(\" + tid + \");'>GPX</button></td>\";"
                          "table.append(\"<tr\" + ih + \"><td>\" + item.name + \"</td>\" +"
                          "\"<td  class='d-none d-md-table-cell' scope='col' >\" + item.con + \"</td>\" +"
                          "\"<td>\" + item.dst + \"</td>\" +"
                          "\"<td  class='d-none d-md-table-cell' scope='col' >\" + item.area + \"</td>\" +"
                          "\"<td  class='d-none d-md-table-cell' scope='col' >\" + item.date + \"</td>"
                          "\" +bt + \"</tr>\");});}");
}

void QsnWebPageOwnTracks::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" tableusers(data.users);");
}

void QsnWebPageOwnTracks::getContents(QStringList *contents, int )
{ 
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Name"), QString());
    *contents << QsnBsFormTableTheadAddMD(tr("Connect"));
    *contents << QsnBsFormTableTheadAdd(tr("Distance"), QString());
    *contents << QsnBsFormTableTheadAddMD(tr("Area"));
    *contents << QsnBsFormTableTheadAddMD(tr("Time"), QString());
    *contents << QString("<th class=\"text-end p-1 pe-2 align-middle col-1\" ></th>");
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("userstable");

    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID").isEmpty()) {
            location *l = getLocation(modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID"));
            if (l != Q_NULLPTR) {
//                *contents << QsnBsFormTableBodyRowCustomBegin();
                *contents << QString("<tr%1>").arg(l->distance_area == 0?" class=\"success\"":"");
                *contents << QsnBsFormTableBodyRowCustomCell(modules()->auth->atAccount(i)->name);
                *contents << QsnBsFormTableBodyRowCustomCellMD(connectType(l->conn));
                *contents << QsnBsFormTableBodyRowCustomCell(QString("%1%2").arg(l->distance, 0, 'f', 3).arg(tr("km")));
                *contents << QsnBsFormTableBodyRowCustomCellMD(areaLabel(l->distance_area));
                *contents << QsnBsFormTableBodyRowCustomCellMD(QDateTime::fromMSecsSinceEpoch(l->tst.toLongLong() * 1000).toString("dd.MM.yy hh:mm:ss"));
                if (l->fsize > 0 && isEnableRec(l->tid)) {
                    *contents << QString("<td class=\"td-actions text-end p-0 pe-2 align-middle\">");
                    *contents << QString("<button type=\"button\" class=\"btn btn-secondary btn-ms p-0\""
                                         "onclick=\"onclick=downloadGPX(\\\\\"%1\\\\\");\">").arg(l->tid);
                    *contents << QString("GPX</button></td>");
                } else *contents << QsnBsFormTableBodyRowCustomCell("");
                *contents << QsnBsFormTableBodyRowCustomEnd();
            } else {
                *contents << QsnBsFormTableBodyRowCustomBegin();
                *contents << QsnBsFormTableBodyRowCustomCell(modules()->auth->atAccount(i)->name);
                *contents << QsnBsFormTableBodyRowCustomCellMD("-");
                *contents << QsnBsFormTableBodyRowCustomCell("-");
                *contents << QsnBsFormTableBodyRowCustomCellMD("-");
                *contents << QsnBsFormTableBodyRowCustomCellMD("-");
                *contents << QsnBsFormTableBodyRowCustomCell("");
                *contents << QsnBsFormTableBodyRowCustomEnd();
            }
        }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsFormEnd();
}

void QsnWebPageOwnTracks::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("download")) {
            if (options->contains("tid")) {
                QString tid = options->value("tid");
                QString track = createGPXFile(tid);
                if (!track.isEmpty()) *returnItems << QString("\"gpx\": \"%1\"").arg(track);
            }
        }
    }

    *returnItems << QString("\"users\": %1").arg(usersToJSON());
}

void QsnWebPageOwnTracks::actionJSON(QByteArray *INjson, QByteArray *outJSON)
{
    parseJSON(QString::fromUtf8(*INjson));
    //replay
    QTextStream *htextStream = new QTextStream(outJSON);
    htextStream->setCodec("UTF-8");
    *htextStream << "[";
    *htextStream << jsonCards().toUtf8();
    *htextStream << "]";
    delete htextStream;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Read action JSON"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   true), this);
}

void QsnWebPageOwnTracks::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    // set options
    isLog = optionsMap.value("log", false).toBool();
    quint16 radius = static_cast<quint16>(area_home * 1000);
    radius = static_cast<quint16>(optionsMap.value("RD", radius).toUInt());
    area_home = static_cast<double>(radius) / 1000;
    radius = static_cast<quint16>(area_approach * 1000);
    radius = static_cast<quint16>(optionsMap.value("RDA", radius).toUInt());
    area_approach = static_cast<double>(radius) / 1000;
    configUpdateEnd();
}

void QsnWebPageOwnTracks::urlChanged(int )
{

}

QString QsnWebPageOwnTracks::widgetState()
{
    return wState;
}

void QsnWebPageOwnTracks::snBUSInput(QSNContainer container, QObject *)
{
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        configUpdateBegin();
    }
}

QString QsnWebPageOwnTracks::jsonCards()
{
    QString cards;
    for (int i = 0; i < modules()->auth->countAccounts(); i ++) {
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID").isEmpty()) {
            if (!cards.isEmpty())cards += ",";
            cards += QString("{\"_type\":\"card\",\"tid\":\"%1\",\"name\":\"%2\"}")
                    .arg(modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID"),
                         modules()->auth->atAccount(i)->name);
            location *l = getLocation(modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID"));
            if (l != Q_NULLPTR) {
                cards += QString(",{\"_type\":\"location\",\"tid\":\"%1\",\"lat\":%2,\"lon\":%3,\"tst\":%4}")
                        .arg(l->tid, l->lat, l->lon, l->tst);
            }
        }
    }

    //set home
    if (!cards.isEmpty()) cards += QLatin1String(",");
    cards += QString("{\"_type\":\"waypoint\",\"desc\":\"%1\",\"lat\":%2,\"lon\":%3,\"rad\":%4}")
            .arg(tr("home")).arg(mds->latitude, 0, 'f', 6).arg(mds->longitude, 0, 'f', 6).arg(area_home * 1000, 0, 'f', 3);
    return cards;
}

QString QsnWebPageOwnTracks::usersToJSON()
{
    QString ret = "[";

    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID").isEmpty()) {
            if (ret.count() > 1)ret += ",";
            ret += "{";
            ret += QString("\"name\": \"%1\",").arg(modules()->auth->atAccount(i)->name);
            location *l = getLocation(modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID"));
            if (l != Q_NULLPTR) {
                ret += QString("\"con\": \"%1\",").arg(connectType(l->conn));
                ret += QString("\"dst\": \"%1%2\",").arg(l->distance, 0, 'f', 3).arg("km");
                ret += QString("\"area\": \"%1\",").arg(areaLabel(l->distance_area));
                ret += QString("\"tid\": \"%1\",").arg(l->fsize > 0 && isEnableRec(l->tid)?l->tid:"-");
                ret += QString("\"ih\": \"%1\",").arg(l->distance_area == 0?"in":"out");
                ret += QString("\"date\": \"%1\"").arg(QDateTime::fromMSecsSinceEpoch(l->tst.toLongLong() * 1000).toString("dd.MM.yy hh:mm:ss"));
            } else {
                ret += QString("\"con\": \"%1\",").arg("-");
                ret += QString("\"dst\": \"%1\",").arg("-");
                ret += QString("\"area\": \"%1\",").arg("-");
                ret += QString("\"tid\": \"%1\",").arg("-");
                ret += QString("\"date\": \"%1\"").arg("-");
            }
            ret += "}";
        }

    ret += "]";
    return ret;
}

void QsnWebPageOwnTracks::parseJSON(QString ljson)
{
    QString cjson = ljson;
    if (cjson.left(1) == "{") cjson.remove(0, 1);
    if (cjson.right(1) == "}") cjson.remove(cjson.length() - 1, 1);
    cjson += ",";
    QMap<QString, QVariant> options;
    QString key;
    QString vol;
    bool iskey = true;

    for (int i = 0; i < cjson.length(); i ++) {
        if (cjson[i] == '"') continue;
        if (cjson[i] == ':') iskey = false;
        else if (cjson[i] == ',') {
            iskey = true;
            options.insert(key, vol);
            key.clear();
            vol.clear();
        }
        else if (iskey) key += cjson[i];
        else vol += cjson[i];
    }
    //==================================
    if (options.value("_type").toString() == "location") {
        location l;
        l.tid = options.value("tid").toString();
        l.lat = options.value("lat").toString();
        l.lon = options.value("lon").toString();
        l.tst = options.value("tst").toString();
        l.batt = options.value("batt", "100").toString();
        l.conn = options.value("conn", QString()).toString();
        l.distance = 0;
        l.distance_last = 0;
        l.fsize = 0;
        l.distance_area = 0;
        writeTrackPosition(&l);
        locationAdd(l);
    }

    if (options.value("_type").toString() == "waypoint" && !options.value("desc").toString().isEmpty()) {
        waypoint w;
        w.tid = options.value("tid").toString();
        w.lat = options.value("lat").toString();
        w.lon = options.value("lon").toString();
        w.tst = options.value("tst").toString();
        w.desc = options.value("desc", QString()).toString();
        w.rad = options.value("rad", "50").toString();
        waypointAdd(w);
    }
}

void QsnWebPageOwnTracks::locationAdd(QsnWebPageOwnTracks::location l)
{
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Location add time:%1, Lon:%2; Lat:%3").arg(l.tst, l.lon, l.lat),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID") == l.tid) {
            for (int j = 0; j < locations.count(); j ++)
                if (locations[j].tid == l.tid) {
                    locations[j].lat = l.lat;
                    locations[j].lon = l.lon;
                    locations[j].tst = l.tst;
                    locations[j].batt = l.batt;
                    locations[j].conn = l.conn;
                    checkDistance(&locations[j]);
                    wState = locations[j].tid + " " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
                    return;
                }
            checkDistance(&l);
            locations.append(l);

        }
}

void QsnWebPageOwnTracks::waypointAdd(QsnWebPageOwnTracks::waypoint w)
{
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Waypoint add time:%1, Lon:%2; Lat:%3, Rad:%4, Desc:%5").arg(w.tst).arg(w.lon).arg(w.lat).arg(w.rad).arg(w.desc),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID") == w.tid) {
            for (int j = 0; j < waypoints.count(); j ++)
                if (waypoints[j].tid == w.tid && waypoints[j].tst == w.tst) {
                    waypoints[j] = w;
                    return;
                }
            waypoints.append(w);
        }
}

int QsnWebPageOwnTracks::countUsers()
{
    int u = 0;
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (!modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID").isEmpty()) u ++;
    return u;
}

QsnWebPageOwnTracks::location *QsnWebPageOwnTracks::getLocation(QString tid)
{
    for (int j = 0; j < locations.count(); j ++)
        if (locations[j].tid == tid) return &locations[j];
    return Q_NULLPTR;
}

QString QsnWebPageOwnTracks::connectType(QString conn)
{
    if (conn == "w") return "WiFi";
    if (conn == "m") return tr("mobile Internet");
    if (conn == "o") return tr("Offline mode");
    return tr("Unknown");
}

double QsnWebPageOwnTracks::getDistance(double lat1, double lon1, double lat2, double lon2)
{
    double p = 0.017453292519943295;    // Math.PI / 180
    double a = 0.5 - cos((lat2 - lat1) * p)/2 +
            cos(lat1 * p) * cos(lat2 * p) *
            (1 - cos((lon2 - lon1) * p))/2;

    return 12742 * asin(sqrt(a)); // 2 * R; R = 6371 km
}

void QsnWebPageOwnTracks::checkDistance(QsnWebPageOwnTracks::location *l)
{
    bool ok;
    double lat = l->lat.toDouble(&ok);
    if (!ok) return;
    double lon = l->lon.toDouble(&ok);
    if (!ok) return;
    l->distance_last = l->distance;
    l->distance = static_cast<double>(getDistance(mds->latitude, mds->longitude, lat, lon));
    l->distance_area = 3;
    if (l->distance < area_home) l->distance_area = 0;
    else if (l->distance < area_approach) l->distance_area = 1;

    if (l->distance_area == 1) {
        if (l->distance_last > l->distance) l->distance_area = 2;
        if (l->distance_last < l->distance) l->distance_area = 4;
    }

    checkUsersArea();
}

void QsnWebPageOwnTracks::writeTrackPosition(QsnWebPageOwnTracks::location *l)
{
    if (!isEnableRec(l->tid)) return;
    QFile file(QSNHomeSubPath("server", "owntracks").absoluteFilePath(QString("%1.ot").arg(l->tid)));
    if (!file.open(QIODevice::Append | QIODevice::Text)) return;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1 %2").arg(tr("Write track position"), l->tid),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    QTextStream Out(&file);

    Out << QString("%1;%2;%3;%4;%5")
           .arg(l->tst, l->lon, l->lat, l->batt, l->conn);
    mds->db->writeCR(&Out);
    l->fsize = file.size();
    file.close();
}

void QsnWebPageOwnTracks::loadLastTrackPosition(QString tid)
{
    if (!isEnableRec(tid)) return;
    QFile file(QSNHomeSubPath("server", "owntracks").absoluteFilePath(QString("%1.ot").arg(tid)));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    if (file.size() > 100) file.seek(file.size() - 60);
    QTextStream in(&file);
    QString lastLine;
    while (!in.atEnd()) lastLine = in.readLine();
    //trackPositionToLocation(lastLine);
    QStringList list = lastLine.split(";");
    if (list.count() >= 5) {
        location l;
        l.tid = tid;
        l.tst = list[0];
        l.lon = list[1];
        l.lat = list[2];
        l.batt = list[3];
        l.conn = list[4];
        l.fsize = file.size();
        checkDistance(&l);
        l.distance_last = l.distance;
        locationAdd(l);
    }
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_warning,
                                   QString("%1 %2").arg(tr("Load last track position"), tid),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    file.close();
}

QString QsnWebPageOwnTracks::createGPXFile(QString tid)
{
    QFile file(QSNHomeSubPath("server", "owntracks").absoluteFilePath(QString("%1.ot").arg(tid)));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) QString();

    QString ret;
    QTextStream in(&file);
    //    QTextStream out(&filegpx);
    ret += QString("<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
                   "<gpx xmlns='http://www.topografix.com/GPX/1/1' version='1.1' creator='SignalNet'>"
                   "<time>%1</time>")
            .arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddThh:mm:ssZ"));
    ret += QString("<metadata><name>%1</name></metadata>").arg(tid);
    ret += QString("<trk><name>%1</name><trkseg>").arg(tid);

    QStringList list;
    while (!in.atEnd()) {
        list = in.readLine().split(";");
        if (list.count() >= 5) {
            location l;
            l.tid = tid;
            l.tst = list[0];
            l.lon = list[1];
            l.lat = list[2];
            l.batt = list[3];
            l.conn = list[4];
            ret += QString("<trkpt lat='%1' lon='%2'>").arg(l.lat, l.lon);
            ret += QString("<ele>0.0</ele>");
            ret += QString("<time>%1</time>").arg(QDateTime::fromMSecsSinceEpoch(l.tst.toLongLong() * 1000).toUTC().toString("yyyy-MM-ddThh:mm:ssZ"));
            ret += QString("</trkpt>");
        }
    }
    ret += QString("</trkseg></trk></gpx>");
    file.close();
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_warning,
                                   QString("%1 %2").arg(tr("Create GPX file"), tid),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    return ret;
}

QString QsnWebPageOwnTracks::areaLabel(quint8 index)
{
    switch (index) {
    case 0: return tr("in the home area");
    case 1: return tr("outside the home area");
    case 2: return tr("approaching the house");
    case 3: return tr("far from home");
    case 4: return tr("away from home");
    }
    return tr("unknown");
}

void QsnWebPageOwnTracks::checkUsersArea()
{
    double cons_dist = -1;
    location *l;

    for (int i = 0; i < modules()->auth->countAccounts(); i ++) {
        l = getLocation(modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID"));
        if (l == Q_NULLPTR) continue;
        if (cons_dist > l->distance || cons_dist < 0) cons_dist = l->distance;
    }
    if (cons_dist_last >= area_home && cons_dist < area_home) toRPresent();
    if (cons_dist_last < area_home && cons_dist >= area_home) toRAbsent();
    if (cons_dist < cons_dist_last && cons_dist < area_approach && cons_dist_last > area_approach) toRApproaching();
    cons_dist_last = cons_dist;
}

void QsnWebPageOwnTracks::toRAbsent()
{
    QByteArray data;
    data[0] = 0;
    if (enableSignal) mds->io->widgetReciveIndexSignal(0, &data, this);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_warning,
                                   tr("The status of the absence"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    wState = tr("absence");
    //    mds->registry->setValue("SOWTA", false);
    //    mds->registry->setValue("SOWTP", false);
    //    mds->registry->setValue("owntracks", wState);
    mds->db->registryPublicData("owntracks", stateData(false), this);
}

void QsnWebPageOwnTracks::toRPresent()
{
    QByteArray data;
    data[0] = 0;
    if (enableSignal) mds->io->widgetReciveIndexSignal(1, &data, this);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("The status of the presence"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    wState = tr("presence");
    //    mds->registry->setValue("SOWTA", false);
    //    mds->registry->setValue("SOWTP", true);
    //    mds->registry->setValue("owntracks", wState);
    mds->db->registryPublicData("owntracks", stateData(true), this);
}

void QsnWebPageOwnTracks::toRApproaching()
{
    QByteArray data;
    data[0] = 0;
    if (enableSignal) mds->io->widgetReciveIndexSignal(2, &data, this);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("The status of the approach"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("owntracks"),
                                   false), this);
    wState = tr("approach");
    //    mds->registry->setValue("SOWTA", true);
    //    mds->registry->setValue("SOWTP", false);
    //    mds->registry->setValue("owntracks", wState);
    mds->db->registryPublicData("owntracks", stateData(true), this);
}

bool QsnWebPageOwnTracks::isEnableRec(QString tid)
{
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (modules()->auth->parametrValue(mds->auth->atAccount(i), "trackID") == tid
                && modules()->auth->parametrValue(mds->auth->atAccount(i), "trackRec") == "true")
            return true;
    return false;
}

QByteArray QsnWebPageOwnTracks::stateData(bool state)
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, state);
    QSNByteToRAW(&data, 2, 7);
    return data;
}

void QsnWebPageOwnTracks::configUpdateBegin()
{
    mds->db->tempSettings.insert("ownTracksCount", locations.count());
    for (int i = 0; i < locations.count(); i ++) {
        mds->db->tempSettings.insert(QString("ownTracks%1tid").arg(i), locations.at(i).tid);
        mds->db->tempSettings.insert(QString("ownTracks%1lat").arg(i), locations.at(i).lat);
        mds->db->tempSettings.insert(QString("ownTracks%1lon").arg(i), locations.at(i).lon);
        mds->db->tempSettings.insert(QString("ownTracks%1tst").arg(i), locations.at(i).tst);
        mds->db->tempSettings.insert(QString("ownTracks%1conn").arg(i), locations.at(i).conn);
        mds->db->tempSettings.insert(QString("ownTracks%1batt").arg(i), locations.at(i).batt);
        mds->db->tempSettings.insert(QString("ownTracks%1label").arg(i), locations.at(i).label);
        mds->db->tempSettings.insert(QString("ownTracks%1distance").arg(i), locations.at(i).distance);
        mds->db->tempSettings.insert(QString("ownTracks%1distance_last").arg(i), locations.at(i).distance_last);
        mds->db->tempSettings.insert(QString("ownTracks%1distance_area").arg(i), locations.at(i).distance_area);
        mds->db->tempSettings.insert(QString("ownTracks%1fsize").arg(i), locations.at(i).fsize);
    }
    mds->db->tempSettings.insert("ownTrackswState", wState);
    mds->db->tempSettings.insert("ownTrackswCons_dist_last", cons_dist_last);
    mds->db->tempSettings.insert("ownTrackswEnableSignal", enableSignal);
}

void QsnWebPageOwnTracks::configUpdateEnd()
{
    int count = mds->db->tempSettings.value(QString("ownTracksCount"), 0).toInt();
    location l;
    for (int i = 0; i < count; i ++) {
        l.tid = mds->db->tempSettings.value(QString("ownTracks%1tid").arg(i), QString()).toString();
        l.lat = mds->db->tempSettings.value(QString("ownTracks%1lat").arg(i), QString()).toString();
        l.lon = mds->db->tempSettings.value(QString("ownTracks%1lon").arg(i), QString()).toString();
        l.tst = mds->db->tempSettings.value(QString("ownTracks%1tst").arg(i), QString()).toString();
        l.conn = mds->db->tempSettings.value(QString("ownTracks%1conn").arg(i), QString()).toString();
        l.batt = mds->db->tempSettings.value(QString("ownTracks%1batt").arg(i), QString()).toString();
        l.label = mds->db->tempSettings.value(QString("ownTracks%1label").arg(i), QString()).toString();
        l.distance = mds->db->tempSettings.value(QString("ownTracks%1distance").arg(i), 0).toDouble();
        l.distance_last = mds->db->tempSettings.value(QString("ownTracks%1distance_last").arg(i), 0).toDouble();
        l.distance_area = mds->db->tempSettings.value(QString("ownTracks%1distance_area").arg(i), 0).toDouble();
        l.fsize = mds->db->tempSettings.value(QString("ownTracks%1fsize").arg(i), 0).toLongLong();
        locations.append(l);
    }
    wState = mds->db->tempSettings.value(QString("ownTrackswState"), wState).toString();
    cons_dist_last = mds->db->tempSettings.value(QString("ownTrackswState"), cons_dist_last).toDouble();
    enableSignal = mds->db->tempSettings.value(QString("ownTrackswState"), enableSignal).toBool();
}
