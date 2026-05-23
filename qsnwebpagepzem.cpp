#include "qsnwebpagepzem.h"

QsnWebPagePZEM::QsnWebPagePZEM(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("PZEM"));
    widgetAccessRights = 2;
    widgetUrl = "/pzem";
    widgetIcon = "subicon-pzem";
    serial = new QSerialPort(this);
    isLog = false;
    stateText = QString();
    PZEMnoResp = false;

    //init
    reconnectTimer = 0;
    checkCount = 0;
    voltage = 0;
    current = 0;
    power = 0;
    energy = 0;
    frequency = 0;
    powerfactor = 1;

    sendPowerTimer = 0;
    sendCurrentTimer = 0;
    sendVoltageTimer = 0;

    //Energy send
    powersend_sendtime = QDateTime::currentDateTime();
    //Energy calculation
    countpower_power = 0;
    countpower_msec = 0;
    countpower_begin = powersend_sendtime.toMSecsSinceEpoch();
    countpower_count = 0;

    //Power state
    powerState = false;
    mds = modules;
    connect(serial, SIGNAL(readyRead()), this, SLOT(analysisCicle()));
    connect(serial, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(serialError(QSerialPort::SerialPortError)));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));

}

void QsnWebPagePZEM::getFunctions(QStringList *functions, int , QString )
{ 
    *functions << QString("$(\"#reset\").click(function(){"
                          "sendState('action', 'reset', 'reset');"
                          "}); ");
}

void QsnWebPagePZEM::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#energy').text(data.energy);");
    *fjson << QString(" $('#voltage').text(data.voltage);");
    *fjson << QString(" $('#current').text(data.current);");
    *fjson << QString(" $('#power').text(data.power);");
    *fjson << QString(" $('#frequency').text(data.frequency);");
    *fjson << QString(" $('#powerfactor').text(data.powerfactor);");
}

void QsnWebPagePZEM::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{  
    if (options->contains("action")) {
        QString state = options->value("action");
//        if (state == QLatin1String("action") && options->value("id", "") == "send")  sendNow();

        if (state == QLatin1String("action") && options->value("id", "") == "reset") counterReset();
    }

    *returnItems << QString("\"energy\": \"%1\"").arg(energy);
    *returnItems << QString("\"voltage\": \"%1\"").arg(voltage);
    *returnItems << QString("\"current\": \"%1\"").arg(current);
    *returnItems << QString("\"power\": \"%1\"").arg(power);
    *returnItems << QString("\"frequency\": \"%1\"").arg(frequency);
    *returnItems << QString("\"powerfactor\": \"%1\"").arg(powerfactor);
}

void QsnWebPagePZEM::getContents(QStringList *contents, int )
{ 
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();

    //        if (mds->auth->atAccount(accountIndex)->email.isEmpty())
    //            *contents << QsnBsAlertWarning(tr("The current user does not have an email address!"));

    *contents << QsnBsFormLabel(tr("Energy"), QString("%1 %2").arg(energy, 0, 'f', 3).arg(tr("kWh")), "energy");
    *contents << QsnBsFormLabel(tr("Voltage"), QString("%1 %2").arg(voltage ,0, 'f', 1).arg(tr("V")), "voltage");
    *contents << QsnBsFormLabel(tr("Current"), QString("%1 %2").arg(current ,0, 'f', 1).arg(tr("A")), "current");
    *contents << QsnBsFormLabel(tr("Power"), QString("%1 %2").arg(power ,0, 'f', 1).arg(tr("Wt")), "power");
    *contents << QsnBsFormLabel(tr("Frequency"), QString("%1 %2").arg(frequency ,0, 'f', 1).arg(tr("Hz")), "frequency");
    *contents << QsnBsFormLabel(tr("Power factor"), QString("%1 %2").arg(powerfactor ,0, 'f', 1).arg(tr("Wt")), "powerfactor");
    *contents << QsnBsFormLabel(tr("Port name"), serial->portName(), "port");
    *contents << QsnBsFormLabel(tr("State"), deviceState(), "state");

    *contents << QsnBsFormGroupButtonBegin();
//    *contents << QsnBsFormGroupButton(tr("Send now"), QLatin1String("send"), QString("btn-info"));
    *contents << QsnBsFormGroupButton(tr("Reset"), QLatin1String("reset"), QString("btn-warning"));
    *contents << QsnBsFormGroupButtonEnd();

    *contents << QsnBsFormEnd();
}

void QsnWebPagePZEM::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items

    dbName = optionsMap.value("YDB", "pzem").toString();
//    isLog = optionsMap.value("log", false).toBool();

    stateText = QString();
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->setPortName(optionsMap.value("PRT", QLatin1String("COM1")).toString());
    serialConnect();
}

QString QsnWebPagePZEM::widgetState()
{
    return QString("%1%2").arg(energy, 0, 'f', 3).arg(tr("kWh"));
}

