#include "qsnwebpagegsm.h"

QsnWebPageGSM::QsnWebPageGSM(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("GSM"));
    widgetUrl = "/gsm";
    widgetIcon = "subicon-gsm";
    mds->auth->addParametr("phoneNumber", tr("Phone"), QString());
    gsmSate = 0;
    timerStatusUpdate = 0;
    signalLevel = tr("no data");
    balance = tr("no data");
    devModel = tr("no data");
    signalLevel = tr("no data");
    devManufacturer = tr("no data");
    devIMEI = tr("no data");
    connect(&serial, SIGNAL(readyRead()), this, SLOT(gsmRead()));
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageGSM::~QsnWebPageGSM()
{

}

void QsnWebPageGSM::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#error').text(data.error);");
    *fjson << QString(" $('#signalLevel').text(data.signalLevel);");
    *fjson << QString(" $('#balance').text(data.balance);");
    *fjson << QString(" $('#devModel').text(data.devModel);");
    *fjson << QString(" $('#devManufacturer').text(data.devManufacturer);");
    *fjson << QString(" $('#devIMEI').text(data.devIMEI);");
}

void QsnWebPageGSM::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormLabel(tr("Error"), error, "error");
    *contents << QsnBsFormLabel(tr("Signal"), signalLevel, "signalLevel");
    *contents << QsnBsFormLabel(tr("Balance"), balance, "balance");
    *contents << QsnBsFormLabel(tr("Model"), devModel, "devModel");
    *contents << QsnBsFormLabel(tr("Manufacturer"), devManufacturer, "devManufacturer");
    *contents << QsnBsFormLabel("IMEI", devIMEI, "devIMEI");
    *contents << QsnBsFormEnd();
}

void QsnWebPageGSM::actionItem(QString , QMap<QString, QString> *, QStringList *returnItems, qint64 )
{
    *returnItems << QString("\"error\": \"%1\"").arg(error);
    *returnItems << QString("\"signalLevel\": \"%1\"").arg(signalLevel);
    *returnItems << QString("\"balance\": \"%1\"").arg(balance);
    *returnItems << QString("\"devModel\": \"%1\"").arg(devModel);
    *returnItems << QString("\"devManufacturer\": \"%1\"").arg(devManufacturer);
    *returnItems << QString("\"devIMEI\": \"%1\"").arg(devIMEI);
}

void QsnWebPageGSM::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);


    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
    }

    // set options

    balanceNumber = optionsMap.value("BL", "*105#").toString();

    error = QString();
    serial.setPortName(optionsMap.value("PR", QLatin1String("COM7")).toString());
    serial.setBaudRate(9600);
    if (!serial.open(QIODevice::ReadWrite)) {
        error = QString(tr("No open port, %1, %2")).arg(serial.portName(), serial.errorString());
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       error,
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("gsm"),
                                       false), this);
        return;
    } else requestInfo();
}

QString QsnWebPageGSM::widgetState()
{
    if (!error.isEmpty()) return error;
    if (!balance.isEmpty()) return balance;
    return tr("ok");
}

char QsnWebPageGSM::widgetNotifState()
{
    if (!error.isEmpty()) return 'c';
    if (!balance.isEmpty()) return 'w';
    return 'n';
}

void QsnWebPageGSM::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 3) sendmessage(container.info,
                                            QString::fromUtf8(container.Data),
                                            container.Signal,
                                            static_cast<quint8>(container.Sender));
    if (container.Command == 2) timeUpdate();
}

