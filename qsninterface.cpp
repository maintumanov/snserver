#include "qsninterface.h"

QsnInterface::QsnInterface(QObject *parent) :
    QObject(parent)
{
    autoConnectEnable = false;
    iConnected = 0;
    debuglevel = 0;
    countBroadSendMsg = 0;
    deviceAddress = 4000;
    waitListFlag = false;
    waitingTimeMO = waitingTimeMemoryOperations;
    setObjectName(tr("Interface"));

    memoryIOCount = 0;
    memoryFileName = QString();
    memoryIsCompressed = false;

    timeTimer.setInterval(1000);
    timeTimer.setTimerType(Qt::VeryCoarseTimer);
    timeTimer.start();

    connect(&waitTimer,SIGNAL(timeout()),this,SLOT(timerTimeOut()));
    connect(&timeTimer, SIGNAL(timeout()), this, SLOT(secondOut()));

}

bool QsnInterface::isAutoConnectEnable()
{
    return autoConnectEnable;
}

bool QsnInterface::isDebugProtocolV1()
{
    return debugProtocolV1;
}

bool QsnInterface::isStartLoaded()
{
    return startLoaded;
}

bool QsnInterface::isConnected()
{
    return iConnected;
}

void QsnInterface::snBUSInput(QSNContainer container, QObject *sender)
{
    if (container.role == QSNContainer::service ) {
        ServiceProcessing(container);
        emit snBUSOutput(container, sender);
        return;
    }

    if (container.role == QSNContainer::signal && container.Signal == 0) return;
    if (container.role == QSNContainer::signal)
    {
        checkInputSignal(&container);
        emit eventReciveSignal(containerToSignal(container));
        emit snBUSOutput(container, sender);
        return;
    }
    if (container.role == QSNContainer::message && container.Address == deviceAddress)
    {
        PacketHandling(containerToMessage(container));
        return;
    }
    if (container.role == QSNContainer::message && container.Address == 0 && container.Sender != deviceAddress)
        PacketHandling(containerToMessage(container));

    emit snBUSOutput(container, sender);
}

void QsnInterface::PacketHandling(QSNMessage message)
{
    if ((message.Address == 0) && (message.Command > 0)) return;
    switch (message.Command)
    {
    case 0: netActionGetInfo(&message);     break;
    case 1: netActionNewDevice(&message);   break;
    case 2: netActionGetMemory(&message);   break;
    case 3: netActionReadMemory(&message);  break;
    case 4: netActionSetMemory(&message);  break;
    case 5: netActionWriteMemory(&message);  break;
    case 6: netActionSetAddress(&message);  break;
    case 7: netActionAnswerAddress(&message);  break;
    case 9: netActionAnswerParametr(&message);  break;
    default:
        emit eventReciveMessage(message);
    }
}

void QsnInterface::ServiceProcessing(QSNContainer container)
{
    switch (container.Command)
    {
    case BUSSERV_ADAPTER_STATE:
        if (container.Signal == BUSSERV_ADAPTER_STATE_connected) moduleConnected();
        if (container.Signal == BUSSERV_ADAPTER_STATE_disconnected) moduleDisconnected();
    }
}

void QsnInterface::netActionSetAddress(QSNMessage *message)
{
    quint16 oldAddress = deviceAddress;
    deviceAddress = QSNRAWtoUInt16(&message->Data, 0);

    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = message->Sender;
    QSNUInt16ToRAW(&container.Data, 0, deviceAddress);
    container.Sender = oldAddress;
    container.Command = 7;
    emit snBUSOutput(container, this);
    actionSetAddress(deviceAddress);
    emit eventInfo(QString(tr("Address changed from %1 to %2")).arg(oldAddress).arg(deviceAddress));
}

void QsnInterface::netActionGetInfo(QSNMessage *message)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = message->Sender;
    QSNUInt16ToRAW(&container.Data, 0, deviceType);
    QSNUInt16ToRAW(&container.Data, 2, static_cast<quint16>(memory.size()));
    container.Sender = deviceAddress;
    container.Command = 1;
    emit snBUSOutput(container, this);
}

