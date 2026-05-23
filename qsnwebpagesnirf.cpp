#include "qsnwebpagesnirf.h"

// ————————————————————————————————————————————————
// CONSTRUCTOR / DESTRUCTOR
// ————————————————————————————————————————————————


QsnWebPageSNIRF::QsnWebPageSNIRF(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    isLog = false;

    setObjectName(tr("Device Health"));
    widgetUrl = "/snirf";
    widgetIcon = "subicon-device";

    // Subscribe to internal bus events (time ticks, config updates)
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageSNIRF::~QsnWebPageSNIRF()
{
    disconnect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    qDeleteAll(devicesMQTT);
    devicesMQTT.clear();
}

void QsnWebPageSNIRF::endConfiguration()
{

}

// ————————————————————————————————————————————————
// WEB UI GENERATION
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::getFunctions(QStringList *functions, int /*accountIndex*/, QString /*pageURL*/)
{
    *functions << QString("function showToast(message, type) {"
                          "  var icon = '';"
                          "  var bgClass = 'bg-success';"
                          "  if (type === 'success') { icon = '✅'; bgClass = 'bg-success'; }"
                          "  else if (type === 'error') { icon = '❌'; bgClass = 'bg-danger'; }"
                          "  else { icon = 'ℹ️'; bgClass = 'bg-info'; }"
                          "  var toastHtml = '<div class=\"toast align-items-center text-white ' + bgClass + ' border-0 rounded-3 shadow-lg\" role=\"alert\" aria-live=\"assertive\" aria-atomic=\"true\" style=\"position: fixed; top: 20px; right: 20px; z-index: 9999; min-width: 250px; backdrop-filter: blur(4px);\">' +"
                          "    '<div class=\"d-flex align-items-center p-2\">' +"
                          "      '<div class=\"me-2 fs-4\">' + icon + '</div>' +"
                          "      '<div class=\"toast-body flex-grow-1 fw-semibold\">' + message + '</div>' +"
                          "      '<button type=\"button\" class=\"btn-close btn-close-white me-1\" data-bs-dismiss=\"toast\" aria-label=\"Close\"></button>' +"
                          "    '</div>' +"
                          "  '</div>';"
                          "  var toast = $(toastHtml);"
                          "  $('body').append(toast);"
                          "  toast.hide().fadeIn(300);"
                          "  var bsToast = new bootstrap.Toast(toast[0], { autohide: true, delay: 3500 });"
                          "  bsToast.show();"
                          "  toast.on('hidden.bs.toast', function() { toast.remove(); });"
                          "}");

    if (itemLog >= 0 && itemLog < devices.count()) {
        // JS function to render device log table
        *functions << QString("function tablelog(data) {"
                              "var table = $(\"#devicelogtable\");"
                              "table.empty();"
                              "$.each(data, function (i, item) {"
                              "table.append(\"<tr><td  scope='col' >\" + item.date + \"</td>"
                              "<td  scope='col' >\" + item.data + \"</td>"
                              "</tr>\");});}");
        return;
    }

    // JS function to render main device table with actions
    *functions << QString("function tabledevices(data) {"
                          "var table = $(\"#devicestable\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr class='\" + item.stcls + \"'><td><a href='snirf?log=\" + item.log + \"'>\" + item.name + \"</a></td>\" +"
                          "\"<td>\" + item.address + \"</td>\" +"
                          "\"<td  class='d-none d-md-table-cell' scope='col' >\" + item.signal + \"</td>\" +"
                          "\"<td>\" + item.batt + \"</td>\" +"
                          "\"<td  class='d-none d-md-table-cell' scope='col' >\" + item.data + \"</td>\" +"
                          "\"<td  scope='col' >\" + item.date + \"</td>"
//                          "<td class='td-actions text-right p-0 pe-2 align-middle'>"
//                          "<button type='button' class='btn btn-danger btn-ms p-0' "
//                          "onclick='sendState(`remove`, `%1`, `\" + item.log + \"`);'"
//                          "><svg class='icon-sprite icon-05x '><use xlink:href='assets/images/icons-sprite.svg#subicon-delete' /></svg>"
//                          "</button>"
//                          "</td>"
                          "</tr>\");});}");


}

void QsnWebPageSNIRF::getFunctionsJSON(QStringList *fjson, int /*accountIndex*/)
{
    // Redirect if someone lands on log view but device no longer exists
    *fjson << QString(" if (data.snirfmain === true) location.replace('/snirf');");

    // Показываем уведомление при успешном переименовании
    *fjson << QString(" if (data.renameSuccess === true) {"
                      "   showToast('%1', 'success');"
                      "}").arg(tr("Device renamed successfully"));

    if (itemLog >= 0 && itemLog < devices.count()) {
        *fjson << QString(" tablelog(data.deviceslog);");
        return;
    }

    *fjson << QString(" tabledevices(data.devices);");
}

void QsnWebPageSNIRF::getContents(QStringList *contents, int /*accountIndex*/)
{
    if (itemLog >= 0 && itemLog < devices.count()) {
        device *dev = &devices[itemLog];

        if (dev->isSNIRF) {
            *contents << QString("<div class=\"w-100\"></div>");
            *contents << QString("<div style=\"display: flex; align-items: stretch; gap: 5px; padding-left: 15px; margin-bottom: 1rem;\">");

            // Кнопка "Назад" вместо заголовка
            *contents << QString("<a href='snirf' class='btn btn-outline-secondary' style=\"display: inline-flex; align-items: center; height: 38px; white-space: nowrap;\">"
                                 "← %1</a>").arg(tr("Devices"));
            // Квадрат с адресом устройства (стилизован под кнопку, но без действия)
            *contents << QString("<span class=\"btn btn-outline-secondary\" style=\"height: 38px; width: 38px; display: inline-flex; align-items: center; justify-content: center; pointer-events: none;\" "
                                 "title=\"%2\">%1</span>")
                                 .arg(dev->address).arg(tr("Device address"));
            // Поле ввода имени
            *contents << QString("<input class=\"form-control\" type=\"text\" value=\"%1\" id=\"custom-name\" style=\"flex: 1; min-width: 120px; height: 38px;\">")
                         .arg(getDevName(dev).toHtmlEscaped());

            // Кнопка Rename
            *contents << QString("<button type='button' class='btn btn-primary' style=\"height: 38px; white-space: nowrap;\" "
                                 "onclick=\"sendState('rename', '%1', $('#custom-name').val());\">"
                                 "<svg class='icon-sprite icon-05x'><use xlink:href='assets/images/icons-sprite.svg#subicon-edit' /></button>")
                         .arg(dev->address);

            // Кнопка Delete
            *contents << QString("<button type='button' class='btn btn-danger' style=\"height: 38px; white-space: nowrap;\" "
                                 "onclick=\"sendState('remove', '%1', '%2');\">"
                                 "<svg class='icon-sprite icon-05x'><use xlink:href='assets/images/icons-sprite.svg#subicon-delete' /></svg> "
                                 "%3</button>")
                         .arg(itemID()).arg(itemLog).arg(tr("Delete"));

            *contents << QString("</div>");
        } else {

            *contents << QsnBsPanelTitle(tr("<a href='snirf'>Device</a> %1").arg(getDevName(dev)));
        }

        if (dev->isSNIRF) {
            // Принудительный перенос на новую строку
            *contents << QString("<div class=\"w-100\"></div>");
            *contents << QString("<div class=\"d-flex flex-wrap mt-3 mb-3\" style=\"padding-left: 15px; gap: 10px;\">");


            if (dev->battChangeDate.isValid()) {
                QString batteryLife = formatBatteryLife(dev->battChangeDate);
                *contents << buildIndicator("🕒", batteryLife.toHtmlEscaped(), "", "life");
            }

            if (dev->estimatedRemainingHours != -1) {
                QString remainingStr = formatRemainingTime(dev->estimatedRemainingHours);
                *contents << buildIndicator("⏳", remainingStr, "", "remaining");
            }

            // Уровень заряда
            *contents << buildIndicator("🔋", QString::number(dev->batt), "%", "level");

            *contents << buildIndicator("📶", getSignalGraphHTML(*dev), "", "q");

            *contents << buildIndicator("⏱", QString("%1 - %2").arg(formatInterval(dev->minInterval), formatInterval(dev->maxInterval)), "", "tm");

            *contents << QString("</div>"); // <-- закрываем вторую строку (индикаторы)
        }

        // Render log table
        *contents << QsnBsFormBegin();
        *contents << QsnBsFormTableBegin();
        *contents << QsnBsFormTableTheadBegin();
        *contents << QsnBsFormTableTheadAdd(tr("Time"), QString());
        *contents << QsnBsFormTableTheadAdd(tr("Data"));
        *contents << QsnBsFormTableTheadEnd();
        *contents << QsnBsFormTableBodyBegin("devicelogtable");

        for (int i = dev->log.count() - 1; i >= 0; i --) {
            *contents << QsnBsFormTableBodyRowCustomBegin();
            *contents << QsnBsFormTableBodyRowCustomCell(dev->log[i].timestamp.toString("dd.MM.yy hh:mm:ss"));
            *contents << QsnBsFormTableBodyRowCustomCell(QString("%1").arg(dev->log[i].data));
            *contents << QsnBsFormTableBodyRowCustomEnd();
        }
        *contents << QsnBsFormTableBodyEnd();
        *contents << QsnBsFormTableEnd();
        *contents << QsnBsFormEnd();
        return;
    }

    // Main device list view
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAdd(tr("Name"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Address"), QString());
    *contents << QsnBsFormTableTheadAddMD(tr("Signal"), QString());
    *contents << QsnBsFormTableTheadAdd(tr("Batt"), QString());
    *contents << QsnBsFormTableTheadAddMD(tr("Data"));
    *contents << QsnBsFormTableTheadAdd(tr("Time"), QString());
//    *contents << QString("<th class=\"text-right p-1 pe-2 align-middle col-1\" ></th>");
    *contents << QsnBsFormTableTheadEnd();
    *contents << QsnBsFormTableBodyBegin("devicestable");

    for (int i = 0; i < devices.count(); i ++) {
        *contents << QString("<tr class='%1'>").arg(deviceItemClassState(devices[i].isTimeout, devices[i].batt));

        if (devices[i].isSNIRF) {
            // Имя устройства со ссылкой
            *contents << QsnBsFormTableBodyRowCustomCell(QString("<a href='snirf?log=%1'>%2</a>")
                                                         .arg(i).arg(getDevName(&devices[i])));
            // Адрес устройства (отдельный столбец)
            *contents << QsnBsFormTableBodyRowCustomCell(QString("%1").arg(devices[i].address));
            // Качество сигнала
            *contents << QsnBsFormTableBodyRowCustomCellMD(getSNIRFSignalStatus(devices[i]));
        } else {// Not applicable for MQTT
            *contents << QsnBsFormTableBodyRowCustomCell(QString("<a href='snirf?log=%1'>%2</a>")
                                                         .arg(i).arg(getDevName(&devices[i])));
            *contents << QsnBsFormTableBodyRowCustomCell("-");  // У MQTT нет адреса
            *contents << QsnBsFormTableBodyRowCustomCell("-");
        }

        *contents << QsnBsFormTableBodyRowCustomCell(QString::number(devices[i].batt)+"%");
        *contents << QsnBsFormTableBodyRowCustomCell(devices[i].lastData);
        *contents << QsnBsFormTableBodyRowCustomCellMD(devices[i].lastTimeUpdate.toString("dd.MM.yy hh:mm:ss"));

        // Delete button
//        *contents << QString("<td class=\"td-actions text-right p-0 pe-2 align-middle\" >");
//        *contents << QString("<button type=\"button\" class=\"btn btn-danger btn-ms p-0\" "
//                             "onclick=\"sendState('remove', '%1', '%2');\">").arg(itemID()).arg(devices[i].address);
//        *contents << QString("<svg class=\"icon-sprite icon-05x \"><use xlink:href=\"assets/images/icons-sprite.svg#subicon-delete\" /></svg>");
//        *contents << QString("</button>");
//        *contents << QString("</td>");
        *contents << QsnBsFormTableBodyRowCustomEnd();
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsFormEnd();
}

// ————————————————————————————————————————————————
// USER ACTIONS (RENAME, REMOVE)
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 /*unused*/)
{
    if (options->contains("action") && options->value("action") == "remove") {
        QString state = options->value("state");
        bool ok = false;
        int item = state.toInt(&ok);
        if (ok) removeDevice(item);
        *returnItems << QString("\"snirfmain\": true");
        return;
    }

    // Return appropriate JSON data based on current view
    if (options->contains("action") && options->value("action") == "rename") {
        QString newname = QSNDecodeText(options->value("state"));
        QString id = options->value("id");
        bool ok = false;
        int item = id.toInt(&ok);
        if (ok) {
            renameDevice(item, newname);
            *returnItems << QString("\"renameSuccess\": true");
        }
    }

    if (itemLog >= 0) {
        if (itemLog < devices.count()) *returnItems << QString("\"deviceslog\": %1").arg(logToJSON(itemLog));
        else  *returnItems << QString("\"snirfmain\": true");
    } else *returnItems << QString("\"devices\": %1").arg(devicesToJSON());
}

// ————————————————————————————————————————————————
// STATE PERSISTENCE & CONFIGURATION
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::fromStream(QDataStream *stream)
{
    int count;
    int id;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    // Clean up MQTT sub-modules
    for(int i = 0; i < devicesMQTT.size(); ++i) {
        disconnect(devicesMQTT[i], nullptr, this, nullptr);
    }
    qDeleteAll(devicesMQTT);

    devicesMQTT.clear();

    // Load generic options and I/O mappings
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items

    // Deserialize MQTT sub-modules
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;

        if (sig == QLatin1String("MQSM")) {
            QsnWebPageSNIRFMQTT *imqtt = new QsnWebPageSNIRFMQTT(static_cast<quint32>(id), modules(), this);
            devicesMQTT.append(imqtt);
            imqtt->fromStream(stream, getPath());
            connect(imqtt, SIGNAL(onMQTT(QString, QString, quint8, quint16, QString)), this, SLOT(addDeviceMQTT(QString, QString, quint8, quint16, QString)));
        }
    }


    // Apply settings
    isLog = optionsMap.value("log", false).toBool();
    SNIRF_TimeOut_global = optionsMap.value("TO", 90).toInt() * 60;
    configUpdateEnd();
}

// ————————————————————————————————————————————————
// DEVICE REGISTRATION & PROCESSING
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (data && indexIO == 0) addDeviceSNIRF(data); // raw SNIRF packet on I/O index 0
}

void QsnWebPageSNIRF::urlChanged(int /*accountIndex*/)
{
    // Parse ?log=N from URL
    itemLog = getValue("log", "-1").toInt();
}

// ————————————————————————————————————————————————
// WIDGET STATUS (FOR NOTIFICATIONS & BADGES)
// ————————————————————————————————————————————————

QString QsnWebPageSNIRF::widgetState()
{
    if (devices.count() == 0) return tr("devices are missing");
    QString ret;
    if (battLowCount) ret = tr("disch.:%1").arg(battLowCount);
    if (timeoutCount) {
        if (!ret.isEmpty()) ret += ", ";
        ret += tr("off.:%1").arg(timeoutCount);
    }
    if (ret.isEmpty()) ret = tr("devices:%1").arg(devices.count());
    return ret;
}

char QsnWebPageSNIRF::widgetNotifState()
{
    if (timeoutCount) return 'c'; // critical
    if (battLowCount) return 'w'; // warning
    return 'n'; // normal
}

// ————————————————————————————————————————————————
// BUS EVENT HANDLING
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return; // avoid echo
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) timeProcessing(); // called every second
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        configUpdateBegin(); // save state before reload
    }
}