void QsnWebPageGSM::sendmessage(QString theme, QString text, quint16 type, quint16 mPriority, quint16 user)
{
    if (mPriority != Priority) return;
    if (!serial.isOpen()) {
        sendMessageBUS(theme, text, type, mPriority + 1, user);
        return;
    }
    if (user < 65535) {
        if (user >= mds->auth->countAccounts()) return;
        queueItem msg;
        msg.name = mds->auth->atAccount(user)->name;
        msg.theme = theme;
        msg.text = text;
        msg.user = user;
        msg.phone = mds->auth->parametrValue(mds->auth->atAccount(user), "phoneNumber");
        if (!mds->auth->checkAccountMSGType(user, type)) return;
        if (msg.phone.isEmpty()) sendMessageBUS(theme, text, type, mPriority + 1, user);
        else {
            gsmqueue.append(msg);
            checkMSG();
        }
    } else {
        queueItem msg;
        for (int i = 0; i < mds->auth->countAccounts(); i ++) {
            msg.name = mds->auth->atAccount(i)->name;
            msg.theme = theme;
            msg.text = text;
            msg.user = static_cast<quint16>(i);
            msg.phone = mds->auth->parametrValue(mds->auth->atAccount(i), "phoneNumber");
            if (!mds->auth->checkAccountMSGType(i, type)) continue;
            if (msg.phone.isEmpty()) sendMessageBUS(theme, text, type, mPriority + 1, static_cast<quint16>(i));
            else {
                gsmqueue.append(msg);
                checkMSG();
            }
        }
    }
}


void QsnWebPageGSM::gsmRead()
{
    QString response = QString::fromLocal8Bit(serial.readAll());

    while (response.indexOf("\n") != -1) response = response.remove("\n");
    while (response.indexOf("\r") != -1)response =  response.replace("\r", ";");
    QStringList list = response.split(";");

    for (int i = 0; i < list.count(); i ++) {
        if (list[i].isEmpty())  continue;
        if (list[i] == "OK" && gsmSate > 0) gsmNext();
        if (list[i] == "> " && gsmSate > 0) gsmNext();
        if (list[i].indexOf("ERROR") != -1 && gsmSate > 0) { gsmSate += 100; error = list[i]; }
        if (list[i].indexOf("+CMTI:") != -1 && gsmSate == 0) checkSMS(list[i]);
        if (list[i].indexOf("+CUSD:") != -1) processedUSD(list[i]);
        if (list[i].indexOf("+CMGL:") != -1)  { i ++; readSMS(list[i]);}
        if (list[i].indexOf("Manufacturer:") != -1) setManufacturer(list[i]);
        if (list[i].indexOf("Model:") != -1) setModel(list[i]);
        if (list[i].indexOf("IMEI:") != -1) setIMEI(list[i]);
        if (list[i].indexOf("+CSQ:") != -1) setSignalLevel(list[i]);
    }
}

void QsnWebPageGSM::gsm_Action(QString, QMap<QString, QString> *options, QStringList *)
{
    QString id;
    if (options->contains("id")) id = options->value("id");
}

void QsnWebPageGSM::sendMessageBUS(QString theme, QString text, quint16 type, quint16 mPriority, quint16 user)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 3;
    container.Signal = type;
    container.info = QString(tr("Notification"));
    container.Sender = mPriority;
    container.Address = user;
    QString msg = theme;
    if (!text.isEmpty())
        msg += text + QString(" [%1]").arg(QTime::currentTime().toString());
    container.Data = msg.toUtf8();
    mds->interface->snBUSInput(container, this);
}

// MAIL QUEUE

void QsnWebPageGSM::sendremove()
{
    if (gsmSate == 0) return;
    gsmqueue.removeFirst();
    gsmSate = 0;
    checkMSG();
}

void QsnWebPageGSM::senderror(QString info)
{
    sendMessageBUS(gsmqueue.first().theme, gsmqueue.first().text, gsmqueue.first().type, static_cast<quint16>(Priority + 1), gsmqueue.first().user);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString(tr("Error sending, %1, %2").arg(gsmqueue.first().name, info)),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("gsm"),
                                   false), this);
    sendremove();
}