bool QsnInterface::netActionNewDevice(QSNMessage *message)
{
    checkDevInfoTasks(message->Sender, QSNRAWtoUInt16(&message->Data,0), QSNRAWtoUInt16(&message->Data,2));
    emit eventReciveDeviceInfo(message->Sender, QSNRAWtoUInt16(&message->Data,0), QSNRAWtoUInt16(&message->Data,2));
    return true;
}

void QsnInterface::netActionGetMemory(QSNMessage *message)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = message->Sender;
    container.Sender = deviceAddress;
    container.Command = 3;
    getMemoryFromAddress(&message->Data, &container.Data);
    emit snBUSOutput(container, this);
}

void QsnInterface::netActionReadMemory(QSNMessage *message)
{
    for (int i = tasks.count() - 1; i >= 0; i--)
        if (tasks[i].operations == taskDevice::read &&
                message->Sender == tasks[i].devAddress &&
                QSNRAWtoUInt16(&message->Data,0) == tasks[i].blockPointer) {
            checkMemoryReadTasks(message, &tasks[i]);
            return;
        }
}

void QsnInterface::netActionSetMemory(QSNMessage *message)
{
    QSNContainer container = newContainer();
    bool isEnd = false;
    container.role = QSNContainer::message;
    container.Address = message->Sender;
    container.Sender = deviceAddress;
    container.Command = 5;
    setMemoryFromAddress(&message->Data, &container.Data, &isEnd);
    emit snBUSOutput(container, this);

    if (isEnd) {
        saveMemory();
        emit eventMemoryLoaded();
    }
}

void QsnInterface::netActionWriteMemory(QSNMessage *message)
{
    for (int i = tasks.count() - 1; i >= 0; i--)
        if (tasks[i].operations == taskDevice::write &&
                message->Sender == tasks[i].devAddress &&
                QSNRAWtoUInt16(&message->Data,0) == tasks[i].blockPointer) {
            checkMemoryWriteTasks(&tasks[i]);
            return;
        }
}

void QsnInterface::netActionAnswerAddress(QSNMessage *message)
{
    for (int i = tasks.count() - 1; i >= 0; i--)
        if (tasks[i].operations == taskDevice::address &&
                message->Sender == tasks[i].devAddress &&
                QSNRAWtoUInt16(&message->Data,0) == tasks[i].devNewAddress) {
            emit eventDevAddressAnswer(tasks[i].devAddress, tasks[i].devNewAddress);
            tasks.remove(i);
            return;
        }
}

void QsnInterface::netActionAnswerParametr(QSNMessage *message)
{
    if (message->Address == deviceAddress) {
        QByteArray D = message->Data;
        if (D.length() < 3) return;
        quint8 channel;
        if (debugProtocolV1) {
            channel = static_cast<quint8>(D[0]) >> 6;
            D[0] = D[0] & 63;
            emit eventParametrAnswer(message->Sender, channel, D);
        } else {
            channel = static_cast<quint8>(D[0]);
            D.remove(0, 1);
        }
        emit eventParametrAnswer(message->Sender, channel, D);
    }
}

//void QsnInterface::busAlertText(int level, QString message, QObject *sender)
//{
//    eventAlert(level, QString("[%1] %2 - %3")
//               .arg(QTime::currentTime().toString("hh:mm:ss"))
//               .arg(sender->objectName())
//               .arg(message));
//}

void QsnInterface::moduleConnected()
{
    iConnected++;
    if (iConnected == 1) emit  eventConnect();
    emit eventConected(true);
    emit eventDisconected(false);
    emit eventStateChange(QString(tr("Device")),QString(tr("Address")),QString("%1").arg(deviceAddress));
}

void QsnInterface::moduleDisconnected()
{

    if (iConnected > 0) iConnected --;
    if (iConnected == 0) emit  eventDisconnect();
    emit eventConected(false);
    emit eventDisconected(true);
}