QString QsnWebPageSNIRF::devicesToJSON()
{
    QJsonArray arr;
    for (int i = 0; i < devices.count(); i ++) {
        QJsonObject obj;
        obj["batt"] = QString("%1%").arg(devices[i].batt);
        obj["data"] = devices[i].lastData; // QJsonObject автоматически экранирует строки
        obj["stcls"] = deviceItemClassState(devices[i].isTimeout, devices[i].batt);
        obj["date"] = devices[i].lastTimeUpdate.toString("dd.MM.yy hh:mm:ss");
        obj["log"] = i; // Или храните индекс отдельно, indexOf медленный O(N)
        obj["address"] = devices[i].address; // Если address int, он станет числом в JSON. Если нужен строкой - оберните в QString
        obj["name"] = getDevName(&devices[i]);

        if (devices[i].isSNIRF) {
            obj["signal"] = getSNIRFSignalStatus(devices[i]);
        } else {
            obj["signal"] = "-";
        }
        arr.append(obj);
    }

    return QJsonDocument(arr).toJson(QJsonDocument::Compact);
}

QString QsnWebPageSNIRF::logToJSON(int dev)
{
    QString ret = "[";
    for (int i = devices[dev].log.count() - 1; i >= 0 ; i --) {
        if (ret.count() > 1)ret += ",";
        ret += "{";
        ret += QString("\"data\": \"%1\",").arg((devices[dev].log[i].data));
        ret += QString("\"date\": \"%1\"").arg(devices[dev].log[i].timestamp.toString("dd.MM.yy hh:mm:ss"));
        ret += "}";
    }

    ret += "]";
    return ret;
}

