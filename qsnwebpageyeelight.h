#ifndef QSNWEBPAGEYEELIGHT_H
#define QSNWEBPAGEYEELIGHT_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkInterface>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include "qsnwebitemyeelightdevice.h"
#include "qsnbsshapes.h"
#define YeelightDefaultTimeBetweenRequests 15

class QsnWebPageYeelight : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString yeelight READ widgetState)
public:
    QsnWebPageYeelight(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageYeelight();
    void endConfiguration();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    //    void sendMessage(QString theme, QString text);


private slots:
    void processPendingDatagrams();

signals:
    void snBUSOutput(QSNContainer container, QObject *sender);

private:
    struct FItem {
        quint8 iteration;
        QString ip;
        QString port;
        QString id;
        QString power;
        QString model;
    };

    //yeelight
    QVector<QUdpSocket*> udpSockets;
    QVector<QsnWebItemYeelightDevice*> devices;
    QVector<FItem> foundDevices;
    //    QUdpSocket UDPSocket;
    QHostAddress multiCastAddress;
    QHostAddress LastReceivedDeviceIP;
    //    QHostAddress groupAddress4;
    QString localIPAddress;
    QsnGlobalModules *mds;
    QString UDPerror;
    quint8 foundIteration;
    int timerCount;
    int timeBetweenRequests;


    void initUDP();
    QString devicesToJSON();
    QString devicesItemToString(FItem* item);
    void findDevices();
    void addDevice(QString data);
//    void setDeviceAttribute(FItem *item, QString line);
    void answerToMap(QString answer, QMap<QString, QString> *options);
    void extractIPandPort(QString name, QMap<QString, QString> *options);
//    bool deviceUpdateState(FItem *item);
    int getConectedDevices();
};

#endif // QSNWEBPAGEYEELIGHT_H
