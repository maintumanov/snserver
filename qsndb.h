#ifndef QSNDB_H
#define QSNDB_H

#include <QObject>
#include <QDateTime>
#include <QDebug>
#include "qsnshapes.h"
#include <QStringList>

class QsnDB : public QObject
{
    Q_OBJECT
public:
    explicit QsnDB(QObject *parent = Q_NULLPTR);

public:
    struct dbItem {
        bool used;
        QVariant vol;
        QVariant min;
        QVariant max;
        QDateTime dateTime;
    };

    struct dbSeries {
        QString name;
        quint8 type;
        QDateTime begin;
        QDateTime end;
        QList<dbItem> series;
        int seriesCount;
        bool withoutBreaks;
        int precision;
        QString axisYlabel;
        QString axisXlabel;
        QString label;
        QVariant max;
        QVariant min;
        int indexRange;
        quint8 aindex;
    };

    struct dbFileItem {
        QString name;
        QString label;
        int beginYear;
        int endYear;
        int type;
    };

    struct dbFileList {
        QList<dbFileItem> items;
    };

    struct dbRegistryItem {
        QString name;
        QByteArray lastData;
    };

    QMap<QString, QVariant> tempSettings;

    // ==old==============================================================================
    void writeRAWtoRRD(QString RRDname, QByteArray *RAW);
    bool requestBegin(QFile *file, quint8 typeIndex, QString RRDname);
    qreal requestGetField(QFile *file, quint8 typeIndex, QDateTime rtime, qreal prevVol = 0);
    void requestEnd(QFile *file);
    //========================================================================================

    void writeRAWtoYBD(QString name, QByteArray *RAW, QDateTime dt, QString label, QObject *sender);
    void requestDataFromYBD(dbSeries *data);
    // 0 - 4 ybd int16 (bool and null)
    void writeBNToYBD(QString name, bool state, quint8 noteIndex, QDateTime dt, QString label, quint8 type = 8);
    void requestBNFromYBD(dbSeries *data, quint8 type = 8);
    // 9 ybd temperature
    void writeTemperatureToYBD(QString YBDname, qreal value, QDateTime dt, QString label);
    void requestTemperatureFromYBD(dbSeries *data);
    // 19 ybd power
    void writeEnergyToYBD(QString name, QSNPower power, QDateTime dt, QString label);
    void writeEnergyToYBDmin(QString name, QSNPower power, QDateTime dt, QString label);
    void requestEnergyFromYBD(dbSeries *data);
    // 26 ybd uint32
    void writeUInt32ToYBD(QString name, quint32 vol, QDateTime dt, QString label, quint8 type = 26);
    void requestUInt32FromYBD(dbSeries *data, quint8 type = 26);
    // 27 ybd int32
    void writeInt32ToYBD(QString name, qint32 vol, QDateTime dt, QString label, quint8 type = 27);
    void requestInt32FromYBD(dbSeries *data, quint8 type = 27);

    QByteArray requestLastVolumeRawYBD(QString name);
    void requestEmptyYBD(dbSeries *data);

    void typeYBD(dbSeries *data);
    void listYBD(dbFileList *list);

    //regisry
    QByteArray registryLastData(QString name);
    void registryAddData(QString name, QByteArray *data);
    void registryPublicData(QString name, QByteArray data, QObject *sender);

signals:
    void snBUSOutput(QSNContainer container, QObject *sender);

public slots:
    // log
    void writeLog(QString msg, QString name, QString title = tr("log"));
    void writeCR(QTextStream *stream);

private:
    struct ybdrect {
        long long currentPos;
        long long tablePos;
        int currentYear;
        QDateTime currentDate;
        long long secQuant;
        QFile file;
        quint8 typeIndex;
        quint8 dataSize;
        quint8 tableDataSize;
        QString name;
        QString label;
    };

    QList<dbRegistryItem> registry;

    bool initFileName(QFile *file, QString RRDname, quint8 typeIndex);

    bool initYBD(ybdrect *v);
    void openYBD(ybdrect *v);

    void setPosYBD(ybdrect *v);
    void setTablePos(ybdrect *v);
    void setTableItemPos(ybdrect *v);
    void infoYBD(dbFileList *list, QString fileName);
    void renameYBD(QFile *file, QString RRDname);

};

#endif // QSNDB_H