void QsnInterface::checkDevInfoTasks(quint16 devAddr, quint16 devType, quint16 memorySize)
{
    for (int i = tasks.count() - 1; i >= 0; i--) {
        if (tasks[i].operations == taskDevice::info &&
                devAddr == tasks[i].devAddress) {
            tasks.remove(i);
            emit eventDevInfoAnswer(devAddr, devType, memorySize);
            return;
        }

        if (tasks[i].operations == taskDevice::allInfo) {
            tasks[i].waitTime = waitingTimeInfoOperations * 4;
            tasks[i].attempts = 0;
            emit eventDevAllInfoAnswer(devAddr, devType, memorySize);
            return;
        }
    }
    emit eventDevInfoUnsolicited(devAddr, devType, memorySize);
}

void QsnInterface::checkMemoryReadTasks(QSNMessage *message, taskDevice *task)
{
    if (QSNRAWtoUInt16(&message->Data,0) == 65535) {
        task->memory.resize(QSNRAWtoUInt16(&message->Data,2) * 6);
        task->blockPointer = 0;
        task->version = QSNRAWtoVersion(&message->Data, 4);
    } else {
        for (int i = 2; i < message->Data.count(); i ++)
            task->memory[(task->blockPointer * 6) + i - 2] = message->Data[i];
        task->blockPointer ++;
    }

    if (task->blockPointer * 6 >= task->memory.count())   {
        emit eventMemoryReadAnswer(task->devAddress, task->memory, task->version);
        removeTask(task);
        return;
    }
    task->attempts = attemptsOperations;
    emit eventProgressTask(task->blockPointer, task->memory.count() / 6);
    checkMemoryReadTasksSend(task);
}

void QsnInterface::checkMemoryReadTasksSend(QsnInterface::taskDevice *task)
{
    task->waitTime = waitingTimeMO;
    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = task->devAddress;
    container.Sender = deviceAddress;
    container.Command = 2;
    container.Data.append(static_cast<char>(task->blockPointer >> 8));
    container.Data.append(static_cast<char>(task->blockPointer & 255));
    emit snBUSOutput(container, this);
}

void QsnInterface::checkMemoryWriteTasks(QsnInterface::taskDevice *task)
{
    if ( task->blockPointer == 65535) {
        task->blockPointer = 0;
    } else {
        task->blockPointer ++;
    }
    quint16 addr = static_cast<quint16>(task->blockPointer * 6);
    if (addr >= task->memory.count())   {
        removeTask(task);
        emit eventMemoryWriteAnswer(task->devAddress);
        return;
    }
    task->memoryAddress = addr;
    task->attempts = attemptsOperations;
    emit eventProgressTask(task->blockPointer, task->memory.count() / 6);
    checkMemoryWriteTasksSend(task);
}

void QsnInterface::checkMemoryWriteTasksSend(QsnInterface::taskDevice *task)
{
    task->waitTime = waitingTimeMO;
    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = task->devAddress;
    container.Sender = deviceAddress;
    container.Command = 4;
    container.Data.append(static_cast<char>(task->blockPointer >> 8));
    container.Data.append(static_cast<char>(task->blockPointer & 255));
    for (int i = 0; i < 6; i ++)
        container.Data.append(task->memory[i + task->memoryAddress]);
    emit snBUSOutput(container, this);
}

