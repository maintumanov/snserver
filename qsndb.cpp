#include "qsndb.h"

QsnDB::QsnDB(QObject *parent) : QObject(parent)
{

}

void QsnDB::writeRAWtoRRD(QString RRDname, QByteArray *RAW)
{
    if (RAW->count() < 2) return;
    quint8 typeIndex = static_cast<quint8>(RAW->at(0));
    int dataSize = QSNTypeDataSize(typeIndex);
    if (dataSize == 0) return;
    QFile file;
    if (!initFileName(&file, RRDname, typeIndex)) return;


    quint16 currentYear = static_cast<quint16>(QDate::currentDate().year());
    qint64 currentPosition = 4 + (QDate(currentYear, 1, 1).daysTo(QDate::currentDate())* 24 * 60
                                  + QTime::currentTime().hour() * 60
                                  + QTime::currentTime().minute()) * (dataSize * 2 + 2);
    QByteArray field;
    if(!file.open(QIODevice::ReadWrite)) return;
    file.seek(currentPosition);
    field = file.read(dataSize * 2 + 2);
    if (QSNRAWtoUInt16(&field, 0) != currentYear) {
        if (QSNRAWtoUInt16(&field, 0) == currentYear - 1)
            for(int i = 0; i < dataSize; i ++) field[i + dataSize + 2] = field[i + 2];
        if (QSNRAWtoUInt16(&field, 0) < currentYear - 1)
            for(int i = 0; i < dataSize; i ++) field[i + dataSize + 2] = 0;
        QSNUInt16ToRAW(&field, 0, currentYear);
        for(int i = 0; i < dataSize; i ++) field[i + 2] = (*RAW)[i + 1];
        file.seek(currentPosition);
        file.write(field);
    }
    file.close();
}

bool QsnDB::requestBegin(QFile *file, quint8 typeIndex, QString RRDname)
{
    file->setFileName(QSNHomeSubPath("server", "db").absoluteFilePath(QString("%1_%2.rrd").arg(RRDname, QSNTypeLatianName(typeIndex))));
    QByteArray sig("RRD");
    sig.append(static_cast<char>(typeIndex));
    QByteArray block;
    if (file->exists()) {
        if(file->open(QIODevice::ReadOnly)) {
            file->seek(0);
            block = file->read(4);
            file->close();
            if (block == sig) return true;
        }
    }
    return false;
}

qreal QsnDB::requestGetField(QFile *file, quint8 typeIndex, QDateTime rtime, qreal prevVol)
{
    if (rtime.date() > QDate::currentDate()) return prevVol;
    QByteArray block;
    quint16 rYear = static_cast<quint16>(rtime.date().year());
    quint16 readYear;
    int dataSize = QSNTypeDataSize(typeIndex);
    qint64 currentPosition = 4 + (QDate(rtime.date().year(), 1, 1).daysTo(rtime.date())* 24 * 60
                                  + rtime.time().hour() * 60
                                  + rtime.time().minute()) * (dataSize * 2 + 2);
    file->seek(currentPosition);
    block = file->read(dataSize * 2 + 2);
    readYear = QSNRAWtoUInt16(&block, 0);
    if (readYear == rYear) {
        return QSNRAWtoReal(&block, typeIndex, 2);
    }
    if (readYear - 1 == rYear) {
        return QSNRAWtoReal(&block, typeIndex, 2 + dataSize);
    }
    return prevVol;
}

void QsnDB::requestEnd(QFile *file)
{
    file->close();
}
// ===================== ybd ==============================
void QsnDB::writeRAWtoYBD(QString name, QByteArray *RAW, QDateTime dt, QString label, QObject *sender)
{
    if (RAW->count() == 0) RAW->append(static_cast<char>(0));
    switch (RAW->at(0)){
    case 0: writeBNToYBD(name, true, 0, dt, label, 0); break;
    case 1: writeBNToYBD(name, QSNRAWtoBool(RAW, 1), QSNRAWtoByte(RAW, 2), dt, label, 1); break;
    case 2: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 2); break;
    case 3: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 3); break;
    case 4: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 4); break;
    case 5: writeUInt32ToYBD(name, QSNRAWtoByte(RAW, 1), dt, label, 5); break;
    case 6: writeInt32ToYBD(name, QSNRAWtoInt8(RAW, 1), dt, label, 6); break;
    case 7: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 7); break;
    case 8: writeInt32ToYBD(name, QSNRAWtoInt16(RAW, 1), dt, label, 8); break;
    case 9: writeTemperatureToYBD(name, QSNRAWtoTemperature(RAW, 1), dt, label); break;
    case 10: writeInt32ToYBD(name, QSNRAWtoInt32(RAW, 1), dt, label, 10); break;
    case 11: writeInt32ToYBD(name, QSNRAWtoInt32(RAW, 1), dt, label, 11); break;
    case 12: writeInt32ToYBD(name, QSNRAWtoInt32(RAW, 1), dt, label, 12); break;
    case 13: break;
    case 14: break;
    case 15: break;
    case 16: break;
    case 17: break;
    case 18: break;
    case 19: writeEnergyToYBD(name, QSNRAWtoPower(RAW, 1), dt, label); break;
    case 20: writeUInt32ToYBD(name, QSNRAWtoByte(RAW, 1), dt, label, 20); break;
    case 21: writeUInt32ToYBD(name, QSNRAWtoUInt32(RAW, 1), dt, label, 21);  break;
    case 22: writeUInt32ToYBD(name, QSNRAWtoByte(RAW, 1), dt, label, 22); break;
    case 23: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 23); break;
    case 24: writeInt32ToYBD(name, QSNRAWtoInt16(RAW, 1), dt, label, 24); break;
    case 25: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 25); break;
    case 26: writeUInt32ToYBD(name, QSNRAWtoUInt32(RAW, 1), dt, label, 26); break;
    case 27: writeInt32ToYBD(name, QSNRAWtoInt32(RAW, 1), dt, label, 27); break;
    case 28: writeUInt32ToYBD(name, QSNRAWtoUInt32(RAW, 1), dt, label, 28); break;
    case 29: writeUInt32ToYBD(name, QSNRAWtoUInt32(RAW, 1), dt, label, 29); break;
    case 30: writeUInt32ToYBD(name, QSNRAWtoUInt32(RAW, 1), dt, label, 30); break;
    case 31: break;
    case 32: break;
    case 33: break;
    case 34: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 34); break;
    case 35: writeUInt32ToYBD(name, QSNRAWtoUInt32(RAW, 1), dt, label, 35); break;
    case 36: break;
    case 37: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 37); break;
    case 38: writeUInt32ToYBD(name, QSNRAWtoUInt16(RAW, 1), dt, label, 38); break;
    case 39: break;
    }

    registryPublicData(name, *RAW, sender);
}

