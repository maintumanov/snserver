#include "qsnshapes.h"

/**
 * @brief Global flag indicating whether temperature values are displayed in Celsius.
 *
 * If true, temperatures are interpreted and shown in degrees Celsius.
 * If false, they are converted to and displayed in Fahrenheit.
 */
bool QSNCelsius = true;

/**
 * @brief Global flag indicating whether pressure values are displayed in millimeters of mercury (mmHg).
 *
 * If true, pressure is shown in mmHg; otherwise, it's shown in hectopascals (hPa).
 */
bool QSNMillimetersOfMercury = true;

/**
 * @brief Returns a human-readable localized label for a given sensor/data type index.
 *
 * Used primarily in UI elements to display type names like "Temperature", "Humidity", etc.
 *
 * @param index Type identifier (0–63). Unknown indices return an empty string.
 * @return QString Localized type name (e.g., "Temperature").
 */
QString QSNTypeLabel(quint8 index) //================================================
{
    switch (index) {
    case 0: return QString(QObject::tr("Signal"));
    case 1: return QString(QObject::tr("State"));
    case 2: return QString(QObject::tr("CO2"));
    case 3: return QString(QObject::tr("Air velocity"));
    case 4: return QString(QObject::tr("Water consumption"));
    case 5: return QString(QObject::tr("uInt8"));
    case 6: return QString(QObject::tr("Int8"));
    case 7: return QString(QObject::tr("uInt16"));
    case 8: return QString(QObject::tr("Int16"));
    case 9: return QString(QObject::tr("Temperature"));
    case 10: return QString(QObject::tr("Power"));
    case 11: return QString(QObject::tr("Current"));
    case 12: return QString(QObject::tr("Voltage"));
    case 13: return QString(QObject::tr("Date and time"));
    case 14: return QString(QObject::tr("Alert"));
    case 15: return QString(QObject::tr("Error"));
    case 16: return QString(QObject::tr("Text"));
    case 17: return QString(QObject::tr("Date"));
    case 18: return QString(QObject::tr("Time"));
    case 19: return QString(QObject::tr("Consumed electricity"));
    case 20: return QString(QObject::tr("Humidity"));
    case 21: return QString(QObject::tr("Pressure"));
    case 22: return QString(QObject::tr("Level"));
    case 23: return QString(QObject::tr("Distance"));
    case 24: return QString(QObject::tr("Angle"));
    case 25: return QString(QObject::tr("Illumination"));
    case 26: return QString(QObject::tr("uInt32"));
    case 27: return QString(QObject::tr("Int32"));
    case 28: return QString(QObject::tr("Capacity"));
    case 29: return QString(QObject::tr("Weight"));
    case 30: return QString(QObject::tr("Hex code"));
    case 31: return QString(QObject::tr("Security alarm"));
    case 32: return QString(QObject::tr("SNIR package"));
    case 33: return QString(QObject::tr("EM-Marine UID"));
    case 34: return QString(QObject::tr("Rainfall"));
    case 35: return QString(QObject::tr("Turnovers"));
    case 36: return QString(QObject::tr("Weather"));
    case 37: return QString(QObject::tr("Dust"));
    case 38: return QString(QObject::tr("AQI"));
    case 39: return QString(QObject::tr("RM"));
    case 63: return QString(QObject::tr("Variant"));
    }
    return QString();
}

/**
 * @brief Returns a machine-readable Latin (ASCII-safe) name for a given type index.
 *
 * Used in JSON keys, MQTT topics, configuration files, etc.
 *
 * @param index Type identifier (0–63).
 * @return QString Latin name (e.g., "temp", "humidity").
 */
QString QSNTypeLatianName(quint8 index) //================================================
{
    switch (index) {
    case 0: return QLatin1String("signal");
    case 1: return QLatin1String("state");
    case 2: return QLatin1String("co2");
    case 3: return QLatin1String("airVelocity");
    case 4: return QLatin1String("waterConsumption");
    case 5: return QLatin1String("uint8");
    case 6: return QLatin1String("int8");
    case 7: return QLatin1String("uint16");
    case 8: return QLatin1String("int16");
    case 9: return QLatin1String("temp");
    case 10: return QLatin1String("power");
    case 11: return QLatin1String("current");
    case 12: return QLatin1String("voltage");
    case 13: return QLatin1String("datetime");
    case 14: return QLatin1String("alert");
    case 15: return QLatin1String("error");
    case 16: return QLatin1String("text");
    case 17: return QLatin1String("date");
    case 18: return QLatin1String("time");
    case 19: return QLatin1String("energy");
    case 20: return QLatin1String("humidity");
    case 21: return QLatin1String("pressure");
    case 22: return QLatin1String("level");
    case 23: return QLatin1String("distance");
    case 24: return QLatin1String("angle");
    case 25: return QLatin1String("illumination");
    case 26: return QLatin1String("uint32");
    case 27: return QLatin1String("int32");
    case 28: return QLatin1String("capacity");
    case 29: return QLatin1String("weight");
    case 30: return QLatin1String("hex");
    case 31: return QLatin1String("security");
    case 32: return QLatin1String("snir");
    case 33: return QLatin1String("emUID");
    case 34: return QLatin1String("rainfall");
    case 35: return QLatin1String("turnovers");
    case 36: return QLatin1String("weather");
    case 37: return QLatin1String("dust");
    case 38: return QLatin1String("AQI");
    case 39: return QLatin1String("RM");
    case 63: return QLatin1String("variant");
    }
    return QString();
}

/**
 * @brief Converts a Latin type name back to its numeric type index.
 *
 * Performs case-insensitive lookup. Returns 63 ("Variant") if no match is found.
 *
 * @param type Latin type name (e.g., "temp").
 * @return quint8 Corresponding type index (0–63).
 */
quint8 QSNLatianNameToType(QString type)
{
    for (quint8 i = 0; i < 64; i ++)
        if (QSNTypeLatianName(i).toLower() == type.toLower()) return i;
    return 63;
}

/**
 * @brief Maps a type index to a predefined icon index used in GUI.
 *
 * Returns a default icon (index 1) if the type has no specific icon assigned.
 *
 * @param index Type identifier.
 * @return int Icon index (e.g., 22 for temperature).
 */
int QSNTypeIconIndex(quint8 index) //================================================
{
    switch (index) {
    case 0: return 1;
    case 1: return 76;
        //    case 2: return 2;
        //    case 3: return 77;
        //    case 4: return 76;
    case 5: return 78;
    case 6: return 78;
    case 7: return 78;
    case 8: return 78;
    case 9: return 22;
    case 10: return 12;
    case 11: return 79;
    case 12: return 80;
    case 13: return 81;
    case 14: return 82;
    case 15: return 83;
    case 16: return 99;
    case 17: return 36;
    case 18: return 35;
    case 19: return 12;
    case 20: return 85;
    case 21: return 86;
    case 22: return 87;
    case 23: return 88;
    case 24: return 89;
    case 25: return 90;
    case 26: return 78;
    case 27: return 78;
    case 28: return 91;
    case 29: return 92;
    case 30: return 93;
    case 31: return 23;
    case 32: return 97;
    case 33: return 98;
    case 36: return 40;
    case 37: return 93;
    case 63: return 78;
    }
    return 1;
}

/**
 * @brief Returns a descriptive note (with range/unit info) for a given sensor/data type.
 *
 * Used in tooltips or configuration dialogs to explain expected value format.
 *
 * @param index Type identifier (0–63).
 * @return QString Human-readable description (e.g., "Temperature -127.99..+128 C").
 */
QString QSNTypeNote(quint8 index) //================================================
{
    switch (index) {
    case 0: return QString(QObject::tr("None"));
    case 1: return QString(QObject::tr("TRUE/FALSE"));
    case 2: return QString(QObject::tr("CO2 0...65535ppm"));
    case 3: return QString(QObject::tr("Air velocity 0...65535dm/sec"));
    case 4: return QString(QObject::tr("Water consumption -32767...32767dm³"));
    case 5: return QString(QObject::tr("uInt8 0...255"));
    case 6: return QString(QObject::tr("Int8 -127...127"));
    case 7: return QString(QObject::tr("uInt16 0...65535"));
    case 8: return QString(QObject::tr("Int16 -32767…32767"));
    case 9: return QString(QObject::tr("Temperature -127.99..+128 C"));
    case 10: return QString(QObject::tr("Power -2147483647…+2147483647mW"));
    case 11: return QString(QObject::tr("Current -2147483647…+2147483647mA"));
    case 12: return QString(QObject::tr("Voltage -2147483647…+2147483647mV"));
    case 13: return QString(QObject::tr("Date and time"));
    case 14: return QString(QObject::tr("Alert"));
    case 15: return QString(QObject::tr("Error"));
    case 16: return QString(QObject::tr("Text"));
    case 17: return QString(QObject::tr("Date"));
    case 18: return QString(QObject::tr("Time"));
    case 19: return QString(QObject::tr("Consumed electricity"));
    case 20: return QString(QObject::tr("Humidity 0..100%"));
    case 21: return QString(QObject::tr("Pressure 0...500000Pa"));
    case 22: return QString(QObject::tr("Level -100..100%"));
    case 23: return QString(QObject::tr("Distance -32767...32767mm"));
    case 24: return QString(QObject::tr("Angle -21600…21600minutus"));
    case 25: return QString(QObject::tr("Lux 0...65535lx"));
    case 26: return QString(QObject::tr("uInt32 0...4294967296"));
    case 27: return QString(QObject::tr("Int32 -2147483647…+2147483647"));
    case 28: return QString(QObject::tr("Capacity 0...4294967296ml"));
    case 29: return QString(QObject::tr("Weight -2147483647...2147483647mg"));
    case 30: return QString(QObject::tr("Hex code 32bit"));
    case 31: return QString(QObject::tr("Security alarm"));
    case 32: return QString(QObject::tr("SNIR package"));
    case 33: return QString(QObject::tr("EM-Marine UID"));
    case 34: return QString(QObject::tr("Rainfall 0...65535mm"));
    case 35: return QString(QObject::tr("Turnovers 0...100000rpm"));
    case 36: return QString(QObject::tr("Weather"));
    case 37: return QString(QObject::tr("Dust 0...2000µg/m³"));
    case 38: return QString(QObject::tr("AQI 0...500"));
    case 39: return QString(QObject::tr("RM package"));
    case 63: return QString(QObject::tr("Variant (Any)"));
    }
    return QString();
}

/**
 * @brief Converts a numeric value to a formatted string with appropriate scaled unit postfix.
 *
 * Applies dynamic unit scaling (e.g., mW → W → kW) based on magnitude and type.
 *
 * @param index Sensor/data type (e.g., power, distance).
 * @param num Numeric value to format.
 * @return QString Formatted string like "1.5kWt" or "250mm".
 */
QString QSNScalingNumToString(quint8 index, qreal num) //================================================
{
    switch (index) {
    case 3:
        if (num >= 10) return QString("%1%2").arg(num / 10, 0, 'f', 1).arg(QSNScaledPostFix(index, 1));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 4:
        if (num >= 10) return QString("%1%2").arg(num / 10, 0, 'f', 1).arg(QSNScaledPostFix(index, 1));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 10:
        if (num >= 1000000) return QString("%1%2").arg(num / 1000000, 0, 'f', 1).arg(QSNScaledPostFix(index, 6));
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 1).arg(QSNScaledPostFix(index, 3));
        return QString("%1%2").arg(num, 0, 'f', -1).arg(QSNScaledPostFix(index, 0));
    case 11:
        if (num >= 1000000) return QString("%1%2").arg(num / 1000000, 0, 'f', 1).arg(QSNScaledPostFix(index, 6));
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 1).arg(QSNScaledPostFix(index, 3));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 12:
        if (num >= 1000000) return QString("%1%2").arg(num / 1000000, 0, 'f', 1).arg(QSNScaledPostFix(index, 6));
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 1).arg(QSNScaledPostFix(index, 3));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 19:
        if (num >= 1000000) return QString("%1%2").arg(num / 1000000, 0, 'f', 1).arg(QSNScaledPostFix(index, 6));
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 1).arg(QSNScaledPostFix(index, 3));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 23:
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 2).arg(QSNScaledPostFix(index, 3));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 26:
        return QString("%1%2").arg(num).arg(QSNScaledPostFix(index, 0));
    case 27:
        return QString("%1 %2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 28:
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 1).arg(QSNScaledPostFix(index, 0));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, -1));
    case 29:
        if (num >= 1000000) return QString("%1%2").arg(num / 1000000, 0, 'f', 2).arg(QSNScaledPostFix(index, 6));
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 2).arg(QSNScaledPostFix(index, 3));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    case 34:
        if (num >= 1000) return QString("%1%2").arg(num / 1000, 0, 'f', 3).arg(QSNScaledPostFix(index, 3));
        return QString("%1%2").arg(num, 0, 'f', 0).arg(QSNScaledPostFix(index, 0));
    }
    return QString("%1%2").arg(num, 0, 'f', 3).arg(QSNScaledPostFix(index));
}

/**
 * @brief Determines the optimal scaling exponent (0, 1, 3, or 6) for a given type and value.
 *
 * Used to decide whether to display in base units (e.g., mm) or scaled (e.g., m).
 *
 * @param type Sensor/data type.
 * @param vol QVariant containing the raw value.
 * @return quint8 Scaling exponent (e.g., 3 for kilo-, 6 for mega-).
 */
quint8 QSNScalingIndex(quint8 type, QVariant vol)
{
    switch (type) {
    case 3:
        if (vol.toUInt() >= 10) return 1;
        break;
    case 4:
        if (vol.toInt() >= 10 || vol.toInt() <= -10) return 1;
        break;
    case 10:
        if (vol.toInt() >= 1000000 || vol.toInt() <= -1000000) return 6;
        if (vol.toInt() >= 1000 || vol.toInt() <= -1000) return 3;
        break;
    case 11:
        if (vol.toInt() >= 1000000 || vol.toInt() <= -1000000) return 6;
        if (vol.toInt() >= 1000 || vol.toInt() <= -1000) return 3;
        break;
    case 12:
        if (vol.toInt() >= 1000000 || vol.toInt() <= -1000000) return 6;
        if (vol.toInt() >= 1000 || vol.toInt() <= -1000) return 3;
        break;
    case 19:
        if (vol.toInt() >= 1000000 || vol.toInt() <= -1000000) return 6;
        if (vol.toInt() >= 1000 || vol.toInt() <= -1000) return 3;
        break;
    case 23:
        if (vol.toInt() >= 1000 || vol.toInt() <= -1000) return 3;
        break;
    case 28:
        if (vol.toUInt() >= 1000) return 3;
        break;
    case 29:
        if (vol.toInt() >= 1000000 || vol.toInt() <= -1000000) return 6;
        if (vol.toInt() >= 1000 || vol.toInt() <= -1000) return 3;
        break;
    case 34:
        if (vol.toUInt() >= 1000) return 3;
        break;
    }
    return 0;
}

/**
 * @brief Returns the number of decimal places to show based on type and scaling level.
 *
 * Ensures correct precision when displaying scaled values (e.g., 1.23 kg vs 1230 g).
 *
 * @param type Sensor/data type.
 * @param scaling Scaling exponent (from QSNScalingIndex).
 * @return quint8 Number of fractional digits (0–3).
 */
quint8 QSNPrecisionFromScaing(quint8 type, quint8 scaling)
{
    switch (type) {
    case 3:
        if (scaling == 1) return 1;
        break;
    case 4:
        if (scaling == 1) return 1;
        break;
    case 10:
        if (scaling >= 3) return 3;
        break;
    case 11:
        if (scaling >= 3) return 3;
        break;
    case 12:
        if (scaling >= 3) return 3;
        break;
    case 19:
        if (scaling >= 3) return 3;
        break;
    case 23:
        if (scaling >= 3) return 3;
        break;
    case 28:
        if (scaling >= 3) return 3;
        break;
    case 29:
        if (scaling >= 6) return 1;
        if (scaling >= 3) return 2;
        break;
    case 34:
        if (scaling >= 3) return 3;
        break;
    }
    return 0;
}

/**
 * @brief Serializes a QVariant value into a raw byte array according to its sensor type.
 *
 * Prepends the type ID as the first byte. Handles special structures (e.g., Power, Notification).
 * Returns empty QByteArray on conversion failure.
 *
 * @param vol Input value (e.g., temperature as float, state as bool).
 * @param type Sensor/data type (0–63).
 * @return QByteArray Serialized data with type prefix.
 */
QByteArray QSNVariantToRAW(QVariant vol, quint8 type)
{
    bool ok = true;
    QByteArray data;
    switch (type)
    {
    case 1:
        //if (vol.type() == )
        QSNBoolToRAW(&data, 1, vol.toUInt(&ok) & 0b00000001);
        // QSNByteToRAW(&data, 2, (vol.toUInt(&ok) & 0b11111110) >> 1);
        break;
    case 2: QSNUInt16ToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 3: QSNUInt16ToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 4: QSNInt16ToRAW(&data, 1, vol.toInt(&ok)); break;
    case 5: QSNByteToRAW(&data, 1, vol.toUInt(&ok));  break;
    case 6: QSNInt8ToRAW(&data, 1, vol.toInt(&ok));  break;
    case 7: QSNUInt16ToRAW(&data, 1, vol.toUInt(&ok));  break;
    case 8: QSNInt16ToRAW(&data, 1, vol.toInt(&ok));  break;
    case 9: QSNTemperatureToRAW(&data, 1, vol.toFloat(&ok)); break;
    case 10: QSNInt32ToRAW(&data, 1, vol.toDouble(&ok) * 1000); break;
    case 11: QSNInt32ToRAW(&data, 1, vol.toULongLong(&ok) * 1000); break;
    case 12: QSNInt32ToRAW(&data, 1, vol.toULongLong(&ok) * 1000); break;
    case 13: QSNDateTimeToRAW(&data, 1, vol.toDateTime()); break;
    case 14: { QSNNotification notif;
        notif.notificationType = vol.toUInt(&ok);
        notif.idp = 0;
        notif.notificationSource = 0;
        QSNNotificationToRAW(&data, 1, notif);
    } break;
    case 15: { QSNError error;
        error.errorIndex = vol.toUInt(&ok);
        error.idp = 0;
        error.deviceType = 0;
        QSNErrorToRAW(&data, 1, error);
    } break;
    case 16: QSNByteToRAW(&data, 1, 0); break;
    case 17: QSNDateToRAW(&data, 1, vol.toDate()); break;
    case 18: QSNTimeToRAW(&data, 1, vol.toTime()); break;
    case 19: { QSNPower pw;
        pw.power = vol.toLongLong(&ok);
        pw.seconds = 60;
        pw.idp = 0;
        QSNPowerToRAW(&data, 1, pw);
    } break;
    case 20: QSNHumidityToRAW(&data, 1, vol.toFloat(&ok)); break;
    case 21: QSNUInt32ToRAW(&data, 1, vol.toLongLong(&ok));  break;
    case 22: QSNInt8ToRAW(&data, 1, vol.toInt(&ok)); break;
    case 23: QSNInt16ToRAW(&data, 1, vol.toInt(&ok)); break;
    case 24: QSNInt16ToRAW(&data, 1, vol.toInt(&ok)); break;
    case 25: QSNUInt16ToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 26: QSNUInt32ToRAW(&data, 1, vol.toLongLong(&ok)); break;
    case 27: QSNInt32ToRAW(&data, 1, vol.toLongLong(&ok)); break;
    case 28: QSNUInt32ToRAW(&data, 1, vol.toDouble(&ok)); break;
    case 29: QSNInt32ToRAW(&data, 1, vol.toLongLong(&ok)); break;
    case 30: QSNHEXcodeToRAW(&data, 1, vol.toByteArray().toHex()); break;
    case 31:  { QSNNotification notif;
        notif.notificationType = vol.toUInt(&ok);
        notif.idp = 0;
        notif.notificationSource = 0;
        QSNNotificationToRAW(&data, 1, notif);
    } break;
    case 33: {QSNEMUID uid;
        uid.number = vol.toUInt(&ok);
        uid.family = 0;
        QSNEmUIDToRAW(&data, 1, uid);
    } break;
    case 34: QSNUInt16ToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 35: QSNUInt32ToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 36: QSNByteToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 37: QSNUInt16ToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 38: QSNUInt16ToRAW(&data, 1, vol.toUInt(&ok)); break;
    case 39: QSNRMcodeToRAW(&data, 1, vol.toByteArray()); break;
    }
    if (!ok) data.clear();
    if (data.count()) QSNByteToRAW(&data, 0, type);
    return data;
}