void QsnInterface::checkTaskTime()
{
    for (int i = tasks.count() - 1; i >= 0; i--)
    {
        if (tasks[i].waitTime > 0) tasks[i].waitTime --;
        if (tasks[i].waitTime == 0) {
            if (tasks[i].attempts > 0) tasks[i].attempts--;
            switch (tasks[i].operations) {
            case taskDevice::write:
                emit eventMemoryTimeOut(tasks[i].devAddress , static_cast<quint8>(tasks[i].attempts));
                if (tasks[i].attempts == 0) tasks.remove(i);
                else checkMemoryWriteTasksSend(&tasks[i]);
                break;
            case taskDevice::read:
                emit eventMemoryTimeOut(tasks[i].devAddress , static_cast<quint8>(tasks[i].attempts));
                if (tasks[i].attempts == 0) tasks.remove(i);
                else checkMemoryReadTasksSend(&tasks[i]);
                break;
            case taskDevice::info:
                emit eventDevInfoTimeOut(tasks[i].devAddress);
                tasks.remove(i);
                break;
            case taskDevice::address:
                emit eventDevAddressTimeOut(tasks[i].devAddress, tasks[i].devNewAddress);
                tasks.remove(i);
                break;
            case taskDevice::allInfo:
                emit eventDevAllInfoEnd();
                tasks.remove(i);
                break;
            }
        }
    }
}

void QsnInterface::removeTask(QsnInterface::taskDevice *task)
{
    for (int i = tasks.count() - 1; i >= 0; i--)  {
        if (&tasks[i] == task) {
            tasks.remove(i);
            break;
        }
    }
}

void QsnInterface::getMemoryFromAddress(QByteArray *indata, QByteArray *outdata)
{
    quint16 block = QSNRAWtoUInt16(indata, 0);
    QSNUInt16ToRAW(outdata, 0, block);
    if (block == 65535) {
        outdata->resize(4);
        QSNUInt16ToRAW(outdata, 2, static_cast<quint16>((memory.size() / 6)));
        QSNVersionToRAW(outdata, 4, QCoreApplication::applicationVersion());
    } else {
        int addr = block * 6;
        outdata->resize(8);
        for (int i = 0; i < 6; i ++) (*outdata)[i + 2] = memory.at(addr + i);
    }
}

void QsnInterface::setMemoryFromAddress(QByteArray *indata, QByteArray *outdata, bool *isEnd)
{
    quint16 block = QSNRAWtoUInt16(indata, 0);
    QSNUInt16ToRAW(outdata, 0, block);
    if (block == 65535) {
        memory.resize(QSNRAWtoUInt16(indata, 2) * 6);
        outdata->resize(2);
    } else {
        int addr = block * 6;
        for (int i = 0; i < 6; i ++)  memory[addr + i] = static_cast<char>(indata->at(i + 2));
        if (addr + 6 >= memory.count()) *isEnd = true;
    }
}

void QsnInterface::checkInputSignal(QSNContainer *container)
{
    quint16 signal = container->Signal;
    int memoryEnd = memory.count() - 1;
    for (int i = 0; i < memoryIOCount; i ++)
        if (static_cast<quint8>(memory[memoryEnd - i * 3]) > 0) {
            if (QSNRAWtoUInt16(&memory, memoryEnd - i * 3 - 2) == signal)
                emit eventNumInput(memory[memoryEnd - i * 3] - 1, container->Data);
        }
}

void QsnInterface::actionSendData(quint8 cmd, quint16 address, QByteArray data)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = address;
    container.Data = data;
    container.Sender = deviceAddress;
    container.Command = cmd;
    emit snBUSOutput(container, this);
}

void QsnInterface::actionSendSignal(quint16 signal)
{
    QByteArray Data;
    actionSendSignal(signal, &Data);
}

void QsnInterface::actionSendSignal(quint16 signal, QByteArray *data)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::signal;
    container.Signal = signal;
    container.Data = *data;
    emit snBUSOutput(container, this);
}

void QsnInterface::actionSetAddress(quint16 address)
{
    deviceAddress = address;
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Sender = deviceAddress;
    container.Command = 6;
    emit eventAddressChange();
    emit snBUSOutput(container, this);
}

void QsnInterface::actionNetDevicesRefresh()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = 0;
    container.Sender = deviceAddress;
    container.Command = 0;
    emit snBUSOutput(container, this);
    emit eventDevicesUpdate();
}