void QsnDB::requestDataFromYBD(QsnDB::dbSeries *data)
{
    switch (data->type){
    case 0: requestBNFromYBD(data, 0); break;
    case 1: requestBNFromYBD(data, 1); break;
    case 2: requestUInt32FromYBD(data, 2); break;
    case 3: requestUInt32FromYBD(data, 3); break;
    case 4: requestUInt32FromYBD(data, 4); break;
    case 5: requestUInt32FromYBD(data, 5); break;
    case 6: requestInt32FromYBD(data, 6); break;
    case 7: requestUInt32FromYBD(data, 7); break;
    case 8: requestInt32FromYBD(data, 8); break;
    case 9: requestTemperatureFromYBD(data); break;
    case 10: requestInt32FromYBD(data, 10); break;
    case 11: requestInt32FromYBD(data, 11); break;
    case 12: requestInt32FromYBD(data, 12); break;
    case 13: requestEmptyYBD(data); break;
    case 14: requestEmptyYBD(data); break;
    case 15: requestEmptyYBD(data); break;
    case 16: requestEmptyYBD(data); break;
    case 17: requestEmptyYBD(data); break;
    case 18: requestEmptyYBD(data); break;
    case 19: requestEnergyFromYBD(data); break;
    case 20: requestUInt32FromYBD(data, 20); break;
    case 21: requestUInt32FromYBD(data, 21); break;
    case 22: requestUInt32FromYBD(data, 22); break;
    case 23: requestUInt32FromYBD(data, 23); break;
    case 24: requestInt32FromYBD(data, 24); break;
    case 25: requestUInt32FromYBD(data, 25); break;
    case 26: requestUInt32FromYBD(data, 26); break;
    case 27: requestInt32FromYBD(data, 27);  break;
    case 28: requestUInt32FromYBD(data, 28); break;
    case 29: requestUInt32FromYBD(data, 29); break;
    case 30: requestUInt32FromYBD(data, 30); break;
    case 31: requestEmptyYBD(data); break;
    case 32:requestEmptyYBD(data); break;
    case 33:requestEmptyYBD(data); break;
    case 34:requestUInt32FromYBD(data, 34); break;
    case 35:requestUInt32FromYBD(data, 35); break;
    case 36:requestEmptyYBD(data); break;
    case 37:requestUInt32FromYBD(data, 37); break;
    case 38:requestUInt32FromYBD(data, 38); break;
    case 39:requestEmptyYBD(data); break;
    default:
        requestEmptyYBD(data);
    }
}

//================= 4 ybd int16 (bool and null)  ===================================
void QsnDB::writeBNToYBD(QString name, bool state, quint8 noteIndex, QDateTime dt, QString label, quint8 type)
{
    ybdrect v;
    v.currentYear = dt.date().year();
    v.currentDate = dt;
    v.name = name;
    v.tablePos = 0;
    v.label = label;
    v.typeIndex = type;
    v.dataSize = 2;
    v.tableDataSize = 4;
    qint16 readVol;
    qint16 vol;
    qint32 readTableVol;
    QByteArray pa;
    bool itemUsed = false;
    char isUsed;
    if (!initYBD(&v)) return;
    setPosYBD(&v);
    if(!v.file.open(QIODevice::ReadWrite)) return;
    v.file.seek(v.currentPos);
    vol = (state?1:0);
    QSNInt16ToRAW(&pa, 0, (static_cast<qint16>(noteIndex) << 8) + vol);
    isUsed = 1;
    v.file.write(&isUsed, 1);
    v.file.write(pa);

    //hour table
    setTableItemPos(&v);
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    pa = v.file.read(v.tableDataSize + v.dataSize * 2);
    v.file.seek(v.currentPos);

    readTableVol = QSNRAWtoInt32(&pa, 0);
    readTableVol += vol;
    if (readTableVol > 1) readTableVol = 1;
    QSNInt32ToRAW(&pa, 0, readTableVol);
    if (!isUsed) {
        QSNInt16ToRAW(&pa, v.tableDataSize, vol);
        QSNInt16ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    } else {
        readVol = QSNRAWtoInt16(&pa, v.tableDataSize);
        if (readVol > vol) QSNInt16ToRAW(&pa, v.tableDataSize, vol);
        readVol = QSNRAWtoInt16(&pa, v.tableDataSize + v.dataSize);
        if (readVol < vol) QSNInt16ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    }

    //if (!itemUsed) isUsed ++;
    if (!itemUsed) isUsed = 1;
    v.file.write(&isUsed, 1);
    v.file.write(pa);
    renameYBD(&v.file, label);
    v.file.close();
}