void QsnWebPageGSM::readSMS(QString pdu)
{
    bool ok;
    quint8 DCS;
    QString txt = pdu;
    QString SenderPhone;
    //Remove smscentre number
    int l = txt.leftRef(2).toInt(&ok, 16);
    txt.remove(0, (l + 1) * 2);
    //Remove PDU type
    txt.remove(0, 2);
    //Read sender number
    l = txt.leftRef(2).toInt(&ok, 16);
    txt.remove(0, 4);
    SenderPhone = txt.left(l + 1);
    for (int i = 0; i < SenderPhone.count() - 1; i += 2) {
        QChar n = SenderPhone.at(i);
        SenderPhone[i] = SenderPhone[i + 1];
        SenderPhone[i + 1] = n;
    }
    SenderPhone.chop(1);
    txt.remove(0, l + 1); // remove sender number
    txt.remove(0, 2); // remove TP-PID
    DCS = static_cast<quint8>(txt.leftRef(2).toUInt(&ok, 16));
    txt.remove(0, 2); // remove TP-DCS
    txt.remove(0, 14); // remove time shtamp
    //Read message
    l = txt.leftRef(2).toInt(&ok, 16);
    txt.remove(0, 2);
    smsg m;
    m.phoneNumber = "+" + SenderPhone;
    if (DCS == 0x08) m.text = UCS2ToString(txt.left(l * 2));
    else m.text = DCS7BitToString(txt.left(l * 2), l);
    readedsms.append(m);
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString(tr("Read SMS, phone %1, text %2").arg(m.phoneNumber, m.text)),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString(),
                                   true), this);
    smsProcessed(m.phoneNumber, m.text);
}

void QsnWebPageGSM::requestInfo()
{
    if (gsmSate) return;
    gsmSate = 20;
    gsmNext();
}

void QsnWebPageGSM::requestBalance()
{
    if (gsmSate) return;
    gsmSate = 34;
    gsmNext();
}

void QsnWebPageGSM::checkSMS(QString text)
{
    if (!text.isEmpty()) {
        QString m = text;
        m.remove(0, 8);
        m.chop(3);
        smsMemoryName = m;
    }
    if (smsMemoryName.isEmpty()) gsmSate = 1;
    else gsmSate = 4;
    gsmNext();
}

void QsnWebPageGSM::checkMSG()
{
    if (gsmqueue.count() == 0) {
        checkSMS(QString());
        return;
    }
    checkQueue();
}

void QsnWebPageGSM::checkQueue()
{
    if (gsmSate != 0) return;
    if (gsmqueue.isEmpty()) return;
    getPDUPack(gsmqueue[0].phone, gsmqueue[0].theme + char(10) + char(13) + gsmqueue[0].text, &gsmPDU, &gsmPDUSize);
    gsmSate = 10;
    gsmNext();
}

void QsnWebPageGSM::processedUSD(QString usd)
{
    QString pdu = usd;
    int index  = pdu.indexOf("\"");
    pdu.remove(0, index + 1);
    index = pdu.indexOf("\"");
    pdu.remove(index, pdu.count() - index);
    getCost(UCS2ToString(pdu));
    checkMSG();
}

void QsnWebPageGSM::gsmNext()
{
    switch (gsmSate) {
    case 1:
        gsmRAWsend("AT+CMGL=4,1");
        gsmSate ++;
        break;
    case 2:
        gsmRAWsend("AT+CMGD=0,4");
        gsmSate ++;
        break;
    case 3:
        gsmSate = 0;
        checkQueue();
        break;
    case 4:
        gsmRAWsend(QString("AT+CPMS=\"%1\"").arg(smsMemoryName));
        gsmSate = 1;
        break;
    case 10:
        gsmRAWsend("AT+CMGF=0");
        gsmSate ++;
        break;
    case 11:
        gsmRAWsend(QString("AT+CMGS=%1").arg(gsmPDUSize));             // Отправляем длину PDU-пакета
        gsmSate ++;
        break;
    case 12:
        gsmRAWsendText(gsmPDU);
        timerStatusUpdate = 10;
        gsmSate ++;
        break;
    case 13:
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Send to %1 - %2").arg(gsmqueue[0].phone, gsmqueue[0].text),
                BUSSERV_LOG_CATEGORY_module,
                QString(),
                true), this);
        gsmSate = 0;
        gsmqueue.removeFirst();
        checkMSG();
        break;
    case 20:
        gsmRAWsend(QString("AT+CMGF=0"));
        gsmSate ++;
        break;
    case 21:
        gsmRAWsend(QString("AT+CSCS=\"UCS2\""));
        gsmSate ++;
        break;
    case 22:
        gsmRAWsend(QString("AT+CLIP=1"));
        gsmSate ++;
        break;
    case 23:
        gsmRAWsend(QString("ATI"));
        gsmSate ++;
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Successful initialization"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString(),
                                       true), this);
        break;
    case 24:
        gsmRAWsend(QString("AT+CSQ"));
        gsmSate ++;
        break;
    case 25:
        gsmRAWsend(QString("AT+CUSD=1,\"%1\",15").arg(balanceNumber));
        gsmSate ++;
        break;
    case 26:
        gsmSate = 0;
        timerStatusUpdate = 0;
        break;
        //errors
    case 111-113:
        senderror("send error");
        break;

    default: gsmSate = 0;

    }
}