QString QsnWebPageSNIRF::deviceItemClassState(bool offline, quint8 batt)
{
    if (offline) return "caution";
    if (batt <= 10) return "warning";
    return QString();
}

void QsnWebPageSNIRF::addDeviceSNIRF(QByteArray *data) {
    if (!data) {
        qWarning() << "QsnWebPageSNIRF::addDeviceSNIRF: received null data";
        return;
    }

    // Parse raw packet into structured SNIRF format
    QSNSNIR snir = QSNRAWtoSNIR(data, 1);
    // Create new device
    device dev;
    dev.address = snir.address;
    dev.name = "SNIRF";
    dev.customName = getDeviceCustomName(dev.address, QString());
    dev.batt = QSNSNIRBatToPercent(snir.battery);
    dev.retry = snir.retry;
    dev.isLowBattery = 0;
    dev.parametr = snir.parameter;
    dev.battChangeDate = mds->settings->value(QString("snirdev%1battchangedate").arg(snir.address), QDateTime::currentDateTime()).toDateTime();

    if (snir.data.isEmpty()) {
        dev.lastData = QString("[%1] No data").arg(snir.parameter);
    } else {
        dev.lastData = QString("[%1] %2 %3")
                .arg(snir.parameter)
                .arg(QSNTypeLabel(snir.data.at(0)), QSNRAWtoScaledVolume(&snir.data));
    }

    dev.lastTimeUpdate = QDateTime::currentDateTime();
    dev.isTimeout = false;
    dev.isSNIRF = true;
    dev.lastBattPercent = 0;
    dev.lastBattTime = QDateTime();
    dev.estimatedRemainingHours = -1;
    addDevice(dev);  // register or update

    busPuplic(snir);     // emit on internal bus for other modules
}