void QsnDB::requestBNFromYBD(QsnDB::dbSeries *data, quint8 type)
{
    ybdrect v;
    v.secQuant = data->begin.secsTo(data->end) / data->seriesCount;
    v.currentDate = data->begin;
    v.currentYear = 0;
    v.dataSize = 2;
    v.tableDataSize = 4;
    v.name = data->name;
    v.typeIndex = type;
    setTablePos(&v);
    data->precision = 0;
    data->aindex = 0;
    QsnDB::dbItem item;
    qint16 cmax = -32768;
    qint16 cmin = 32767;
    qint32 cvol = 0;
    qint16 cmax_old = -32768;
    qint16 cmin_old = 32767;
    qint32 cvol_old = 0;
    qint16 rvol;
    int ccount = 0;
    char isUsed;
    QDateTime timeNextBlock = v.currentDate.addSecs(v.secQuant);
    QByteArray raw;

    while (v.currentDate <= data->end) {
        if (v.currentYear < v.currentDate.date().year()) openYBD(&v);
        isUsed = 0;
        if (v.currentDate >= timeNextBlock) {
            if (data->withoutBreaks && !ccount) {
                cmax = cmax_old;
                cmin = cmin_old;
                cvol = cvol_old;
            }
            if (cvol == 0) {
                cmin = 0;
                cmax = 0;
            }
            if (ccount) cvol = cvol / ccount;
            item.vol = cvol;
            item.min = cmin;
            item.max = cmax;
            item.used = ccount;
            item.dateTime = v.currentDate;
            data->series.append(item);
            cmax_old = cmax;
            cmin_old = cmin;
            cvol_old = cvol;
            ccount = 0;
            cvol = 0;
            cmax = -32768;
            cmin = 32767;

            timeNextBlock = timeNextBlock.addSecs(v.secQuant);
            if (timeNextBlock > data->end) timeNextBlock = data->end;
        }

        if (v.currentDate.time().minute() == 0)
            if (v.currentDate.addSecs(3600) < timeNextBlock) {
                if (v.file.isOpen()) {
                    setTableItemPos(&v);
                    v.file.seek(v.currentPos);
                    v.file.read(&isUsed, 1);
                    if (isUsed) {
                        raw = v.file.read(v.tableDataSize + v.dataSize * 2);
                        cvol += QSNRAWtoInt32(&raw, 0);
                        rvol = QSNRAWtoInt16(&raw, v.tableDataSize);
                        if (cmin > rvol) cmin = rvol;
                        rvol = QSNRAWtoInt16(&raw, v.tableDataSize + v.dataSize);
                        if (cmax < rvol) cmax = rvol;
                    }
                }
                if (isUsed) ccount += isUsed;
                v.currentDate = v.currentDate.addSecs(3600);
                continue;
            }

        if (v.file.isOpen()) {
            if (v.currentPos >= v.tablePos) {
                setPosYBD(&v);
                v.file.seek(v.currentPos);
            }
            v.file.read(&isUsed, 1);
            raw = v.file.read(v.dataSize);
            if (isUsed) {
                rvol = QSNRAWtoInt16(&raw, 0);
                data->aindex = rvol & 0xff;
                rvol = rvol & 0x0001;
                cvol += rvol;
                if (cmin > rvol) cmin = rvol;
                if (cmax < rvol) cmax = rvol;
            }
        }

        if (isUsed) ccount ++;
        v.currentDate = v.currentDate.addSecs(60);
    }
    data->label = v.label;
    if (v.file.isOpen()) v.file.close();
}

//================= 9 == Temperature ===================================
void QsnDB::writeTemperatureToYBD(QString name, qreal temp, QDateTime dt, QString label)
{
    if (temp < -99) return;
    ybdrect v;
    v.currentYear = dt.date().year();
    v.currentDate = dt;
    v.name = name;
    v.tablePos = 0;
    v.label = label;
    v.typeIndex = 9;
    v.dataSize = 2;
    v.tableDataSize = 4;
    qint16 readVol;
    qint16 oldVol = 0;
    qint32 readTableVol;
    QByteArray pa;
    qint16 vol = static_cast<qint16>(temp * 100);
    bool itemUsed = false;
    char isUsed;
    if (!initYBD(&v)) return;
    setPosYBD(&v);
    if(!v.file.open(QIODevice::ReadWrite)) return;
    // minits table
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    itemUsed = isUsed;
    pa = v.file.read(v.dataSize);
    v.file.seek(v.currentPos);
    oldVol = QSNRAWtoInt16(&pa, 0);
    QSNInt16ToRAW(&pa, 0, vol);
    isUsed = 1;
    v.file.write(&isUsed, 1);
    v.file.write(pa);

    //hour table
    setTableItemPos(&v);
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    pa = v.file.read(v.tableDataSize + v.dataSize * 2);
    v.file.seek(v.currentPos);

    readTableVol = QSNRAWtoInt32(&pa, 0);
    readTableVol += vol;
    readTableVol -= oldVol;
    QSNInt32ToRAW(&pa, 0, readTableVol);
    if (!isUsed) {
        QSNInt16ToRAW(&pa, v.tableDataSize, vol);
        QSNInt16ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    } else {
        readVol = QSNRAWtoInt16(&pa, v.tableDataSize);
        if (readVol > vol) QSNInt16ToRAW(&pa, v.tableDataSize, vol);
        readVol = QSNRAWtoInt16(&pa, v.tableDataSize + v.dataSize);
        if (readVol < vol) QSNInt16ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    }

    if (!itemUsed) isUsed ++;
    v.file.write(&isUsed, 1);
    v.file.write(pa);
    renameYBD(&v.file, label);
    v.file.close();
}

void QsnDB::requestTemperatureFromYBD(dbSeries *data)
{
    ybdrect v;
    v.secQuant = data->begin.secsTo(data->end) / data->seriesCount;
    v.currentDate = data->begin;
    v.currentYear = 0;
    v.dataSize = 2;
    v.tableDataSize = 4;
    v.name = data->name;
    v.typeIndex = 9;
    setTablePos(&v);
    data->precision = 2;

    QsnDB::dbItem item;
    qint16 cmax = -10000;
    qint16 cmin = 15000;
    qint32 cvol = 0;
    qint16 cmax_old = -10000;
    qint16 cmin_old = 15000;
    qint32 cvol_old = 0;
    qint16 rvol;
    data->max = cmax;
    data->min = 15000;
    int ccount = 0;
    char isUsed;
    QDateTime timeNextBlock = v.currentDate.addSecs(v.secQuant);
    QByteArray raw;

    while (v.currentDate <= data->end) {
        if (v.currentYear < v.currentDate.date().year()) openYBD(&v);
        isUsed = 0;
        if (v.currentDate >= timeNextBlock) {
            if (!ccount) {
                cmax = cmax_old;
                cmin = cmin_old;
                cvol = cvol_old;
            }
            if (cvol == 0) {
                cmin = 0;
                cmax = 0;
            }
            if (ccount) cvol = cvol / ccount;
            item.vol = static_cast<qreal>(cvol) / 100;
            item.min = static_cast<qreal>(cmin) / 100;
            item.max = static_cast<qreal>(cmax) / 100;
            if (data->max < item.max) data->max = item.max;
            if (data->min < item.min) data->min = item.min;
            item.dateTime = v.currentDate;
            item.used = ccount;
            data->series.append(item);
            cmax_old = cmax;
            cmin_old = cmin;
            cvol_old = cvol;
            ccount = 0;
            cvol = 0;
            cmax = -10000;
            cmin = 15000;

            timeNextBlock = timeNextBlock.addSecs(v.secQuant);
            if (timeNextBlock > data->end) timeNextBlock = data->end;
        }

        if (v.currentDate.time().minute() == 0)
            if (v.currentDate.addSecs(3600) < timeNextBlock) {
                if (v.file.isOpen()) {
                    setTableItemPos(&v);
                    v.file.seek(v.currentPos);
                    v.file.read(&isUsed, 1);
                    if (isUsed) {
                        raw = v.file.read(v.tableDataSize + v.dataSize * 2);
                        cvol += QSNRAWtoInt32(&raw, 0);
                        rvol = QSNRAWtoInt16(&raw, v.tableDataSize);
                        if (cmin > rvol) cmin = rvol;
                        rvol = QSNRAWtoInt16(&raw, v.tableDataSize + v.dataSize);
                        if (cmax < rvol) cmax = rvol;
                    }
                }
                if (isUsed) ccount += isUsed;
                v.currentDate = v.currentDate.addSecs(3600);
                continue;
            }

        if (v.file.isOpen()) {
            if (v.currentPos >= v.tablePos) {
                setPosYBD(&v);
                v.file.seek(v.currentPos);
            }
            v.file.read(&isUsed, 1);
            raw = v.file.read(v.dataSize);
            if (isUsed) {
                rvol = QSNRAWtoInt16(&raw, 0);
                cvol += rvol;
                if (cmin > rvol) cmin = rvol;
                if (cmax < rvol) cmax = rvol;
            }
        }
        if (isUsed) ccount ++;
        v.currentDate = v.currentDate.addSecs(60);
    }
    data->label = v.label;
    if (v.file.isOpen()) v.file.close();
}

