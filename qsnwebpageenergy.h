#ifndef QSNWEBPAGEENERGY_H
#define QSNWEBPAGEENERGY_H

#define EnergyPageTimeSanitation 86400

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "qsnweb.h"
#include "qsnwebitemenergydevice.h"

/**
 * @brief Modern Energy Dashboard Page following world best practices
 * 
 * Features:
 * - Real-time power consumption monitoring
 * - Interactive charts with ApexCharts
 * - Time period selection (hour/day/week/month/year)
 * - Device-level consumption breakdown
 * - Cost tracking and analytics
 * - Peak demand indicators
 * - Comparative period analysis
 * - Responsive card-based layout
 */
class QsnWebPageEnergy : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString energy READ widgetState)
    
public:
    explicit QsnWebPageEnergy(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageEnergy();
    
    // Web framework integration
    void getJSDepending(QStringList *depending, int accountIndex);
    void getCSSDepending(QStringList *depending, int accountIndex);
    void getHTMLOnLoad(QStringList *functions, int accountIndex);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void endConfiguration();
    QString widgetState();
    void urlChanged(int accountIndex);

public slots:
    void devicePower(QByteArray *data);
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private:
    // Device management
    QList<QsnWebItemEnergyDevice *> devices;
    
    // Configuration
    QString dbName;
    QsnGlobalModules *mds;
    quint32 cost;               // Cost per kWh in cents
    QString CU;                 // Currency unit
    
    // Chart settings
    bool isminmax;
    bool iscolumns;
    bool issmoothing;
    QString maxColor;
    QString minColor;
    QString defaultColor;
    
    // Time period tracking
    int period;
    QString currentPeriod;      // Current selected period (hour/day/week/month/year)
    QDateTime beginDT;
    QDateTime endDT;
    int defaultMinutes;
    
    // Cached values
    QString lastValue;
    QString lastDay;
    QString lastWeek;
    QString lastMonth;
    QString lastYear;
    QString currentPower;       // Current power in Watts
    QString peakPower;          // Peak power demand
    QString peakTime;           // Time of peak demand
    
    // Bounds
    QString upperBound;
    QString lowerBound;
    
    // Sanitation
    quint32 sanitationTimeout;
    bool isEOM;
    quint8 requestQueue;

    // Helper methods
    void spanGraph(QsnDB::dbSeries *data);
    void getMaxMin(bool onlyValues, QsnDB::dbSeries *data, qreal *max, qreal *min, bool isNew = true);
    QString getLabelData(int minutes);
    QString getLabelDataWithCost(int minutes);
    QString linkOptions();
    int getMinutes(const QString &periodName);
    QString getPeriodLabel(const QString &periodName);
    void addPowerData(QByteArray *data);
    void addMeterData(QByteArray *data);
    
    // JSON generation
    QString totalToJSON();
    QString devicesToJSON();
    QString getChartSeries(QString name, QDateTime begin, QDateTime end, bool showMinMax);
    QString getDeviceBreakdownJSON();
    QString getSummaryCardsJSON();
    
    // Utilities
    quint16 nameToAddress(QString name);
    void deviceSanitation();
    QString formatPower(double watts);
    QString formatEnergy(double kwh);
    QString formatCost(double amount);
    QString getCurrentPowerFromData();
};

#endif // QSNWEBPAGEENERGY_H