void QsnWebPageGSM::gsmRAWsend(QString text)
{
    QByteArray data = text.toLocal8Bit();
    data.append(char(13));
    data.append(char(10));
    serial.write(data);
    serial.flush();
    timeOut = 20;
}

void QsnWebPageGSM::gsmRAWsendText(QString text)
{
    QByteArray data = text.toLocal8Bit();
    data.append(char(26));
    serial.write(data);
    serial.flush();
    timeOut = 20;
}

void QsnWebPageGSM::timeUpdate()
{
    if (timeOut) timeOut --;
    if (timeOut == 1 && gsmSate != 0) {
        gsmSate = 0;
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Timeout"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString(),
                                       true), this);
    }
    if (timerStatusUpdate == 0) return;
    if (timerStatusUpdate > 1) timerStatusUpdate --;
    if (gsmSate != 0 && timerStatusUpdate == 1) return;
    requestBalance();
    timerStatusUpdate = 0;
}

//utils

void QsnWebPageGSM::getPDUPack(QString phone, QString message, QString *result, int *PDUlen)
{
    // Поле SCA добавим в самом конце, после расчета длины PDU-пакета
    *result = "01";                                // Поле PDU-type - байт 00000001b
    *result += "00";                                // Поле MR (Message Reference)
    *result += getDAfield(&phone, true);             // Поле DA
    *result += "00";                                // Поле PID (Protocol Identifier)
    *result += "08";                                // Поле DCS (Data Coding Scheme)
    //*result += "";                                // Поле VP (Validity Period) - не используется

    QString msg = stringToUCS2(message);            // Конвертируем строку в UCS2-формат

    *result += QString::number(msg.count() / 2, 16);   // Поле UDL (User Data Length). Делим на 2, так как в UCS2-строке каждый закодированный символ представлен 2 байтами.
    *result += msg;

    *PDUlen = (*result).count() / 2;               // Получаем длину PDU-пакета без поля SCA
    *result = "00" + *result;                       // Добавляем поле SCA
}

QString QsnWebPageGSM::getDAfield(QString *phone, bool fullnum)
{
    QString result = "";
    for (int i = 0; i < phone->count(); i++) {
        if (phone->at(i) != '+')  result += phone->at(i);
    }
    int phonelen = result.count();
    if (phonelen % 2 != 0) result += "F";

    for (int i = 0; i < result.count(); i += 2) {
        QChar symbol = result[i + 1];
        result[i + 1] = result[i];
        result[i] = symbol;
    }

    result = fullnum ? "91" + result : "81" + result;
    if (phonelen < 16)  result = "0" + QString::number(phonelen, 16) + result;
    else result = QString::number(phonelen, 16) + result;
    return result;
}

QString QsnWebPageGSM::stringToUCS2(QString s)
{
    QString output;
    QByteArray b = s.toUtf8();
    for (int k = 0; k < b.length(); k++) {
        quint8 actualChar = static_cast<quint8>(b[k]);
        quint8 charSize = getCharSize(actualChar);

        QByteArray symbolBytes;
        symbolBytes.reserve(charSize);
        for (int i = 0; i < charSize; i++)  symbolBytes[i] = b[k + i];
        unsigned int charCode = symbolToUInt(&symbolBytes);

        if (charCode > 0)  {
            output += byteToHexString((charCode & 0xFF00) >> 8) +
                    byteToHexString(charCode & 0xFF);
        }
        k += charSize - 1;
    }
    return output;
}

quint8 QsnWebPageGSM::getCharSize(quint8 achar) {

    if (achar < 128) return 1;
    for (int i = 1; i <= 7; i++) {
        if (((achar << i) & 0xFF) >> 7 == 0) {
            return static_cast<quint8>(i);
        }
    }
    return 1;
}