void QsnInterface::actionDevInfoRequest(quint16 addr)
{
    taskDevice di;
    di.devAddress = addr;
    di.waitTime = waitingTimeInfoOperations;
    di.operations = taskDevice::info;
    tasks.append(di);

    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = addr;
    container.Sender = deviceAddress;
    container.Command = 0;
    if (waitTimer.isActive() == false) waitTimer.start(100);
}

void QsnInterface::actionDevAllInfoRequest()
{
    taskDevice di;
    di.devAddress = 0;
    di.waitTime = waitingTimeInfoOperations * 4;
    di.operations = taskDevice::allInfo;
    tasks.append(di);

    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = 0;
    container.Sender = deviceAddress;
    container.Command = 0;
    emit snBUSOutput(container, this);
    if (waitTimer.isActive() == false) waitTimer.start(100);
}

void QsnInterface::actionDevAllInfoStop()
{
    for (int i = tasks.count() - 1; i >= 0; i--)
        if (tasks[i].operations == taskDevice::allInfo) {
            emit eventDevAllInfoEnd();
            tasks.remove(i);
            return;
        }
}

void QsnInterface::actionDevAddressChangeRequest(quint16 addr, quint16 newAddr)
{
    taskDevice di;
    di.devAddress = addr;
    di.waitTime = waitingTimeInfoOperations * 20;
    di.operations = taskDevice::address;
    di.devNewAddress = newAddr;
    tasks.append(di);

    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = addr;
    container.Sender = deviceAddress;
    container.Command = 6;
    container.Data.append(static_cast<char>(newAddr >> 8));
    container.Data.append(static_cast<char>(newAddr & 255));
    emit snBUSOutput(container, this);

    if (waitTimer.isActive() == false) waitTimer.start(100);
}
void QsnInterface::actionDevParametrRequest(quint16 addr, quint8 parametr)
{
    QByteArray D;
    actionDevParametrRequest(addr, parametr, &D);
}

void QsnInterface::actionDevParametrRequest(quint16 addr, quint8 parametr, QByteArray *data)
{  
    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = addr;
    container.Sender = deviceAddress;
    container.Command = 8;
    container.Data = *data;
    if (container.Data.count() == 0) container.Data.append(static_cast<char>(0));
    if (debugProtocolV1) container.Data[0] = static_cast<char>(container.Data[0] + (parametr << 6));
    else container.Data.prepend(parametr);
    snBUSOutput(container, this);
}

void QsnInterface::actionNumOutput(int numOutput)
{
    QByteArray Data;
    actionNumOutput(numOutput, Data);
}

void QsnInterface::actionNumOutput(int numOutput, QByteArray Data)
{
    if (numOutput > 126) return;
    quint16 signal;
    int memoryEnd = memory.count() - 1;
    for (int i = 0; i < memoryIOCount; i ++)
        if (static_cast<quint8>(memory.at(memoryEnd - i * 3)) == 127 + numOutput) {
            signal = QSNRAWtoUInt16(&memory, memoryEnd - i * 3 - 2);
            if (signal > 0) actionSendSignal(signal, &Data);
        }
}

void QsnInterface::actionDevMemoryReadRequest(quint16 addr, quint16 blockCount)
{
    taskDevice dmr;
    dmr.devAddress = addr;
    dmr.memory.resize(blockCount * 6);
    dmr.operations = taskDevice::read;
    dmr.blockPointer = 65535;
    dmr.waitTime = waitingTimeMO;
    dmr.attempts = attemptsOperations;
    tasks.append(dmr);

    emit eventProgressTask(0, blockCount);

    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = addr;
    container.Sender = deviceAddress;
    container.Command = 2;
    container.Data.append(static_cast<char>(dmr.blockPointer >> 8));
    container.Data.append(static_cast<char>(dmr.blockPointer & 255));
    emit snBUSOutput(container, this);
    if (waitTimer.isActive() == false) waitTimer.start(100);
}