/**
 * @brief Deserializes a raw byte array into a QVariant based on its embedded type ID.
 *
 * Interprets the first byte as the type, then parses the rest accordingly.
 * Returns invalid QVariant if data is empty or malformed.
 *
 * @param data Pointer to QByteArray containing serialized data (first byte = type).
 * @return QVariant Reconstructed value (e.g., float, string, struct-as-string).
 */
QVariant QSNRAWToVariant(QByteArray *data)
{
    if (data->count() == 0) return QVariant();
    quint8 t = static_cast<quint8>(data->at(0));
    switch (t) {
    case 0: return QVariant();
    case 1: return QSNRAWtoBool(data, 1);
    case 2: return QSNRAWtoUInt16(data, 1);
    case 3: return QSNRAWtoUInt16(data, 1);
    case 4: return QSNRAWtoInt16(data, 1);
    case 5: return QSNRAWtoByte(data, 1);
    case 6: return QSNRAWtoInt8(data, 1);
    case 7: return QSNRAWtoUInt16(data, 1);
    case 8: return QSNRAWtoInt16(data, 1);
    case 9: return QSNRAWtoTemperature(data, 1);
    case 10: return QSNRAWtoInt32(data, 1);
    case 11: return QSNRAWtoInt32(data, 1);
    case 12: return QSNRAWtoInt32(data, 1);
    case 13: return QSNRAWtoDateTime(data, 1).toString(QLatin1String("hh:mm:ss-dd.MM.yyyy"));
    case 14: {
        QSNNotification notif = QSNRAWtoNotification(data, 1);
        return notif.notificationType;
    }
    case 15: {
        QSNError error = QSNRAWtoError(data, 1);
        return error.errorIndex;
    }
    case 16: return QSNRAWtoString(data, QString(), 1);
    case 17: return QSNRAWtoDate(data, 1).toString(QLatin1String("dd.MM.yyyy"));
    case 18: return QSNRAWtoTime(data, 1).toString(QLatin1String("hh:mm:ss"));
    case 19: {
        QSNPower pw;
        pw = QSNRAWtoPower(data, 1);
        return static_cast<double>(pw.power) * pw.seconds / 3600;
    }
    case 20: return QSNRAWtoHumidity(data, 1);
    case 21: return QSNToMillimetersOfMercury(QSNRAWtoUInt32(data, 1));
    case 22: return QSNRAWtoInt8(data, 1);
    case 23: return QSNRAWtoInt16(data, 1);
    case 24: return QSNAngleToString(QSNRAWtoInt16(data, 1));
    case 25: return QSNRAWtoUInt16(data, 1);
    case 26: return QSNRAWtoUInt32(data, 1);
    case 27: return QSNRAWtoInt32(data, 1);
    case 28: return QSNRAWtoUInt32(data, 1);
    case 29: return QSNRAWtoInt32(data, 1);
    case 30: return QSNRAWtoHEXcode(data, 1);
    case 31: {
        QSNNotification notif = QSNRAWtoNotification(data, 1);
        return notif.notificationType;
    }
    case 32: {
        QSNSNIR snir = QSNRAWtoSNIR(data, 1);
        return QString("A%1/P%2/B%3% [%4]").arg(snir.address).arg(snir.parameter).arg(QSNSNIRBatToPercent(snir.battery)).arg(QSNTypeRAWtoLabel(&snir.data));
    }
    case 33: {
        QSNEMUID uid = QSNRAWtoEmUID(data, 1);
        return QString("%1/%2").arg(uid.family).arg(uid.number);
    }
    case 34: return QSNRAWtoUInt16(data, 1);
    case 35: return QSNRAWtoUInt32(data, 1);
    case 36: return QSNWeatherTypeLabel(QSNRAWtoByte(data, 1));
    case 37: return QSNRAWtoUInt16(data, 1);
    case 38: return QSNRAWtoUInt16(data, 1);
    case 39: return QSNRAWtoRMcode(data, 1);
    }
    return QVariant();
}

/**
 * @brief Converts raw data to a human-readable string using a specified scaling factor.
 *
 * Used for UI display where manual scale control is needed (e.g., in logs or graphs).
 *
 * @param data Pointer to raw byte array (type-prefixed).
 * @param scale Scaling exponent (0, 1, 3, 6).
 * @param text Fallback or additional text (used for string types).
 * @return QString Formatted value with unit (e.g., "25.5°C", "1.2kWt").
 */
