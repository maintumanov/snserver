#ifndef QSNWEBITEMENERGYDEVICE_H
#define QSNWEBITEMENERGYDEVICE_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebItemEnergyDevice : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebItemEnergyDevice(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void  setChartMinMax(bool enable);
    void  setChartSmoothing(bool enable);
    void  setChartColumns(bool enable);
    void  setPeriod(int per);
    void  setCost(quint32 icost);
    void  setCU(QString icu);
    void setAddrss(quint16 addr);
    quint16 address();
    QString location();
    void setPower(QByteArray *data);
    void setDBname(QString name);
    bool isDBfound();
    QString getJSONContent(bool last);
    QString getLabelData(int minutes);
    QString getLabelLastData();

signals:
    void readPower(QByteArray *data);

private:
    QsnGlobalModules *mds;
    QString dbName;
    QString devLocation;
    int numIcon;
    bool isminmax;
    bool iscolumns;
    bool issmoothing;
    int period;
    quint32 cost;
    QString CU;
    quint16 devAddress;

    double lastData;

    QString linkOptions();

    int getMinutes(int p);

};


#endif // QSNWEBITEMENERGYDEVICE_H
