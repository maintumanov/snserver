#include "qsnwebwidgetcardindicator.h"

/**
 * @brief Constructs a QsnWebWidgetCardIndicator instance.
 *
 * This widget acts as a dynamic indicator that can display real-time data from various sources:
 * MQTT topics, SNIRF streams, database entries, or internal I/O signals.
 * It supports visual state (active/inactive), warning levels, and optional persistent storage.
 *
 * @param iID Unique identifier for this widget.
 * @param modules Pointer to global application modules (settings, DB, MQTT, etc.).
 * @param parent Parent QObject (for memory management).
 */
QsnWebWidgetCardIndicator::QsnWebWidgetCardIndicator(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Indicator"));
    labelString = tr("no data");
    mds = modules;

    // Default states
    isActive = 0;           // Activity counter in minutes (0 = inactive, >0 = active, 100 = always active)
    currentType = 0;        // Last known data type (first byte of payload)
    currentSubType = -1;    // Subtype byte from type-1 packets; -1 means unknown
    emptysignal = 0;        // Behavior on empty input: 0=ignore, 1=timestamp, 2=count
    activeMin = 0;          // Seconds counter for minute-based processing
    fdbTimer = 0;           // Seconds since last DB write (for interval filling)
    confidenceTime = 0;   //< Seconds after which indicator becomes inactive if no updates

    // Initialize child components to nullptr
    mqtt = Q_NULLPTR;
    mqtts = Q_NULLPTR;
    mqtta = Q_NULLPTR;
    output = Q_NULLPTR;
    comp = Q_NULLPTR;
    msg = Q_NULLPTR;

    // Connect to global bus for external data (e.g., time ticks, DB updates)
    if (mds->interface != Q_NULLPTR)
        connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

/**
 * @brief Appends JSON representation of available functions for this widget.
 *
 * Used by the web UI to bind interactive controls (e.g., button actions).
 *
 * @param fjson Output list to append JSON strings.
 * @param accountIndex Ignored (reserved for future multi-account support).
 */
void QsnWebWidgetCardIndicator::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QsnBsPanelCardItemLabelFunJSON(itemID());
}

/**
 * @brief Generates HTML content for rendering this indicator in a Bootstrap-based web UI.
 *
 * Includes icon, alias/title, formatted value, and visual state (active/warning/danger).
 *
 * @param contents Output list to append HTML fragments.
 * @param accountIndex Ignored.
 */
void QsnWebWidgetCardIndicator::getContents(QStringList *contents, int )
{
    QString wr;
    if (warningState() != "none") wr = " text-" + warningState();   // e.g., "text-warning"
    *contents << QString("<li class=\"list-group-item d-flex  px-2%2\" data-unit=\"label-%1\">").arg(itemID()).arg((isActive?" active":""));
    *contents << QsnBsIconNum(QSNAutoIcon(iconNum, currentType), getLink());
    *contents << QString("<p class=\"specs\">%1</p>").arg(widgetAlias.isEmpty() ? objectName() : widgetAlias);
    *contents << QString("<h6 class=\"ms-auto text-primary%3\" id=\"label-%1\">%2</h6>").arg(itemID()).arg(labelString, wr);
    *contents << QString("</li>");
}

/**
 * @brief Serializes current state into JSON for AJAX/API responses.
 *
 * Exports: label text, active status, and warning level.
 *
 * @param jsonItems Output list of JSON property strings.
 */
void QsnWebWidgetCardIndicator::getItemJSON(QStringList *jsonItems)
{
    *jsonItems << QsnBsJsonItemText("label", QString::number(itemID()), "data", labelString);
    *jsonItems << QsnBsJsonItem("label", QString::number(itemID()), "active", isActive?"true":"false");
    *jsonItems << QsnBsJsonItemText("label", QString::number(itemID()), "warning", warningState());
}

/**
 * @brief Deserializes widget configuration from a binary stream.
 *
 * Loads child components (MQTT, output, comparator, etc.), options, and restores last known state
 * from temporary settings (e.g., after restart).
 *
 * @param stream Input data stream (must be valid and in correct format).
 * @param path Base path used for MQTT topic resolution and file references.
 */