QString QSNRAWtoScaledVolString(QByteArray *data, quint8 scale, QString text)
{
    if (data->count() == 0) return QString();
    quint8 t = static_cast<quint8>(data->at(0));
    switch (t) {
    case 0: return QString();
    case 1: {
        if (data->size() > 2) return QSNBoolTypetoLabel(QSNRAWtoBool(data, 1), static_cast<quint8>(data->at(2)));
        else return QSNBoolTypetoLabel(QSNRAWtoBool(data, 1), 0);
    }
    case 2: return QString("%1").arg(QSNRAWtoUInt16(data, 1));
    case 3: return QString("%1").arg(static_cast<double>(QSNRAWtoUInt16(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 4: return QString("%1").arg(static_cast<double>(QSNRAWtoInt16(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 5: return QString("%1").arg(QSNRAWtoByte(data, 1));
    case 6: return QString("%1").arg(QSNRAWtoInt8(data, 1));
    case 7: return QString("%1").arg(QSNRAWtoUInt16(data, 1));
    case 8: return QString("%1").arg(QSNRAWtoInt16(data, 1));
    case 9: return QString("%1").arg(QSNRAWtoTemperature(data, 1), 0, 'f', 1);
    case 10: return QString("%1").arg(static_cast<double>(QSNRAWtoInt32(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 11: return QString("%1").arg(static_cast<double>(QSNRAWtoInt32(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 12: return QString("%1").arg(static_cast<double>(QSNRAWtoInt32(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 13: return QString("%1").arg(QSNRAWtoDateTime(data, 1).toString(QLatin1String("hh:mm:ss-dd.MM.yyyy")));
    case 14: {
        QSNNotification notif = QSNRAWtoNotification(data, 1);
        if (notif.notificationSource) return QString("%1 (%2)")
                .arg(QSNAlertTypeLabel(notif.notificationType), QSNAlertTypeLabelSource(notif.notificationSource));
        else return QString("%1").arg(QSNAlertTypeLabel(notif.notificationType));
    }
    case 15: {
        QSNError error = QSNRAWtoError(data, 1);
        return QObject::tr("%1, device address: %2, device type: %3")
                .arg(error.errorIndex, 0, 10)
                .arg(error.idp)
                .arg(error.deviceType);
    }
    case 16: return QSNRAWtoString(data, text, 1);
    case 17: return QString("%1").arg(QSNRAWtoDate(data, 1).toString(QLatin1String("dd.MM.yyyy")));
    case 18: return QString("%1").arg(QSNRAWtoTime(data, 1).toString(QLatin1String("hh:mm:ss")));
    case 19: {
        QSNPower pw;
        pw = QSNRAWtoPower(data, 1);
        return QString("%1").arg(static_cast<double>((static_cast<double>(pw.power) * pw.seconds / 3600) / QSNPOW(10, scale)), 0, 'f', QSNPrecisionFromScaing(t, scale));
    }
    case 20: return QString("%1").arg(QSNRAWtoHumidity(data, 1));
    case 21: return QString("%1").arg(QSNToMillimetersOfMercury(QSNRAWtoUInt32(data, 1)), 0, 'f', 2);
    case 22: return QString("%1").arg(QSNRAWtoInt8(data, 1));
    case 23: return QString("%1").arg(static_cast<double>(QSNRAWtoInt16(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 24: return QSNAngleToString(QSNRAWtoInt16(data, 1));
    case 25: return QString("%1").arg(QSNRAWtoUInt16(data, 1));
    case 26: return QString("%1%2").arg(QSNRAWtoUInt32(data, 1)).arg(QSNRAWtoPostFix(data, 5));
    case 27: return QString("%1%2").arg(QSNRAWtoInt32(data, 1)).arg(QSNRAWtoPostFix(data, 5));
    case 28: return QString("%1").arg(static_cast<double>(QSNRAWtoUInt32(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 29: return QString("%1").arg(static_cast<double>(QSNRAWtoInt32(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 30: return QString("%1").arg(QSNRAWtoHEXcode(data, 1));
    case 31: {
        QSNNotification notif = QSNRAWtoNotification(data, 1);
        return QString("%1").arg(QSNSecurityAlertTypeLabel(notif.notificationType));
    }
    case 32: {
        QSNSNIR snir = QSNRAWtoSNIR(data, 1);
        return QString("A%1/P%2/B%3% [%4]").arg(snir.address).arg(snir.parameter).arg(QSNSNIRBatToPercent(snir.battery)).arg(QSNTypeRAWtoLabel(&snir.data));
    }
    case 33: {
        QSNEMUID uid = QSNRAWtoEmUID(data, 1);
        return QString("%1/%2").arg(uid.family).arg(uid.number);
    }
    case 34: return QString("%1").arg(static_cast<double>(QSNRAWtoUInt16(data, 1)) / QSNPOW(10, scale), 0, 'f', QSNPrecisionFromScaing(t, scale));
    case 35: return QString("%1").arg(QSNRAWtoUInt32(data, 1));
    case 36: return QSNWeatherTypeLabel(QSNRAWtoByte(data, 1));
    case 37: return QString("%1").arg(QSNRAWtoUInt16(data, 1));
    case 38: return QString("%1").arg(QSNRAWtoUInt16(data, 1));
    case 39: {
        QByteArray b = QSNRAWtoRMcode(data, 1);
        return QString("%1 %2").arg(QSNRMType(b[0]), QString(b.toHex()));
        }
    }

    return QString();
}

/**
 * @brief Converts a QVariant to a scaled string representation.
 *
 * Similar to QSNRAWtoScaledVolString but works directly with high-level values.
 *
 * @param vol Input value.
 * @param scale Scaling exponent.
 * @param type Sensor/data type.
 * @return QString Formatted string.
 */
QString QSNVariantToScaledString(QVariant vol, quint8 scale, quint8 type)
{
    if (type == 0) return QString();
    bool ok = false;
    switch (type) {
    case 0: return QString();
    case 1: return QSNBoolToText(vol.toBool());
    case 2: return QString("%1").arg(vol.toUInt());
    case 3: return QString("%1").arg(static_cast<double>(vol.toUInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 4: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 5: return QString("%1").arg(vol.toUInt());
    case 6: return QString("%1").arg(vol.toInt());
    case 7: return QString("%1").arg(vol.toUInt());
    case 8: return QString("%1").arg(vol.toInt());
    case 9: return QString("%1").arg(vol.toDouble(&ok), 0, 'f', 1);
    case 10: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 11: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 12: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 13: return QString("%1").arg(vol.toDateTime().toString(QLatin1String("hh:mm:ss-dd.MM.yyyy")));
    case 14: return QString("%1").arg(vol.toUInt());
    case 15: return QString("%1").arg(vol.toUInt());
    case 16: return vol.toString();
    case 17: return QString("%1").arg(vol.toDate().toString(QLatin1String("dd.MM.yyyy")));
    case 18: return QString("%1").arg(vol.toTime().toString(QLatin1String("hh:mm:ss")));
    case 19: return QString("%1").arg(static_cast<double>(vol.toLongLong(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 20: return QString("%1").arg(vol.toFloat(&ok));
    case 21: return QString("%1").arg(QSNToMillimetersOfMercury(static_cast<double>(vol.toLongLong()) / QSNPOW(10, scale)), 0, 'f', QSNPrecisionFromScaing(type, scale));
    case 22: return QString("%1").arg(vol.toInt());
    case 23: return QString("%1").arg(static_cast<double>(vol.toInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 24: return QString("%1").arg(static_cast<double>(vol.toInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 25: return QString("%1").arg(vol.toUInt(&ok));
    case 26: return QString("%1").arg(vol.toUInt(&ok));
    case 27: return QString("%1").arg(vol.toInt(&ok));
    case 28: return QString("%1").arg(static_cast<double>(vol.toUInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 29: return QString("%1").arg(static_cast<double>(vol.toInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 30: return vol.toByteArray().toHex();
    case 31: return QString("%1").arg(QSNSecurityAlertTypeLabel(vol.toUInt(&ok)));
    case 32: return QString("%1").arg(vol.toUInt(&ok));
    case 33: return QString("%1").arg(vol.toUInt(&ok));
    case 34: return QString("%1").arg(static_cast<double>(vol.toUInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 35: return QString("%1").arg(vol.toUInt(&ok));
    case 36: return QSNWeatherTypeLabel(vol.toUInt(&ok));
    case 37: return QString("%1").arg(vol.toUInt(&ok));
    case 38: return QString("%1").arg(vol.toUInt(&ok));
    case 39: return vol.toByteArray().toHex();
    }

    return QString();
}

/**
 * @brief Converts a QVariant to a scaled numeric string (without labels, for parsing/export).
 *
 * Omits non-numeric types (e.g., text, datetime). Used in CSV/JSON exports.
 *
 * @param vol Input value.
 * @param scale Scaling exponent.
 * @param type Sensor/data type.
 * @return QString Numeric string (e.g., "1250", "25.5").
 */
QString QSNVariantToScaledNumberString(QVariant vol, quint8 scale, quint8 type)
{
    if (type == 0) return QString();
    bool ok = false;
    switch (type) {
    case 0: return QString("1");
    case 1: return QString("%1").arg(vol.toBool()?"1":"0");
    case 2: return QString("%1").arg(vol.toUInt());
    case 3: return QString("%1").arg(static_cast<double>(vol.toUInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 4: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 5: return QString("%1").arg(vol.toUInt());
    case 6: return QString("%1").arg(vol.toInt());
    case 7: return QString("%1").arg(vol.toUInt());
    case 8: return QString("%1").arg(vol.toInt());
    case 9: return QString("%1").arg(vol.toDouble(&ok), 0, 'f', 1);
    case 10: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 11: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 12: return QString("%1").arg(static_cast<double>(vol.toInt()) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 14: return QString("%1").arg(vol.toUInt());
    case 15: return QString("%1").arg(vol.toUInt());
    case 19: return QString("%1").arg(static_cast<double>(vol.toLongLong(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 20: return QString("%1").arg(vol.toFloat(&ok));
    case 21: return QString("%1").arg(QSNToMillimetersOfMercury(static_cast<double>(vol.toLongLong()) / QSNPOW(10, scale)), 0, 'f', QSNPrecisionFromScaing(type, scale));
    case 22: return QString("%1").arg(vol.toInt());
    case 23: return QString("%1").arg(static_cast<double>(vol.toInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 24: return QString("%1").arg(static_cast<double>(vol.toInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 25: return QString("%1").arg(vol.toUInt(&ok));
    case 26: return QString("%1").arg(vol.toUInt(&ok));
    case 27: return QString("%1").arg(vol.toInt(&ok));
    case 28: return QString("%1").arg(static_cast<double>(vol.toUInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 29: return QString("%1").arg(static_cast<double>(vol.toInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 32: return QString("%1").arg(vol.toUInt(&ok));
    case 33: return QString("%1").arg(vol.toUInt(&ok));
    case 34: return QString("%1").arg(static_cast<double>(vol.toUInt(&ok)) / QSNPOW(10, scale), 0, 'g', QSNPrecisionFromScaing(type, scale));
    case 35: return QString("%1").arg(vol.toUInt(&ok));
    case 37: return QString("%1").arg(vol.toUInt(&ok));
    case 38: return QString("%1").arg(vol.toUInt(&ok));
  }

    return QString();
}

/**
 * @brief Automatically scales and formats raw data into a user-friendly string with unit.
 *
 * Internally calls QSNScalingIndex to determine best scale, then formats with postfix.
 *
 * @param data Pointer to raw byte array.
 * @param text Optional fallback text (for string types).
 * @return QString Fully formatted value (e.g., "25°C", "1.5m").
 */
QString QSNRAWtoScaledVolume(QByteArray *data, QString text)
{
    if (data->isEmpty()) return QString();
    quint8 t = static_cast<quint8>(data->at(0));
    quint8 scale = QSNScalingIndex(t, QSNRAWToVariant(data));
    return QSNRAWtoScaledVolString(data, scale, text) + QSNScaledPostFix(t, scale);
}

/**
 * @brief Returns the appropriate unit symbol/postfix for a given type and scale.
 *
 * Respects global settings (e.g., °C vs °F, mmHg vs hPa).
 *
 * @param type Sensor/data type.
 * @param scale Scaling exponent (0 = base unit, 3 = kilo, etc.).
 * @return QString Unit string (e.g., "°C", "kWt", "mm").
 */
QString QSNScaledPostFix(quint8 type, quint8 scale)
{
    switch (type) {
    case 2: return QObject::tr("ppm");
    case 3:
        if (scale == 1) return QObject::tr("m/sec");
        return QObject::tr("dm/sec");
    case 4:
        if (scale == 1) return QObject::tr("m³");
        return QObject::tr("dm3");
    case 9: return QSNCelsius?QObject::tr("°C"):QObject::tr("°F");
    case 10:
        if (scale == 6) return QObject::tr("kWt");
        if (scale == 3) return QObject::tr("Wt");
        return QObject::tr("mWt");
    case 11:
        if (scale == 6) return QObject::tr("kA");
        if (scale == 3) return QObject::tr("A");
        return QObject::tr("mA");
    case 12:
        if (scale == 6) return QObject::tr("kV");
        if (scale == 3) return QObject::tr("V");
        return QObject::tr("mV");
    case 19:
        if (scale == 6) return QObject::tr("kW.h");
        if (scale == 3) return QObject::tr("W.h");
        return QObject::tr("mW.h");
    case 20: return QObject::tr("%");
    case 21: return QSNMillimetersOfMercury?QObject::tr("mmHg"):QObject::tr("hPa");
    case 22: return QObject::tr("%");
    case 23: return (scale == 3)?QObject::tr("m"):QObject::tr("mm");
    case 24: return (scale == 3)?QObject::tr("Degrees"):QObject::tr("minutus");
    case 25: return QObject::tr("lx");
    case 28: return (scale == 3)?QObject::tr("l"):QObject::tr("ml");
    case 29:
        if (scale == 6) return QObject::tr("kg");
        if (scale == 3) return QObject::tr("g");
        return QObject::tr("mg");
    case 34: return (scale == 3)?QObject::tr("m"):QObject::tr("mm");
    case 35: return QObject::tr("rpm");
    case 37: return QObject::tr("µg/m³");
    }
    return QString();
}

/**
 * @brief Returns a short descriptive prefix for a given sensor/data type.
 *
 * Used in composite labels (e.g., "Temperature: 25°C").
 *
 * @param index Type identifier (0–63).
 * @return QString Prefix string (e.g., "Temperature", "Power").
 */
QString QSNTypePrefix(quint8 index) //================================================
{
    if (index == 1) return QObject::tr("State");
    if (index == 2) return QString(QObject::tr("CO2"));
    if (index == 3) return QString(QObject::tr("Air velocity"));
    if (index == 4) return QString(QObject::tr("Water consumption"));
    if (index > 4 && index < 9) return QObject::tr("Number");
    if (index == 9) return QObject::tr("Temperature");
    if (index == 10) return QObject::tr("Power");
    if (index == 11) return QObject::tr("Current");
    if (index == 12) return QObject::tr("Voltage");
    if (index == 13) return QObject::tr("Time");
    if (index == 14) return QObject::tr("Alert");
    if (index == 15) return QObject::tr("Error");
    if (index == 16) return QObject::tr("Text");
    if (index == 17) return QObject::tr("Date");
    if (index == 18) return QObject::tr("Time");
    if (index == 19) return QObject::tr("Electricity");
    if (index == 20) return QObject::tr("Humidity");
    if (index == 21) return QObject::tr("Pressure");
    if (index == 22) return QObject::tr("Level");
    if (index == 23) return QObject::tr("Distance");
    if (index == 24) return QObject::tr("Angle");
    if (index == 25) return QObject::tr("Illumination");
    if (index == 26) return QObject::tr("Number");
    if (index == 27) return QObject::tr("Number");
    if (index == 28) return QObject::tr("Capacity");
    if (index == 29) return QObject::tr("Weight");
    if (index == 30) return QObject::tr("Code");
    if (index == 31) return QObject::tr("Security alarm");
    if (index == 32) return QObject::tr("SNIR");
    if (index == 33) return QObject::tr("UID");
    if (index == 34) return QObject::tr("Rainfall");
    if (index == 35) return QObject::tr("Turnovers");
    if (index == 36) return QObject::tr("Weather");
    if (index == 37) return QObject::tr("Dust");
    if (index == 38) return QObject::tr("AQI");
    if (index == 39) return QObject::tr("RM");
    if (index == 63) return QObject::tr("Variant");
    return QString();
}

/**
 * @brief Returns the expected raw data size (in bytes) for a given type.
 *
 * Used to validate or allocate buffers during serialization/deserialization.
 * Includes type byte + payload.
 *
 * @param index Type identifier.
 * @return int Size in bytes (e.g., 2 for temperature, 7 for datetime).
 */
int QSNTypeDataSize(quint8 index) //================================================
{
    if (index == 1) return 1;
    if (index > 1 && index < 5) return 2;
    if (index > 4 && index < 7) return 1;
    if (index > 6 && index < 10) return 2;
    if (index > 9 && index < 13) return 4;
    if (index == 13) return 7;
    if (index == 14) return 4;
    if (index > 16 && index < 19) return 4;
    if (index == 19) return 5;
    if (index == 20) return 2;
    if (index == 21) return 4;
    if (index == 22) return 1;
    if (index > 22 && index <= 25) return 2;
    if (index > 25 && index < 31) return 4;
    if (index == 31) return 3;
    if (index == 32) return 8;
    if (index == 33) return 4;
    if (index == 34) return 2;
    if (index == 35) return 4;
    if (index == 36) return 1;
    if (index == 37) return 3;
    if (index == 38) return 2;
    if (index == 39) return 7;
    return 0;
}

/**
 * @brief Returns the valid numeric range (min/max) for a given sensor type.
 *
 * Useful for input validation or UI sliders. Non-numeric types return (0,1).
 *
 * @param index Type identifier.
 * @return QPointF .x() = min, .y() = max.
 */
QPointF QSNTypeRange(quint8 index) //================================================
{
    switch (index) {
    case 1: return QPointF(0, 1);
    case 2: return QPointF(0, 65535);
    case 3: return QPointF(0, 65535);
    case 4: return QPointF(-32767, 32767);
    case 5: return QPointF(0, 255);
    case 6: return QPointF(-127, 127);
    case 7: return QPointF(0, 65535);
    case 8: return QPointF(-32767, 32767);
    case 9: return QPointF(-127.99, 128);
    case 10: return QPointF(-2147483647, 2147483647);
    case 11: return QPointF(-2147483647, 2147483647);
    case 12: return QPointF(-2147483647, 2147483647);
    case 19: return QPointF(0, 65535);
    case 20: return QPointF(0, 100);
    case 21: return QPointF(0, 4294967296);
    case 22: return QPointF(-100, 100);
    case 23: return QPointF(-32767, 32767);
    case 24: return QPointF(-21600, 21600);
    case 25: return QPointF(0, 65535);
    case 26: return QPointF(0, 4294967296);
    case 27: return QPointF(-2147483647, 2147483647);
    case 28: return QPointF(0, 4294967296);
    case 29: return QPointF(-2147483647, 2147483647);
    case 30: return QPointF(0, 4294967296);
    case 33: return QPointF(0, 4294967296);
    case 34: return QPointF(0, 65535);
    case 35: return QPointF(0, 100000);
    case 36: return QPointF(0, 8);
    case 37: return QPointF(0, 2000);
    case 38: return QPointF(0, 500);
    }
    return QPointF(0, 1);
}

/**
 * @brief Resolves an icon index, preferring a custom icon if provided.
 *
 * Falls back to default type-based icon if `iconNum <= 0`.
 *
 * @param iconNum Custom icon index (0 = use default).
 * @param dataType Sensor/data type (0–63).
 * @return int Final icon index.
 */
int QSNAutoIcon(int iconNum, int dataType)
{
    if (iconNum > 0) return iconNum;
    return QSNTypeIconIndex(static_cast<quint8>(dataType));
}

/**
 * @brief Checks if two data types are compatible for assignment or conversion.
 *
 * Handles special cases like Variant (63), HEX ↔ UInt32, and RM ↔ HEX.
 *
 * @param type1 First type ID.
 * @param input1 True if first value is from input (affects RM/HEX logic).
 * @param type2 Second type ID.
 * @param input2 True if second value is from input.
 * @return bool True if types can be interchanged.
 */
bool QSNTypeCompatibility(quint8 type1, bool input1, quint8 type2, bool input2)
{
    if (type1 == type2) return true;
    if ((type1 == 30 && type2 == 26) || (type1 == 26 && type2 == 30)) return true;
    if (input1 && type1 == 39 && !input2 && type2 == 30) return true;
    if (input2 && type2 == 39 && !input1 && type1 == 30) return true;

    if (type1 == 63 || type2 == 63) return true;
    return false;
}

/**
 * @brief Converts raw serialized data into a human-readable formatted string.
 *
 * Automatically applies unit scaling, postfixes, and localized labels.
 *
 * @param Data Pointer to QByteArray (first byte = type).
 * @param text Fallback text for string types.
 * @return QString Fully formatted value (e.g., "25.5°C", "on").
 */
QString QSNTypeRAWtoString(QByteArray *Data, QString text) //================================================
{
    if (Data->count() == 0) Data->append(static_cast<char>(0));
    quint8 t = static_cast<quint8>(Data->at(0));
    if (t == 0) return QString();
    if (t == 1) {
        if (Data->size() > 2) return QSNBoolTypetoLabel(QSNRAWtoBool(Data, 1), static_cast<quint8>(Data->at(2)));
        else return QSNBoolTypetoLabel(QSNRAWtoBool(Data, 1), 0);
    }
    if (t == 2) return QString("%1%2").arg(QSNRAWtoUInt16(Data, 1)).arg(QSNScaledPostFix(2));
    if (t == 3) return QString("%1%2").arg(QSNRAWtoUInt16(Data, 1)).arg(QSNScaledPostFix(3));
    if (t == 4) return QString("%1%2").arg(QSNRAWtoInt16(Data, 1)).arg(QSNScaledPostFix(4));
    if (t == 5) return QString("%1%2").arg(QSNRAWtoByte(Data, 1)).arg(QSNRAWtoPostFix(Data, 2));
    if (t == 6) return QString("%1%2").arg(QSNRAWtoInt8(Data, 1)).arg(QSNRAWtoPostFix(Data, 2));
    if (t == 7) return QString("%1%2").arg(QSNRAWtoUInt16(Data, 1)).arg(QSNRAWtoPostFix(Data, 3));
    if (t == 8) return QString("%1%2").arg(QSNRAWtoInt16(Data, 1)).arg(QSNRAWtoPostFix(Data, 3));
    if (t == 9) return QString("%1%2").arg(QSNRAWtoTemperature(Data, 1), 0, 'f', 1).arg(QSNScaledPostFix(9));
    if (t == 10) return QSNScalingNumToString(10, static_cast<qreal>(QSNRAWtoInt32(Data, 1)));
    if (t == 11) return QSNScalingNumToString(11, static_cast<qreal>(QSNRAWtoInt32(Data, 1)));
    if (t == 12) return QSNScalingNumToString(12, static_cast<qreal>(QSNRAWtoInt32(Data, 1)));
    if (t == 13) return QString("%1").arg(QSNRAWtoDateTime(Data, 1).toString(QLatin1String("hh:mm:ss-dd.MM.yyyy")));
    if (t == 14) {
        QSNNotification notif = QSNRAWtoNotification(Data, 1);
        if (notif.notificationSource) return QString("%1 (%2)")
                .arg(QSNAlertTypeLabel(notif.notificationType), QSNAlertTypeLabelSource(notif.notificationSource));
        else return QString("%1").arg(QSNAlertTypeLabel(notif.notificationType));
    }
    if (t == 15) {
        QSNError error = QSNRAWtoError(Data, 1);
        return QObject::tr("%1, device address: %2, device type: %3")
                .arg(error.errorIndex, 0, 10)
                .arg(error.idp)
                .arg(error.deviceType);
    }
    if (t == 16) return QSNRAWtoString(Data, text, 1);
    if (t == 17) return QString("%1").arg(QSNRAWtoDate(Data, 1).toString(QLatin1String("dd.MM.yyyy")));
    if (t == 18) return QString("%1").arg(QSNRAWtoTime(Data, 1).toString(QLatin1String("hh:mm:ss")));
    if (t == 19) {
        QSNPower pw;
        pw = QSNRAWtoPower(Data, 1);
        return QSNScalingNumToString(19, static_cast<double>(pw.power) * pw.seconds / 3600);
    }
    if (t == 20) return QString("%1%2").arg(QSNRAWtoHumidity(Data, 1)).arg(QSNScaledPostFix(20));
    if (t == 21) return QString("%1%2").arg(QSNToMillimetersOfMercury(QSNRAWtoUInt32(Data, 1)), 0, 'f', 2).arg(QSNScaledPostFix(21));
    if (t == 22) return QString("%1%2").arg(QSNRAWtoInt8(Data, 1)).arg(QSNScaledPostFix(22));
    if (t == 23) return QSNScalingNumToString(23, static_cast<qreal>(QSNRAWtoInt16(Data, 1)));
    if (t == 24) return QSNAngleToString(QSNRAWtoInt16(Data, 1));
    if (t == 25) return QString("%1%2").arg(QSNRAWtoUInt16(Data, 1)).arg(QSNScaledPostFix(25));
    if (t == 26) return QString("%1%2").arg(QSNRAWtoUInt32(Data, 1)).arg(QSNRAWtoPostFix(Data, 5));
    if (t == 27) return QString("%1%2").arg(QSNRAWtoInt32(Data, 1)).arg(QSNRAWtoPostFix(Data, 5));
    if (t == 28) return QSNScalingNumToString(28, static_cast<qreal>(QSNRAWtoUInt32(Data, 1)));
    if (t == 29) return QSNScalingNumToString(29, static_cast<qreal>(QSNRAWtoInt32(Data, 1)));
    if (t == 30) return QString("%1").arg(QSNRAWtoHEXcode(Data, 1));
    if (t == 31) {
        QSNNotification notif = QSNRAWtoNotification(Data, 1);
        return QString("%1").arg(QSNSecurityAlertTypeLabel(notif.notificationType));
    }
    if (t == 32) {
        QSNSNIR snir = QSNRAWtoSNIR(Data, 1);
        return QString("A%1/P%2/B%3% [%4]").arg(snir.address).arg(snir.parameter).arg(QSNSNIRBatToPercent(snir.battery)).arg(QSNTypeRAWtoLabel(&snir.data));
    }
    if (t == 33) {
        QSNEMUID uid = QSNRAWtoEmUID(Data, 1);
        return QString("%1/%2").arg(uid.family).arg(uid.number);
    }
    if (t == 34) return QSNScalingNumToString(34, static_cast<qreal>(QSNRAWtoUInt16(Data, 1)));
    if (t == 35) return QString("%1%2").arg(QSNRAWtoUInt32(Data, 1)).arg(QSNScaledPostFix(35));
    if (t == 36) return QSNWeatherTypeLabel(QSNRAWtoByte(Data, 1));
    if (t == 37) return QString("%1%2").arg(QSNRAWtoUInt16(Data, 1)).arg(QSNScaledPostFix(37));
    if (t == 38) return QString("%1").arg(QSNRAWtoUInt16(Data, 1));
    if (t == 39) {
        QByteArray b = QSNRAWtoRMcode(Data, 1);
        return QString("%1 %2").arg(QSNRMType(b[0]), QString(b.toHex()));
        }
    return QString();
}

/**
 * @brief Extracts a postfix index from raw data at a specified offset.
 *
 * Used internally by dynamic unit systems.
 *
 * @param data Pointer to raw data.
 * @param index Byte offset to read.
 * @return quint8 Postfix index (0 = default unit).
 */
quint8 QSNRAWtoPostFixIndex(QByteArray *data, quint8 index)
{
    if (data->size() <= index) return 0;
    return static_cast<quint8>(data->at(index));
}

/**
 * @brief Converts a postfix index into a human-readable unit symbol.
 *
 * Delegates to QSNScaledPostFix().
 *
 * @param data Pointer to raw data.
 * @param index Byte offset of postfix index.
 * @return QString Unit symbol (e.g., "°C", "mWt").
 */
QString QSNRAWtoPostFix(QByteArray *data, quint8 index)
{
    return QSNScaledPostFix(QSNRAWtoPostFixIndex(data, index));
}

/**
 * @brief Extracts a special notice message from boolean-type raw data.
 *
 * Used for states like "intermediate", "in process", etc. (index >= 100).
 *
 * @param data Pointer to raw data.
 * @return QString Notice string or empty if not applicable.
 */
QString QSNRAWtoNotice(QByteArray *data)
{
    if (data->size() < 3) return QString();
    if (data->at(0) == 1) {
        if (static_cast<quint8>(data->at(2)) < 100) return QString();
        return QSNBoolTypetoLabel(false, static_cast<quint8>(data->at(2)));
    }
    return QString();
}

QString QSNBoolTypetoLabel(bool state, quint8 index)
{
    switch (index) {
    case 100: return QString(QObject::tr("intermediate state"));
    case 101: return QString(QObject::tr("switch"));
    case 102: return QString(QObject::tr("in process"));
    case 103: return QString(QObject::tr("closes"));
    case 104: return QString(QObject::tr("opens"));
    case 105: return QString(QObject::tr("runs"));
    case 106: return QString(QObject::tr("stops"));
    case 107: return QString(QObject::tr("absents"));
    case 200: return QString(QObject::tr("error"));
    case 201: return QString(QObject::tr("accident"));
    case 202: return QString(QObject::tr("failure"));
    };

    if (state)
        switch (index) {
        case 1: return QString(QObject::tr("on"));
        case 2: return QString(QObject::tr("yes"));
        case 3: return QString(QObject::tr("open"));
        case 4: return QString(QObject::tr("elevated"));
        case 5: return QString(QObject::tr("open"));
        case 6: return QString(QObject::tr("allowed"));
        case 7: return QString(QObject::tr("presents"));
        case 8: return QString(QObject::tr("detected"));
        case 9: return QString(QObject::tr("activated"));
        case 10: return QString(QObject::tr("absence"));
        case 11: return QString(QObject::tr("night"));
        case 12: return QString(QObject::tr("day"));
        case 13: return QString(QObject::tr("armed"));
        default: return QString(QObject::tr("true"));
        }
    else
        switch (index) {
        case 1: return QString(QObject::tr("off"));
        case 2: return QString(QObject::tr("no"));
        case 3: return QString(QObject::tr("close"));
        case 4: return QString(QObject::tr("omitted"));
        case 5: return QString(QObject::tr("close"));
        case 6: return QString(QObject::tr("prohibited"));
        case 7: return QString(QObject::tr("absents"));
        case 8: return QString(QObject::tr("not detected"));
        case 9: return QString(QObject::tr("not activated"));
        case 10: return QString(QObject::tr("presence"));
        case 11: return QString(QObject::tr("not night"));
        case 12: return QString(QObject::tr("not day"));
        case 13: return QString(QObject::tr("disarmed"));
        default: return QString(QObject::tr("false"));
        }
}


QString QSNTypeRAWtoLabel(QByteArray *Data)
{
    if (Data->count() == 0) Data->append(static_cast<char>(0));
    qint8 index = QSNRAWtoByte(Data, 0);
    QString ret = QSNTypePrefix(index);
    QString spf = QSNTypeRAWtoSubIndex(Data, index, 1);
    if (!spf.isEmpty()) ret = ret + QLatin1String(" ") + spf;
    if (!ret.isEmpty()) ret = ret + QLatin1String(": ");
    ret = ret + QSNRAWtoScaledVolume(Data);
    return ret;
}


QString QSNTypeRAWtoSubIndex(QByteArray *Data, quint8 index, int addr)
{
    switch (index) {
    case 37: return QSNDustTypeSize(QSNRAWtoByte(Data, addr + 2));
    }
    return QString();
}

/**
 * @brief Converts raw serialized data to a normalized floating-point value.
 *
 * Handles special composite types like Power (converts to watt-hours).
 * Used in comparisons, graphs, or mathematical operations.
 *
 * @param data Pointer to raw byte array (type-prefixed).
 * @param index Sensor/data type ID.
 * @param addr Offset within the data payload (usually 1, after type byte).
 * @return qreal Normalized numeric value (e.g., 25.5 for temperature).
 */
qreal QSNRAWtoReal(QByteArray *data, quint8 index, int addr) //================================================
{
    QSNPower pw;
    switch (index)
    {
    case 1: return QSNRAWtoBool(data, addr);
    case 2: return QSNRAWtoUInt16(data, addr);
    case 3: return QSNRAWtoUInt16(data, addr);
    case 4: return QSNRAWtoInt16(data, addr);
    case 5: return QSNRAWtoByte(data, addr);
    case 6: return QSNRAWtoInt8(data, addr);
    case 7: return QSNRAWtoUInt16(data, addr);
    case 8: return QSNRAWtoInt16(data, addr);
    case 9: return QSNRAWtoTemperature(data, addr);
    case 10: return QSNRAWtoInt32(data, addr);
    case 11: return QSNRAWtoInt32(data, addr);
    case 12: return QSNRAWtoInt32(data, addr);
    case 19: pw = QSNRAWtoPower(data, addr);
        return (static_cast<double>(pw.power) / 3600000 * pw.seconds);
    case 20: return QSNRAWtoHumidity(data, addr);
    case 21: return QSNToMillimetersOfMercury(QSNRAWtoUInt32(data, addr));
    case 22: return QSNRAWtoInt8(data, addr);
    case 23: return QSNRAWtoInt16(data, addr);
    case 24: return QSNRAWtoInt16(data, addr) * 3600;
    case 25: return QSNRAWtoUInt16(data, addr);
    case 26: return QSNRAWtoUInt32(data, addr);
    case 27: return QSNRAWtoInt32(data, addr);
    case 28: return QSNRAWtoUInt32(data, addr);
    case 29: return QSNRAWtoInt32(data, addr);
    case 33: return QSNRAWtoUInt32(data, addr);
    case 34: return QSNRAWtoUInt16(data, addr);
    case 35: return QSNRAWtoUInt32(data, addr);
    case 36: return QSNRAWtoUInt16(data, addr);
    case 37: return QSNRAWtoUInt16(data, addr);
    case 38: return QSNRAWtoUInt16(data, addr);
    }
    return 0;
}

/**
 * @brief Extracts the device IDP (Internal Device Protocol address) from raw data.
 *
 * Assumes standard layout: [type][payload][IDP:2 bytes]. Returns 0 if absent.
 *
 * @param data Pointer to raw byte array.
 * @return quint16 Device IDP address.
 */
quint16 QSNTypeRAWtoIDP(QByteArray *data)
{
    if (data->size() < 1) return 0;
    int s = QSNTypeDataSize(static_cast<quint8>(data->at(0)));
    if (data->size() < s + 3) return 0;
    return QSNRAWtoUInt16(data, s + 1);
}

/**
 * @brief Converts a full IDP into a short device address (lower 12 bits).
 *
 * Used in addressing schemes where upper bits carry network info.
 *
 * @param idp Full 16-bit IDP.
 * @return quint16 Truncated device address (0–4095).
 */
quint16 QSNIDPtoDevAddr(quint16 idp)
{
    return (idp & 0x0FFF);
}

/**
 * @brief Converts Celsius to Fahrenheit.
 *
 * @param celsius Input temperature in °C.
 * @return qreal Temperature in °F.
 */
qreal QSNCelsiusToFahrenheit(qreal celsius)
{
    return 1.8 * celsius + 32;
}

/**
 * @brief Converts Fahrenheit to Celsius.
 *
 * @param fahrenheit Input temperature in °F.
 * @return qreal Temperature in °C.
 */
qreal QSNFahrenheitToCelsius(qreal fahrenheit)
{
    return (fahrenheit - 32) / 1.8;
}

/**
 * @brief Applies global Celsius/Fahrenheit setting to a Celsius input.
 *
 * If QSNCelsius == true → returns unchanged; else → converts to Fahrenheit.
 *
 * @param celsius Input in °C.
 * @return qreal Value in currently selected unit.
 */
qreal QSNFromCelsius(qreal celsius)
{
    if (QSNCelsius) return celsius;
    return QSNCelsiusToFahrenheit(celsius);
}

/**
 * @brief Converts a displayed temperature back to Celsius (inverse of QSNFromCelsius).
 *
 * @param value Temperature in current global unit (°C or °F).
 * @return qreal Equivalent in °C.
 */
qreal QSNToCelsius(qreal value)
{
    if (QSNCelsius) return value;
    return QSNFahrenheitToCelsius(value);
}


/**
 * @brief Converts pressure from Pascals to either mmHg or hPa based on global setting.
 *
 * @param value Pressure in Pascals (as uint32).
 * @return qreal Converted value (mmHg ≈ Pa × 0.0075; hPa = Pa / 100).
 */
qreal QSNToMillimetersOfMercury(quint32 value)
{
    if (!QSNMillimetersOfMercury) return static_cast<qreal>(value / 100);
    return static_cast<qreal>(value * 0.0075);
}

/**
 * @brief Returns a list of all known alert type labels (e.g., "Fire", "Water leakage").
 *
 * Used to populate combo boxes or validate inputs.
 *
 * @return QStringList Localized alert names.
 */
QStringList QSNAlertTypeList()
{
    QStringList ret;
    quint8 index = 0;
    while (QSNAlertTypeLabel(index) != QString()) {
        ret.append(QSNAlertTypeLabel(index));
        index ++;
    }
    return ret;
}

/**
 * @brief Maps an alert type code to its human-readable label.
 *
 * @param index Alert code (0–10).
 * @return QString Localized description.
 */
QString QSNAlertTypeLabel(quint8 index)
{
    switch (index) {
    case 0: return QString(QObject::tr("Canceling alert"));
    case 1: return QString(QObject::tr("Fire"));
    case 2: return QString(QObject::tr("Fire is absent"));
    case 3: return QString(QObject::tr("Water leakage"));
    case 4: return QString(QObject::tr("Water leakage is absent"));
    case 5: return QString(QObject::tr("No electricity supply"));
    case 6: return QString(QObject::tr("Electricity supply is normal"));
    case 7: return QString(QObject::tr("Poor air quality"));
    case 8: return QString(QObject::tr("Normal air quality"));
    case 9: return QString(QObject::tr("Gas leak"));
    case 10: return QString(QObject::tr("Gas leak is absent"));
    }
    return QString();
}

/**
 * @brief Maps an alert source code (e.g., CO2, PM2.5) to its label.
 *
 * Used in compound alerts like "Poor air quality (PM2.5)".
 *
 * @param index Source code (1–7).
 * @return QString Source name.
 */
QString QSNAlertTypeLabelSource(quint8 index)
{
    switch (index) {
    case 1: return QString(QObject::tr("CO2"));
    case 2: return QString(QObject::tr("PM1.0"));
    case 3: return QString(QObject::tr("PM2.5"));
    case 4: return QString(QObject::tr("PM10"));
    case 5: return QString(QObject::tr("НСНО"));
    case 6: return QString(QObject::tr("TVOC"));
    case 7: return QString(QObject::tr("CO"));
    }
    return QString();
}

/**
 * @brief Maps a security event code (e.g., arming, digit press) to its label.
 *
 * @param index Security code (0–14).
 * @return QString Description.
 */
QString QSNSecurityAlertTypeLabel(quint8 index)
{
    switch (index) {
    case 0: return QString(QObject::tr("Digit 0"));
    case 1: return QString(QObject::tr("Digit 1"));
    case 2: return QString(QObject::tr("Digit 2"));
    case 3: return QString(QObject::tr("Digit 3"));
    case 4: return QString(QObject::tr("Digit 4"));
    case 5: return QString(QObject::tr("Digit 5"));
    case 6: return QString(QObject::tr("Digit 6"));
    case 7: return QString(QObject::tr("Digit 7"));
    case 8: return QString(QObject::tr("Digit 8"));
    case 9: return QString(QObject::tr("Digit 9"));
    case 10: return QString(QObject::tr("Arming"));
    case 11: return QString(QObject::tr("Disarming"));
    case 12: return QString(QObject::tr("Penetration"));
    case 13: return QString(QObject::tr("Cancel the alarm"));
    case 14: return QString(QObject::tr("Arming (secretly)"));
    }
    return QString();
}

/**
 * @brief Converts a weather condition code to its description.
 *
 * @param index Weather code (0–7).
 * @return QString e.g., "Rain", "Thunderstorm".
 */
QString QSNWeatherTypeLabel(quint8 index)
{
    switch (index) {
    case 0: return QString(QObject::tr("Clear sky"));
    case 1: return QString(QObject::tr("Few clouds"));
    case 2: return QString(QObject::tr("Scattered clouds"));
    case 3: return QString(QObject::tr("Broken clouds"));
    case 4: return QString(QObject::tr("Shower rain"));
    case 5: return QString(QObject::tr("Rain"));
    case 6: return QString(QObject::tr("Thunderstorm"));
    case 7: return QString(QObject::tr("Snow"));
    }
    return QString();
}

/**
 * @brief Maps dust particle size code to label (PM1.0, PM2.5, PM10).
 *
 * @param index Size code (0–2).
 * @return QString Particle size class.
 */
QString QSNDustTypeSize(quint8 index)
{
    switch (index) {
    case 0: return QString(QObject::tr("PM1.0"));
    case 1: return QString(QObject::tr("PM2.5"));
    case 2: return QString(QObject::tr("PM10"));
    }
    return QString();
}

/**
 * @brief Maps an infrared remote control protocol code to its name.
 *
 * @param index Protocol ID (0–11).
 * @return QString Protocol name (e.g., "NEC", "SAMSUNG").
 */
QString QSNRMType(quint8 index) {
    switch (index) {
    case 0: return QString(QObject::tr("NEC"));
    case 1: return QString(QObject::tr("SAMSUNG"));
    case 2: return QString(QObject::tr("NONE"));
    case 3: return QString(QObject::tr("RC6"));
    case 4: return QString(QObject::tr("SHARP"));
    case 5: return QString(QObject::tr("TV"));
    case 6: return QString(QObject::tr("PT2262"));
    case 7: return QString(QObject::tr("EV1527"));
    case 8: return QString(QObject::tr("DOOYA DC2007"));
    case 9: return QString(QObject::tr("SONY 12bit"));
    case 10: return QString(QObject::tr("SONY 15bit"));
    case 11: return QString(QObject::tr("SONY 20bit"));
    }
    return QString(QObject::tr("Unknown"));
}

//==============================
/**
 * @brief Reads a single bit from a specified position in a byte array.
 *
 * Used for compact boolean flags stored within bytes.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Byte index.
 * @param bit Bit index within the byte (0 = LSB).
 * @return bool Value of the bit (true/false).
 */
bool QSNRAWtoBit(QByteArray *Data, int addr, int bit)
{
    if (Data->count() <= addr) return false;
    quint8 vol = static_cast<quint8>(Data->at(addr));
    vol >>= bit;
    vol &= 1;
    return (vol != 0);
}

/**
 * @brief Writes a single bit into a specified position in a byte array.
 *
 * Modifies only the target bit; others remain unchanged.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Byte index.
 * @param bit Bit index (0–7).
 * @param state Desired bit value.
 */
void QSNBitToRAW(QByteArray *Data, int addr, int bit, bool state)
{
    if (Data->count() <= addr) Data->resize(addr + 1);
    quint8 msk = 1;
    msk <<= bit;
    quint8 vol = static_cast<quint8>(Data->at(addr));
    if (state) vol |= msk;
    else { msk = ~msk; vol &= msk; }
    (*Data)[addr] = static_cast<char>(vol);
}

/**
 * @brief Interprets a byte as a boolean (non-zero = true).
 *
 * @param Data Pointer to QByteArray.
 * @param addr Byte index.
 * @return bool True if byte != 0.
 */
bool QSNRAWtoBool(QByteArray *Data, int addr)
{
    if (Data->count() <= addr) return false;
    return (static_cast<quint8>(Data->at(addr)) > 0);
}

/**
 * @brief Serializes a boolean as a single byte (1 or 0).
 *
 * @param Data Pointer to QByteArray.
 * @param addr Target byte index.
 * @param state Boolean value to store.
 */
void QSNBoolToRAW(QByteArray *Data, int addr, bool state)
{
    if (Data->count() <= addr) Data->resize(addr + 1);
    if (state) (*Data)[addr] = static_cast<quint8>(1);
    else (*Data)[addr] = static_cast<quint8>(0);
}

/**
 * @brief Reads an unsigned 8-bit integer from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Byte index.
 * @return quint8 Value at that position.
 */
quint8 QSNRAWtoByte(QByteArray *Data, int addr)
{
    if (Data->count() <= addr) return 0;
    return static_cast<quint8>(Data->at(addr));
}

/**
 * @brief Writes an unsigned 8-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Target byte index.
 * @param value Value to store.
 */
void QSNByteToRAW(QByteArray *Data, int addr, quint8 value)
{
    if (Data->count() <= addr) Data->resize(addr + 1);
    (*Data)[addr] = static_cast<char>(value);
}

/**
 * @brief Reads a signed 8-bit integer (with proper sign extension) from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Byte index.
 * @return qint8 Signed value.
 */
qint8 QSNRAWtoInt8(QByteArray *Data, int addr)
{
    if (Data->count() <= addr) return 0;
    union t_u {qint8 vol; quint8 D;};
    t_u t;
    t.D = static_cast<quint8>(Data->at(addr));
    return t.vol;
}

/**
 * @brief Writes a signed 8-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Target byte index.
 * @param value Value to store.
 */
void QSNInt8ToRAW(QByteArray *Data, int addr, qint8 value)
{
    if (Data->count() <= addr) Data->resize(addr + 1);
    union t_u {qint8 vol; quint8 D;};
    t_u t;
    t.vol = value;
    (*Data)[addr] = static_cast<char>(t.D);
}

/**
 * @brief Reads a big-endian unsigned 16-bit integer from a byte array.
 *
 * Bytes are stored as [MSB, LSB].
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index (reads addr and addr+1).
 * @return quint16 Reconstructed value.
 */
quint16 QSNRAWtoUInt16(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 1) return 0;
    union t_u {quint16 vol; quint8 D[2];};
    t_u t;
    t.D[0] = static_cast<quint8>(Data->at(addr + 1));
    t.D[1] = static_cast<quint8>(Data->at(addr));
    return t.vol;
}

/**
 * @brief Writes a big-endian unsigned 16-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Value to serialize.
 */
void QSNUInt16ToRAW(QByteArray *Data, int addr, quint16 value)
{
    if (Data->count() <= addr + 1) Data->resize(addr + 2);
    union t_u {quint16 vol; quint8 D[2];};
    t_u t;
    t.vol = value;
    (*Data)[addr + 1] = static_cast<char>(t.D[0]);
    (*Data)[addr] = static_cast<char>(t.D[1]);
}

/**
 * @brief Reads a big-endian signed 16-bit integer from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @return qint16 Reconstructed value.
 */
qint16 QSNRAWtoInt16(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 1) return 0;
    union t_u {qint16 vol; quint8 D[2];};
    t_u t;
    t.D[0] = static_cast<quint8>(Data->at(addr + 1));
    t.D[1] = static_cast<quint8>(Data->at(addr));
    return t.vol;
}

/**
 * @brief Writes a big-endian signed 16-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Value to serialize.
 */
void QSNInt16ToRAW(QByteArray *Data, int addr, qint16 value)
{
    if (Data->count() <= addr + 1) Data->resize(addr + 2);
    union t_u {qint16 vol; quint8 D[2];};
    t_u t;
    t.vol = value;
    (*Data)[addr + 1] = static_cast<char>(t.D[0]);
    (*Data)[addr] = static_cast<char>(t.D[1]);
}

/**
 * @brief Reads a big-endian signed 32-bit integer from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index (reads 4 bytes).
 * @return qint32 Reconstructed value.
 */
qint32 QSNRAWtoInt32(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 3) return 0;
    union t_u {qint32 vol; quint8 D[4];};
    t_u t;
    for (int i = 0; i < 4; i ++)
        t.D[i] = static_cast<quint8>(Data->at(addr + 3 - i));
    return t.vol;
}

/**
 * @brief Writes a big-endian signed 32-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Value to serialize.
 */
void QSNInt32ToRAW(QByteArray *Data, int addr, qint32 value)
{
    if (Data->count() <= addr + 3) Data->resize(addr + 4);
    union t_u {qint32 vol; quint8 D[4];};
    t_u t;
    t.vol = value;
    for (int i = 0; i < 4; i ++)
        (*Data)[addr + 3 - i] = static_cast<char>(t.D[i]);
}

/**
 * @brief Reads a signed 24-bit integer (stored in 3 bytes, big-endian) with sign extension.
 *
 * Used primarily in power measurements (QSNPower).
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @return qint32 Sign-extended 32-bit integer.
 */
qint32 QSNRAWtoInt24(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 2) return 0;
    union t_u {qint32 vol; quint8 D[4];};
    t_u t;
    if (Data->at(addr) & 128) t.D[3] = 255;
    else t.D[3] = 0;
    for (int i = 0; i < 3; i ++)
        t.D[i] = static_cast<quint8>(Data->at(addr + 2 - i));
    return t.vol;
}

/**
 * @brief Writes a signed 24-bit integer into 3 bytes (big-endian).
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Value to serialize (only lower 24 bits used).
 */
void QSNInt24ToRAW(QByteArray *Data, int addr, qint32 value)
{
    if (Data->count() <= addr + 2) Data->resize(addr + 3);
    union t_u {qint32 vol; quint8 D[4];};
    t_u t;
    t.vol = value;
    for (int i = 0; i < 3; i ++)
        (*Data)[addr + 2 - i] = static_cast<char>(t.D[i]);
}

/**
 * @brief Reads a big-endian signed 64-bit integer from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index (reads 8 bytes).
 * @return qint64 Reconstructed value.
 */
qint64 QSNRAWtoInt64(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 7) return 0;
    union t_u {qint64 vol; quint8 D[8];};
    t_u t;
    for (int i = 0; i < 8; i ++)
        t.D[i] = static_cast<quint8>(Data->at(addr + 7 - i));
    return t.vol;
}

/**
 * @brief Writes a big-endian signed 64-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Value to serialize.
 */
void QSNInt64ToRAW(QByteArray *Data, int addr, qint64 value)
{
    if (Data->count() <= addr + 7) Data->resize(addr + 8);
    union t_u {qint64 vol; quint8 D[8];};
    t_u t;
    t.vol = value;
    for (int i = 0; i < 8; i ++)
        (*Data)[addr + 7 - i] = static_cast<char>(t.D[i]);
}

/**
 * @brief Reads a big-endian unsigned 32-bit integer from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @return quint32 Reconstructed value.
 */
quint32 QSNRAWtoUInt32(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 3) return 0;
    union t_u {quint32 vol; quint8 D[4];};
    t_u t;
    for (int i = 0; i < 4; i ++)
        t.D[i] = static_cast<quint8>(Data->at(addr + 3 - i));
    return t.vol;
}

/**
 * @brief Writes a big-endian unsigned 32-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Value to serialize.
 */
void QSNUInt32ToRAW(QByteArray *Data, int addr, quint32 value)
{
    if (Data->count() <= addr + 3) Data->resize(addr + 4);
    union t_u {quint32 vol; quint8 D[4];};
    t_u t;
    t.vol = value;
    for (int i = 0; i < 4; i ++)
        (*Data)[addr + 3 - i] = static_cast<char>(t.D[i]);
}

/**
 * @brief Reads a big-endian unsigned 64-bit integer from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @return quint64 Reconstructed value.
 */
quint64 QSNRAWtoUInt64(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 7) return 0;
    union t_u {quint64 vol; quint8 D[8];};
    t_u t;
    for (int i = 0; i < 8; i ++)
        t.D[i] = static_cast<quint8>(Data->at(addr + 7 - i));
    return t.vol;
}

/**
 * @brief Writes a big-endian unsigned 64-bit integer into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Value to serialize.
 */
void QSNUInt64ToRAW(QByteArray *Data, int addr, quint64 value)
{
    if (Data->count() <= addr + 7) Data->resize(addr + 8);
    union t_u {quint64 vol; quint8 D[8];};
    t_u t;
    t.vol = value;
    for (int i = 0; i < 8; i ++)
        (*Data)[addr + 7 - i] = static_cast<char>(t.D[i]);
}

/**
 * @brief Converts raw bytes to a temperature value in the current global unit (°C or °F).
 *
 * Temperature is stored as a signed 16-bit fixed-point number (1/256°C resolution).
 * Handles two's complement for negative values.
 *
 * @param Data Pointer to raw byte array.
 * @param addr Byte offset of the temperature value (typically 1, after type byte).
 * @return qreal Temperature in °C or °F based on QSNCelsius setting.
 */
qreal QSNRAWtoTemperature(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 1) return 0;
    quint16 vl = static_cast<quint8>(Data->at(addr));
    qreal temp;
    vl = static_cast<quint16>((vl << 8));
    vl += static_cast<quint8>(Data->at(addr + 1));
    if ((vl >> 15) == 0) {
        temp = (vl * 0.00390625);
    } else {
        vl = 0xFFFF^vl;
        vl ++;
        temp = (vl * -0.00390625);
    }
    if (QSNCelsius) return temp;
    else return QSNCelsiusToFahrenheit(temp);
}

/**
 * @brief Serializes a temperature value into raw bytes using fixed-point format.
 *
 * Converts input to Celsius if needed, then encodes as 16-bit signed fixed-point.
 *
 * @param Data Pointer to QByteArray to write into.
 * @param addr Target byte offset.
 * @param value Temperature in current global unit (°C or °F).
 */
void QSNTemperatureToRAW(QByteArray *Data, int addr, qreal value)
{
    if (Data->count() <= addr + 1) Data->resize(addr + 2);
    quint16 cv;
    qreal temp;
    if (QSNCelsius) temp = value;
    else temp = QSNFahrenheitToCelsius(value);
    if (value < 0) {
        cv = static_cast<quint16>(temp / -0.00390625);
        cv --;
        cv = 0xFFFF ^ cv;
        (*Data)[addr] = static_cast<char>(cv >> 8);
        (*Data)[addr + 1] = static_cast<char>(cv & 255);

    } else {
        cv = static_cast<quint16>(temp / 0.00390625);
        (*Data)[addr] = static_cast<char>(cv >> 8);
        (*Data)[addr + 1] = static_cast<char>(cv & 255);
    }
}

/**
 * @brief Reads humidity from raw data (supports fractional part).
 *
 * Format: [integer][fractional] → e.g., 45 + 30/100 = 45.30%
 *
 * @param data Pointer to raw data.
 * @param addr Byte offset.
 * @return qreal Humidity percentage (0.00–100.99).
 */
qreal QSNRAWtoHumidity(QByteArray *data, int addr)
{
    if (data->count() <= addr) return 0;
    if (data->count() <= addr + 1) return (*data)[addr];
    return static_cast<qreal>((*data)[addr]) + static_cast<qreal>((*data)[addr + 1]) / 100;
}

/**
 * @brief Writes a humidity value into raw data with 2-digit fractional precision.
 *
 * @param data Pointer to QByteArray.
 * @param addr Target offset.
 * @param value Humidity in percent.
 */
void QSNHumidityToRAW(QByteArray *data, int addr, qreal value)
{
    if (data->count() <= addr + 1) data->resize(addr + 2);
    (*data)[addr] = static_cast<quint8>(value);
    (*data)[addr + 1] = static_cast<quint8>((value - data->at(addr)) * 100);
}

/**
 * @brief Serializes a fixed-point decimal number into 32-bit big-endian integer.
 *
 * Used for generic scaled integers (e.g., distance in mm → m with 2 decimals).
 *
 * @param Data Pointer to output buffer.
 * @param addr Write offset.
 * @param value Input floating-point number.
 * @param point Number of decimal places (e.g., 2 → ×100).
 */
void QSNFixedPointToRAW(QByteArray *Data, int addr, qreal value, quint8 point)
{
    qint32 v = value * QSNPOW(10, point);
    if (Data->count() <= addr + 3) Data->resize(addr + 4);
    union t_u {qint32 vol; quint8 D[4];};
    t_u t;
    t.vol = v;
    for (int i = 0; i < 4; i ++)
        (*Data)[addr + 3 - i] = static_cast<char>(t.D[i]);
}

/**
 * @brief Deserializes a fixed-point number from raw 32-bit integer.
 *
 * @param Data Pointer to input buffer.
 * @param addr Read offset.
 * @param point Decimal scale (e.g., 2 → ÷100).
 * @return qreal Reconstructed floating-point value.
 */
qreal QSNRAWtoFixedPoint(QByteArray *Data, int addr, quint8 point)
{
    if (Data->count() <= addr + 3) return 0;
    union t_u {qint32 vol; quint8 D[4];};
    t_u t;
    for (int i = 0; i < 4; i ++)
        t.D[i] = static_cast<quint8>(Data->at(addr + 3 - i));
    return static_cast<qreal>(t.vol) / QSNPOW(10, point);
}

/**
 * @brief Reads a composite "Power" structure from raw data.
 *
 * Contains:
 *   - power: signed 24-bit (mW·s or similar),
 *   - seconds: unsigned 16-bit (integration interval),
 *   - idp: optional device ID (2 bytes).
 *
 * @param Data Pointer to raw data.
 * @param addr Starting offset.
 * @return QSNPower Struct with decoded fields.
 */
QSNPower QSNRAWtoPower(QByteArray *Data, int addr)
{
    QSNPower ret;
    ret.power = 0;
    ret.seconds = 0;
    ret.idp = 0;
    if (Data->count() <= addr + 5) return ret;
    ret.power = QSNRAWtoInt24(Data, addr) ;
    ret.seconds = QSNRAWtoUInt16(Data, addr + 3);
    if (Data->count() > addr + 6) ret.idp = QSNRAWtoUInt16(Data, addr + 5);
    return ret;
}

/**
 * @brief Serializes a QSNPower structure into raw bytes.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param power Source structure.
 */
void QSNPowerToRAW(QByteArray *Data, int addr, QSNPower power)
{
    if (Data->count() <= addr + 7) Data->resize(addr + 7);
    QSNInt24ToRAW(Data, addr, power.power);
    QSNUInt16ToRAW(Data, addr + 3, power.seconds);
    QSNUInt16ToRAW(Data, addr + 5, power.idp);
}

/**
 * @brief Parses a full datetime (day, month, year, hour, min, sec) from 7 raw bytes.
 *
 * Layout: [day][month][year:2B][hour][min][sec]
 *
 * @param Data Input buffer.
 * @param addr Offset.
 * @return QDateTime Parsed value (local time assumed).
 */
QDateTime QSNRAWtoDateTime(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 6) return QDateTime();
    QDateTime dt;
    dt.setDate(QDate(QSNRAWtoUInt16(Data, addr + 2), static_cast<quint8>(Data->at(addr + 1)) , static_cast<quint8>(Data->at(addr))));
    dt.setTime(QTime(static_cast<quint8>(Data->at(addr + 4)), static_cast<quint8>(Data->at(addr + 5)), static_cast<quint8>(Data->at(addr + 6))));
    // dt.setTimeSpec(Qt::UTC);
    return dt;
}

/**
 * @brief Serializes a QDateTime into 7-byte compact format.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param dateTime Source datetime.
 */
void QSNDateTimeToRAW(QByteArray *Data, int addr, QDateTime dateTime)
{
    if (Data->count() <= addr + 6) Data->resize(addr + 7);
    (*Data)[addr] = static_cast<char>(dateTime.date().day());
    (*Data)[addr + 1] = static_cast<char>(dateTime.date().month());
    QSNUInt16ToRAW(Data, addr + 2, static_cast<quint16>(dateTime.date().year()));
    (*Data)[addr + 4] = static_cast<char>(dateTime.time().hour());
    (*Data)[addr + 5] = static_cast<char>(dateTime.time().minute());
    (*Data)[addr + 6] = static_cast<char>(dateTime.time().second());
}

/**
 * @brief Decodes an alert/notification structure from raw data.
 *
 * Contains: type (1B), IDP (2B), optional source (1B).
 *
 * @param Data Input buffer.
 * @param addr Offset.
 * @return QSNNotification Decoded alert info.
 */
QSNNotification QSNRAWtoNotification(QByteArray *Data, int addr)
{
    QSNNotification notif;
    notif.notificationType = 255;
    notif.idp = 0;
    notif.notificationSource = 0;
    if (Data->count() <= addr + 2) return notif;
    notif.notificationType = static_cast<quint8>(Data->at(addr));
    notif.idp = QSNRAWtoUInt16(Data, addr + 1);
    if (Data->count() > addr + 3) notif.notificationSource = static_cast<quint8>(Data->at(addr + 3));
    return notif;
}

/**
 * @brief Serializes a notification into raw bytes.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param notification Source struct.
 */
void QSNNotificationToRAW(QByteArray *Data, int addr, QSNNotification notification)
{
    if (Data->count() <= addr + 2) Data->resize(addr + 4);
    QSNByteToRAW(Data, addr, notification.notificationType);
    QSNUInt16ToRAW(Data, addr + 1, notification.idp);
    QSNByteToRAW(Data, addr + 3, notification.notificationSource);
}

/**
 * @brief Decodes an error report structure (error code + device info).
 *
 * Layout: [error:2B][IDP:2B][deviceType:2B]
 *
 * @param Data Input buffer.
 * @param addr Offset.
 * @return QSNError Decoded error info.
 */
QSNError QSNRAWtoError(QByteArray *Data, int addr)
{
    QSNError err;
    err.errorIndex = 0;
    err.idp = 0;
    err.deviceType = 0;
    if (Data->count() <= addr + 5) return err;
    err.errorIndex = QSNRAWtoUInt16(Data, addr);
    err.idp = QSNRAWtoUInt16(Data, addr + 2);
    err.deviceType = QSNRAWtoUInt16(Data, addr + 4);
    return err;
}

/**
 * @brief Serializes an error report into raw bytes.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param error Source error struct.
 */
void QSNErrorToRAW(QByteArray *Data, int addr, QSNError error)
{
    if (Data->count() <= addr + 5) Data->resize(addr + 6);
    QSNUInt16ToRAW(Data, addr, error.errorIndex);
    QSNUInt16ToRAW(Data, addr + 2, error.idp);
    QSNUInt16ToRAW(Data, addr + 4, error.deviceType);
}

/**
 * @brief Decodes a fixed-length ASCII string from raw data.
 *
 * Stops at first null byte. Assumes local 8-bit encoding (e.g., CP1251).
 *
 * @param Data Input buffer.
 * @param addr Starting offset.
 * @param textLength Maximum number of bytes to read.
 * @return QString Decoded ASCII string.
 */
QString QSNRAWtoASCII(QByteArray *Data, int addr, int textLength) //TODO restruct
{
    QByteArray ascii;
    if (Data->count() < addr + textLength) return ascii;
    for (int i = 0; i < textLength; i ++){
        if (static_cast<quint8>(Data->at(addr + i)) == 0) return ascii;
        ascii.append(static_cast<char>(Data->at(addr + i)));
    }
    return QString::fromLocal8Bit(ascii);
}

/**
 * @brief Encodes a QString into raw ASCII bytes using local 8-bit encoding.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param text Source string.
 */
void QSNASCIIToRAW(QByteArray *Data, int addr, QString text)
{
    QByteArray ascii = text.toLocal8Bit();
    //  if (text.count() > 7) text.resize(7);
    if (Data->count() <= addr + text.count() - 1) Data->resize(addr + text.count());
    for (int i = 0; i < ascii.count(); i ++)
        (*Data)[addr + i] = ascii[i];
}


/**
 * @brief Reads a generic string with optional prefix padding from raw data.
 *
 * Used in legacy formats where string position is encoded in first byte.
 *
 * @param Data Input buffer.
 * @param text Fallback if data is invalid.
 * @param addr Offset of type/prefix byte.
 * @return QString Formatted string.
 */
QString QSNRAWtoString(QByteArray *Data, QString text, int addr)
{
    QString ret = text;
    if (Data->count() <= addr) return ret;
    int pos = Data->at(addr);
    if (pos > 30) return QSNRAWtoUTF8(Data, addr);
    pos *= 5;
    QString nt = QSNRAWtoUTF8(Data, addr + 1);
    ret.resize(pos + nt.size(), ' ');
    ret.replace(pos, nt.size(), nt);
    return ret;
}

/**
 * @brief Reads a UTF-8 encoded string from raw data starting at given offset.
 *
 * Interprets all bytes from `addr` to end as UTF-8.
 *
 * @param Data Input buffer.
 * @param addr Starting offset.
 * @return QString Decoded UTF-8 string.
 */
QString QSNRAWtoUTF8(QByteArray *Data, int addr)
{
    QByteArray utf8 = *Data;
    if (Data->count() <= addr) return QString::fromUtf8(utf8);
    utf8.remove(0, addr);
    return QString::fromUtf8(utf8);
}

/**
 * @brief Encodes a QString into UTF-8 and writes it into raw data.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param text Source string.
 */
void QSNUTF8ToRAW(QByteArray *Data, int addr, QString text)
{
    QByteArray utf8 = text.toUtf8();
    if (Data->count() <= addr + utf8.count()) Data->resize(addr + utf8.count());
    for (int i = 0; i < utf8.count(); i ++)
        if (addr + i < 8)(*Data)[addr + i] = utf8[i];
}

/**
 * @brief Reads a date-only value (day, month, year) from 4 bytes.
 *
 * @param Data Input buffer.
 * @param addr Offset.
 * @return QDate Parsed date.
 */
QDate QSNRAWtoDate(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 3) return QDate();
    QDate date;
    date.setDate(QSNRAWtoUInt16(Data, addr + 2), static_cast<quint8>(Data->at(addr + 1)), static_cast<quint8>(Data->at(addr)));
    return date;
}

/**
 * @brief Serializes a QDate into 4-byte format.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param date Source date.
 */
void QSNDateToRAW(QByteArray *Data, int addr, QDate date)
{
    if (Data->count() <= addr + 3) Data->resize(addr + 4);
    quint16 year = static_cast<quint16>(date.year());
    (*Data)[addr] = static_cast<char>(date.day());
    (*Data)[addr + 1] = static_cast<char>(date.month());
    QSNUInt16ToRAW(Data, addr + 2, year);
}

/**
 * @brief Reads a time-only value (hour, min, sec) from 3 bytes.
 *
 * @param Data Input buffer.
 * @param addr Offset.
 * @return QTime Parsed time.
 */
QTime QSNRAWtoTime(QByteArray *Data, int addr)
{
    if (Data->count() <= addr + 2) return QTime();
    QTime time;
    time.setHMS(static_cast<quint8>(Data->at(addr)), static_cast<quint8>(Data->at(addr + 1)), static_cast<quint8>(Data->at(addr + 2)));
    return time;
}


/**
 * @brief Serializes a QTime into 3-byte format.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param time Source time.
 */
void QSNTimeToRAW(QByteArray *Data, int addr, QTime time)
{
    if (Data->count() <= addr + 2) Data->resize(addr + 3);
    (*Data)[addr] = static_cast<char>(time.hour());
    (*Data)[addr + 1] = static_cast<char>(time.minute());
    (*Data)[addr + 2] = static_cast<char>(time.second());
}

QString QSNRAWtoHEXcode(QByteArray *Data, int addr)
{
    if (Data->count() <= addr) return QLatin1String("00");
    int a = addr;
    QString ret;
    do {
        ret.append(QString("%1").arg(QString().number(QSNRAWtoByte(Data, a), 16)).toUpper());
        a ++;
    } while (a < Data->count());
    return ret;
}

void QSNHEXcodeToRAW(QByteArray *Data, int addr, QString HEXstring)
{
    QByteArray ba = QByteArray::fromHex(HEXstring.toUtf8());
    if (ba.count() > 7) ba.resize(7);
    Data->resize(addr);
    Data->append(ba);
}

/**
 * @brief Reads a big-endian IEEE 754 32-bit float from a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index (4 bytes).
 * @return float Reconstructed floating-point value.
 */
float QSNRAWtoFloat(QByteArray *Data, int addr) {
    if (Data->count() <= addr + 3) return 0;
    union t_u {float vol; quint8 D[4];};
    t_u t;
    for (int i = 0; i < 4; i ++)
        t.D[i] = static_cast<quint8>(Data->at(addr + 3 - i));
    return t.vol;
}

/**
 * @brief Writes a big-endian IEEE 754 32-bit float into a byte array.
 *
 * @param Data Pointer to QByteArray.
 * @param addr Starting byte index.
 * @param value Float value to serialize.
 */
void QSNFloatToRAW(QByteArray *Data, int addr, float value) {
    if (Data->count() <= addr + 3) Data->resize(addr + 4);
    union t_u {float vol; quint8 D[4];};
    t_u t;
    t.vol = value;
    for (int i = 0; i < 4; i ++)
        (*Data)[addr + 3 - i] = static_cast<char>(t.D[i]);
}

/**
 * @brief Decodes a SNIR (Simple Network Identification Report) packet.
 *
 * Contains: 7-bit address, 6-bit parameter, 2-bit battery level, 5-byte payload.
 *
 * @param Data Input buffer.
 * @param addr Starting offset.
 * @return QSNSNIR Parsed structure.
 */
QSNSNIR QSNRAWtoSNIR(QByteArray *Data, int addr)
{
    QSNSNIR ret;
    if (Data->count() <= addr + 3) {
        ret.address = 0;
        ret.parameter = 0;
        ret.data = QByteArray(5, 0);
        return ret;
    }

    ret.address = static_cast<quint8>((*Data)[addr]) & 0b00111111;
    ret.retry = static_cast<quint8>((*Data)[addr]) >> 6;
    ret.battery = static_cast<quint8>((*Data)[addr + 1]) >> 5;
    ret.parameter = ((*Data)[addr + 1] & 0b00011111);
    for (int i = 0; i < 5; i ++)
        if (Data->count() > (i + addr + 2))  ret.data.append(Data->at(i + addr + 2));
        else ret.data.append(char(0));
    return ret;
}

/**
 * @brief Serializes a SNIR structure into raw bytes.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param snir Source structure.
 */
void QSNSNIRToRAW(QByteArray *Data, int addr, QSNSNIR snir) {
    if (Data->count() <= addr + 6) Data->resize(addr + 7);
    (*Data)[addr] = static_cast<char>((snir.address & 0b00111111));
    (*Data)[addr + 1] = static_cast<char>((snir.battery << 5) + (snir.parameter & 0b00011111));
    for (int i = 0; i < 5; i ++) (*Data)[addr + 2 + i] = snir.data[i];
}

/**
 * @brief Reads a 7-byte infrared remote control (RM) command.
 *
 * Used for NEC, Samsung, etc. protocols.
 *
 * @param Data Input buffer.
 * @param addr Starting offset.
 * @return QByteArray Raw RM command (7 bytes).
 */
QByteArray QSNRAWtoRMcode(QByteArray *Data, int addr)
{
    QByteArray ret;
    if (Data->count() <= addr + 1) {
        ret = QByteArray(7, 0);
        return ret;
    }

    for (int i = addr; i < addr + 7; i ++)
        if (Data->count() > i)  ret.append(Data->at(i));
        else ret.append(char(0));
    return ret;
}

/**
 * @brief Writes a 7-byte RM command into raw data.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param code 7-byte command.
 */
void QSNRMcodeToRAW(QByteArray *Data, int addr, QByteArray code)
{
    if (Data->count() < addr + 7) Data->resize(addr + 7);
    for (int i = 0; i < 7; i ++) (*Data)[addr + i] = (i < code.count()?code[i]:0);
}

/**
 * @brief Decodes an EM-Marine RFID UID (4-byte: 2B family + 2B serial).
 *
 * @param Data Input buffer.
 * @param addr Starting offset.
 * @return QSNEMUID Parsed UID.
 */
QSNEMUID QSNRAWtoEmUID(QByteArray *Data, int addr)
{
    QSNEMUID ret;
    if (Data->count() <= addr + 3) {
        ret.family = 0;
        ret.number = 0;
        return ret;
    }
    ret.family = QSNRAWtoUInt16(Data, addr);
    ret.number = QSNRAWtoUInt16(Data, addr + 2);
    return ret;
}

/**
 * @brief Serializes an EM-Marine UID into raw bytes.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param uid Source UID structure.
 */
void QSNEmUIDToRAW(QByteArray *Data, int addr, QSNEMUID uid)
{
    if (Data->count() <= addr + 3) Data->resize(addr + 4);
    QSNUInt16ToRAW(Data, addr, uid.family);
    QSNUInt16ToRAW(Data, addr + 2, uid.number);
}

/**
 * @brief Converts raw bytes into a dotted version string (e.g., "1.2.3").
 *
 * Each byte becomes a decimal component.
 *
 * @param Data Input buffer.
 * @param addr Starting offset.
 * @return QString Version string.
 */
QString QSNRAWtoVersion(QByteArray *Data, int addr)
{
    QString ver;
    int c = Data->count() - addr;
    for (int i = 0; i < c; i ++)
        ver += QString::number(Data->at(addr + i)) + '.';
    if (ver.right(1) == QString(".")) ver.remove(ver.count() - 1, 1);
    return ver;
}

/**
 * @brief Encodes a version string (e.g., "1.2.3") into raw bytes.
 *
 * Only digits are parsed; non-numeric parts are ignored.
 *
 * @param Data Output buffer.
 * @param addr Write offset.
 * @param version Dotted version string.
 */
void QSNVersionToRAW(QByteArray *Data, int addr, QString version)
{
    bool ok;
    int a = addr;
    QStringList sl = version.split('.');
    if (Data->count() < addr + sl.count()) Data->resize(addr + sl.count());
    foreach(QString item, sl) if (a < 8)(*Data)[a ++] = static_cast<char>(item.toInt(&ok));
}

//=========================containers==============================================
/**
 * @brief Creates a new default-initialized QSNContainer instance.
 *
 * Used as a template for all message types (signal, service, registration, etc.).
 *
 * @return QSNContainer Initialized with default values.
 */
QSNContainer newContainer()
{
    QSNContainer cn;
    cn.Address = 0;
    cn.Command = 0;
    cn.Data = QByteArray();
    cn.Sender = 0;
    cn.Signal = 0;
    cn.role = QSNContainer::service;
    cn.info = QString();
    return cn;
}

/**
 * @brief Serializes a QSNContainer into a QDataStream (e.g., for network or file storage).
 *
 * @param container Pointer to source container.
 * @param stream Pointer to output stream.
 */
void containerToStream(QSNContainer *container, QDataStream *stream)
{
    *stream << static_cast<int>(container->role);
    *stream << container->Signal;
    *stream << container->Command;
    *stream << container->Data;
    *stream << container->Address;
    *stream << container->Sender;
    *stream << container->info;
}

/**
 * @brief Deserializes a QSNContainer from a QDataStream.
 *
 * @param stream Pointer to input stream.
 * @return QSNContainer Reconstructed container.
 */
QSNContainer containerFromStream(QDataStream *stream)
{
    QSNContainer container = newContainer();
    int role;
    *stream >> role;
    container.role = static_cast<QSNContainer::type>(role);
    *stream >> container.Signal;
    *stream >> container.Command;
    *stream >> container.Data;
    *stream >> container.Address;
    *stream >> container.Sender;
    *stream >> container.info;
    return container;
}

/**
 * @brief Validates a container against size restrictions (e.g., max 120 bytes for data/info).
 *
 * Prevents oversized payloads in constrained environments.
 *
 * @param container Pointer to container to validate.
 * @return bool True if within limits.
 */
bool checkingContainerForRestrictions(QSNContainer *container)
{
    if (container->Data.count() > 120) return false;
    if (container->info.count() > 120) return false;
    return true;
}

/**
 * @brief Encrypts a portion of a byte array using a simple XOR-based scheme with a key.
 *
 * Appends a checksum byte at the end. Modifies the input array in-place.
 *
 * @param data Pointer to QByteArray to encrypt.
 * @param start Byte offset to begin encryption.
 * @param key Secret string used for key derivation.
 */
void dataEncript(QByteArray *data, int start, QString key)
{
    QByteArray keyarray = key.toLocal8Bit();
    int k = 0;
    int kl = keyarray.count() - 1;
    data->append(static_cast<char>(0));
    int c = data->count() - 1;
    char s = 0;
    char se = 0;
    for (int i = start; i <= c; i ++) {
        se = s;
        if (i < c) s = s ^ (*data)[i];
        else (*data)[i] = s;
        (*data)[i] = (*data)[i] ^ keyarray[k];
        (*data)[i] = (*data)[i] ^ se;
        (*data)[i] = (*data)[i] ^ keyarray[kl - k];
        k ++;
        if (k > kl) k = 0;
    }
}

/**
 * @brief Decrypts and verifies integrity of an encrypted byte array.
 *
 * Uses the last byte as a checksum. Returns false if verification fails.
 *
 * @param data Pointer to encrypted data (modified in-place).
 * @param start Offset to begin decryption.
 * @param key Secret key (must match encryption key).
 * @return bool True if decryption and checksum succeed.
 */
bool dataDecript(QByteArray *data, int start, QString key)
{
    QByteArray keyarray = key.toLocal8Bit();
    int k = 0;
    int kl = keyarray.count() - 1;
    int c = data->count() - 1;
    char s = 0;
    for (int i = start; i <= c; i ++) {
        (*data)[i] = (*data)[i] ^ keyarray[k];
        (*data)[i] = (*data)[i] ^ s;
        (*data)[i] = (*data)[i] ^ keyarray[kl - k];
        if (i < c) s = s ^ (*data)[i];
        k ++;
        if (k > kl) k = 0;
    }
    if (s == (*data)[c]) return true;
    return false;
}

/**
 * @brief Converts a log event into a standardized QSNContainer.
 *
 * Used by logging subsystem to transmit or store messages uniformly.
 *
 * @param level Log severity (e.g., info=2, warning=1).
 * @param message Human-readable log text.
 * @param category Optional numeric category (e.g., module ID).
 * @param filename Source file name.
 * @param isDebug True if debug-level message.
 * @return QSNContainer Encapsulated log entry.
 */
QSNContainer QSNLogToContainer(quint16 level, QString message, quint8 category, QString filename, bool isDebug)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_LOG;
    container.Address = isDebug;
    container.Sender = category;
    container.info = message;
    container.Signal = level;
    container.Data = QString("%1").arg(filename).toUtf8();
    return container;
}

QSNDefaultChannel QSNDefChanFromType(quint8 type)
{
    QSNDefaultChannel ret;
    switch (type) {
    case 0:
        ret.defaultName = QObject::tr("No type");
        ret.defaultSignal = 65000;
        ret.defaultNote = QObject::tr("Description channel");
        ret.defaultSignalType = 0;
        break;
    case 1:
        ret.defaultName = QObject::tr("Departure");
        ret.defaultSignal = 50001;
        ret.defaultNote = QObject::tr("The signal informs all devices on the long-term absence");
        ret.defaultSignalType = 0;
        break;
    case 2:
        ret.defaultName = QObject::tr("Presence");
        ret.defaultSignal = 50002;
        ret.defaultNote = QObject::tr("The signal notifies the device presence.");
        ret.defaultSignalType = 0;
        break;
    case 3:
        ret.defaultName = QObject::tr("Absence");
        ret.defaultSignal = 50003;
        ret.defaultNote = QObject::tr("The signal notifies the device of the absence.");
        ret.defaultSignalType = 0;
        break;
    case 4:
        ret.defaultName = QObject::tr("Nighttime");
        ret.defaultSignal = 50004;
        ret.defaultNote = QObject::tr("The signal notifies the device of night time.");
        ret.defaultSignalType = 0;
        break;
    case 5:
        ret.defaultName = QObject::tr("No nighttime");
        ret.defaultSignal = 50005;
        ret.defaultNote = QObject::tr("The signal notifies the device is not on a night time.");
        ret.defaultSignalType = 0;
        break;
    case 6:
        ret.defaultName = QObject::tr("Daytime");
        ret.defaultSignal = 50006;
        ret.defaultNote = QObject::tr("The signal notifies the device of day time.");
        ret.defaultSignalType = 0;
        break;
    case 7:
        ret.defaultName = QObject::tr("Not daytime");
        ret.defaultSignal = 50007;
        ret.defaultNote = QObject::tr("The signal notifies the device is not on a day time.");
        ret.defaultSignalType = 0;
        break;
    case 8:
        ret.defaultName = QObject::tr("Errors");
        ret.defaultSignal = 50008;
        ret.defaultNote = QObject::tr("Channel errors, transmits the address and the error code.");
        ret.defaultSignalType = 15;
        break;
    case 9:
        ret.defaultName = QObject::tr("Alert");
        ret.defaultSignal = 50009;
        ret.defaultNote = QObject::tr("A signal for notifying occurrence of an alarm event.");
        ret.defaultSignalType = 14;
        break;
    case 10:
        ret.defaultName = QObject::tr("Time");
        ret.defaultSignal = 50010;
        ret.defaultNote = QObject::tr("The time signal, the device transmits to the clock or the device receives the time signal.");
        ret.defaultSignalType = 12;
        break;
    case 11:
        ret.defaultName = QObject::tr("Security alarm");
        ret.defaultSignal = 50011;
        ret.defaultNote = QObject::tr("A signal for security occurrence of an security alarm event.");
        ret.defaultSignalType = 31;
        break;
    case 12:
        ret.defaultName = QObject::tr("Bell");
        ret.defaultSignal = 50012;
        ret.defaultNote = QObject::tr("A signal notifies the device of bell event.");
        ret.defaultSignalType = 0;
        break;
    case 13:
        ret.defaultName = QObject::tr("Silent mode");
        ret.defaultSignal = 50013;
        ret.defaultNote = QObject::tr("The signal sets the device to silent mode.");
        ret.defaultSignalType = 1;
        break;
    case 14:
        ret.defaultName = QObject::tr("Temperature");
        ret.defaultSignal = 50014;
        ret.defaultNote = QObject::tr("Signal transmits temperature, with the sender's IDP.");
        ret.defaultSignalType = 9;
        break;
    case 15:
        ret.defaultName = QObject::tr("Electricity");
        ret.defaultSignal = 50015;
        ret.defaultNote = QObject::tr("The signal transmits the consumed electricity, with the sender's IDP.");
        ret.defaultSignalType = 19;
        break;
    }
    return ret;
}

QStringList QSNDefChanListLabels()
{
    QStringList list;
    QString chName;
    quint8 i = 0;
    do {
        chName = QSNDefChanFromType(i).defaultName;
        if (!chName.isEmpty()) list.append(chName);
        i ++;
    } while (!chName.isEmpty() );
    return list;
}

/**
 * @brief Returns the application's home directory (e.g., ~/.signalnet).
 *
 * Creates the directory if it doesn't exist.
 *
 * @return QDir Application-specific config/data folder.
 */
QDir QSNHomeDir()
{
    QDir HomeDir = QDir::home();
    const QString Folder = QLatin1String("signalnet");
    if (!HomeDir.exists(Folder)) HomeDir.mkpath(Folder);
    HomeDir.cd(Folder);
    return HomeDir;
}

/**
 * @brief Ensures a subdirectory exists under the home directory.
 *
 * @param path Relative path (e.g., "devices").
 * @return QDir Full path to the subdirectory.
 */
QDir QSNHomePath(QString path)
{
    QDir HomeDir = QSNHomeDir();
    if (!HomeDir.exists(path)) HomeDir.mkpath(path);
    HomeDir.cd(path);
    return HomeDir;
}

/**
 * @brief Ensures a nested subdirectory exists (home/path/sub).
 *
 * @param path Parent path.
 * @param sub Subdirectory name.
 * @return QDir Full path.
 */
QDir QSNHomeSubPath(QString path, QString sub)
{
    QDir HomeDir = QSNHomePath(path);
    if (!HomeDir.exists(sub)) HomeDir.mkpath(sub);
    HomeDir.cd(sub);
    return HomeDir;
}

/**
 * @brief Normalizes a filename by replacing spaces/punctuation with underscores.
 *
 * Ensures compatibility with filesystems and avoids special characters.
 *
 * @param name Input filename.
 * @return QString Safe filename.
 */
QString QSNNormalizationFileName(QString name)
{
    QString ret = name.toLower();
    for (int i = 0; i < ret.count(); i ++)
    {
        if (ret[i] == ' ') ret[i] = '_';
        if (ret[i] == ',') ret[i] = '_';
        if (ret[i] == '.') ret[i] = '_';
        if (ret[i] == ';') ret[i] = '_';
        if (ret[i] == ':') ret[i] = '_';
    }
    return ret;
}

int QSNmemorySizeToBlockCount(int memorySize)
{
    int blocks = memorySize / 6;
    if (blocks * 6 < memorySize) blocks ++;
    return blocks;
}

/**
 * @brief Formats an angle (in arcminutes) as degrees and minutes (e.g., "45°30").
 *
 * @param minutes Total arcminutes (signed).
 * @return QString Formatted angle string.
 */
QString QSNAngleToString(qint16 minutes)
{
    qint16 deg = minutes / 60;
    quint8 min = static_cast<quint8>(abs(minutes - deg * 60));
    if (min == 0) return QString("%1°").arg(deg);
    else return QString("%1°%2").arg(deg).arg(min);
}

/**
 * @brief Compares two version strings up to the last dot (e.g., "1.2.3" == "1.2.4" → true).
 *
 * Useful for compatibility checks ignoring patch versions.
 *
 * @param ver1 First version.
 * @param ver2 Second version.
 * @return bool True if major.minor match.
 */
bool QSNCompareVersion(QString ver1, QString ver2)
{
    return (ver1.left(ver1.lastIndexOf('.')) == ver2.left(ver2.lastIndexOf('.')));
}

/**
 * @brief Parses a custom datetime string format ("yyyy-M-d_h-m") into QDateTime.
 *
 * @param strDT String like "2025-2-8_14-30".
 * @return QDateTime Parsed value.
 */
QDateTime QSNStrToDateTime(QString strDT)
{
    return QDateTime::fromString(strDT, QLatin1String("yyyy-M-d_h-m"));
}

/**
 * @brief Converts QDateTime to custom string format ("yyyy-MM-dd_h-mm").
 *
 * @param DT Source datetime.
 * @return QString Formatted string.
 */
QString QSNStrToDateTime(QDateTime DT)
{
    return DT.toString(QLatin1String("yyyy-MM-d_h-mm"));
}

/**
 * @brief Returns abbreviated day-of-week name (e.g., "Mo", "Tu").
 *
 * @param day 1 = Monday, ..., 7 = Sunday.
 * @return QString Abbreviation.
 */
QString QSNDayWeekAbbreviated(int day)
{
    switch (day) {
    case 1: return QObject::tr("Mo");
    case 2: return QObject::tr("Tu");
    case 3: return QObject::tr("We");
    case 4: return QObject::tr("Th");
    case 5: return QObject::tr("Fr");
    case 6: return QObject::tr("Sa");
    case 7: return QObject::tr("Su");
    default: return QObject::tr("none");
    }
}

/**
 * @brief Converts a bitmask of weekdays into a comma-separated string (e.g., "Mo,We").
 *
 * @param week Bit 0 = Monday, ..., Bit 6 = Sunday.
 * @return QString List of active days.
 */
QString QSNDaysWeekToLine(quint8 week)
{
    QString r;
    for (int i = 0; i < 7; i ++)
        if (week & (static_cast<quint8>(1) << i)) {
            if (!r.isEmpty()) r += ",";
            r += QSNDayWeekAbbreviated(i + 1);
        }
    return r;
}

/**
 * @brief Normalizes identifiers by removing punctuation and camel-casing spaces.
 *
 * E.g., "My Device, v2" → "MyDevice_v2".
 *
 * @param name Input string.
 * @return QString Normalized identifier.
 */
QString QSNNameNormalization(QString name)
{
    QString ret = name;
    for (int i = 0; i < ret.count(); i ++)  {
        if (ret[i] == ',') ret[i] = '_';
        if (ret[i] == '.') ret[i] = '_';
        if (ret[i] == ';') ret[i] = '_';
        if (ret[i] == ':') ret[i] = '_';
        if (ret[i] == ' ') {
            ret.remove(i, 1);
            if (i < ret.count())
                ret[i] = ret[i].toUpper();
        }
    }
    return ret;
}

/**
 * @brief Converts boolean to lowercase "true"/"false" string.
 *
 * @param state Boolean value.
 * @return QString "true" or "false".
 */
QString QSNBoolToText(bool state)
{
    if (state) return QLatin1String("true");
    return QLatin1String("false");
}

/**
 * @brief Parses flexible boolean strings ("yes", "on", "1", etc.) into bool.
 *
 * @param state Input string.
 * @param ok Optional output flag indicating parse success.
 * @return bool Converted value.
 */
bool QSNTextToBool(QString state, bool *ok)
{
    qint8 ret = -1;
    if (state.contains("true", Qt::CaseInsensitive)) ret = 1;
    else if (state == "1") ret = 1;
    else if (state.contains("on", Qt::CaseInsensitive)) ret = 1;
    else if (state.contains("yes", Qt::CaseInsensitive)) ret = 1;
    else if (state.contains("open", Qt::CaseInsensitive)) ret = 1;
    else if (state.contains("allowed", Qt::CaseInsensitive)) ret = 1;
    else if (state.contains("elevated", Qt::CaseInsensitive)) ret = 1;
    else if (state.contains("presents", Qt::CaseInsensitive)) ret = 1;
    else if (state.contains("detected", Qt::CaseInsensitive)) ret = 1;
    else if (state.contains("activated", Qt::CaseInsensitive)) ret = 1;

    if (state.contains("false", Qt::CaseInsensitive)) ret = 0;
    else if (state == "0") ret = 0;
    else if (state.contains("off", Qt::CaseInsensitive)) ret = 0;
    else if (state.contains("no", Qt::CaseInsensitive)) ret = 0;
    else if (state.contains("close", Qt::CaseInsensitive)) ret = 0;
    else if (state.contains("prohibited", Qt::CaseInsensitive)) ret = 0;
    else if (state.contains("omitted", Qt::CaseInsensitive)) ret = 0;
    else if (state.contains("absents", Qt::CaseInsensitive)) ret = 0;
    else if (state.contains("not detected", Qt::CaseInsensitive)) ret = 0;
    else if (state.contains("not activated", Qt::CaseInsensitive)) ret = 0;

    if (ok != Q_NULLPTR) *ok = (ret != -1);
    if (ret == 1) return true;
    return false;
}

/**
 * @brief Converts boolean to localized "yes"/"no".
 *
 * @param state Boolean value.
 * @return QString Localized string.
 */
QString QSNBoolToYesNo(bool state)
{
    if (state) return QObject::tr("yes");
    return QObject::tr("no");
}

/**
 * @brief Decodes percent-encoded strings (e.g., "%D0%90" → "А").
 *
 * Assumes Latin1/Local8Bit encoding after hex decoding.
 *
 * @param text Percent-encoded input.
 * @return QString Decoded string.
 */
QString QSNDecodeText(QString text)
{
    QByteArray ret;
    QString source = text;
    bool bStatus = false;
    int i = 0;
    while (i < source.length()) {
        if (source[i] != '%') {
            ret.append(source[i].toLatin1());
            i ++;
        } else {
            ret.append(static_cast<char>(source.midRef(i + 1, 2).toInt(&bStatus, 16)));
            i += 3;
        }
    }
    return QString::fromLocal8Bit(ret);
}

/**
 * @brief Cleans IPv6 addresses for display (e.g., "::ffff:192.168.1.1" → "192.168.1.1").
 *
 * @param address Raw IP string.
 * @return QString Human-readable form.
 */
QString QSNCleanIP(QString address)
{
    QString a = address;
    if (a.left(7) == QString("::ffff:​").left(7)) a = a.remove(0, 7);
    else if (a.left(3) == QString("::1​").left(3)) {
        a = a.remove(0, 3);
        a = a.prepend("loopback");
    } else if (a.left(6) == QString("fc00::​").left(6)) {
        a = a.remove(0, 6);
        a = a.prepend("Unique Local Unicast");
    } else if (a.left(6) == QString("ff00::​").left(6)) {
        a = a.remove(0, 6);
        a = a.prepend("multicast");
    }
    return a;
}

/**
 * @brief Extracts the first numeric substring from a string (supports signs).
 *
 * @param str Input like "Temp: -25.5°C".
 * @return QString "-25.5".
 */
QString QSNAllocateNumberFromString(QString str)
{
    int start=str.indexOf(QRegExp("[\\+\\-]?[0-9]"));
    int finish=str.lastIndexOf(QRegExp("[\\+\\-]?[0-9]"));
    return str.mid(start, finish-start + 1);
}

/**
 * @brief Calculates uptime duration from a start timestamp.
 *
 * Returns "3 days 14:30" or "02:15" if <1 day.
 *
 * @param begin Start QDateTime.
 * @return QString Formatted duration.
 */
QString QSNUpTime(QDateTime begin)
{
    long long ms = QDateTime::currentDateTime().toMSecsSinceEpoch() - begin.toMSecsSinceEpoch();
    quint32 days = static_cast<quint32>(ms / 86400000);
    ms = ms - (days * 86400000);
    if (days == 0) return QTime::fromMSecsSinceStartOfDay(static_cast<int>(ms)).toString("hh:mm");
    return QString("%1 %2 - %3").arg(days).arg(QObject::tr("days"), QTime::fromMSecsSinceStartOfDay(static_cast<int>(ms)).toString("hh:mm"));
}

/**
 * @brief Generates a safe ASCII-only filename from arbitrary input (including Cyrillic).
 *
 * Transliterates Russian letters and removes unsafe characters.
 *
 * @param str Input string (e.g., device name).
 * @return QString Valid filename.
 */
QString QSNNameToFileName(QString str)
{
    QString fn;
    int i, rU, rL;
    QString validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890-_,.()[]{}<>~!@#$%^&*+=?";
    QString rusUpper = QObject::tr("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЫЭЮЯ");
    QString rusLower = QObject::tr("абвгдеёжзийклмнопрстуфхцчшщыэюя");
    QStringList latUpper, latLower;
    latUpper <<"A"<<"B"<<"V"<<"G"<<"D"<<"E"<<"Jo"<<"Zh"<<"Z"<<"I"<<"J"<<"K"<<"L"<<"M"<<"N"
            <<"O"<<"P"<<"R"<<"S"<<"T"<<"U"<<"F"<<"H"<<"C"<<"Ch"<<"Sh"<<"Sh"<<"I"<<"E"<<"Ju"<<"Ja";
    latLower <<"a"<<"b"<<"v"<<"g"<<"d"<<"e"<<"jo"<<"zh"<<"z"<<"i"<<"j"<<"k"<<"l"<<"m"<<"n"
            <<"o"<<"p"<<"r"<<"s"<<"t"<<"u"<<"f"<<"h"<<"c"<<"ch"<<"sh"<<"sh"<<"i"<<"e"<<"ju"<<"ja";
    for (i=0; i < str.size(); ++i){
        if ( validChars.contains(str[i]) ){
            fn = fn + str[i];
        } else if (str[i] == ' '){  //replace spaces
            fn = fn + "_";
        } else{
            rU = rusUpper.indexOf(str[i]);
            rL = rusLower.indexOf(str[i]);
            if (rU > 0) fn = fn + latUpper[rU];
            else if (rL > 0) fn = fn + latLower[rL];
            else fn = fn + str[i];
        }
    }
    if (fn.isEmpty() ) fn = "file";
    return fn;
}

/**
 * @brief Converts numeric log level to symbolic marker (e.g., 1 → "/!\\" for warning).
 *
 * @param level 0–3.
 * @return QString Symbolic representation.
 */
QString QSNWarningLevelToText(quint8 level)
{
    switch (level) {
    case 0: return QString("(!)");
    case 1: return QString("/!\\");
    case 2: return QString("(i)");
    case 3: return QString("(*)");
    }
    return QString("(?)");
}

/**
 * @brief Converts numeric log level to localized label (e.g., "Warning").
 *
 * @param level Severity code.
 * @return QString Human-readable label.
 */
QString QSNWarningLevelToLabel(quint8 level)
{
    switch (level) {
    case 0: return QObject::tr("Caution");
    case 1: return QObject::tr("Warning");
    case 2: return QObject::tr("Information");
    case 3: return QObject::tr("Note");
    }
    return QObject::tr("Tip");
}

/**
 * @brief Parses symbolic marker back to numeric level.
 *
 * @param text e.g., "/!\\".
 * @return quint8 Level code (4 = unknown).
 */
quint8 QSNWarningLevelFromText(QString text)
{
    if (text.indexOf("(!)") != -1) return 0;
    if (text.indexOf("/!\\") != -1) return 1;
    if (text.indexOf("(i)") != -1) return 2;
    if (text.indexOf("(*)") != -1) return 3;
    return 4;
}

/**
 * @brief Extracts optional note from structured log message "{note}".
 *
 * @param msg Message like "Error occurred {check cable}".
 * @return QString "check cable" or empty.
 */
QString QSNGetWarninMSGNote(QString msg)
{
    QString note = msg;
    int i = note.indexOf("{");
    if (i == -1) note = QString();
    else {
        note.remove(0, i + 1);
        i = note.indexOf("}");
        note.remove(i, note.count() - i);
        if (i == -1) note = QString();
    }
    return note;
}

/**
 * @brief Extracts main body from structured log message (removes markers/notes).
 *
 * @param msg Full message.
 * @return QString Cleaned body.
 */
QString QSNGetWarninMSGBody(QString msg)
{
    QString text = msg;
    int i = text.indexOf("\\");
    if (i >= 2) text.remove(i - 2, 3);
    i = text.indexOf(")");
    if (i >= 2) text.remove(i - 2, 3);
    i = text.indexOf("{");
    if (i == -1) return text;
    text.remove(i, text.count() - i);
    return text;
}

/**
 * @brief Escapes double/single quotes for HTML attribute safety.
 *
 * @param text Input string.
 * @return QString Escaped version.
 */
QString QSNEscapingHTMLCharacters(QString text)
{
    QString ret;
    QChar c;
    for (int i = 0; i < text.count(); i ++) {
        c = text[i];
        if (c == '"') ret.append("&Prime;");
        else if (c == '\'') ret.append("&prime;");
        else ret.append(c);
    }
    return ret;
}

/**
 * @brief Maps Qt/QSerialPort error codes to localized descriptions.
 *
 * @param error Error enum value.
 * @return QString Human-readable error.
 */
QString QSNGetSerialErrorName(int error)
{
    switch(error)
    {
    case 1: return QObject::tr("Device not found error");
    case 2: return QObject::tr("Permission error");
    case 3: return QObject::tr("Open error");
    case 4: return QObject::tr("Parity error");
    case 5: return QObject::tr("Framing error");
    case 6: return QObject::tr("Break condition error");
    case 7: return QObject::tr("Write error");
    case 8: return QObject::tr("Read error");
    case 9: return QObject::tr("Resource error");
    case 10: return QObject::tr("Unsupported operation error");
    case 11: return QObject::tr("Unknown error");
    case 12: return QObject::tr("Timeout error");
    case 13: return QObject::tr("Not open error");

    default:
        return QObject::tr("No error occurred");
    }
}

/**
 * @brief Generates human-readable log line from a QSNContainer.
 *
 * Includes direction (send/receive), role, command, and payload.
 *
 * @param container Message to describe.
 * @param directionIn 1 = received, 0 = sent.
 * @return QString Log-friendly summary.
 */
QString QSNContainerToLogText(QSNContainer container, int directionIn)
{
    QString text;

    if (directionIn == 1) text += QObject::tr("receive") + " ";
    if (directionIn == 0) text += QObject::tr("send") + " ";

    switch (container.role) {
    case QSNContainer::signal:
        text += QObject::tr("signal") + " " + QString::number(container.Signal);
        if (!container.Data.isEmpty())  text +=  " " + QObject::tr(" data") + " " + container.Data.toHex();
        break;
    case  QSNContainer::message:
        text += QObject::tr("message") + " " + QSNMessageTypeToName(container.Command);
        if (!container.Data.isEmpty())  text +=  " " + QObject::tr(" data") + " " + container.Data.toHex();
        break;
    case  QSNContainer::service:
        text += QObject::tr("service");
        switch (container.Command) {
        case 0: text += " CONNECT_CHANGE";
            text += container.Signal?" disconnect":" connect";
            break;
        case 1: text += " ADAPTER_STATE";
            text += container.Signal?" disconnected":" connected";
            break;
        case 2: text += " TIME_SEC_EVENT"; break;
        case 3: text += " MESSAGE"; break;
        case 4: text += " MODULE_INIT"; break;
        case 5: text += " MODULE_STATE_CHANGE";
            switch (container.Signal) {
            case 1: text += " day"; break;
            case 2: text += " night"; break;
            case 3: text += " absence"; break;
            case 8: text += " alert"; break;
            case 9: text += " alarm"; break;
            default: text += " " + QString::number(container.Signal);
            }
            break;
        case 6: text += " ADDR_CHANGE_BEGIN"; break;
        case 7: text += " ADDR_CHANGED"; break;
        case 8: text += " ALERT";
            switch (container.Signal) {
            case 0: text += " cancel"; break;
            default: text += " " + QString::number(container.Signal);
            }
            break;
        case 9: text += " ALARM";
            switch (container.Signal) {
            case 0: text += " cancel"; break;
            case 1: text += " arming"; break;
            case 2: text += " arming_quietly"; break;
            case 3: text += " disarming"; break;
            case 4: text += " alarm"; break;
            default: text += " " + QString::number(container.Signal);
            }
            break;
        case 10: text += " ACCAUNT_CHANGED"; break;
        case 11: text += " MGTT";
            switch (container.Signal) {
            case 0: text += " reception_sn"; break;
            case 1: text += " request_sn"; break;
            case 2: text += " reception_raw"; break;
            case 3: text += " publication_sn"; break;
            case 4: text += " publication_raw"; break;
            default: text += " " + QString::number(container.Signal);
            }

            break;
        case 12: text += " LOG";
            switch (container.Signal) {
            case 0: text += " caution"; break;
            case 1: text += " warning"; break;
            case 2: text += " information"; break;
            case 3: text += " note"; break;
            default: text += " " + QString::number(container.Signal);
            }
            break;
        case 13: text += " DB"; break;
        }

        if (!container.info.isEmpty()) text += " " + QObject::tr("info:") + " " + container.info;
        if (!container.Data.isEmpty()) text += " " + QObject::tr("data:") + " " + container.Data.toHex(' ');

        break;
    case  QSNContainer::registration:
        if (container.Signal == 1) text += QObject::tr("registration");
        else text += QObject::tr("unregistration");
        text += ", " + QObject::tr("sender") + " " + QString::number(container.Sender);
        break;
    case  QSNContainer::authorization:
        text += QObject::tr("authorization");
        break;

    case  QSNContainer::information:
        text += QObject::tr("Information");
        break;
    default:
        break;
    }
    return text;
}

/**
 * @brief Converts numeric message type to localized name (e.g., 2 → "get memory").
 *
 * @param type Command code.
 * @return QString Description.
 */
QString QSNMessageTypeToName(quint8 type)
{
    switch (type)
    {
    case 0: return QObject::tr("get info");
    case 1: return QObject::tr("new device");
    case 2: return QObject::tr("get memory");
    case 3: return QObject::tr("read memory");
    case 4: return QObject::tr("set memory");
    case 5: return QObject::tr("write memory");
    case 6: return QObject::tr("set address");
    case 7: return QObject::tr("answer address");
    case 9: return QObject::tr("answer parametr");

    }
    return QObject::tr("not known");
}

/**
 * @brief Converts a JSON string into raw binary data based on schema rules.
 *
 * Supports structured types like Power, Notification, EMUID. Uses altIDP as fallback device address.
 *
 * @param data Output buffer (will be filled with type-prefixed RAW).
 * @param json Input JSON string (e.g., {"temp":25.5,"idp":100}).
 * @param altIDP Fallback IDP if not specified in JSON.
 * @return bool True if parsing and conversion succeeded.
 */
bool QSNJsonToRAW(QByteArray *data, QString json, quint16 altIDP)
{
    QMap<QString, QString> options;
    if (!QSNParseJSON(json, &options)) return false;
    return QSNJsonOptionsToRAW(data, &options, altIDP);
}


/**
 * @brief Parses a simplified JSON object into a key-value map.
 *
 * Handles strings, numbers, and nested objects (as opaque strings). Does not use QJsonDocument.
 *
 * @param json Input JSON string.
 * @param options Output map of field names to values.
 * @return bool True if parsing completed without structural errors.
 */
bool QSNParseJSON(QString json, QMap<QString, QString> *options)
{
    int index = -1;
    int max = json.count() - 1;
    quint8 state = 0;
    int bracketscount = 0;
    QString whitespace;
    QString value;
    while (index < max) {
        index ++;
        if (json[index] < static_cast<char>(32)) continue;
        if (index > 1 && json[index - 1] == ' ' && json[index] == ' ') continue;

        switch (state) {
        case 0: if (json[index] == '{') state = 1; break;
        case 1:
            if (json.at(index) == '"') {state = 2; whitespace.clear(); continue;}
            if (json[index] == ':') {state = 5; value.clear(); continue;}
            if (whitespace.isEmpty() && json[index] == ' ') continue;
            whitespace.append(json[index]);
            break;
        case 2:
            if (json[index] == '"') {state = 3; continue;}
            whitespace.append(json[index]);
            break;
        case 3:
            if (json[index] != ':') continue;
            state = 5;
            value.clear();
            break;
        case 5: //Чтение значения
            if (value.isEmpty() && json[index] == ' ') continue;
            if (json[index] == '"') {state = 6; value.clear(); continue;}
            if (json[index] == ',') {state = 1; options->insert(whitespace, value); whitespace.clear(); continue;}//переход к следущей паре
            if (json[index] == '}') { state = 0;options->insert(whitespace, value); whitespace.clear(); continue;} //окончание и переход к началу
            value.append(json[index]);
            if (json[index] == '{' || json[index] == '[') {state = 8;bracketscount ++;}
            break;
        case 6: //чтение значения в кавычках
            if (json[index] == '"') { state = 7;continue;}
            value.append(json[index]);
            break;
        case 7:
            if (json[index] != ',' && json[index] != '}') continue;
            state = 1;
            options->insert(whitespace, value);
            whitespace.clear();
            if (json[index] == '}') state = 0;
            break;
        case 8: //чтение значения в скобках
            value.append(json[index]);
            if (json[index] == '}' || json[index] == ']') bracketscount --;
            if (json[index] == '{' || json[index] == '[') bracketscount ++;
            if (!bracketscount) state = 5;
            break;
        }
    }

    return true;
}


/**
 * @brief Converts a parsed JSON option map into raw binary data.
 *
 * Dispatches based on detected type (e.g., "temp", "power") and fills optional fields (note, idp, etc.).
 *
 * @param data Output buffer.
 * @param options Parsed JSON fields.
 * @param altIDP Fallback device address.
 * @return bool True if conversion succeeded.
 */
bool QSNJsonOptionsToRAW(QByteArray *data, QMap<QString, QString> *options, quint16 altIDP)
{
    quint8 type = 0;
    QString value1;
    QString value2;
    QString value3;
    QString key;
    bool ok = false;

    if (options->contains("state") && options->contains("status")) {
        type = 1;
        value1 = options->value("state").toLower();
        value2 = options->value("status").toLower();
        options->remove("state");
        options->remove("status");
        ok = true;
    } else if (options->contains("power") && options->contains("seconds")) {
        type = 19;
        value1 = options->value("power").toLower();
        value2 = options->value("seconds").toLower();
        options->remove("power");
        options->remove("seconds");
        ok = true;
    } else if (options->contains("emUID") && options->contains("family")) {
        type = 33;
        value1 = options->value("emUID").toLower();
        value2 = options->value("family").toLower();
        options->remove("emUID");
        options->remove("family");
        ok = true;
    } else {
        QMap<QString, QString>::iterator i;
        for (i = options->begin(); i != options->end(); i++) {
            key = i.key();
            type = QSNLatianNameToType(key);
            if (type && type < 63) ok = true;
            if (ok) break;
        }
        value1 = options->value(key);
        options->remove(key);
    }

    if (ok) {
        if (options->contains("note")) value2 = options->value("note");
        else if (options->contains("status")) value2 = options->value("status");

        if (options->contains("postfix")) value2 = options->value("postfix");
        else if (options->contains("reason")) value2 = options->value("reason");
        else if (options->contains("devType")) value2 = options->value("devType");
        else if (options->contains("fraction")) value2 = options->value("fraction");

        if (options->contains("idp")) value3 = options->value("idp");
        else if (options->contains("address")) value3 = options->value("address");
        else value3 = QString::number(altIDP);

        if (options->contains("snir")) {
            data->prepend(options->value("snir").toUInt(&ok));
            data->prepend(32);
            options->remove("snir");
        }

        QSNStringToRAW(data, type, 1, value1, value2, value3);
    }
    return ok;
}

/**
 * @brief Converts raw binary data into a JSON-compatible string (without outer braces).
 *
 * Used internally by QSNRAWToJSON().
 *
 * @param Data Input raw data (type-prefixed).
 * @return QString Key-value pairs like "temp":25.5,"idp":100.
 */
void QSNStringToRAW(QByteArray *data, quint8 type, qreal multiplier, QString value1, QString value2, QString value3) //================================================
{
    bool ok = true;
    QSNByteToRAW(data, 0, type);
    switch (type)
    {
    case 1:
        QSNBoolToRAW(data, 1, QSNTextToBool(value1, &ok));
        if (ok) QSNByteToRAW(data, 2, QSNBoolTypeStringToType(QSNTextToBool(value1), value2));
        break;
    case 2: QSNUInt16ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 3: QSNUInt16ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 4: QSNUInt16ToRAW(data, 1, value1.toInt(&ok)); break;
    case 5: QSNByteToRAW(data, 1, value1.toUInt(&ok)); if (ok) QSNByteToRAW(data, 2, value2.toUInt()); break;
    case 6: QSNInt8ToRAW(data, 1, value1.toInt(&ok)); if (ok) QSNByteToRAW(data, 2, value2.toUInt()); break;
    case 7: QSNUInt16ToRAW(data, 1, value1.toUInt(&ok)); if (ok) QSNByteToRAW(data, 3, value2.toUInt()); break;
    case 8: QSNInt16ToRAW(data, 1, value1.toInt(&ok)); if (ok) QSNByteToRAW(data, 3, value2.toUInt()); break;
    case 9: QSNTemperatureToRAW(data, 1, value1.toFloat(&ok)); if (ok) QSNUInt16ToRAW(data, 3, value3.toUInt());break;
    case 10: QSNInt32ToRAW(data, 1, value1.toDouble(&ok) * multiplier); break;
    case 11: QSNInt32ToRAW(data, 1, value1.toLong(&ok) * multiplier); break;
    case 12: QSNInt32ToRAW(data, 1, value1.toLong(&ok) * multiplier); break;
    case 13: {
        QDateTime dt = QDateTime::fromString(value1, Qt::ISODate);
        if (dt.isValid()) QSNDateTimeToRAW(data, 1, dt);
        else ok = false;
    } break;
    case 14: {
        QSNNotification notif;
        notif.notificationType = value1.toUInt(&ok);
        if (ok) notif.idp = value3.toUInt();
        if (ok) notif.notificationSource = value2.toUInt();
        if (ok) QSNNotificationToRAW(data, 1, notif);

    } break;
    case 15: {
        QSNError error;
        error.errorIndex = value1.toUInt(&ok);
        if (ok) error.idp = value3.toUInt(&ok);
        if (ok) error.deviceType = value2.toUInt(&ok);
        if (ok) QSNErrorToRAW(data, 1, error);

    } break;
    case 16: QSNByteToRAW(data, 1, 0); QSNUTF8ToRAW(data, 2, value1); break;
    case 17: {
        QDate dt = QDate::fromString(value1, Qt::ISODate);
        if (dt.isValid()) QSNDateToRAW(data, 1, dt);
    } break;
    case 18: {
        QTime dt = QTime::fromString(value1, Qt::ISODate);
        QSNTimeToRAW(data, 1, dt); break;
    }
    case 19: {
        QSNPower pw;
        pw.power = value1.toLong(&ok);
        if (ok) {
            pw.seconds = value2.toLong(&ok);
            if (!pw.seconds) pw.seconds = 60;
        }
        if (ok) pw.idp = value3.toLong(&ok);
        if (ok) QSNPowerToRAW(data, 1, pw);
    } break;
    case 20: QSNHumidityToRAW(data, 1, value1.toFloat(&ok)); if (ok) QSNUInt16ToRAW(data, 2, value3.toUInt()); break;
    case 21: QSNUInt32ToRAW(data, 1, value1.toLong(&ok));  QSNUInt16ToRAW(data, 5, value3.toUInt()); break;
    case 22: QSNInt8ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 23: QSNInt16ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 24: QSNInt16ToRAW(data, 1, value1.toInt(&ok)); break;
    case 25: QSNUInt16ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 26: QSNUInt32ToRAW(data, 1, value1.toLong(&ok)); if (ok) QSNByteToRAW(data, 5, value2.toUInt()); break;
    case 27: QSNInt32ToRAW(data, 1, value1.toLong(&ok)); if (ok) QSNByteToRAW(data, 5, value2.toUInt()); break;
    case 28: QSNUInt32ToRAW(data, 1, value1.toDouble(&ok) * multiplier); break;
    case 29: QSNInt32ToRAW(data, 1, value1.toDouble(&ok) * multiplier); break;
    case 30: QSNHEXcodeToRAW(data, 1, value1); break;
    case 31:  { QSNNotification notif;
        notif.notificationType = value1.toUInt(&ok);
        notif.idp = value3.toUInt();
        notif.notificationSource = 0;
        QSNNotificationToRAW(data, 1, notif);
    } break;
    case 33: {QSNEMUID uid;
        uid.number = value1.toUInt(&ok);
        uid.family = value2.toUInt();
        QSNEmUIDToRAW(data, 1, uid);
    } break;
    case 34: QSNUInt16ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 35: QSNUInt32ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 36: QSNByteToRAW(data, 1, value1.toUInt(&ok)); break;
    case 37: QSNUInt16ToRAW(data, 1, value1.toUInt(&ok)); if (ok) QSNByteToRAW(data, 3, value2.toUInt(&ok)); break;
    case 38: QSNUInt16ToRAW(data, 1, value1.toUInt(&ok)); break;
    case 39: QSNRMcodeToRAW(data, 1, QByteArray::fromHex(value1.toUtf8())); break;
    }
    if (!ok) data->clear();
}

quint8 QSNBoolTypeStringToType(bool state, QString type)
{
    for (quint8 i = 0; i < 64; i ++)
        if (QSNBoolTypetoLabel(state, i).toLower() == type.toLower()) return i;
    return 0;
}

/**
 * @brief Wraps QSNRAWToJSONString() in curly braces to produce valid JSON object.
 *
 * @param Data Input raw data.
 * @return QString Full JSON object string.
 */
QString QSNRAWToJSON(QByteArray *Data)
{
    return "{" + QSNRAWToJSONString(Data) + "}";
}

QString QSNRAWToJSONString(QByteArray *Data) //================================================
{
    if (Data->count() == 0) Data->append(static_cast<char>(0));
    quint8 t = static_cast<quint8>(Data->at(0));
    QString ret;
    switch (t) {
    case 0: break;
    case 1: ret = "\"" + QSNTypeLatianName(t) + "\":" + QSNBoolToText(QSNRAWtoBool(Data, 1));
        if (Data->size() > 2) ret += ",\"status\":\"" + QSNBoolTypetoLabel(QSNRAWtoBool(Data, 1), static_cast<quint8>(Data->at(2))) + "\"";
        break;
    case 2: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 3: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 4: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoInt16(Data, 1)); break;
    case 5: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoByte(Data, 1));
        if (Data->size() > 2) ret += ",\"postfix\":" + QString::number(QSNRAWtoByte(Data, 2));
        break;
    case 6: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoInt8(Data, 1));
        if (Data->size() > 2) ret += ",\"postfix\":" + QString::number(QSNRAWtoByte(Data, 2));
        break;
    case 7: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1));
        if (Data->size() > 3) ret += ",\"postfix\":" + QString::number(QSNRAWtoByte(Data, 3));
        break;
    case 8: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoInt16(Data, 1));
        if (Data->size() > 3) ret += ",\"postfix\":" + QString::number(QSNRAWtoByte(Data, 3));
        break;
    case 9: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoTemperature(Data, 1),'f', 1);
        if (Data->size() > 3) ret += ",\"idp\":" + QString::number(QSNRAWtoUInt16(Data, 3));
        break;
    case 10: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / 1000); break;
    case 11: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / 1000); break;
    case 12: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / 1000); break;
    case 13: ret = "\"" + QSNTypeLatianName(t) + "\":" + QSNRAWtoDateTime(Data, 1).toString(Qt::ISODate); break;
    case 14: {
        QSNNotification notif = QSNRAWtoNotification(Data, 1);
        ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(notif.notificationType);
        ret += ",\"idp\":" + QString::number(notif.idp);
        if (notif.notificationSource) ret += ",\"reason\":" + QString::number(notif.notificationSource);
    } break;
    case 15: {
        QSNError error = QSNRAWtoError(Data, 1);
        ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(error.errorIndex);
        ret += ",\"address\":" + QString::number(error.idp);
        ret += ",\"devType\":" + QString::number(error.deviceType);
    } break;
    case 16: ret = "\"" + QSNTypeLatianName(t) + "\":" + QSNRAWtoString(Data, QString(), 1); break;
    case 17: ret = "\"" + QSNTypeLatianName(t) + "\":" + QSNRAWtoDate(Data, 1).toString(Qt::ISODate); break;
    case 18: ret = "\"" + QSNTypeLatianName(t) + "\":" + QSNRAWtoTime(Data, 1).toString(Qt::ISODate); break;
    case 19: {
        QSNPower pw = QSNRAWtoPower(Data, 1);
        ret = "\"power\":" + QString::number(pw.power);
        ret += ",\"seconds\":" + QString::number(pw.seconds);
        ret += ",\"idp\":" + QString::number(pw.idp);
    } break;
    case 20:
        ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoHumidity(Data, 1),'f', 1);
        ret += ",\"idp\":" + QString::number(QSNRAWtoUInt16(Data, 2));
        break;
    case 21:
        ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt32(Data, 1));
        ret += ",\"idp\":" + QString::number(QSNRAWtoUInt16(Data, 5));
        break;
    case 22: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoInt8(Data, 1)); break;
    case 23: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoInt16(Data, 1)); break;
    case 24: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 25: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 26: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt32(Data, 1));
        if (Data->size() > 3) ret += ",\"postfix\":" + QString::number(QSNRAWtoByte(Data, 5));
        break;
    case 27: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoInt32(Data, 1));
        if (Data->size() > 3) ret += ",\"postfix\":" + QString::number(QSNRAWtoByte(Data, 5));
        break;
    case 28: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / 1000); break;
    case 29: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / 1000000); break;
    case 30: ret = "\"" + QSNTypeLatianName(t) + "\":" + QSNRAWtoHEXcode(Data, 1); break;
    case 31: {
        QSNNotification notif = QSNRAWtoNotification(Data, 1);
        ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(notif.notificationType);
        ret += ",\"idp\":" + QString::number(notif.idp);
    } break;
    case 32: {
        QSNSNIR snir = QSNRAWtoSNIR(Data, 1);
        ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(snir.address);
//        ret += "," + QString::number(snir.parameter);
//        ret += "," + QString::number(snir.battery);
        ret += "," + QSNRAWToJSONString(&snir.data);
    } break;
    case 33: {
        QSNEMUID uid = QSNRAWtoEmUID(Data, 1);
        ret = "\"emUID\":" + QString::number(uid.number);
        ret += ",\"family\":" + QString::number(uid.family);
    } break;
    case 34: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 35: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt32(Data, 1)); break;
    case 36: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoByte(Data, 1)); break;
    case 37: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1));
        if (Data->size() > 3) ret += ",\"fraction\":" + QString::number(QSNRAWtoByte(Data, 3));
        break;
    case 38: ret = "\"" + QSNTypeLatianName(t) + "\":" + QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 39: ret = "\"" + QSNTypeLatianName(t) + "\":\"" + QSNRAWtoRMcode(Data, 1).toHex() + "\"";break;
    }
    return ret;
}


/**
 * @brief Extracts only the primary value from raw data as a plain string (no keys or units).
 *
 * Used in MQTT payload extraction or CSV exports.
 *
 * @param Data Input raw data.
 * @param multiplier Scaling factor (e.g., 1000 for kW → W).
 * @return QString Numeric or hex string representation.
 */
QString QSNRAWToValueString(QByteArray *Data, qreal multiplier) //================================================
{
    if (Data->count() == 0) Data->append(static_cast<char>(0));
    quint8 t = static_cast<quint8>(Data->at(0));
    QString ret;
    switch (t) {
    case 0: break;
    case 1: ret = QSNBoolToText(QSNRAWtoBool(Data, 1)); break;
    case 2: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 3: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 4: ret = QString::number(QSNRAWtoInt16(Data, 1)); break;
    case 5: ret = QString::number(QSNRAWtoByte(Data, 1)); break;
    case 6: ret = QString::number(QSNRAWtoInt8(Data, 1)); break;
    case 7: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 8: ret = QString::number(QSNRAWtoInt16(Data, 1)); break;
    case 9: ret = QString::number(QSNRAWtoTemperature(Data, 1),'f', 1); break;
    case 10: ret = QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / multiplier, 'f', 2); break;
    case 11: ret = QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / multiplier, 'f', 2); break;
    case 12: ret = QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / multiplier, 'f', 2); break;
    case 13: ret = QSNRAWtoDateTime(Data, 1).toString(Qt::ISODate); break;
    case 14: {
        QSNNotification notif = QSNRAWtoNotification(Data, 1);
        ret = QString::number(notif.notificationType);
    } break;
    case 15: {
        QSNError error = QSNRAWtoError(Data, 1);
        ret = QString::number(error.errorIndex);
    } break;
    case 16: ret = QSNRAWtoString(Data, QString(), 1); break;
    case 17: ret = QSNRAWtoDate(Data, 1).toString(Qt::ISODate); break;
    case 18: ret = QSNRAWtoTime(Data, 1).toString(Qt::ISODate); break;
    case 19: {
        QSNPower pw = QSNRAWtoPower(Data, 1);
        if (pw.seconds > 0) ret = QString::number(pw.power / (pw.seconds / 60));
        else ret = QString::number(pw.power);
    } break;
    case 20: ret = QString::number(QSNRAWtoHumidity(Data, 1)); break;
    case 21: ret = QString::number(QSNRAWtoUInt32(Data, 1)); break;
    case 22: ret = QString::number(QSNRAWtoInt8(Data, 1)); break;
    case 23: ret = QString::number(QSNRAWtoInt16(Data, 1)); break;
    case 24: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 25: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 26: ret = QString::number(QSNRAWtoUInt32(Data, 1)); break;
    case 27: ret = QString::number(QSNRAWtoInt32(Data, 1)); break;
    case 28: ret = QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / multiplier, 'f', 2); break;
    case 29: ret = QString::number(static_cast<double>(QSNRAWtoInt32(Data, 1)) / multiplier, 'f', 2); break;
    case 30: ret = QSNRAWtoHEXcode(Data, 1); break;
    case 31: {
        QSNNotification notif = QSNRAWtoNotification(Data, 1);
        ret = QString::number(notif.notificationType);
    } break;
    case 32: ret = Data->toHex(); break;
    case 33: {
        QSNEMUID uid = QSNRAWtoEmUID(Data, 1);
        ret = QString::number(uid.number);
        ret += "," + QString::number(uid.family);
    } break;
    case 34: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 35: ret = QString::number(QSNRAWtoUInt32(Data, 1)); break;
    case 36: ret = QString::number(QSNRAWtoByte(Data, 1)); break;
    case 37: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 38: ret = QString::number(QSNRAWtoUInt16(Data, 1)); break;
    case 39: ret = QSNRAWtoRMcode(Data, 1).toHex(); break;
    }
    return ret;
}

/**
 * @brief Checks if an MQTT topic matches a subscription filter (supports '+' and '#').
 *
 * Implements standard MQTT topic filtering rules.
 *
 * @param topic Full topic (e.g., "home/living/temp").
 * @param filter Subscription pattern (e.g., "home/+/temp" or "home/#").
 * @return bool True if topic matches filter.
 */
bool QSNMQTTTopicFilterCompare(QString topic, QString filter)
{
    QStringList tp = QSNMQTTTopicFromString(topic);
    QStringList fl = QSNMQTTTopicFromString(filter);
    int count = tp.count();
    if (fl.count() < count) count = fl.count();
    for (int i = 0; i < count; i ++) {
        if (fl.at(i) == '#') return true;
        if (fl.at(i) == '+') continue;
        if (fl.at(i) != tp.at(i)) return false;
    }
    if (tp.count() != fl.count()) return false;
    return true;
}

/**
 * @brief Splits an MQTT topic string into hierarchical components.
 *
 * Splits by '/' and handles trailing parts correctly.
 *
 * @param topic Topic string.
 * @return QStringList List of topic levels.
 */
QStringList QSNMQTTTopicFromString(QString topic)
{
    QStringList t;
    int index = -1;
    QString tsrc = topic;

    do {
        index = tsrc.indexOf('/');
        if (index > 0) t.append(tsrc.left(index));
        tsrc.remove(0, index + 1);
    } while (index != -1);
    if (!tsrc.isEmpty()) t.append(tsrc);
    return t;
}

/**
 * @brief Extracts a value from an MQTT message (plain or JSON) and converts it to raw data.
 *
 * If jsonItem is empty, treats msg as plain value; otherwise parses JSON and extracts named field.
 *
 * @param msg MQTT payload (string).
 * @param valueType Expected sensor/data type.
 * @param jsonItem Field name to extract from JSON (empty = use whole payload).
 * @param multiplier Scaling factor (e.g., 1000 for power).
 * @return QByteArray Serialized raw data.
 */
QByteArray QSNMQTTTopicMsgToRAW(QString msg, quint8 valueType, QString jsonItem, qreal multiplier) {
    QByteArray data;
    if (jsonItem.isEmpty()) {
        QSNStringToRAW(&data, valueType, multiplier, msg, QString());
    } else {
        QMap<QString, QString> options;
        if(!QSNParseJSON(msg, &options)) return data;
        if (!options.contains(jsonItem)) return data;
        QSNStringToRAW(&data, valueType, multiplier, options.value(jsonItem), QString(), QString());
    }
    return data;
}

/**
 * @brief Computes integer power: x^n (for non-negative n).
 *
 * Optimized recursive implementation (O(log n)).
 *
 * @param x Base.
 * @param n Exponent (≥0).
 * @return long Result (may overflow for large inputs).
 */
long QSNPOW(long x, unsigned int n)
{
    if (n==0)
        return 1;
    else if (n==1)
        return x;
    else if (n % 2 == 0 )
        return QSNPOW( x * x, n/2);
    else
        return QSNPOW( x * x, n /2)*x;
}

/**
 * @brief Converts a 32-bit unsigned integer to uppercase zero-padded 8-digit hex string.
 *
 * @param vol Input value.
 * @return QString e.g., "00A1B2C3".
 */
QString QSNUInt32ToHEXString(quint32 vol)
{
    QString ret = QString().number(vol, 16).toUpper();
    ret = ret.prepend(QString().fill('0', 8 - ret.length()));
    return ret;
}

/**
 * @brief Parses a hex string into a 32-bit unsigned integer.
 *
 * @param str Hex string (case-insensitive, no prefix).
 * @return quint32 Parsed value (0 on error).
 */
quint32 QSNHEXStringToUInt32(QString str)
{
    bool res = false;
    return str.toUInt(&res, 16);
}

/**
 * @brief Converts SNIR battery level code (0–3) to percentage estimate.
 *
 * @param raw Battery code from SNIR packet.
 * @return quint8 Estimated charge: 0%, 30%, 70%, or 100%.
 */
quint8 QSNSNIRBatToPercent(quint8 raw)
{
    quint8 level = raw & 0x07;

    switch (level) {
        case 0: return 5;   // 000 — критически разряжена (≤5%)
        case 1: return 15;  // 001 — очень низкий заряд (>5%, ≤15%)
        case 2: return 30;  // 010 — низкий заряд (>15%, ≤30%)
        case 3: return 50;  // 011 — умеренный заряд (>30%, ≤50%)
        case 4: return 70;  // 100 — нормальный заряд (>50%, ≤70%)
        case 5: return 85;  // 101 — хороший заряд (>70%, ≤85%)
        case 6: return 95;  // 110 — почти полный заряд (>85%, ≤95%)
        case 7: return 100; // 111 — полный / свежий заряд (>95%)
        default: return 0;  // На случай непредвиденных ошибок
    }
}

/**
 * @brief Returns a null/invalid QDateTime used as sentinel value (01.01.2000 00:00:00).
 *
 * @return QDateTime Constant representing "no time set".
 */
QDateTime QSNNULLDateTime()
{
    return QDateTime(QDate(2000,1,1), QTime(0,0,0));
}

QString QSNDateTimeToString(QDateTime dtime)
{
    if (dtime == QSNNULLDateTime() || !dtime.isValid()) return "----";
    return dtime.toString("dd.MM.yy hh:mm");
}

/**
 * @brief Formats time difference between now and given timestamp.
 *
 * Shows relative duration (<1 month) or absolute date otherwise.
 *
 * @param dtime Target timestamp.
 * @return QString e.g., "02:15:30", "-1d 04:20", or "15.03.25 10:30".
 */
QString QSNToTimeString(QDateTime dtime)
{
    if (dtime == QSNNULLDateTime() || !dtime.isValid()) return "----";
    QDateTime dt = QDateTime::currentDateTime();
    bool old = false;
    if (dt > dtime) {
       old = true;
       dt = QDateTime::fromMSecsSinceEpoch(dt.toMSecsSinceEpoch() - dtime.toMSecsSinceEpoch()).toUTC();
    } else {
       dt = QDateTime::fromMSecsSinceEpoch(dtime.toMSecsSinceEpoch() - dt.toMSecsSinceEpoch()).toUTC();
    }

    qint64 sec = dt.toSecsSinceEpoch();
    if (sec < 86400) return (old?"-":"") + dt.toString("hh:mm:ss");
    if (sec < 2419200) return  (old?"-":"") + dt.toString("dd hh:mm");
    return dtime.toString("dd.MM.yy hh:mm");
}