void QsnWebPageSNIRF::addDeviceMQTT(QString name, QString value, quint8 batt, quint16 timeout, QString /*unused*/)
{
    device dev;
    dev.address = 0;  // not used for MQTT
    dev.name = name; // MQTT devices don't support custom names (currently)
    dev.batt = batt;
    dev.isLowBattery = 0;
    dev.battChangeDate = mds->settings->value(QString("snirdev%1battchangedate").arg(name), QDateTime::currentDateTime()).toDateTime();
    dev.lastData = QString("%1").arg(value);
    dev.parametr = 0;
    dev.isTimeout = false;
    dev.isSNIRF = false;
    dev.timeout = timeout;
    dev.lastBattPercent = 0;
    dev.lastBattTime = QDateTime();
    dev.estimatedRemainingHours = -1;
    dev.retry = 0;
    addDevice(dev);
}

void QsnWebPageSNIRF::renameDevice(int addr, QString newName)
{
    for (int i = 0; i < devices.count(); i ++)
        if (devices[i].isSNIRF && devices[i].address == addr) {
            devices[i].customName = newName.trimmed();
            mds->settings->setValue(QString("snirdev%1customname").arg(devices[i].address), devices[i].customName);
        }
}

void QsnWebPageSNIRF::addDevice(device dev)
{
    // Try to find an existing device by unique identifier:
    // - For SNIRF devices: match by 'address'
    // - For MQTT devices: match by 'name'
    for (int i = 0; i < devices.count(); i ++) {
        bool match = (devices[i].isSNIRF && devices[i].address == dev.address) || (!devices[i].isSNIRF && devices[i].name == dev.name);

        if (match) {
            // --- Hysteresis-based battery state update ---
            bool wasLow = devices[i].isLowBattery;
            bool nowLow = (dev.batt <= LOW_BATT_THRESHOLD);
            bool nowRecovered = (dev.batt >= RECOVERY_THRESHOLD);

            if (!wasLow && nowLow) {
                devices[i].isLowBattery = true;
                sendMessage(1, tr("The device has a low battery charge {device with: %1}").arg(getDevName(&devices[i])));
            }
            if (wasLow && nowRecovered) {
                devices[i].isLowBattery = false;
                sendMessage(2, tr("The device has a normal battery charge {device with: %1}").arg(getDevName(&devices[i])));
            }

            // --- Offline → Online notification ---
            if (devices[i].isTimeout) {
                sendMessage(2, tr("The device is working again {device with: %1}").arg(getDevName(&devices[i])));
            }

            quint8 oldBatt = devices[i].batt;
            // Update fields
            devices[i].address = dev.address;
            devices[i].batt = dev.batt;
            devices[i].retry = dev.retry;
            if (devices[i].parametr != 31) devices[i].lastData = dev.lastData;
            devices[i].parametr = dev.parametr;
            devices[i].isTimeout = false; // Device is active now
            devices[i].name = dev.name;
            devices[i].customName = dev.customName;
            devices[i].isSNIRF = dev.isSNIRF;
            devices[i].timeout = dev.timeout;



            // --- Detect battery replacement ---
            bool isBatteryReplaced = false;
            if (dev.isSNIRF) {
                if (dev.batt == 100 && devices[i].batt < 100) {
                    isBatteryReplaced = true;
                }
            } else {
                if (devices[i].batt + 40 < dev.batt) {
                    isBatteryReplaced = true;
                }
            }

            if (isBatteryReplaced) {
                QDateTime now = QDateTime::currentDateTime();
                QString key = dev.isSNIRF
                        ? QString("snirdev%1battchangedate").arg(devices[i].address)
                        : QString("snirdev%1battchangedate").arg(devices[i].name);
                mds->settings->setValue(key, now);
                devices[i].battChangeDate = now;
            }

            // Обновление прогноза батареи, если уровень изменился и это не замена
            if (oldBatt != dev.batt && !isBatteryReplaced) {
                updateBatteryPrediction(devices[i], dev.batt);
            }
            // Если замена батареи – сбрасываем прогноз
            if (isBatteryReplaced) {
                devices[i].lastBattPercent = dev.batt;
                devices[i].lastBattTime = QDateTime::currentDateTime();
                devices[i].estimatedRemainingHours = -1;
            }

            // Обработка в зависимости от parametr
            if (dev.parametr != 31) {
                // Нормальный пакет с данными
                devices[i].parametr = dev.parametr;
                devices[i].lastData = dev.lastData;
                addDeviceLog(&devices[i]);          // логируем только реальные данные
            } else {
                // Keep-alive пакет: не меняем parametr и lastData, только обновляем время
                // (parametr и lastData остаются прежними)
            }

            updateTimes(&devices[i]);
            checkDevices();
            return; // Device updated — no need to add as new
        }
    }

    // --- New Device Case ---
    // If no matching device was found, treat this as a newly discovered device

    if (dev.isSNIRF) {
        // Инициализируем весь массив специальным значением (например, 255 = "нет данных")
        memset(dev.errorlog, 255, sizeof(dev.errorlog));
        memset(dev.countlog, 0, sizeof(dev.countlog));

        int currentHour = QTime::currentTime().hour();
        // Текущий час отмечаем как хороший (0 ошибок)
        dev.errorlog[currentHour] = dev.retry;
        dev.countlog[currentHour] = 1;
        dev.minInterval = 0;
        dev.maxInterval = 0;
        dev.estimatedRemainingHours = -1;
        if (dev.parametr!= 31) addDeviceLog(&dev);
    } else {
        // Для MQTT устройств errorlog не используется, но на всякий случай обнулим
        memset(dev.errorlog, 0, sizeof(dev.errorlog));
    }

    dev.lastBattPercent = dev.batt;
    dev.lastBattTime = QDateTime::currentDateTime();
    dev.estimatedRemainingHours = -1;

    // Record initial "battery changed" timestamp (used for battery life estimation)
    QDateTime now = QDateTime::currentDateTime();
    QString key = dev.isSNIRF
            ? QString("snirdev%1battchangedate").arg(dev.address)
            : QString("snirdev%1battchangedate").arg(dev.name);
    mds->settings->setValue(key, now);
    dev.battChangeDate = now;


    // Initialize hysteresis state
    dev.isLowBattery = (dev.batt <= LOW_BATT_THRESHOLD);

    // Add to registry
    devices.append(dev);


    // Recalculate global counters (low battery / timeout devices)
    checkDevices(); // recompute counters
}