unsigned int QsnWebPageGSM::symbolToUInt(QByteArray *bytes)
{
    unsigned int charSize = static_cast<unsigned int>(bytes->count());
    unsigned int result = 0;
    if (charSize == 1) {
        return static_cast<unsigned int>(bytes->at(0));
    }
    else  {
        unsigned char actualByte = static_cast<unsigned char>(bytes->at(0));
        result = actualByte & (0xFF >> (charSize + 1));
        result = result << (6 * (charSize - 1));

        for (unsigned int i = 1; i < charSize; i++) {
            actualByte = static_cast<unsigned char>(bytes->at(static_cast<int>(i)));

            if ((actualByte >> 6) != 2) return 0;
            result |= static_cast<unsigned int>((actualByte & 0x3F) << (6 * (charSize - 1 - i)));
        }
        return result;
    }
}

QString QsnWebPageGSM::byteToHexString(quint8 i)
{
    QString hex = QString::number(i, 16);
    if (hex.length() == 1) hex = "0" + hex;
    return hex.toUpper();
}

QString QsnWebPageGSM::UCS2ToString(QString s)
{
    QString result = "";
    for (int i = 0; i < s.count() - 3; i += 4) {       // Перебираем по 4 символа кодировки
        unsigned long code = (HexSymbolToChar(s[i]) << 12) +    // Получаем UNICODE-код символа из HEX представления
                (HexSymbolToChar(s[i + 1]) << 8) +
                (HexSymbolToChar(s[i + 2]) << 4) +
                (HexSymbolToChar(s[i + 3]));
        QByteArray c;
        if (code <= 0x7F) {                               // Теперь в соответствии с количеством байт формируем символ
            c.append(static_cast<char>(code));
            result += QString::fromUtf8(c);                                      // Не забываем про завершающий ноль
        } else if (code <= 0x7FF) {
            c.append(static_cast<char>(0xC0 | (code >> 6)));
            c.append(static_cast<char>(0x80 | (code & 0x3F)));
            result += QString::fromUtf8(c);
        } else if (code <= 0xFFFF) {
            c.append(static_cast<char>(0xE0 | (code >> 12)));
            c.append(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            c.append(static_cast<char>(0x80 | (code & 0x3F)));
            result += QString::fromUtf8(c);
        } else if (code <= 0x1FFFFF) {
            c.append(static_cast<char>(0xE0 | (code >> 18)));
            c.append(static_cast<char>(0xE0 | ((code >> 12) & 0x3F)));
            c.append(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            c.append(static_cast<char>(0x80 | (code & 0x3F)));
            result += QString::fromUtf8(c);
        }
    }
    return result;
}

QString QsnWebPageGSM::DCS7BitToString(QString s, int len)
{
    QByteArray c;
    QByteArray r;
    for (int i = 0; i < s.count() - 1; i += 2)       // Перебираем по 2 символа кодировки
        c.append(static_cast<char>((HexSymbolToChar(s[i]) << 4) +
                                   (HexSymbolToChar(s[i + 1]))));

    int l = 0;
    for (int i = 0; i < c.count(); i += 7) {
        r.append(c[i] & 127);
        if (++ l >= len) break;
        r.append(((c[i + 1] << 1) & 126) | ((c[i + 0] >> 7) & 1));
        if (++ l >= len) break;
        r.append(((c[i + 2] << 2) & 124) | ((c[i + 1] >> 6) & 3));
        if (++ l >= len) break;
        r.append(((c[i + 3] << 3) & 120) | ((c[i + 2] >> 5) & 7));
        if (++ l >= len) break;
        r.append(((c[i + 4] << 4) & 112) | ((c[i + 3] >> 4) & 15));
        if (++ l >= len) break;
        r.append(((c[i + 5] << 5) & 96) | ((c[i + 4] >> 3) & 31));
        if (++ l >= len) break;
        r.append(((c[i + 6] << 6) & 64) | ((c[i + 5] >> 2) & 63));
        if (++ l >= len) break;
        r.append(((c[i + 6] >> 1) & 127));
    }

    return QString::fromLocal8Bit(r);
}

unsigned int QsnWebPageGSM::HexSymbolToChar(QChar ch)
{
    bool ok;
    return QString(ch).toUInt(&ok, 16);
}

void QsnWebPageGSM::getCost(QString text)
{
    QString result;
    bool flag = false;
    result.replace(",", ".");                          // Если в качестве разделителя десятичных используется запятая - меняем её на точку.
    for (int i = 0; i < text.length(); i++) {
        if (text[i].isNumber() || (text[i] == '.' && flag)) { // Если начинается группа цифр (при этом, на точку без цифр не обращаем внимания),
            result += text[i];                           // начинаем собирать их вместе
            if (!flag) flag = true;                     // Выставляем флаг, который указывает на то, что сборка числа началась.
        }
        else {                                        // Если цифры закончились и флаг говорит о том, что сборка уже была,
            if (flag) break;                            // считаем, что все.
        }
    }
    balance = result;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Current balance %1").arg(balance),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("gsm"),
                                   false), this);
}