//=================19== Energy ===================================
void QsnDB::writeEnergyToYBD(QString name, QSNPower power, QDateTime dt, QString label) {
    QDateTime end = dt;
    QDateTime begin = dt.addSecs(- power.seconds);
    QSNPower pw = power;
    int dSec;
    while (begin < end) {
        if (begin.time().minute() == end.time().minute()) dSec = end.time().second() - begin.time().second();
        else dSec = 60 - begin.time().second();
        pw.seconds = static_cast<quint16>(dSec);
        writeEnergyToYBDmin(name, pw, begin, label);
        begin = begin.addSecs(dSec);
    }
}

void QsnDB::writeEnergyToYBDmin(QString name, QSNPower power, QDateTime dt, QString label) {
    ybdrect v;
    v.currentYear = dt.date().year();
    v.currentDate = dt;
    v.name = name;
    v.tablePos = 0;
    v.label = label;
    v.typeIndex = 19;
    v.dataSize = 8;
    v.tableDataSize = 8;
    qint64 readVol;
    qint64 oldVol = 0;
    qint64 readTableVol;
    QByteArray pa;
    qint64 vol = power.power * power.seconds;
    char isUsed;
    if (!initYBD(&v)) return;
    setPosYBD(&v);
    if(!v.file.open(QIODevice::ReadWrite)) return;
    // minits table
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    pa = v.file.read(v.dataSize);
    v.file.seek(v.currentPos);
    oldVol = QSNRAWtoInt64(&pa, 0);
    QSNInt64ToRAW(&pa, 0, oldVol + vol);
    isUsed = 1;
    v.file.write(&isUsed, 1);
    v.file.write(pa);

    //hour table
    setTableItemPos(&v);
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    pa = v.file.read(v.tableDataSize + v.dataSize * 2);
    v.file.seek(v.currentPos);

    readTableVol = QSNRAWtoInt64(&pa, 0);
    readTableVol += vol;
    QSNInt64ToRAW(&pa, 0, readTableVol);
    if (!isUsed) {
        QSNInt64ToRAW(&pa, v.tableDataSize, vol);
        QSNInt64ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    } else {
        readVol = QSNRAWtoInt64(&pa, v.tableDataSize);
        if (readVol > vol) QSNInt64ToRAW(&pa, v.tableDataSize, vol);
        readVol = QSNRAWtoInt64(&pa, v.tableDataSize + v.dataSize);
        if (readVol < vol) QSNInt64ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    }

    isUsed = 1;
    v.file.write(&isUsed, 1);
    v.file.write(pa);
    renameYBD(&v.file, label);
    v.file.close();
}