// ————————————————————————————————————————————————
// DEVICE MANAGEMENT
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::removeDevice(int index) {
    if (index >= 0 && index < devices.count()) {
        if (devices[index].isSNIRF) mds->settings->remove(QString("snirdev%1customname").arg(devices[index].address));
        if (devices[index].isSNIRF) mds->settings->remove(QString("snirdev%1battchangedate").arg(devices[index].address));
        else mds->settings->remove(QString("snirdev%1battchangedate").arg(devices[index].name));
        devices.removeAt(index);
    }
}

// ————————————————————————————————————————————————
// NOTIFICATIONS & BUS INTEGRATION
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::busPuplic(QSNSNIR snir) {
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_SNIRF;
    container.Signal = snir.address;
    container.Sender = snir.parameter;
    container.info = QString::number(snir.battery);
    container.Data = snir.data;
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageSNIRF::checkDevices() {
    battLowCount = 0;
    timeoutCount = 0;
    for (int i = 0; i < devices.count(); i ++) {
        if (devices[i].batt <= 10) battLowCount ++;
        if (devices[i].lastTimeUpdate.addSecs(devices[i].isSNIRF?SNIRF_TimeOut_global:devices[i].timeout) < QDateTime::currentDateTime()) {
            timeoutCount ++;
            if (!devices[i].isTimeout) sendMessage(0, tr("The device is not available {device with %1}").arg(getDevName(&devices[i])));
            devices[i].isTimeout = true;
        }
    }
}

// ————————————————————————————————————————————————
// LOGGING
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::addDeviceLog(device *dev)
{
    // Do not add a duplicate if the log is not empty and the data matches.
    if (!dev->log.isEmpty()) {
        const dev_log &last = dev->log.last();
        if (last.data == dev->lastData && last.parametr == dev->parametr) {
            // Update only the time of the last event
            dev->log[dev->log.size() - 1].timestamp = dev->lastTimeUpdate;
            return;
        }
    }

    dev_log logitem;
    logitem.timestamp = dev->lastTimeUpdate;
    logitem.data = dev->lastData;
    logitem.parametr = dev->parametr;
    dev->log.append(logitem);

    while (dev->log.count() > 50)
        dev->log.removeFirst();
}

void QsnWebPageSNIRF::sendMessage(quint8 importance, QString message)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MESSAGE;
    container.Signal = 3;
    container.Sender = 0;
    container.info = QSNWarningLevelToText(importance) + message;
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageSNIRF::timeProcessing()
{
    QTime now = QTime::currentTime();
    int minute = now.minute();
    int hour = now.hour();

    // Проверка смены минуты (не чаще раза в минуту)
    if (minute != lastMinute) {
        lastMinute = minute;
        checkDevices();
    }

    // Проверка смены часа (один раз в час)
    if (hour != lastHour) {
        lastHour = hour;
        // Определяем предыдущий час, который только что закончился
        int prevHour = (hour - 1 + 24) % 24;
        for (int i = 0; i < devices.size(); ++i) {
            if (!devices[i].isSNIRF) continue;
            devices[i].errorlog[hour] = 255;
            devices[i].countlog[hour] = 0;
            if (devices[i].errorlog[prevHour] == 255) {
                devices[i].errorlog[prevHour] = 3; // потеря пакета
            }
        }
    }
}

QString QsnWebPageSNIRF::getDeviceCustomName(int addr, QString defaultName)
{
    return mds->settings->value(QString("snirdev%1customname").arg(addr), defaultName).toString();
}

// ————————————————————————————————————————————————
// CONFIGURATION SAVE/RESTORE
// ————————————————————————————————————————————————

void QsnWebPageSNIRF::configUpdateBegin()
{
    mds->db->tempSettings.insert("snirCount", devices.count());
    for (int i = 0; i < devices.count(); i ++) {
        mds->db->tempSettings.insert(QString("snir%1address").arg(i), devices.at(i).address);
        mds->db->tempSettings.insert(QString("snir%1batt").arg(i), devices.at(i).batt);
        mds->db->tempSettings.insert(QString("snir%1last").arg(i), devices.at(i).lastTimeUpdate);
        mds->db->tempSettings.insert(QString("snir%1lastData").arg(i), devices.at(i).lastData);
        mds->db->tempSettings.insert(QString("snir%1parametr").arg(i), devices.at(i).parametr);
        mds->db->tempSettings.insert(QString("snir%1istimeout").arg(i), devices.at(i).isTimeout);
        mds->db->tempSettings.insert(QString("snir%1name").arg(i), devices.at(i).name);
        mds->db->tempSettings.insert(QString("snir%1customname").arg(i), devices.at(i).customName);
        mds->db->tempSettings.insert(QString("snir%1issnirf").arg(i), devices.at(i).isSNIRF);
        mds->db->tempSettings.insert(QString("snir%1timeout").arg(i), devices.at(i).timeout); // note: original code overwrites "timeout" key!
        mds->db->tempSettings.insert(QString("snir%1islowbatt").arg(i), devices.at(i).isLowBattery);
        mds->db->tempSettings.insert(QString("snir%1battChangeDate").arg(i), devices.at(i).battChangeDate);
        mds->db->tempSettings.insert(QString("snir%1minInterval").arg(i), devices.at(i).minInterval);
        mds->db->tempSettings.insert(QString("snir%1maxInterval").arg(i), devices.at(i).maxInterval);
        mds->db->tempSettings.insert(QString("snir%1estimatedRemainingHours").arg(i), devices.at(i).estimatedRemainingHours);
        mds->db->tempSettings.insert(QString("snir%1lastBattPercent").arg(i), devices.at(i).lastBattPercent);
        mds->db->tempSettings.insert(QString("snir%1lastBattTime").arg(i), devices.at(i).lastBattTime);

        // Сохраняем errorlog
        if (devices[i].isSNIRF) {
            QByteArray errorlogData(reinterpret_cast<const char*>(devices[i].errorlog), sizeof(devices[i].errorlog));
            mds->db->tempSettings.insert(QString("snir%1errorlog").arg(i), errorlogData);
            QByteArray countlogData(reinterpret_cast<const char*>(devices[i].countlog), sizeof(devices[i].countlog));
            mds->db->tempSettings.insert(QString("snir%1countlog").arg(i), countlogData);
        }

        // Сохраняем лог как QVariantList
        QVariantList logList;
        for (int j = 0; j < devices[i].log.size(); ++j) {
            const auto &entry = devices[i].log.at(j);
            QVariantMap map;
            map["timestamp"] = entry.timestamp;
            map["data"] = entry.data;
            map["parametr"] = entry.parametr;
            logList.append(map);
        }
        mds->db->tempSettings.insert(QString("snir%1log").arg(i), logList);
    }
    checkDevices();
}

void QsnWebPageSNIRF::configUpdateEnd()
{
    int count = mds->db->tempSettings.value(QString("snirCount"), 0).toInt();
    device d;
    for (int i = 0; i < count; i ++) {
        d.address = mds->db->tempSettings.value(QString("snir%1address").arg(i), 0).toUInt();
        d.batt = mds->db->tempSettings.value(QString("snir%1batt").arg(i), 0).toUInt();
        d.lastTimeUpdate = mds->db->tempSettings.value(QString("snir%1last").arg(i), QDateTime::currentDateTime()).toDateTime();
        d.lastData = mds->db->tempSettings.value(QString("snir%1lastData").arg(i), QByteArray()).toByteArray();
        d.parametr = mds->db->tempSettings.value(QString("snir%1parametr").arg(i), 0).toUInt();
        d.isTimeout = mds->db->tempSettings.value(QString("snir%1istimeout").arg(i), 0).toBool();
        d.isSNIRF = mds->db->tempSettings.value(QString("snir%1issnirf").arg(i), 0).toBool();
        d.name = mds->db->tempSettings.value(QString("snir%1name").arg(i), QString()).toString();
        d.customName = mds->db->tempSettings.value(QString("snir%1customname").arg(i), QString()).toString();
        d.timeout = mds->db->tempSettings.value(QString("snir%1timeout").arg(i), 0).toUInt();
        d.isLowBattery = mds->db->tempSettings.value(QString("snir%1islowbatt").arg(i), false).toBool();
        d.battChangeDate = mds->db->tempSettings.value(QString("snir%1battChangeDate").arg(i), QDateTime::currentDateTime()).toDateTime();
        d.minInterval = mds->db->tempSettings.value(QString("snir%1minInterval").arg(i), 0).toUInt();
        d.maxInterval = mds->db->tempSettings.value(QString("snir%1maxInterval").arg(i), 0).toUInt();
        d.estimatedRemainingHours = mds->db->tempSettings.value(QString("snir%1estimatedRemainingHours").arg(i), -1).toLongLong();
        d.lastBattPercent = mds->db->tempSettings.value(QString("snir%1lastBattPercent").arg(i), 0).toUInt();
        d.lastBattTime = mds->db->tempSettings.value(QString("snir%1lastBattTime").arg(i), QDateTime()).toDateTime();

        if (!d.lastBattTime.isValid() && d.batt > 0) {
            d.lastBattTime = QDateTime::currentDateTime();
            d.lastBattPercent = d.batt;
            d.estimatedRemainingHours = -1;
        }

        // Восстанавливаем errorlog
        if (d.isSNIRF) {
            QByteArray errorlogData = mds->db->tempSettings.value(QString("snir%1errorlog").arg(i)).toByteArray();
            if (errorlogData.size() == sizeof(d.errorlog)) {
                memcpy(d.errorlog, errorlogData.data(), sizeof(d.errorlog));
            } else {
                memset(d.errorlog, 255, sizeof(d.errorlog));
            }
            QByteArray countlogData = mds->db->tempSettings.value(QString("snir%1countlog").arg(i)).toByteArray();
            if (countlogData.size() == sizeof(d.countlog)) {
                memcpy(d.countlog, countlogData.data(), sizeof(d.countlog));
            } else {
                memset(d.countlog, 0, sizeof(d.countlog));
            }
        } else {
            memset(d.errorlog, 0, sizeof(d.errorlog));
            memset(d.countlog, 0, sizeof(d.countlog));
        }

        // Восстанавливаем лог
        QVariantList logList = mds->db->tempSettings.value(QString("snir%1log").arg(i)).toList();
        for (int j = 0; j < logList.size(); ++j) {
            QVariantMap map = logList.at(j).toMap();
            dev_log entry;
            entry.timestamp = map["timestamp"].toDateTime();
            entry.data = map["data"].toString();
            entry.parametr = map["parametr"].toUInt();
            d.log.append(entry);
        }

        devices.append(d);
    }
}

QString QsnWebPageSNIRF::getDevName(device *dev)
{
    if (dev->isSNIRF) {
        if (!dev->customName.isEmpty()) return QString("%1").arg(dev->customName);
        else return QString("SNIRF");

    }
    return QString("%1").arg(dev->name);
}

QString QsnWebPageSNIRF::formatBatteryLife(const QDateTime &since)
{
    if (!since.isValid()) {
        return QString();
    }

    const qint64 secs = since.secsTo(QDateTime::currentDateTime());
    if (secs <= 0) {
        return tr("< minute");
    }

    const int totalDays = static_cast<int>(secs / 86400);

    if (totalDays == 0) {
        // Less than a day — show hours or minutes
        const int hours = static_cast<int>(secs / 3600);
        if (hours > 0) {
            return tr("%n hour(s)", "", hours);
        } else {
            const int mins = static_cast<int>(secs / 60);
            return tr("%n minute(s)", "", mins);
        }
    }

    // Show full days
    return tr("%n day(s)", "", totalDays);
}

QString QsnWebPageSNIRF::getSNIRFSignalStatus(const device &dev) const
{
    if (!dev.isSNIRF) return "Unknown";

    bool hasData = false;
    for (int i = 0; i < 24; ++i) {
        if (dev.errorlog[i] != 255) {
            hasData = true;
            break;
        }
    }
    if (!hasData) return "<span style='color: #6c757d;'>No Data</span>";

    int category = getSignalQualityCategory(dev.errorlog);
    int percent = getSignalQualityPercent(dev.errorlog);
    QString graph = getSignalGraph(dev.errorlog);

    QString color;
    switch (category) {
    case 0: color = "#2ecc71"; break; // зелёный
    case 1: color = "#f39c12"; break; // оранжевый
    case 2: color = "#e74c3c"; break; // красный
    }

    return QString("<span style='color: %1; font-weight: 500;'>%2%</span> "
                   "<span style='font-family: monospace; font-size: 0.8em; color: %1;'>%3</span>")
            .arg(color).arg(percent).arg(graph);
}

QString QsnWebPageSNIRF::buildIndicator(const QString &icon, const QString &label, const QString &suffix, const QString &id)
{
    QString ret;
    ret += QString("<div class=\"col-auto\" style=\"padding-left: 1px; padding-right: 1px;\">");
    ret += QString("<div class=\"d-flex\">");
    ret += QString("<h6 class=\"my-auto\" id=\"B-%1\" "
                   "style=\"display: flex; align-items: center; gap: 0.3em; font-size: 0.85em;\">").arg(id);
    ret += QString("<span style=\"font-size: 1em;\">%1</span>").arg(icon);
    ret += QString("<span>%1%2</span>").arg(label, suffix);
    ret += QString("</h6>");
    ret += QString("</div>");
    ret += QString("</div>");
    return ret;
}

int QsnWebPageSNIRF::getSignalQualityCategory(const quint8 errorlog[24]) const
{
    int percent = getSignalQualityPercent(errorlog);
    if (percent >= 70) return 0;      // хороший
    if (percent >= 30) return 1;      // плохой
    return 2;                         // очень плохой
}

int QsnWebPageSNIRF::getSignalQualityPercent(const quint8 errorlog[24]) const
{
    int totalWeight = 0;
    int totalHours = 0;

    for (int i = 0; i < 24; ++i) {
        if (errorlog[i] == 255) continue;
        totalHours++;
        switch (errorlog[i]) {
        case 0: totalWeight += 100; break;
        case 1: totalWeight += 66; break;
        case 2: totalWeight += 33; break;
        default: totalWeight += 0; break; // retry=3 или выше
        }
    }
    return (totalHours == 0) ? 0 : (totalWeight / totalHours);
}

QString QsnWebPageSNIRF::getSignalGraph(const quint8 errorlog[24]) const
{
    QString graph;
    for (int hour = 0; hour < 24; ++hour) {
        if (errorlog[hour] == 255) {
            graph += "·";
        } else if (errorlog[hour] == 0) {
            graph += "█";
        } else if (errorlog[hour] == 1) {
            graph += "▒";
        } else if (errorlog[hour] == 2) {
            graph += "░";
        } else { // retry >= 3
            graph += "·";
        }
    }
    return graph;
}

QString QsnWebPageSNIRF::getSignalGraphHTML(const device &dev) const
{
    const int containerHeight = 16;  // высота контейнера для одного часа (пиксели)
    const int barWidth = 8;          // ширина столбика

    bool hasData = false;
      for (int i = 0; i < 24; ++i) {
          if (dev.errorlog[i] != 255) { hasData = true; break; }
      }
      if (!hasData)
          return "<div style='text-align: center; color: #6c757d;'>No data available</div>";

      QString html = "<div style='display: flex; align-items: flex-end; gap: 2px;'>";

      for (int hour = 0; hour < 24; ++hour) {
          quint8 retry = dev.errorlog[hour];
          quint8 count = dev.countlog[hour];
          if (count > 9) count = 9; // для отображения одной цифры

          // Высота столбика
          int barHeight = containerHeight;
          QColor barColor;
          if (retry == 255) {
              barHeight = 6;
              barColor = QColor(211, 211, 211);
          } else if (retry == 0) {
              barHeight = containerHeight;
              barColor = QColor(46, 204, 113);
          } else if (retry == 1) {
              barHeight = containerHeight * 0.8;
              barColor = QColor(243, 156, 18);
          } else if (retry == 2) {
              barHeight = containerHeight * 0.6;
              barColor = QColor(231, 76, 60);
          } else {
              barHeight = containerHeight * 0.4;
              barColor = QColor(150, 150, 150);
          }

          // Формируем подсказку
          QString title = tr("Hour %1:00 | retry=%2 | count=%3")
                          .arg(hour)
                          .arg(retry == 255 ? tr("no data") : QString::number(retry))
                          .arg(dev.countlog[hour]);

          // Контейнер с относительным позиционированием
          html += "<div style='position: relative; width: " + QString::number(barWidth) +
                  "px; height: " + QString::number(containerHeight) +
                  "px; display: flex; align-items: flex-end;' title='" + title + "'>";

          // Столбик
          html += "<div style='width: 100%; height: " + QString::number(barHeight) +
                  "px; background-color: " + barColor.name() +
                  "; border-radius: 2px;'></div>";

          // Цифра всегда показывается (0..9)
          html += "<div style='position: absolute; top: 50%; left: 0; right: 0; "
                  "transform: translateY(-50%); text-align: center; "
                  "font-size: 10px; font-weight: bold; color: white; "
                  "text-shadow: 0 0 1px black; pointer-events: none;'>" +
                  (count?QString::number(count):"") + "</div>";

          html += "</div>";
      }
      html += "</div>";
      return html;
}

void QsnWebPageSNIRF::updateTimes(device *dev)
{
    if (!dev) return;

    QDateTime now = QDateTime::currentDateTime();
    int currentHour = now.time().hour();

    dev->countlog[currentHour] ++;

    if (dev->retry > dev->errorlog[currentHour] || dev->errorlog[currentHour] == 255) dev->errorlog[currentHour] = dev->retry;

    if (dev->lastTimeUpdate.isValid()) {
        quint32 interval = static_cast<quint32>(dev->lastTimeUpdate.secsTo(now));

        // Обновляем минимум
        if (dev->minInterval == 0 || interval < dev->minInterval) {
            dev->minInterval = interval;
           if (dev->maxInterval > dev->minInterval) dev->maxInterval -= 1;
        }

        // Обновляем максимум
        if (interval > dev->maxInterval) {
            dev->maxInterval = interval;
            if (dev->minInterval < dev->maxInterval) dev->minInterval += 1;
        }

    } else {
        // Первый heartbeat
        dev->minInterval = 0;
        dev->maxInterval = 0;
    }

    dev->lastTimeUpdate = now;
}

QString QsnWebPageSNIRF::formatInterval(quint32 seconds) const
{
    if (seconds == 0) return tr("N/A");

    if (seconds < 60) {
        return tr("%1 sec").arg(seconds);
    } else if (seconds < 3600) {
        return tr("%1 min %2 sec").arg(seconds / 60).arg(seconds % 60);
    } else {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        return tr("%1 h %2 min").arg(hours).arg(minutes);
    }
}



QString QsnWebPageSNIRF::formatRemainingTime(qint64 hours) const
{
    if (hours < 0) return tr("---");
    if (hours == 0) return tr("⚠️ Critical!");

    if (hours < 24) {
        return tr("%1 h").arg(hours);
    }

    int days = hours / 24;
    if (days < 30) {
        int remHours = hours % 24;
        if (remHours > 0) {
            return tr("%1 d %2 h").arg(days).arg(remHours);
        }
        return tr("%1 d").arg(days);
    }

    int months = days / 30;
    return tr("%1 mon").arg(months);
}

void QsnWebPageSNIRF::updateBatteryPrediction(device &dev, quint8 newBatt)
{
    if (!dev.lastBattTime.isValid() || dev.lastBattPercent == newBatt)
        return;

    // Разряд должен идти вниз
    if (newBatt >= dev.lastBattPercent) {
        // Заряд увеличился (возможно замена батареи) – не обновляем прогноз
        dev.lastBattPercent = newBatt;
        dev.lastBattTime = QDateTime::currentDateTime();
        return;
    }

    qint64 deltaSec = dev.lastBattTime.secsTo(QDateTime::currentDateTime());
    int deltaBatt = dev.lastBattPercent - newBatt;
    if (deltaSec > 0 && deltaBatt > 0) {
        double percentPerHour = deltaBatt / (deltaSec / 3600.0);
        if (percentPerHour > 0.0) {
            double remainingHours = newBatt / percentPerHour;
            dev.estimatedRemainingHours = qRound64(remainingHours);
        } else {
            dev.estimatedRemainingHours = -1;
        }
    } else {
        dev.estimatedRemainingHours = -1;
    }

    dev.lastBattPercent = newBatt;
    dev.lastBattTime = QDateTime::currentDateTime();
}

