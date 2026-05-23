// last edit 29.11.2023

#ifndef SNINTERFACE_H
#define SNINTERFACE_H

#include <QObject>
#include <QtCore>
#include <QVector>

#include "qsnshapes.h"

#define attemptsOperations 3
#define waitingTimeMemoryOperations 10
#define waitingTimeInfoOperations 2

class QsnInterface : public QObject
{
    Q_OBJECT
public:
    explicit QsnInterface(QObject *parent = Q_NULLPTR);
    bool isAutoConnectEnable();
    bool isDebugProtocolV1();
    bool isStartLoaded();
    bool isConnected();
    QByteArray vecOfString(QString sHex);
    QString stringOfVec(QByteArray *Data, quint16 start, quint8 base, QString pref);
    quint16 getDeviceAddress();
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void saveSettingsStream(QDataStream *stream);
    void loadSettingsStream(QDataStream *stream);
    void saveMemory();
    void saveMemory(QString filePath);
    QByteArray exportMemory();
    QByteArray *getMemory();
    void loadMemory();
    void loadMemory(QString filePath);
    void loadMemoryFromByteArray(QByteArray *conf);
    QSNSignal containerToSignal(QSNContainer container);
    QSNMessage containerToMessage(QSNContainer container);
    void setMemoryFileName(QString filePath);
    void setDeviceName(QString name);
    QString deviceName();
    void setTimeOutMemoryOp(int time);
    int timeOutMemoryOp();

signals:
    // left-hand side
    void snBUSOutput(QSNContainer container, QObject *sender);

    // right-hand side
    void eventDisconnect();
    void eventConnect();
    void eventConected(bool connected);
    void eventDisconected(bool disconnected);
    void eventReciveMessage(QSNMessage message);
    void eventReciveSignal(QSNSignal signal);
    void eventReciveDeviceInfo(quint16 DevAddr, quint16 devType, quint16 memorySize);
    void eventDevicesUpdate();
    void eventError(QObject *sender, QString text);
    void eventAlert(quint8 SeverityLevel, QString Message);
    void eventInfo(QString Message);
    void eventProgress(quint8 ID, QString caption, int position, int max);
    void eventProgressTask(int position, int max);
    void eventStateChange(QString group, QString caption, QString state);
    void eventParametr(quint8 Parametr, QByteArray Data);
    void eventNumInput(int numInput, QByteArray Data);
    void eventAddressChange();
    void eventMemoryLoaded();
    void eventFailureMemoryLoad();

    // dev info task
    void eventDevInfoAnswer(quint16 DevAddr, quint16 devType, quint16 memorySize);
    void eventDevInfoUnsolicited(quint16 DevAddr, quint16 devType, quint16 memorySize);
    void eventDevInfoTimeOut(quint16 DevAddr);
    void eventDevAllInfoAnswer(quint16 DevAddr, quint16 devType, quint16 memorySize);
    void eventDevAllInfoEnd();
    void eventParametrAnswer(quint16 DevAddr, quint8 Parametr, QByteArray Data);

    // dev info task
    void eventMemoryReadAnswer(quint16 DevAddr, QByteArray memory, QString version);
    void eventMemoryWriteAnswer(quint16 DevAddr);
    void eventMemoryTimeOut(quint16 DevAddr, quint8 attempts);

    // dev address task
    void eventDevAddressAnswer(quint16 DevAddr, quint16 newAddr);
    void eventDevAddressTimeOut(quint16 DevAddr, quint16 newAddr);

public slots:
    // left-hand side
    void snBUSInput(QSNContainer container, QObject *sender);

    // right-hand side
    void actionConnect();
    void actionDisconnect();
    void actionAutoConnect();
    void actionSendData(quint8 cmd, quint16 address, QByteArray data);
    void actionSendSignal(quint16 signal);
    void actionSendSignal(quint16 signal, QByteArray *data);
    void actionSetAddress(quint16 address);
    void actionNetDevicesRefresh();
    void actionDevParametrRequest(quint16 addr, quint8 parametr);
    void actionDevParametrRequest(quint16 addr, quint8 parametr, QByteArray *data);
    void actionNumOutput(int numOutput);
    void actionNumOutput(int numOutput, QByteArray Data);

    // dev task
    void actionDevInfoRequest(quint16 addr);
    void actionDevAllInfoRequest();
    void actionDevAllInfoStop();
    void actionDevAddressChangeRequest(quint16 addr, quint16 newAddr);
    void actionDevMemoryReadRequest(quint16 addr, quint16 blockCount);
    void actionDevMemoryWriteRequest(quint16 addr, QByteArray memory);
    void actionDevMemoryAbortWriteRead();

    // service
    void setAutoConnectEnable(bool enable);
    void setDebugProtocolV1(bool enable);
    void setStartLoaded(bool enable);
    void setIOcount(quint16 count);
    void setMemorySize(quint16 size);
    void setDeviceTypeIndex(quint16 type);

private:

    struct taskDevice {
        enum taskOp {info, read, write, address, allInfo};
        quint8 paramNum;
        quint16 devAddress;
        quint16 devNewAddress;
        quint16 memoryAddress;
        QByteArray memory;
        int blockPointer;
        taskOp operations;
        int waitTime;
        int attempts;
        QString version;
    };

    quint8 debuglevel;
    int countBroadSendMsg;
    quint16 deviceAddress;
    quint16 deviceType;
    bool waitListFlag;
    QTimer waitTimer;
    QTimer timeTimer;
    quint8 iConnected;
    bool autoConnectEnable;
    bool debugProtocolV1;
    bool startLoaded;
    QString devName;
    int waitingTimeMO;

    // dev memory
    quint16 memoryIOCount;
    QByteArray memory;
    QString memoryFileName;
    bool memoryIsCompressed;

    // dev task
    QVector< taskDevice > tasks;

    void PacketHandling(QSNMessage message);
    void ServiceProcessing(QSNContainer container);
    void netActionGetInfo(QSNMessage *message);
    void netActionSetAddress(QSNMessage *message);
    bool netActionNewDevice(QSNMessage *message);
    void netActionGetMemory(QSNMessage *message);
    void netActionReadMemory(QSNMessage *message);
    void netActionSetMemory(QSNMessage *message);
    void netActionWriteMemory(QSNMessage *message);
    void netActionAnswerAddress(QSNMessage *message);
    void netActionAnswerParametr(QSNMessage *message);
    void moduleConnected();
    void moduleDisconnected();


    // dev info task
    void checkDevInfoTasks(quint16 devAddr, quint16 devType, quint16 memorySize);
    void checkMemoryReadTasks(QSNMessage *message, taskDevice *task);
    void checkMemoryReadTasksSend(taskDevice *task);
    void checkMemoryWriteTasks(taskDevice *task);
    void checkMemoryWriteTasksSend(taskDevice *task);

    void checkTaskTime();
    void removeTask(taskDevice *task);

    // local
    void getMemoryFromAddress(QByteArray *indata, QByteArray *outdata);
    void setMemoryFromAddress(QByteArray *indata, QByteArray *outdata, bool *isEnd);
    void checkInputSignal(QSNContainer *container);

private slots:
    void timerTimeOut();
    void secondOut();

};

#endif // SNINTERFACE_H