void QsnDB::requestEnergyFromYBD(dbSeries *data)
{
    ybdrect v;
    v.secQuant = data->begin.secsTo(data->end) / data->seriesCount;
    v.currentDate = data->begin;
    v.currentYear = 0;
    v.dataSize = 8;
    v.tableDataSize = 8;
    v.name = data->name;
    v.typeIndex = 19;
    setTablePos(&v);
    data->precision = 3;


    QsnDB::dbItem item;
    qint64 cmax = Q_INT64_C(-932838457459459);
    qint64 cmin = Q_INT64_C(932838457459459);
    qint64 cvol = 0;
    qint64 rvol;
    data->max = cmax;
    data->min = cmin;
    int ccount = 0;
    char isUsed;
    QDateTime timeNextBlock = v.currentDate.addSecs(v.secQuant);
    QByteArray raw;

    while (v.currentDate <= data->end) {
        if (v.currentYear < v.currentDate.date().year()) openYBD(&v);
        isUsed = 0;
        if (v.currentDate >= timeNextBlock) {
            if (cvol == 0) {
                cmin = 0;
                cmax = 0;
            }
            if (ccount < 60 && ccount > 0) cvol = cvol / ccount;
            else cvol = cvol / 60;
            item.vol = static_cast<qreal>(cvol) / 60;
            item.min = static_cast<qreal>(cmin) / 60;
            item.max = static_cast<qreal>(cmax) / 60;
            if (data->max < item.max) data->max = item.max;
            if (data->min < item.min) data->min = item.min;
            item.dateTime = v.currentDate;
            data->series.append(item);
            ccount = 0;
            cvol = 0;
            cmax = Q_INT64_C(-932838457459459);
            cmin = Q_INT64_C(932838457459459);
            timeNextBlock = timeNextBlock.addSecs(v.secQuant);
            if (timeNextBlock > data->end) timeNextBlock = data->end;
        }

        if (v.currentDate.time().minute() == 0)
            if (v.currentDate.addSecs(3600) < timeNextBlock) {
                if (v.file.isOpen()) {
                    setTableItemPos(&v);
                    v.file.seek(v.currentPos);
                    v.file.read(&isUsed, 1);
                    raw = v.file.read(v.tableDataSize + v.dataSize * 2);
                    cvol += QSNRAWtoInt64(&raw, 0);
                    rvol = QSNRAWtoInt64(&raw, v.tableDataSize);
                    if (cmin > rvol) cmin = rvol;
                    rvol = QSNRAWtoInt64(&raw, v.tableDataSize + v.dataSize);
                    if (cmax < rvol) cmax = rvol;
                }
                ccount += 60;
                v.currentDate = v.currentDate.addSecs(3600);
                continue;
            }

        if (v.file.isOpen()) {
            if (v.currentPos >= v.tablePos) {
                setPosYBD(&v);
                v.file.seek(v.currentPos);
            }
            v.file.read(&isUsed, 1);
            raw = v.file.read(v.dataSize);
            if (isUsed) {
                rvol = QSNRAWtoInt64(&raw, 0);
                cvol += rvol;
                if (cmin > rvol) cmin = rvol;
                if (cmax < rvol) cmax = rvol;
            }
        }
        /*if (isUsed)*/ ccount ++;
        v.currentDate = v.currentDate.addSecs(60);
    }
    data->label = v.label;
    if (v.file.isOpen()) v.file.close();
}
//================= 26 == quint32 ===================================
void QsnDB::writeUInt32ToYBD(QString name, quint32 vol, QDateTime dt, QString label, quint8 type)
{
    ybdrect v;
    v.currentYear = dt.date().year();
    v.currentDate = dt;
    v.name = name;
    v.tablePos = 0;
    v.label = label;
    v.typeIndex = type;
    v.dataSize = 4;
    v.tableDataSize = 8;
    quint32 readVol;
    quint32 oldVol = 0;
    quint64 readTableVol;
    QByteArray pa;
    bool itemUsed = false;
    char isUsed;
    if (!initYBD(&v)) return;

    setPosYBD(&v);
    if(!v.file.open(QIODevice::ReadWrite)) return;
    // minits table
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    itemUsed = isUsed;
    pa = v.file.read(v.dataSize);
    v.file.seek(v.currentPos);
    oldVol = QSNRAWtoUInt32(&pa, 0);
    QSNUInt32ToRAW(&pa, 0, vol);
    isUsed = 1;
    v.file.write(&isUsed, 1);
    v.file.write(pa);

    //hour table
    setTableItemPos(&v);
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    pa = v.file.read(v.tableDataSize + v.dataSize * 2);
    v.file.seek(v.currentPos);

    readTableVol = QSNRAWtoUInt64(&pa, 0);
    readTableVol += vol;
    readTableVol -= oldVol;
    QSNUInt64ToRAW(&pa, 0, readTableVol);
    if (!isUsed) {
        QSNUInt32ToRAW(&pa, v.tableDataSize, vol);
        QSNUInt32ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    } else {
        readVol = QSNRAWtoUInt32(&pa, v.tableDataSize);
        if (readVol > vol) QSNUInt32ToRAW(&pa, v.tableDataSize, vol);
        readVol = QSNRAWtoUInt32(&pa, v.tableDataSize + v.dataSize);
        if (readVol < vol) QSNUInt32ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    }

    if (!itemUsed) isUsed ++;
    v.file.write(&isUsed, 1);
    v.file.write(pa);
    renameYBD(&v.file, label);
    v.file.close();
}

