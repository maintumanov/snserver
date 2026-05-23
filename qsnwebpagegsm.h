#ifndef QSNWEBPAGEGSM_H
#define QSNWEBPAGEGSM_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include <QtSerialPort/QSerialPort>
#include "qsnbsshapes.h"

class QsnWebPageGSM : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageGSM(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageGSM();
    void getFunctionsJSON(QStringList *fjson, int);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    QString widgetState();
    char widgetNotifState();


public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    void sendmessage(QString theme, QString text, quint16 type, quint16 mPriority, quint16 user = 65535);
    //mail queue
    void gsmRead();

private:
    QsnGlobalModules *mds;
    int Priority;

    void gsm_Action(QString, QMap<QString, QString> *options, QStringList *returnData);

    // MAIL QUEUE
    struct queueItem {
        QString name;
        QString theme;
        QString text;
        QString phone;
        quint16 type;
        quint16 user;
    };

    QList<queueItem> gsmqueue;

    struct smsg {
        QString text;
        QString phoneNumber;
    };

    QList<smsg> readedsms;
    QSerialPort serial;
    int gsmSate;
    QString gsmPDU;
    int gsmPDUSize;
    QString balanceNumber;
    QString smsMemoryName;
    //info
    QString error;
    QString balance;
    QString devModel;
    QString devManufacturer;
    QString devIMEI;
    QString signalLevel;
    int timerStatusUpdate;
    int timeOut;

    void sendMessageBUS(QString theme, QString text, quint16 type, quint16 mPriority, quint16 user = 65535);
    void sendremove();
    void senderror(QString log);
    void readAnswer(QString *answer);
    void sendRequest(QString request);

    void readSMS(QString pdu);
    void sendMSG(QString phone, QString text);
    void requestInfo();
    void requestBalance();
    void checkSMS(QString text);
    void checkMSG();
    void checkQueue();
    void processedUSD(QString usd);
    void checkCMD();
    void gsmNext();
    void gsmRAWsend(QString text);
    void gsmRAWsendText(QString text);
    void timeUpdate();

    //utils

    void getPDUPack(QString phone, QString message, QString *result, int *PDUlen);
    QString getDAfield(QString *phone, bool fullnum);
    QString stringToUCS2(QString s);
    quint8 getCharSize(quint8 achar);
    unsigned int symbolToUInt(QByteArray *bytes);
    QString byteToHexString(quint8 i);
    QString UCS2ToString(QString s);
    QString DCS7BitToString(QString s, int len);
    unsigned int HexSymbolToChar(QChar ch);
    void getCost(QString text);
    void setManufacturer(QString text);
    void setModel(QString text);
    void setIMEI(QString text);
    void setSignalLevel(QString text);


    //menu
    void smsProcessed(QString phone, QString smsText);
    void sendInfo(QString phone);
    void actionCancelAlert();
    void actionSecurityArming();
    void actionSecurityArmingSecretly();
    void actionSecurityDisarming();
};

#endif // QSNWEBPAGEGSM_H
