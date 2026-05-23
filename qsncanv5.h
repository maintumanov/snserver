#ifndef QSNCANV5_H
#define QSNCANV5_H

#include <QObject>
#include <QtCore>
#include <QtSerialPort/QSerialPort>

#include "qsnshapes.h"


class QSnCanV5 : public QObject
{
    Q_OBJECT

public:
    struct QRMsg
    {
        quint16 AnalysisState;
        quint8 DataSize;
        QByteArray Data;
        quint8 check;
        quint32 ID;
    };

    struct QSNStat
    {
        quint16 SendRetMsg;
        quint16 SendNotRetMsg;
        quint16 ReciveMsg;
        quint16 DestroedMsg;
    };

    explicit QSnCanV5(QObject *parent = Q_NULLPTR);

    void saveSettingsStream(QDataStream *stream);
    void loadSettingsStream(QDataStream *stream);
    bool isEnabled();
    void setEnabled(bool enable);
    QString getPortName();
    void setPortName(QString portName);
    bool isConnected();
    void openPort();
    void closePort();
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    QString error();
    void setLogEnable(bool enable);

signals:
    //---- For interface module
    void snBUSOutput(QSNContainer container, QObject *sender = Q_NULLPTR);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    void analisisCicle();

private:
    QSerialPort *serial;
    QRMsg NewMsg;
    QStringList SNSpeeds;
    QByteArray Memory;
    bool isLog;
    bool serialEnable;
    QString lastError;
    void analysis(quint8 D);
    void recivePack(QRMsg Msg);
    void serviceProcessing(QSNContainer container);
    void sendFrame(quint32 ID, QByteArray *data);
    void CANv5Initialization();
    void portStateChange(bool state);
    QByteArray sizeLimit(QByteArray *data);
};

#endif // QSNCANV5_H
