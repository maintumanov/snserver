#ifndef QSNWEBPAGEGRAPH_H
#define QSNWEBPAGEGRAPH_H

#include <QObject>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

/**
 * @brief Web widget for visualizing time-series sensor data using ApexCharts.
 *
 * This class renders interactive charts from historical YBD (Yearly Binary Data) files,
 * supports min/max envelopes, upper/lower bounds (static or dynamic), and time range selection.
 * It integrates with the web framework to provide AJAX-based chart updates.
 */
class QsnWebPageGraph : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageGraph(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageGraph();
    /**
     * @brief Declares required JavaScript dependencies (e.g., ApexCharts).
     *
     * @param depending Output list of JS file paths.
     * @param accountIndex Current user index (unused).
     */
    void getJSDepending(QStringList *depending, int accountIndex);

    /**
     * @brief Declares required CSS dependencies.
     *
     * @param depending Output list of CSS file paths.
     * @param accountIndex Current user index (unused).
     */
    void getCSSDepending(QStringList *depending, int accountIndex);

    /**
     * @brief Returns JavaScript code to execute after page load (e.g., chart.render()).
     *
     * @param functions Output list of JS snippets.
     * @param accountIndex Current user index (unused).
     */
    void getHTMLOnLoad(QStringList *functions, int accountIndex);

    /**
     * @brief Generates full JavaScript configuration for the chart.
     *
     * Builds ApexCharts options object, including series data, axes, and toolbar.
     *
     * @param functions Output list of JS code fragments.
     * @param accountIndex Current user index (unused).
     * @param pageURL Unused URL parameter.
     */
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");

    /**
     * @brief Renders the HTML content of the graph page.
     *
     * Shows either a list of available graphs or a detailed chart with controls.
     *
     * @param contents Output list of HTML snippets.
     * @param accountIndex Current user index (unused).
     */
    QString getChartSeries(QString name, QDateTime begin,  QDateTime end, bool maxmin, QString upbound, QString lwbound, bool wbreaks);

    /**
     * @brief Renders the HTML content of the graph page.
     *
     * Shows either a list of available graphs or a detailed chart with controls.
     *
     * @param contents Output list of HTML snippets.
     * @param accountIndex Current user index (unused).
     */
    void getContents(QStringList *contents, int accountIndex);
    /**
     * @brief Handles AJAX requests to update the chart with new time range or settings.
     *
     * @param URL Unused URL.
     * @param options Map of request parameters (time range, bounds, etc.).
     * @param returnItems Output JSON fragments (e.g., new series data).
     * @param uid Unused unique ID.
     */
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);

    /**
     * @brief Restores widget state from persistent storage.
     *
     * @param stream Input QDataStream.
     */
    void fromStream(QDataStream *stream);

    /**
     * @brief Processes URL query parameters to configure the chart.
     *
     * Parses time range, data source, and display options (minmax, bounds, etc.).
     *
     * @param accountIndex Current user index (unused).
     */
    void urlChanged(int accountIndex);

private:
    QsnGlobalModules *mds;          ///< Global application modules.
    QString lUrl;                   ///< Unused legacy field.
    QDateTime beginDT;              ///< Start of time range for chart.
    QDateTime endDT;                ///< End of time range for chart.
    QString dbName;                 ///< Name of YBD data source (e.g., "temp_living").
    bool isminmax;                  ///< True if min/max envelope should be shown.
    bool iscolumns;                 ///< Unused (legacy).
    bool issmoothing;               ///< True if line smoothing is enabled.
    bool iswithoutBreaks;           ///< True if gaps in data should be hidden.
    QString maxColor;               ///< Color for max envelope.
    QString minColor;               ///< Color for min envelope.
    QString defaultColor;           ///< Default line color.
    int precision;                  ///< Number of decimal places for values.
    QString postfix;                ///< Unit postfix (unused, superseded by QSNScaledPostFix).
    QString dbLabel;                ///< Human-readable label of the data source.
    QString ubColor;                ///< Color for upper bound line.
    QString lbColor;                ///< Color for lower bound line.
    bool isupperBound;              ///< True if upper bound is active.
    bool islowerBound;              ///< True if lower bound is active.
    QString dbNameUB;               ///< Optional YBD name for dynamic upper bound.
    QString dbNameLB;               ///< Optional YBD name for dynamic lower bound.
    double upperBound;              ///< Static upper bound value (if numeric).
    double lowerBound;              ///< Static lower bound value (if numeric).
    QString uBound;                 ///< Raw upper bound string (name or number).
    QString lBound;                 ///< Raw lower bound string (name or number).
    QString upperBoundLabel;        ///< Unused legacy field.
    QString lowerBoundLabel;        ///< Unused legacy field.
    QString upperBoundPostfix;      ///< Unused legacy field.
    QString lowerBoundPostfix;      ///< Unused legacy field.
    int upperBoundPrecision;        ///< Unused legacy field.
    int lowerBoundPrecision;        ///< Unused legacy field.
    int defaultMinuts;              ///< Default time span in minutes (e.g., 1440 = 1 day).

    /**
        * @brief Determines optimal data sampling interval and X-axis label format.
        *
        * Adjusts resolution based on time span to avoid overloading the chart.
        *
        * @param data Pointer to dbSeries structure to configure.
        */
    void spanGraph(QsnDB::dbSeries *data);

    /**
        * @brief Computes global min/max values across a data series.
        *
        * Optionally includes min/max sub-values from each point.
        *
        * @param onlyValues If true, ignores min/max fields in data points.
        * @param data Input series.
        * @param max Output maximum value.
        * @param min Output minimum value.
        * @param isNew If true, initializes min/max from first point.
        */
    void getMaxMin(bool onlyValues, QsnDB::dbSeries *data, qreal *max, qreal *min, bool isNew = true);


};

#endif // QSNWEBPAGEGRAPH_H