void QsnInterface::actionDevMemoryWriteRequest(quint16 addr, QByteArray memory)
{
    taskDevice dmr;
    int memoryBlocks = QSNmemorySizeToBlockCount(memory.count());
    dmr.devAddress = addr;
    dmr.memory = memory;
    dmr.memory.resize(memoryBlocks * 6);
    dmr.operations = taskDevice::write;
    dmr.blockPointer = 65535;
    dmr.waitTime = waitingTimeMO;
    dmr.attempts = attemptsOperations;
    tasks.append(dmr);

    emit eventProgressTask(0, memoryBlocks);

    QSNContainer container = newContainer();
    container.role = QSNContainer::message;
    container.Address = addr;
    container.Sender = deviceAddress;
    container.Command = 4;
    container.Data.append(static_cast<char>(dmr.blockPointer >> 8));
    container.Data.append(static_cast<char>(dmr.blockPointer & 255));
    container.Data.append(static_cast<char>(memoryBlocks >> 8));
    container.Data.append(static_cast<char>(memoryBlocks & 255));
    emit snBUSOutput(container, this);
    if (waitTimer.isActive() == false) waitTimer.start(100);
}

void QsnInterface::actionDevMemoryAbortWriteRead()
{
    for (int i = tasks.count() - 1; i >= 0; i--)
        if (tasks[i].operations == taskDevice::read || tasks[i].operations == taskDevice::write) {
            removeTask(&tasks[i]);
        }
}

void QsnInterface::setAutoConnectEnable(bool enable)
{
    autoConnectEnable = enable;
}

void QsnInterface::setDebugProtocolV1(bool enable)
{
    debugProtocolV1 = enable;
}

void QsnInterface::setStartLoaded(bool enable)
{
    startLoaded = enable;
}

void QsnInterface::actionConnect()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 0;
    container.Signal = 0;
    container.Address = deviceAddress;
    emit snBUSOutput(container, this);
}

void QsnInterface::actionDisconnect()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 0;
    container.Signal = 1;
    emit snBUSOutput(container, this);
}

void QsnInterface::actionAutoConnect()
{
    if (autoConnectEnable) actionConnect();
}

QByteArray QsnInterface::vecOfString(QString sHex)
{
    QByteArray vec;
    if (sHex.count() == 0) return(vec);
    bool ok;
    char it;
    quint8 base = 16;
    if (sHex.at(0) == 'd')
    {
        base = 10;
        sHex.remove(0,1);
    }
    sHex = sHex.simplified();
    QStringList hList = sHex.split(" ");
    for (int i = 0; i < hList.count(); i++)
    {
        it = static_cast<char>(hList.value(i).toUInt(&ok,base));
        if (ok) vec.append(it);
    }
    return(vec);
}

QString QsnInterface::stringOfVec(QByteArray *Data, quint16 start, quint8 base, QString pref)
{
    QString str;
    if (Data->count() > start)
    {
        str = pref;
        for (quint16 i = start; i < Data->count(); i++)
            str += " " + QString::number(Data->at(i), base);
    }
    return(str.toUpper());
}

void QsnInterface::timerTimeOut()
{
    if (waitListFlag == true) return;
    waitListFlag = true;
    checkTaskTime();
    if (tasks.count() == 0) waitTimer.stop();
    waitListFlag = false;
}

void QsnInterface::secondOut()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 2;
    snBUSInput(container, this);
}

quint16 QsnInterface::getDeviceAddress()
{
    return deviceAddress;
}

void QsnInterface::saveSettings(QSettings *settings)
{
    settings->setValue(QLatin1String("Interface_Address"), deviceAddress);
    settings->setValue(QLatin1String("Interface_AutoConnect"), autoConnectEnable);
    settings->setValue(QLatin1String("Interface_DebugProtocolV1"), debugProtocolV1);
    settings->setValue(QLatin1String("Interface_StartLoaded"), startLoaded);
    settings->setValue(QLatin1String("Interface_WaitingTimeMO"), waitingTimeMO);
}

