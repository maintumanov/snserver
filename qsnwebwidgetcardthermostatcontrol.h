#ifndef QSNWEBWIDGETCARDTHERMOSTATCONTROL_H
#define QSNWEBWIDGETCARDTHERMOSTATCONTROL_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"
#include "qsnwebwidgetcardthermostatindicator.h"

class QsnWebWidgetCardThermostatControl : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardThermostatControl(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getItemJSON(QStringList *jsonItems);
    void fromStream(QDataStream *stream, QString path = QString());
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
//    void setStateSwitch(bool state, QString source);
//    void setToggle(QString source);

signals:
//    void changeStateSwitch(bool state, QString source);
//    void onRM(QByteArray code);

private:

    QsnGlobalModules *mds;
    QString alias;
    QString dbName;

    int numIcon;
    int isActual;
    qreal temperature;
    qreal tMax;
    qreal tMin;

    QsnWebWidgetCardThermostatIndicator *ind;

    void updateTemperature(qreal newthemperature, QString source);
    QString getTextState();
    void sendDB(double temp);
    void readFromMQTT(QByteArray *data);
    void writeToMQTTPSN(double temp);

    QString getLabel();
    QString getLink();
    QString getIndState();
    QString getIndStateJson();

};

#endif // QSNWEBWIDGETCARDTHERMOSTATCONTROL_H