void QsnDB::requestUInt32FromYBD(dbSeries *data, quint8 type)
{
    ybdrect v;
    v.secQuant = data->begin.secsTo(data->end) / data->seriesCount;
    v.currentDate = data->begin;
    v.currentYear = 0;
    v.dataSize = 4;
    v.tableDataSize = 8;
    v.name = data->name;
    v.typeIndex = type;
    setTablePos(&v);
    data->precision = 0;


    QsnDB::dbItem item;
    quint32 cmax = 0;
    quint32 cmin = 4294967295;
    quint64 cvol = 0;
    quint32 cmax_old = 0;
    quint32 cmin_old = 4294967295;
    quint64 cvol_old = 0;
    quint32 rvol;
    data->max = cmax;
    data->min = cmin;
    int ccount = 0;
    char isUsed;

    QDateTime timeNextBlock = v.currentDate.addSecs(v.secQuant);
    QByteArray raw;

    while (v.currentDate <= data->end) {
        if (v.currentYear < v.currentDate.date().year()) openYBD(&v);
        isUsed = 0;
        if (v.currentDate >= timeNextBlock) {
            if (data->withoutBreaks && !ccount) {
                cmax = cmax_old;
                cmin = cmin_old;
                cvol = cvol_old;
            }
            if (cvol == 0) {
                cmin = 0;
                cmax = 0;
            }
            if (ccount) cvol = cvol / static_cast<quint64>(ccount);
            item.vol = cvol;
            item.min = cmin;
            item.max = cmax;

            if (type == 21) {
                item.vol = QSNToMillimetersOfMercury(static_cast<quint32>(item.vol.toUInt()));
                item.min = QSNToMillimetersOfMercury(static_cast<quint32>(item.min.toUInt()));
                item.max = QSNToMillimetersOfMercury(static_cast<quint32>(item.max.toUInt()));
            }
            if (data->max < item.max) data->max = item.max;
            if (data->min < item.min) data->min = item.min;
            item.used = ccount;
            item.dateTime = v.currentDate;
            data->series.append(item);
            cmax_old = cmax;
            cmin_old = cmin;
            cvol_old = cvol;
            ccount = 0;
            cvol = 0;
            cmax = 0;
            cmin = 4294967295;
            timeNextBlock = timeNextBlock.addSecs(v.secQuant);
            if (timeNextBlock > data->end) timeNextBlock = data->end;
        }

        if (v.currentDate.time().minute() == 0)
            if (v.currentDate.addSecs(3600) < timeNextBlock) {
                if (v.file.isOpen()) {
                    setTableItemPos(&v);
                    v.file.seek(v.currentPos);
                    v.file.read(&isUsed, 1);
                    if (isUsed) {
                        raw = v.file.read(v.tableDataSize + v.dataSize * 2);
                        cvol += QSNRAWtoUInt64(&raw, 0);
                        rvol = QSNRAWtoUInt32(&raw, v.tableDataSize);
                        if (cmin > rvol) cmin = rvol;
                        rvol = QSNRAWtoUInt32(&raw, v.tableDataSize + v.dataSize);
                        if (cmax < rvol) cmax = rvol;
                    }
                }
                if (isUsed) ccount += isUsed;
                v.currentDate = v.currentDate.addSecs(3600);
                continue;
            }

        if (v.file.isOpen()) {
            if (v.currentPos >= v.tablePos) {
                setPosYBD(&v);
                v.file.seek(v.currentPos);
            }
            v.file.read(&isUsed, 1);
            raw = v.file.read(v.dataSize);
            if (isUsed) {
                rvol = QSNRAWtoUInt32(&raw, 0);
                cvol += rvol;
                if (cmin > rvol) cmin = rvol;
                if (cmax < rvol) cmax = rvol;
            }
        }
        if (isUsed) ccount ++;
        v.currentDate = v.currentDate.addSecs(60);
    }
    data->label = v.label;
    if (v.file.isOpen()) v.file.close();
}
//================= 27 == qint32 ===================================
void QsnDB::writeInt32ToYBD(QString name, qint32 vol, QDateTime dt, QString label, quint8 type)
{
    ybdrect v;
    v.currentYear = dt.date().year();
    v.currentDate = dt;
    v.name = name;
    v.tablePos = 0;
    v.label = label;
    v.typeIndex = type;
    v.dataSize = 4;
    v.tableDataSize = 8;
    qint32 readVol;
    qint32 oldVol = 0;
    qint64 readTableVol;
    QByteArray pa;
    bool itemUsed = false;
    char isUsed;
    if (!initYBD(&v)) return;
    setPosYBD(&v);
    if(!v.file.open(QIODevice::ReadWrite)) return;
    // minits table
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    itemUsed = isUsed;
    pa = v.file.read(v.dataSize);
    v.file.seek(v.currentPos);
    oldVol = QSNRAWtoInt32(&pa, 0);
    QSNInt32ToRAW(&pa, 0, vol);
    isUsed = 1;
    v.file.write(&isUsed, 1);
    v.file.write(pa);

    //hour table
    setTableItemPos(&v);
    v.file.seek(v.currentPos);
    v.file.read(&isUsed, 1);
    pa = v.file.read(v.tableDataSize + v.dataSize * 2);
    v.file.seek(v.currentPos);

    readTableVol = QSNRAWtoInt64(&pa, 0);
    readTableVol += vol;
    readTableVol -= oldVol;
    QSNInt64ToRAW(&pa, 0, readTableVol);
    if (!isUsed) {
        QSNInt32ToRAW(&pa, v.tableDataSize, vol);
        QSNInt32ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    } else {
        readVol = QSNRAWtoInt32(&pa, v.tableDataSize);
        if (readVol > vol) QSNInt32ToRAW(&pa, v.tableDataSize, vol);
        readVol = QSNRAWtoInt32(&pa, v.tableDataSize + v.dataSize);
        if (readVol < vol) QSNInt32ToRAW(&pa, v.tableDataSize + v.dataSize, vol);
    }

    if (!itemUsed) isUsed ++;
    v.file.write(&isUsed, 1);
    v.file.write(pa);
    renameYBD(&v.file, label);
    v.file.close();
}

void QsnDB::requestInt32FromYBD(dbSeries *data, quint8 type)
{
    ybdrect v;
    v.secQuant = data->begin.secsTo(data->end) / data->seriesCount;
    v.currentDate = data->begin;
    v.currentYear = 0;
    v.dataSize = 4;
    v.tableDataSize = 8;
    v.name = data->name;
    v.typeIndex = type;
    setTablePos(&v);
    data->precision = 0;

    QsnDB::dbItem item;
    qint32 cmax = -2147483648;
    qint32 cmin = 2147483647;
    qint64 cvol = 0;
    qint32 cmax_old = -2147483648;
    qint32 cmin_old = 2147483647;
    qint64 cvol_old = 0;
    qint32 rvol;
    data->max = cmax;
    data->min = cmin;
    int ccount = 0;
    char isUsed;
    QDateTime timeNextBlock = v.currentDate.addSecs(v.secQuant);
    QByteArray raw;

    while (v.currentDate <= data->end) {
        if (v.currentYear < v.currentDate.date().year()) openYBD(&v);
        isUsed = 0;
        if (v.currentDate >= timeNextBlock) {
            if (data->withoutBreaks && !ccount) {
                cmax = cmax_old;
                cmin = cmin_old;
                cvol = cvol_old;
            }
            if (cvol == 0) {
                cmin = 0;
                cmax = 0;
            }
            if (ccount) cvol = cvol / ccount;
            item.vol = cvol;
            item.min = cmin;
            item.max = cmax;
            if (data->max < item.max) data->max = item.max;
            if (data->min < item.min) data->min = item.min;
            item.used = ccount;
            item.dateTime = v.currentDate;
            data->series.append(item);
            cmax_old = cmax;
            cmin_old = cmin;
            cvol_old = cvol;
            ccount = 0;
            cvol = 0;
            cmax = -2147483648;
            cmin = 2147483647;
            timeNextBlock = timeNextBlock.addSecs(v.secQuant);
            if (timeNextBlock > data->end) timeNextBlock = data->end;
        }

        if (v.currentDate.time().minute() == 0)
            if (v.currentDate.addSecs(3600) < timeNextBlock) {
                if (v.file.isOpen()) {
                    setTableItemPos(&v);
                    v.file.seek(v.currentPos);
                    v.file.read(&isUsed, 1);
                    if (isUsed) {
                        raw = v.file.read(v.tableDataSize + v.dataSize * 2);
                        cvol += QSNRAWtoInt64(&raw, 0);
                        rvol = QSNRAWtoInt32(&raw, v.tableDataSize);
                        if (cmin > rvol) cmin = rvol;
                        rvol = QSNRAWtoInt32(&raw, v.tableDataSize + v.dataSize);
                        if (cmax < rvol) cmax = rvol;
                    }
                }
                if (isUsed) ccount += isUsed;
                v.currentDate = v.currentDate.addSecs(3600);
                continue;
            }

        if (v.file.isOpen()) {
            if (v.currentPos >= v.tablePos) {
                setPosYBD(&v);
                v.file.seek(v.currentPos);
            }
            v.file.read(&isUsed, 1);
            raw = v.file.read(v.dataSize);
            if (isUsed) {
                rvol = QSNRAWtoInt32(&raw, 0);
                cvol += rvol;
                if (cmin > rvol) cmin = rvol;
                if (cmax < rvol) cmax = rvol;
            }
        }

        if (isUsed) ccount ++;
        v.currentDate = v.currentDate.addSecs(60);
    }
    data->label = v.label;
    if (v.file.isOpen()) v.file.close();
}