void QsnWebWidgetCardIndicator::fromStream(QDataStream *stream, QString path)
{
    if (!stream || stream->status() != QDataStream::Ok) {
        qWarning() << "QsnWebWidgetCardIndicator::fromStream(): invalid stream";
        return;
    }

    int id;
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

      // Clear existing SNIRF handlers
    qDeleteAll(snrfs);
    snrfs.clear();

    // Load general options (AL=alias, IC=icon, CT=confidence time, etc.)
    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

     // Load I/O bindings (e.g., signal index mappings)
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;

        // --- MQTT Data Source ---
        if (sig == QString(QLatin1String("MQI"))) {
            QsnWebWidgetCardIndicatorMQTT *imqtt = new QsnWebWidgetCardIndicatorMQTT(static_cast<quint32>(id), modules(), this);
            if (mqtt != Q_NULLPTR) {
                disconnect(mqtt, nullptr, this, nullptr);
                delete mqtt;
            }
            mqtt = imqtt;
            mqtt->fromStream(stream, getPath());
            connect(mqtt, SIGNAL(onMQTT(QByteArray, QString)), this, SLOT(setIndicatorValue(QByteArray, QString)));
        }

        // --- MQTT State Source (activates on message) ---
        if (sig == QString(QLatin1String("MQIS"))) {
            QsnWebWidgetCardIndicatorMQTTState *imqtts = new QsnWebWidgetCardIndicatorMQTTState(static_cast<quint32>(id), modules(), this);
            if (mqtts != Q_NULLPTR) {
                disconnect(mqtts, nullptr, this, nullptr);
                delete mqtts;
            }
            mqtts = imqtts;
            mqtts->fromStream(stream, getPath());
            connect(mqtts, SIGNAL(onMQTT(QByteArray, QString)), this, SLOT(setIndicatorValue(QByteArray, QString)));
            connect(mqtts, SIGNAL(onActive(QString)), this, SLOT(setActive(QString)));
        }

        // --- MQTT Alert Source ---
        if (sig == QString(QLatin1String("MQIA"))) {
            QsnWebWidgetCardIndicatorMQTTAlert *imqtta = new QsnWebWidgetCardIndicatorMQTTAlert(static_cast<quint32>(id), modules(), this);
            if (mqtta != Q_NULLPTR) {
                disconnect(mqtta, nullptr, this, nullptr);
                delete mqtta;
            }
            mqtta = imqtta;
            mqtta->fromStream(stream, getPath());
            connect(mqtta, SIGNAL(onMQTT(QByteArray, QString)), this, SLOT(setIndicatorValue(QByteArray, QString)));
            connect(mqtta, SIGNAL(onActive(QString)), this, SLOT(setActive(QString)));
        }

        // --- Output Trigger ---
        if (sig == QString(QLatin1String("OO"))) {
            QsnWebWidgetCardIndicatorOutput *ioutput = new QsnWebWidgetCardIndicatorOutput(static_cast<quint32>(id), modules(), this);
            if (output) {
                disconnect(output, nullptr, this, nullptr);
                delete output;
            }
            output = ioutput;
            output->fromStream(stream);
        }

        // --- Comparator (determines warning state) ---
        if (sig == QString(QLatin1String("INC"))) {
            QsnWebWidgetCardIndicatorComp *icomp = new QsnWebWidgetCardIndicatorComp(static_cast<quint32>(id), modules(), this);
            if (comp != Q_NULLPTR) {
                disconnect(comp, nullptr, this, nullptr);
                delete comp;
            }
            comp = icomp;
            comp->fromStream(stream, getPath());
            connect(comp, SIGNAL(onStateChange(quint8)), this, SLOT(onCompChange(quint8)));
        }

        // --- Message Sender (e.g., SMS, push) ---
        if (sig == QString(QLatin1String("INM"))) {
            QsnWebWidgetCardIndicatorMsg *imsg = new QsnWebWidgetCardIndicatorMsg(static_cast<quint32>(id), modules(), this);
            if (msg) {
                disconnect(msg, nullptr, this, nullptr);
                delete msg;
            }
            msg = imsg;
            msg->fromStream(stream, getPath());
        }

        // --- SNIRF Stream Handler (multiple allowed) ---
        if (sig == QString(QLatin1String("IIR"))) {
            QsnWebWidgetCardIndicatorSNIRF *isnirf = new QsnWebWidgetCardIndicatorSNIRF(static_cast<quint32>(id), modules(), this);
            isnirf->fromStream(stream);
            snrfs.append(isnirf);
            connect(isnirf, SIGNAL(onSNIRF(QByteArray, QString)), this, SLOT(setIndicatorValue(QByteArray, QString)));
            connect(isnirf, SIGNAL(onSNIRFactive(QString)), this, SLOT(setActive(QString)));
        }

    }

    // Apply loaded options
    widgetAlias = optionsMap.value("AL", QString()).toString();
    iconNum = optionsMap.value("IC", 0).toInt();                          // Icon index (0 = auto)
    confidenceTime = optionsMap.value("CT", 600).toInt();                 // Relevance time in **minutes**
    emptysignal = optionsMap.value("EM", 0).toInt();                      // Empty signal mode
    dbName = optionsMap.value("YDB", QString()).toString();               // YBD database name
    filldb = optionsMap.value("FDB", false).toBool();                     // Enable periodic DB logging

    // Restore last runtime state from temp settings
    QString tsname = dbName.isEmpty()?objectName()+QString::number(itemID()):dbName;
    lastData = mds->db->tempSettings.value(tsname + "lastdata", QByteArray()).toByteArray();
    isActive = mds->db->tempSettings.value(tsname + "active", 0).toInt();
    labelString = mds->db->tempSettings.value(tsname + "label", labelString).toString();
    fdbTimer = mds->db->tempSettings.value(tsname + "fdbtimer", fdbTimer).toInt();

    // Restore remembered data type (for icon selection)
    currentType = mds->settings->value(QString("%1_labelType").arg(itemID()), 0).toInt();

    // If confidenceTime is 0, treat as "always active"
    if (confidenceTime == 0) isActive = 100;
}

