#ifndef QSNUARTCAN_H
#define QSNUARTCAN_H

#include <QObject>
#include <QtCore>
#include <QtSerialPort/QSerialPort>
#include "qsnshapes.h"


class QSnUartCan : public QObject
{
    Q_OBJECT

public:
    struct QRMsg {
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

    explicit QSnUartCan(QObject *parent = Q_NULLPTR);
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void saveSettingsStream(QDataStream *stream);
    void loadSettingsStream(QDataStream *stream);
    bool isEnabled();
    void setEnabled(bool enable);
    QString getPortName();
    void setPortName(QString portName);
    bool isConnected();
    void openPort(quint16 address);
    void closePort();
    QString error();
    void setLogEnable(bool enable);
    int statMsgSent();
    int statMsgRecived();


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
    QByteArray memory;
    bool isLog;
    bool serialEnable;
    QString lastError;
    //stat
    int sentMsg;
    int reciveMsg;

    void analysis(quint8 data);
    void recivePack(QRMsg Msg);
    void serviceProcessing(QSNContainer container);
    void addStart(bool isSignal, QByteArray *sd);
    void addByte(quint8 data, QByteArray *sd, quint8 *acs);
    void addWord(quint16 data, QByteArray *sd, quint8 *acs);
    void addData(QByteArray *data, QByteArray *sd, quint8 *acs);
    void sendPacket(quint16 address, quint16 sender, quint8 cmd, QByteArray *data);
    void sendSignal(quint16 signal, QByteArray *data);
    void portStateChange(bool state);
    QByteArray sizeLimit(QByteArray *data);

};

#endif // QSNUARTCAN_H