void QsnInterface::loadSettings(QSettings *settings)
{
    actionSetAddress(static_cast<quint16>(settings->value(QLatin1String("Interface_Address"), deviceAddress).toUInt()));
    autoConnectEnable = settings->value(QLatin1String("Interface_AutoConnect"), false).toBool();
    debugProtocolV1 = settings->value(QLatin1String("Interface_DebugProtocolV1"), false).toBool();
    startLoaded = settings->value(QLatin1String("Interface_StartLoaded"), false).toBool();
    waitingTimeMO = settings->value(QLatin1String("Interface_WaitingTimeMO"), waitingTimeMemoryOperations).toUInt();
}

void QsnInterface::saveSettingsStream(QDataStream *stream)
{
    *stream << deviceAddress;
    *stream << autoConnectEnable;
    *stream << debugProtocolV1;
    *stream << startLoaded;
    *stream << waitingTimeMO;
}

void QsnInterface::loadSettingsStream(QDataStream *stream)
{
    *stream >> deviceAddress;
    *stream >> autoConnectEnable;
    *stream >> debugProtocolV1;
    *stream >> startLoaded;
    *stream >> waitingTimeMO;
}

void QsnInterface::saveMemory()
{
    if (memoryFileName == QString()) return;
    saveMemory(memoryFileName);
}

void QsnInterface::saveMemory(QString filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream Out(&file);
        Out.setVersion(QDataStream::Qt_4_7);
        for(int i = 0; i < memory.size(); i ++)
            Out << static_cast<quint8>(memory.at(i));
        file.close();
    }
}

QByteArray QsnInterface::exportMemory()
{
    return memory;
}

QByteArray *QsnInterface::getMemory()
{
    return &memory;
}

void QsnInterface::loadMemory()
{
    QFile file(memoryFileName);
    if (file.exists()) {
        loadMemory(memoryFileName);
        emit eventMemoryLoaded();
    } else eventFailureMemoryLoad();
}

void QsnInterface::loadMemory(QString filePath)
{
    QFile file(filePath);
    int i = 0;
    quint8 p;
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        stream.setVersion(QDataStream::Qt_4_7);
        memory.resize(QSNmemorySizeToBlockCount(static_cast<int>(stream.device()->size())) * 6);
        while (!stream.atEnd()) {
            stream >> p;
            memory[i] = static_cast<char>(p);
            i ++;
        }
        file.close();
    }
}

void QsnInterface::loadMemoryFromByteArray(QByteArray *conf)
{
    int i = 0;
    quint8 p;
    QDataStream stream(conf, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_4_7);
    memory.resize(QSNmemorySizeToBlockCount(static_cast<int>(stream.device()->size())) * 6);
    while (!stream.atEnd()) {
        stream >> p;
        memory[i] = static_cast<char>(p);
        i ++;
    }
}

QSNSignal QsnInterface::containerToSignal(QSNContainer container)
{
    QSNSignal tmpSignal;
    tmpSignal.Signal = container.Signal;
    tmpSignal.Data = container.Data;
    return tmpSignal;
}

QSNMessage QsnInterface::containerToMessage(QSNContainer container)
{
    QSNMessage tmpMessage;
    tmpMessage.Address = container.Address;
    tmpMessage.Sender = container.Sender;
    tmpMessage.Command = container.Command;
    tmpMessage.Data = container.Data;
    return tmpMessage;
}

void QsnInterface::setMemoryFileName(QString filePath)
{
    memoryFileName = filePath;
    loadMemory();
}

void QsnInterface::setDeviceName(QString name)
{
    devName = name;
}

QString QsnInterface::deviceName()
{
    return devName;
}

void QsnInterface::setTimeOutMemoryOp(int time)
{
    waitingTimeMO = time;
}

int QsnInterface::timeOutMemoryOp()
{
    return waitingTimeMO;
}

void QsnInterface::setIOcount(quint16 count)
{
    memoryIOCount = count;
}

void QsnInterface::setMemorySize(quint16 size)
{
    memory.resize(size * 6);
}

void QsnInterface::setDeviceTypeIndex(quint16 type)
{
    deviceType = type;
}




