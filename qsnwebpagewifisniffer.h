#ifndef QSNWEBPAGEWIFISNIFFER_H
#define QSNWEBPAGEWIFISNIFFER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include <QtSerialPort/QSerialPort>
#include "qsnbsshapes.h"

#define DefaultAbsenceTimer 30

class QsnWebPageWiFiSniffer : public QsnWeb
{
    Q_OBJECT
    Q_PROPERTY(QString keenetic READ widgetState)
public:
    QsnWebPageWiFiSniffer(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageWiFiSniffer();
    void endConfiguration();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    QString widgetState();
    char widgetNotifState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
//    void sendMessage(QString theme, QString text);


private slots:
    void analysisCicle();
    void serialError(QSerialPort::SerialPortError error);

signals:
    void snBUSOutput(QSNContainer container, QObject *sender);

private:
    struct macItem {
        QString MAC;
        quint8 RSSI;
        int timer;
        bool isUser;
        QTime timeReg;
    };

    QSerialPort *serial;
    QString serialState;
    int serialReconnectTimer;
    QByteArray serialBuffer;
    quint8 serialAnalysisState;
    macItem serialAnalysisItem;
    quint8 serialAnalysisCountCRS;
    int maxDetectTimeSec;

    QsnGlobalModules *mds;
    QList<macItem> items;
    quint8 detectedCount;
    int timeAbsenceSec;
    bool isEnable;
    bool isAddMACmode;

    void serialDisconnect();
    void serialConnect();

    QString listToJSON();
    QString macToJSON();
    int countUsers();

    void addMACitem(macItem *item);
    void checkTimeItems();
    bool isMAC(QString mac);
    bool isUserMAC(QString mac);
    quint8 isDetectionCount();
    QString detectionState();
    QString mdtState();
    void checkDeviceDetected();
    void setNotDetected();
    void setDetected();
    QString macNum(quint8 num);
    QByteArray stateData(bool state);

};

#endif // QSNWEBPAGEWIFISNIFFER_H