void QsnWebPageGSM::setManufacturer(QString text)
{
    devManufacturer = text;
    devManufacturer.remove("Manufacturer: ");
}

void QsnWebPageGSM::setModel(QString text)
{
    devModel = text;
    devModel.remove("Model: ");
}

void QsnWebPageGSM::setIMEI(QString text)
{
    devIMEI = text;
    devIMEI.remove("IMEI: ");
}

void QsnWebPageGSM::setSignalLevel(QString text)
{
    signalLevel = text;
    signalLevel.remove("+CSQ: ");
    signalLevel.chop(signalLevel.count() - signalLevel.indexOf(','));
    bool ok;
    int l = signalLevel.toInt(&ok);
    if (!ok) signalLevel.clear();
    signalLevel = QString::number(static_cast<qreal>(l) / 31 * 100, 'f', 0) + "%";
    if (l == 99) signalLevel = tr("no signal");
}

//menu
void QsnWebPageGSM::smsProcessed(QString phone, QString smsText)
{
    bool e = false;
    for (int i = 0; i < modules()->auth->countAccounts(); i ++)
        if (mds->auth->parametrValue(mds->auth->atAccount(i), "phoneNumber") == phone) e = true;
    if (!e) return;
    if (smsText.indexOf("Info", 0, Qt::CaseInsensitive) != -1
            || smsText.indexOf(tr("Info"), 0, Qt::CaseInsensitive) != -1) sendInfo(phone);
    if (smsText.indexOf("Cancel", 0, Qt::CaseInsensitive) == 0
            || smsText.indexOf(tr("Cancel"), 0, Qt::CaseInsensitive) == 0) actionCancelAlert();
    if (smsText.indexOf("Arming", 0, Qt::CaseInsensitive) == 0
            || smsText.indexOf(tr("Arming"), 0, Qt::CaseInsensitive) == 0) actionSecurityArming();
    if (smsText.indexOf("ArmingS", 0, Qt::CaseInsensitive) == 0
            || smsText.indexOf(tr("ArmingS"), 0, Qt::CaseInsensitive) == 0) actionSecurityArmingSecretly();
    if (smsText.indexOf("Disarm", 0, Qt::CaseInsensitive) == 0
            || smsText.indexOf(tr("Disarm"), 0, Qt::CaseInsensitive) == 0) actionSecurityDisarming();
}

void QsnWebPageGSM::sendInfo(QString phone)
{
    queueItem msg;
    msg.theme = tr("Info:");
    QString txt;
    txt.append(QString(tr("Balance: %1")).arg(balance));
    //    if (mds->alert_state != QLatin1String("none")) {
    //        txt.append(QString(tr("Warning: %1")).arg(mds->alert_text));
    //    }
    //    if (mds->alarm_state != QLatin1String("none")) {
    //        txt.append(QString(tr("Security: %1")).arg(mds->alarm_text));
    //    }
    msg.text = txt;
    msg.phone = phone;
    gsmqueue.append(msg);
}

void QsnWebPageGSM::actionCancelAlert()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 8;
    container.Signal = 0;
    container.Sender = 0;
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageGSM::actionSecurityArming()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 9;
    container.Signal = 1;
    container.Sender = 0;
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageGSM::actionSecurityArmingSecretly()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 9;
    container.Signal = 2;
    container.Sender = 0;
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageGSM::actionSecurityDisarming()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 9;
    container.Signal = 3;
    container.Sender = 0;
    mds->interface->snBUSInput(container, this);
}