QByteArray QsnDB::requestLastVolumeRawYBD(QString name)
{
    QsnDB::dbSeries data;
    data.end = QDateTime::currentDateTime().addSecs(60);
    data.begin = data.end.addSecs(-21600);
    data.name = name;
    data.withoutBreaks = false;
    data.seriesCount = 6;
    typeYBD(&data);
    if (data.type == 255) return QByteArray();
    requestDataFromYBD(&data);
    int i = data.series.count() - 1;
    while (i >= 0 && !data.series[i].used) i --;
    if (i < 0) return QByteArray();
    QByteArray d = QSNVariantToRAW(data.series[i].vol, data.type);
    if (data.type == 1) QSNByteToRAW(&d, 2, data.aindex);
    return d;
}

void QsnDB::requestEmptyYBD(QsnDB::dbSeries *data)
{
    data->axisYlabel = QString(tr("none"));
    data->precision = 0;
    qint64 secQuant = data->begin.secsTo(data->end) / data->seriesCount;
    QDateTime currentDate = data->begin;
    QsnDB::dbItem item;
    while (currentDate <= data->end) {
        item.vol = static_cast<int>(0);
        item.min = static_cast<int>(0);
        item.max = static_cast<int>(0);
        item.used = false;
        item.dateTime = currentDate;
        data->series.append(item);
        currentDate = currentDate.addSecs(secQuant);
    }
    data->label = QString(tr("No data"));
}

void QsnDB::typeYBD(dbSeries *data)
{
    QFile file;
    QDateTime currentDT = data->begin;
    data->type = 255;

    while (currentDT.date().year() <= data->end.date().year()) {
        file.setFileName(QSNHomeSubPath("server", "db").absoluteFilePath(QString("%1_%2.ybd").arg(data->name).arg(currentDT.date().year())));

        if (file.exists()) break;
        currentDT = currentDT.addYears(1);
    }

    QByteArray blocksig;
    if (file.exists()) {
        if(file.open(QIODevice::ReadOnly)) {
            file.seek(0);
            blocksig = file.read(6);
            if (QSNRAWtoASCII(&blocksig, 0, 3) == QLatin1String("YBD") &&
                    QSNRAWtoUInt16(&blocksig, 4) == currentDT.date().year())
                data->type = QSNRAWtoByte(&blocksig, 3);
            file.close();
        }
    }
}

//==============================================================
void QsnDB::listYBD(dbFileList *list)
{
    QStringList listFiles = QSNHomeSubPath("server", "db").entryList(QString("*.ybd").split(","), QDir::Files);
    QStringListIterator i(listFiles);
    while (i.hasNext()) infoYBD(list, i.next());
}
//==============================================================

QByteArray QsnDB::registryLastData(QString name)
{
    QByteArray ld;
    for(int i = 0; i < registry.count(); i ++)
        if (registry.at(i).name == name) {
            if (!registry.at(i).lastData.isEmpty()) return registry.at(i).lastData;
            ld = requestLastVolumeRawYBD(name);
            registry[i].lastData = ld;
            return ld;
        }
    ld = requestLastVolumeRawYBD(name);
    if (!ld.isEmpty()) {
        dbRegistryItem item;
        item.lastData = ld;
        item.name = name;
        registry.append(item);
    }
    return ld;
}

void QsnDB::registryAddData(QString name, QByteArray *data)
{
    for(int i = 0; i < registry.count(); i ++)
        if (registry.at(i).name == name) {
            registry[i].lastData = *data;
            return;
        }

    dbRegistryItem item;
    item.lastData = *data;
    item.name = name;
    registry.append(item);
}

void QsnDB::registryPublicData(QString name, QByteArray data, QObject *sender)
{
    registryAddData(name, &data);
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_DB;
    container.info = name;
    container.Data = data;
    emit snBUSOutput(container, sender);
}

//==============================================================
void QsnDB::writeLog(QString msg, QString name, QString title)
{
    QFile file(QSNHomeSubPath("server", "logs").absoluteFilePath(QString("%1.log").arg(name)));
    if (!file.open(QIODevice::Append | QIODevice::Text)) return;
    QTextStream Out(&file);
    if (file.size() == 0) {
        if (title.isEmpty()) Out << name;
        else Out << title;
        writeCR(&Out);
    }

    Out << QString("[%1] ")
           .arg(QDate::currentDate().toString(QLatin1String("yyyy.MM.dd")) + "-" +
                QTime::currentTime().toString("hh.mm.ss")) << msg;
    writeCR(&Out);
    file.close();
}

void QsnDB::writeCR(QTextStream *stream)
{
#if defined(Q_OS_WIN)
    *stream << QLatin1String("\r\n");
#elif defined(Q_OS_MAC)
    *stream << QLatin1String("\r");
#elif defined(Q_OS_LINUX)
    *stream << QLatin1String("\n");
#endif
}

bool QsnDB::initFileName(QFile *file, QString RRDname, quint8 typeIndex)
{
    file->setFileName(QSNHomeSubPath("server", "db").absoluteFilePath(QString("%1_%2.rrd").arg(RRDname, QSNTypeLatianName(typeIndex))));
    QByteArray sig("RRD");
    sig.append(static_cast<char>(typeIndex));
    QByteArray block;
    if (file->exists()) {
        if(file->open(QIODevice::ReadOnly)) {
            file->seek(0);
            block = file->read(4);
            file->close();
            if (block == sig) return true;
        } else return false;
    }

    //init rrd
    qint64 fileSize = 366 * 24 * 60 * (QSNTypeDataSize(typeIndex) * 2 + 2) + 4;
    if(file->open(QIODevice::WriteOnly)) {
        file->seek(0);
        file->write(sig);
        file->close();
    }
    file->resize(fileSize);
    return true;
}

