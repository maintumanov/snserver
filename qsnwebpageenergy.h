#ifndef QSNWEBPAGEENERGY_H
#define QSNWEBPAGEENERGY_H

#define EnergyPageTimeSanitation 86400

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "qsnweb.h"
#include "qsnwebitemenergydevice.h"

class QsnWebPageEnergy : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString energy READ widgetState)
public:
    QsnWebPageEnergy(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageEnergy();
    void getJavaScript(QStringList *script, int accountIndex);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void endConfiguration();
    QString widgetState();

public slots:
    void devicePower(QByteArray *data);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);


private:
    QList<QsnWebItemEnergyDevice *> devices;
    QString dbName;
    QsnGlobalModules *mds;
    bool isminmax;
    bool iscolumns;
    bool issmoothing;
    QString maxColor;
    QString minColor;
    QString defaultColor;
    int period;
    quint32 sanitationTimeout;
    quint32 cost;
    QString CU;
    QString axisXlabel;
    QString lastVolue;
    quint8 requestQueue;

    //QString lastHour;
    QString lastDay;
    QString lastWeek;
    QString lastMonth;

    QString upperBound;
    QString lowerBound;

    bool isEOM;

    void spanGraph(QsnDB::dbSeries *data);
    QString strFromIndexRange(QDateTime dt, int index);
    void getMaxMin(bool onlyValues, QsnDB::dbSeries *data, qreal *max, qreal *min);
    QString getLabelData(int minutes);
    QString linkOptions();
    int getMinutes(int p);
    QString getPeriodLabel(int p);
    void addPowerData(QByteArray *data);
    void addMeterData(QByteArray *data);
    QString totalToJSON();
    QString devicesToJSON();
    quint16 nameToAddress(QString name);
    void deviceSanitation();

};

#endif // QSNWEBPAGEENERGY_H