/**
 * @brief Handles incoming I/O signal (e.g., from hardware sensor).
 *
 * Only index 0 is accepted (primary data channel).
 *
 * @param indexIO Signal index (ignored unless 0).
 * @param data Raw byte array payload.
 */
void QsnWebWidgetCardIndicator::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO != 0) return;
    setIndicatorValue(*data, "SIGNAL");
}

/**
 * @brief Processes messages from the global system bus (snBUS).
 *
 * Handles:
 * - Time ticks (for confidence timeout)
 * - Database updates (if dbName matches)
 * - MQTT requests/responses
 * - Configuration save events
 *
 * @param container Message payload.
 * @param sender Originator (ignored if self).
 */
void QsnWebWidgetCardIndicator::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) timeProcessing();

    // Database update: apply if name matches
    if (container.Command == BUSSERV_DB) {
        if (!dbName.isEmpty() && container.info == dbName) {
            setIndicatorValue(container.Data, "BUSSERV_DB");
        }
    }

    // MQTT: respond to requests or publish current state
    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_sn && container.info == getPath()) {
            setIndicatorValue(container.Data, "BUSSERV_MGTT_SN");
            writeToMQTTPSN(lastData);
        }
        if (container.Signal == BUSSERV_MGTT_request_sn && QSNMQTTTopicFilterCompare(getPath(), container.info)) writeToMQTTPSN(lastData);
    }

    // Save current state before config reload
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        QString tsname = dbName.isEmpty()?objectName()+QString::number(itemID()):dbName;
        mds->db->tempSettings.insert(tsname + "lastdata", lastData);
        mds->db->tempSettings.insert(tsname + "active", isActive);
        mds->db->tempSettings.insert(tsname + "label", labelString);
        mds->db->tempSettings.insert(tsname + "fdbtimer", fdbTimer);
    }
}

/**
 * @brief Updates the displayed value and internal state based on new data.
 *
 * Supports special modes for empty input:
 * - EM=0: ignore
 * - EM=1: generate zero-value packet with timestamp (13 bytes)
 * - EM=2: increment counter (27-byte packet with 32-bit counter at offset 1)
 *
 * Also triggers output, DB write, and MQTT publish (unless suppressed by source).
 *
 * @param data New raw data (may be empty).
 * @param source Identifier for origin (used to suppress feedback loops).
 */
void QsnWebWidgetCardIndicator::setIndicatorValue(QByteArray data, QString source)
{
    if (data.isEmpty()) {
        if (emptysignal == 0) return;  // Ignore empty signals
        if (emptysignal == 1) {
            // Generate a 13-byte timestamp-only packet:
            // [type=0][12-byte datetime]
            QSNByteToRAW(&lastData, 0, 13);
            QSNDateTimeToRAW(&lastData, 1, QDateTime::currentDateTime());
        }

        if (emptysignal == 2) {
            // Maintain and increment a 32-bit counter in a 27-byte packet:
            // [type=0][counter (4 bytes)][padding...]
            int c = QSNRAWToVariant(&lastData).toUInt();
            c ++;
            QSNByteToRAW(&lastData, 0, 27);
            QSNInt32ToRAW(&lastData, 1, c);
        }
    } else {
        lastData = data;

        // Persist to YBD database (unless from DB itself)
        if (source != "BUSSERV_DB") mds->db->writeRAWtoYBD(dbName, &lastData, QDateTime::currentDateTime(), objectName(), this);

        // Publish to MQTT (unless from MQTT itself)
        if (source != "BUSSERV_MGTT_SN") writeToMQTTPSN(lastData);

        // Trigger output module (e.g., relay, LED)
        if (output != Q_NULLPTR) output->outputSignal(lastData);

        fdbTimer = 0; // Reset fill-db timer

    }

    // Update remembered type and label
    if (!lastData.isEmpty() && currentType != static_cast<quint8>(lastData.at(0))) {
        currentType = static_cast<quint8>(lastData.at(0));
        mds->settings->setValue(QString("%1_labelType").arg(itemID()), currentType);
    }

    // Format human-readable label
    labelString = QSNRAWtoScaledVolume(&lastData);

    // Reset activity timer: value is in **minutes**
    isActive = confidenceTime == 0?100:confidenceTime;

    // Notify comparator (updates warning state)
    if (comp != Q_NULLPTR) comp->importance(lastData);
}