void QsnWebPagePZEM::analysisCicle()
{
    QByteArray data = serial->readAll();
    parse(&data);
}

void QsnWebPagePZEM::serialError(QSerialPort::SerialPortError error)
{
    if (error == 0) stateText = tr("Port open");
    else {
        stateText = QSNGetSerialErrorName(error) + "!";
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       stateText,
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("pzem"),
                                       false), this);
        serialDisconnect();
        reconnectTimer = 3600;
    }
}

void QsnWebPagePZEM::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 2 && container.Signal == 0) {
        sendCmd8(CMD_RIR, 0x00, 0x0A);
        if (reconnectTimer) reconnectTimer --;
        else if (!serial->isOpen()) serialConnect();
        if (powersend_sendtime.secsTo(QDateTime::currentDateTime()) >= 59) sendEnergy();
        sendPowerTimer ++;
        sendCurrentTimer ++;
        sendVoltageTimer ++;
        if (sendPowerTimer >= 59) sendPower();
        if (sendVoltageTimer >= 59) sendVoltage();
        if (sendCurrentTimer >= 59) sendCurrent();
    }
}

void QsnWebPagePZEM::parse(QByteArray *data)
{
    if (data->count() < 18) return;
    quint16 crcp = (quint8)data->at(data->count() - 1);
    crcp = (crcp << 8) | (quint8)data->at(data->count() - 2);
    if (crcp != CRC16(data, 2)) return;

    // Update the current values
    setVoltage((static_cast<quint8>(data->at(3)) << 8 | static_cast<quint8>(data->at(4)))/10.0); // Raw voltage in 0.1V

    setCurrent((static_cast<quint8>(data->at(5)) << 8 | // Raw current in 0.001A
                static_cast<quint8>(data->at(6)) |
                static_cast<quint8>(data->at(7)) << 24 |
                static_cast<quint8>(data->at(8)) << 16) / 1000.0);

    setPower((static_cast<quint8>(data->at(9)) << 8 | // Raw power in 0.1W
              static_cast<quint8>(data->at(10)) |
              static_cast<quint8>(data->at(11)) << 24 |
              static_cast<quint8>(data->at(12)) << 16) / 10.0);

    energy = (static_cast<quint8>(data->at(13)) << 8 | // Raw Energy in 1Wh
              static_cast<quint8>(data->at(14)) |
              static_cast<quint8>(data->at(15)) << 24 |
              static_cast<quint8>(data->at(16)) << 16) / 1000.0;

    frequency = (static_cast<quint8>(data->at(17)) << 8 | // Raw Frequency in 0.1Hz
                 static_cast<quint8>(data->at(18))) / 10.0;

    setPF((static_cast<quint8>(data->at(19)) << 8 | /// Raw pf in 0.01
           static_cast<quint8>(data->at(20))) / 100.0);


    PZEMnoResp = false;
}

void QsnWebPagePZEM::sendCmd8(quint8 cmd, quint16 rAddr, quint16 val, quint16 slave_addr)
{

    if (!serial->isOpen()) return;

    if (PZEMnoResp && powerState) sendPowerFailure();
    else if (!PZEMnoResp && !powerState) sendPowerOn();

    QByteArray sendBuffer; // Send buffer
    if((slave_addr == 0xFFFF) || (slave_addr < 0x01) || (slave_addr > 0xF7)) slave_addr = PZEM_DEFAULT_ADDR;

    sendBuffer.append(slave_addr);                   // Set slave address
    sendBuffer.append(cmd);                     // Set command

    sendBuffer.append((rAddr >> 8) & 0xFF);     // Set high byte of register address
    sendBuffer.append((rAddr) & 0xFF);          // Set low byte =//=

    sendBuffer.append((val >> 8) & 0xFF);       // Set high byte of register value
    sendBuffer.append((val) & 0xFF);            // Set low byte =//=

    quint16 crc = CRC16(&sendBuffer); // CRC of data

    // Write high and low byte to last two positions
    sendBuffer.append(crc & 0xFF); // Low byte first
    sendBuffer.append((crc >> 8) & 0xFF); // High byte second

    //qDebug() << "send " << QByteArray(sendBuffer, 8).toHex();

    serial->write(sendBuffer);
    PZEMnoResp = true;
}

quint16 QsnWebPagePZEM::CRC16(QByteArray *data, int ld)
{
    quint8 nTemp; // CRC table index
    quint16 crc = 0xFFFF; // Default value
    int length = data->count() - ld;

    for (int i = 0; i < length; i ++) {
        nTemp = data->at(i) ^ crc;
        crc >>= 8;
        crc ^= crcTable[nTemp];
    }
    return crc;
}

QString QsnWebPagePZEM::deviceState()
{
    if (!stateText.isEmpty()) {
        if (reconnectTimer) return QString("%1, %2 %3 %4").arg(stateText, tr("reconnecting via")).arg(reconnectTimer).arg(tr("sec"));
        return stateText;
    }
    if (serial->isOpen()) {
        if (!powerState) return tr("Connected, NO POWER!");
        return tr("Connected");
    } else return tr("Disconnected");
}

