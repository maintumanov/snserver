#ifndef QSNRING_H
#define QSNRING_H

// last edit 25.02.2022

#include <QObject>
#include <QtCore>
#include <QtSerialPort/QSerialPort>
#include "qsnshapes.h"

class QSnRing : public QObject
{
    Q_OBJECT

public:
    struct QRMsg
    {
        quint16 analysisState;
        bool isSignal;
        quint16 address;
        quint16 sender;
        quint16 signal;
        quint8 dlc;
        QByteArray data;
        quint8 cacs;
        quint8 acs;
        quint8 cmd;
        quint8 dps;
    };

    struct QSNStat
    {
        quint16 SendRetMsg;
        quint16 SendNotRetMsg;
        quint16 ReciveMsg;
        quint16 DestroedMsg;
    };

    explicit QSnRing(QObject *parent = Q_NULLPTR);

    void saveSettingsStream(QDataStream *stream);
    void loadSettingsStream(QDataStream *stream);
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    bool isEnabled();
    void setEnabled(bool enable);
    QString getPortName();
    void setPortName(QString portName);
    bool isConnected();
    void openPort(quint16 address);
    void closePort();
    QString error();
    void setLogEnable(bool enable);

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    void analysisCicle();

private:
    QSerialPort *serial;
    quint16 thisAddress;
    QRMsg newMsg;
    QStringList SNSpeeds;
    QByteArray Memory;
    bool isLog;
    bool serialEnable;
    QString lastError;
    void analysis(quint8 data);
    void recivePack(QRMsg Msg);
    void serviceProcessing(QSNContainer container);
    void addStart(bool isSignal, QByteArray *sd);
    void addByte(quint8 data, QByteArray *sd, quint8 *acs);
    void addWord(quint16 data, QByteArray *sd, quint8 *acs);
    void addData(QByteArray *data, QByteArray *sd, quint8 *acs);
    void SendPacket(quint16 address, quint16 sender, quint8 cmd, QByteArray *data);
    void SendSignal(quint16 signal, QByteArray *data);
    void Transmit(QSNContainer container);
    void portStateChange(bool state);
   // void info(quint8 warningLevel, QString text, bool dispView = false);

};

#endif // QSNRING_H