bool QsnDB::initYBD(QsnDB::ybdrect *v)
{
    v->file.setFileName(QSNHomeSubPath("server", "db").absoluteFilePath(QString("%1_%2.ybd").arg(v->name).arg(v->currentYear)));
    QByteArray sig("YBD");
    sig.append(static_cast<char>(v->typeIndex));
    QSNUInt16ToRAW(&sig, 4, static_cast<quint16>(v->currentYear));
    QByteArray blocksig;

    if (QDate::isLeapYear(v->currentYear)) v->tablePos = 527040 * (v->dataSize + 1) + 6;
    else v->tablePos = 525600 * (v->dataSize + 1) + 6;

    if (v->file.exists()) {
        if(v->file.open(QIODevice::ReadOnly)) {
            v->file.seek(0);
            blocksig = v->file.read(6);
            v->file.close();

            if (blocksig == sig) return true;
        } else return false;
    }

    if(v->file.open(QIODevice::WriteOnly)) {
        v->file.seek(0);
        v->file.write(sig);
        v->file.close();
    }
    if (QDate::isLeapYear(v->currentYear))v->file.resize(v->tablePos + 8784 * (v->tableDataSize + v->dataSize * 2 + 1));
    else v->file.resize(v->tablePos + 8760 * (v->tableDataSize + v->dataSize * 2 + 1));

    if (!v->file.open(QIODevice::WriteOnly | QIODevice::Append)) return false;

    if (v->label.isEmpty()) v->label = v->name;
    sig.clear();
    blocksig = v->label.toUtf8();
    QSNUInt16ToRAW(&sig, 0, static_cast<quint16>(blocksig.size()));
    v->file.write(blocksig);
    v->file.write(sig);
    v->file.close();
    return true;
}

void QsnDB::openYBD(QsnDB::ybdrect *v)
{
    if (v->file.isOpen()) v->file.close();
    v->currentYear = v->currentDate.date().year();
    v->file.setFileName(QSNHomeSubPath("server", "db").absoluteFilePath(QString("%1_%2.ybd").arg(v->name).arg(v->currentYear)));
    QByteArray sig("YBD");
    sig.append(static_cast<char>(v->typeIndex));
    QSNUInt16ToRAW(&sig, 4, static_cast<quint16>(v->currentYear));
    QByteArray blocksig;
    if (v->file.exists()) {
        if(v->file.open(QIODevice::ReadOnly)) {
            v->file.seek(0);
            blocksig = v->file.read(6);
            if (blocksig == sig) {
                setPosYBD(v);
                if (QDate::isLeapYear(v->currentYear)) v->tablePos = 527040 * (v->dataSize + 1) + 6;
                else v->tablePos = 525600 * (v->dataSize + 1) + 6;
                v->file.seek(v->file.size() - 2);
                sig = v->file.read(2);
                v->file.seek(v->file.size() - QSNRAWtoUInt16(&sig, 0) - 2);
                blocksig = v->file.read(QSNRAWtoUInt16(&sig, 0));
                v->label = QString::fromUtf8(blocksig);
                v->file.seek(v->currentPos);
            }else v->file.close();
        }
    }
}

void QsnDB::setPosYBD(QsnDB::ybdrect *v)
{
    v->currentPos = 6 + (QDate(v->currentYear, 1, 1).daysTo(v->currentDate.date()) * 1440
                         + v->currentDate.time().hour() * 60
                         + v->currentDate.time().minute()) * (v->dataSize + 1);
}

void QsnDB::setTablePos(QsnDB::ybdrect *v)
{
    v->tablePos += (QDate(v->currentYear, 1, 1).daysTo(v->currentDate.date()) * 24
                    + v->currentDate.time().hour()) * (v->tableDataSize + v->dataSize * 2 + 1);
}

void QsnDB::setTableItemPos(QsnDB::ybdrect *v)
{
    v->currentPos = v->tablePos + (QDate(v->currentYear, 1, 1).daysTo(v->currentDate.date()) * 24
                                   + v->currentDate.time().hour()) * (v->tableDataSize + v->dataSize * 2 + 1);
}

void QsnDB::infoYBD(QsnDB::dbFileList *list, QString fileName)
{
    QFile file;
    QsnDB::dbFileItem item;
    item.type = 255;
    item.name = fileName.left(fileName.lastIndexOf("_"));
    item.beginYear = 0;
    item.endYear = 0;
    file.setFileName(QSNHomeSubPath("server", "db").absoluteFilePath(fileName));
    if (!file.exists()) return;

    QByteArray blocksig;
    QByteArray bsize;

    if(file.open(QIODevice::ReadOnly)) {
        file.seek(0);
        blocksig = file.read(6);
        if (QSNRAWtoASCII(&blocksig, 0, 3) == QLatin1String("YBD")){
            item.type = QSNRAWtoByte(&blocksig, 3);
            int currentYear = QSNRAWtoUInt16(&blocksig, 4);
            if (item.beginYear > currentYear || !item.beginYear) item.beginYear = currentYear;
            if (item.endYear < currentYear || !item.endYear) item.endYear = currentYear;
            file.seek(file.size() - 2);
            bsize = file.read(2);
            file.seek(file.size() - QSNRAWtoUInt16(&bsize, 0) - 2);
            blocksig = file.read(QSNRAWtoUInt16(&bsize, 0));
            item.label = QString::fromUtf8(blocksig);
        }
        file.close();
    }

    for (int i = 0; i < list->items.count(); i ++)
        if (list->items[i].name == item.name) {
            if (list->items[i].beginYear > item.beginYear) list->items[i].beginYear = item.beginYear;
            if (list->items[i].endYear < item.endYear) list->items[i].endYear = item.endYear;
            return;
        }
    list->items.append(item);
}

void QsnDB::renameYBD(QFile *file, QString RRDname)
{
    if (!file->isOpen()) return;
    file->seek(file->size() - 2);
    QByteArray blocksize = file->read(2);
    file->seek(file->size() - QSNRAWtoUInt16(&blocksize, 0) - 2);
    QByteArray blocklabel = file->read(QSNRAWtoUInt16(&blocksize, 0));
    QString dblabel = QString::fromUtf8(blocklabel);
    if (dblabel == RRDname) return;
    file->resize(file->size() - QSNRAWtoUInt16(&blocksize, 0) - 2);
    file->seek(file->size());
    blocklabel = RRDname.toUtf8();
    QSNUInt16ToRAW(&blocksize, 0, static_cast<quint16>(blocklabel.size()));
    file->write(blocklabel);
    file->write(blocksize);
}