void QsnWebPagePZEM::counterReset()
{
    if (!serial->isOpen()) return;
    QByteArray sendBuffer; // Send buffer

    sendBuffer.append(PZEM_DEFAULT_ADDR);       // Set slave address
    sendBuffer.append(CMD_REST);                     // Set command

    quint16 crc = CRC16(&sendBuffer); // CRC of data

    // Write high and low byte to last two positions
    sendBuffer.append(crc & 0xFF); // Low byte first
    sendBuffer.append((crc >> 8) & 0xFF); // High byte second

    serial->write(sendBuffer, 4);
    serial->flush();
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Counter reset"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("pzem"),
                                   false), this);
}

void QsnWebPagePZEM::serialDisconnect()
{
    if (!serial->isOpen()) return;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Closing the port"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("pzem"),
                                   false), this);
    serial->close();
}

void QsnWebPagePZEM::serialConnect()
{
    if (serial->isOpen()) return;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Closing the port"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("pzem"),
                                   false), this);
    if (serial->open(QIODevice::ReadWrite)) stateText.clear();
}

void QsnWebPagePZEM::setPower(float pw)
{
    if (power + 0.2 <= pw || power - 0.2 >= pw) sendPower();
    power = pw;
    qint64 newbegin = QDateTime::currentDateTime().toMSecsSinceEpoch();
    countpower_power = countpower_power + (pw * 1000);
    countpower_count ++;
    countpower_msec = countpower_msec + (newbegin - countpower_begin);
    countpower_begin = newbegin;
}

void QsnWebPagePZEM::setVoltage(float volt)
{
    if (voltage + 1 <= volt || voltage - 1 >= volt) sendVoltage();
    voltage = volt;
    QByteArray data;
    QSNByteToRAW(&data, 0, 12);
    QSNUInt32ToRAW(&data, 1, voltage * 1000);
    mds->db->writeRAWtoYBD(QString("%1_voltage").arg(dbName), &data, QDateTime::currentDateTime(), objectName(), this);
}

void QsnWebPagePZEM::setCurrent(float cur)
{
    if (current + 0.005 <= cur || current - 0.005 >= cur) sendCurrent();
    current = cur;
    QByteArray data;
    QSNByteToRAW(&data, 0, 11);
    QSNUInt32ToRAW(&data, 1, current * 1000);
    mds->db->writeRAWtoYBD(QString("%1_current").arg(dbName), &data, QDateTime::currentDateTime(), objectName(), this);
}

void QsnWebPagePZEM::setPF(float pf)
{
    powerfactor = pf;
    QByteArray data;
    QSNByteToRAW(&data, 0, 7);
    QSNUInt16ToRAW(&data, 1, pf * 100);
    mds->db->writeRAWtoYBD(QString("%1_powerfactor").arg(dbName), &data, QDateTime::currentDateTime(), objectName(), this);
}

void QsnWebPagePZEM::sendEnergy()
{
    QSNPower pw;
    pw.seconds = countpower_msec / 1000;
    if (countpower_count) pw.power = countpower_power / countpower_count;
    else pw.power = 0;
    pw.idp = 0;

    countpower_power = 0;
    countpower_msec = 0;
    countpower_count = 0;
    powersend_sendtime = QDateTime::currentDateTime();

    QByteArray data;
    QSNByteToRAW(&data, 0, 19);
    QSNPowerToRAW(&data, 1, pw);
    QSNUInt16ToRAW(&data, 6, static_cast<quint16>(mds->interface->getDeviceAddress()));
    mds->io->widgetReciveIndexSignal(2, &data, this);
}

void QsnWebPagePZEM::sendPowerFailure()
{
    powerState = false;
    QByteArray data;
    mds->io->widgetReciveIndexSignal(0, &data, this);
}

void QsnWebPagePZEM::sendPowerOn()
{
    powerState = true;
    QByteArray data;
    mds->io->widgetReciveIndexSignal(1, &data, this);
}

void QsnWebPagePZEM::sendPower()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 10);
    QSNUInt32ToRAW(&data, 1, power * 1000);
    mds->io->widgetReciveIndexSignal(5, &data, this);
    sendPowerTimer = 0;
}

void QsnWebPagePZEM::sendVoltage()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 12);
    QSNUInt32ToRAW(&data, 1, voltage * 1000);
    mds->io->widgetReciveIndexSignal(5, &data, this);
    sendVoltageTimer = 0;
}

void QsnWebPagePZEM::sendCurrent()
{
    QByteArray data;
    QSNByteToRAW(&data, 0, 11);
    QSNUInt32ToRAW(&data, 1, current * 1000);
    mds->io->widgetReciveIndexSignal(5, &data, this);
    sendCurrentTimer = 0;
}