/**
 * @brief Reactivates the indicator (e.g., from MQTT alert or SNIRF).
 *
 * Resets `isActive` to full confidence duration.
 *
 * @param source Unused (kept for signal compatibility).
 */
void QsnWebWidgetCardIndicator::setActive(QString /*source*/)
{
    isActive = confidenceTime == 0?100:confidenceTime;
}

/**
 * @brief Called when comparator detects state change (e.g., threshold breach).
 *
 * Triggers message notification if configured.
 *
 * @param newState Importance level (0=danger, 1=warning, 2=ok).
 */
void QsnWebWidgetCardIndicator::onCompChange(quint8 newState)
{
    if (msg != Q_NULLPTR) msg->sendMSG(newState, lastData);
}

/**
 * @brief Periodic processing triggered once per second via snBUS.
 *
 * - Decrements `isActive` counter (timeout logic).
 * - Periodically writes to DB if `filldb` is enabled.
 */
void QsnWebWidgetCardIndicator::timeProcessing()
{
    // This function is called once per second via BUSSERV_TIME_SEC_EVENT
    activeMin ++;
    if (activeMin >= 59) {
        // Once per minute:
        activeMin = 0;
        if (confidenceTime != 0 && isActive > 0) {
            isActive --; // Decrement minute counter
            if (!isActive && msg != Q_NULLPTR) msg->sendMSG(4, QByteArray());
        }
    }
    // Periodic database logging (if enabled)
    if (isActive && !dbName.isEmpty() && filldb && !lastData.isEmpty()) {
        if (fdbTimer < 59) fdbTimer ++;
        else {
            // Write to DB every 60 seconds while active
            mds->db->writeRAWtoYBD(dbName, &lastData, QDateTime::currentDateTime(), objectName(), this);
            if (output != Q_NULLPTR) output->outputSignal(lastData);
            fdbTimer = 0;
        }
    }
}

/**
 * @brief Publishes current data to MQTT under this widget's path.
 *
 * Skipped if MQTT is disabled or data is empty.
 *
 * @param data Payload to publish.
 */
void QsnWebWidgetCardIndicator::writeToMQTTPSN(QByteArray data)
{
    if (!mds->supportMQTT) return;
    if (data.count() == 0) return;
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_publication_sn;
    container.Sender = 0;
    container.info = getPath();
    container.Data = data;
    mds->interface->snBUSInput(container, this);
}

/**
 * @brief Injects or enforces a subtype byte in raw data packets.
 *
 * Assumes packet format: [type=1][...][subtype@offset=2].
 * Used to maintain consistent device subtype across updates.
 *
 * @param data Input packet.
 * @param issignal If true, update `currentSubType` from incoming data.
 * @return Modified packet with enforced subtype.
 */
QByteArray QsnWebWidgetCardIndicator::addSubType(QByteArray data, bool issignal = true)
{
    if (data.size() < 2) return data;
    if (data.at(0) != 1) return data;
    if (data.size() < 3) {currentSubType = -1; return data;}
    if (issignal) currentSubType = data.at(2);
    if (defaultSubType > 0) currentSubType = defaultSubType;
    QByteArray ret = data;
    if (currentSubType != -1 && ret.at(2) == 0) ret[2] = currentSubType;
    return ret;
}

/**
 * @brief Determines visual warning state based on comparator output.
 *
 * @return "danger", "warning", or "none".
 */
QString QsnWebWidgetCardIndicator::warningState()
{
    if (comp == Q_NULLPTR) return "none";
    quint8 importance = comp->importance(lastData);
    switch (importance) {
    case 0: return "danger";
    case 1: return "warning";
    case 2: return "none";
    }
    return "none";
}

/**
 * @brief Generates onclick handler for graph navigation.
 *
 * @return JavaScript snippet or empty string if no DB name.
 */
QString QsnWebWidgetCardIndicator::getLink()
{
    if (dbName.isEmpty()) return QString();
    QString chOptions;
    if (!filldb) chOptions += "&withoutbreaks";
    return QString("onclick=\"location.href='/graph?name=%1%2';\"").arg(dbName, chOptions);
}

