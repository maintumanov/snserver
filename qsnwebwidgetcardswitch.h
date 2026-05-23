#ifndef QSNWEBWIDGETCARDSWITCH_H
#define QSNWEBWIDGETCARDSWITCH_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"
#include "qsnwebwidgetcardswitchmqtt.h"
#include "qsnwebwidgetcardswitchrm.h"

class QsnWebWidgetCardSwitch : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardSwitch(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void actionRMCode(QByteArray code);
    void getItemJSON(QStringList *jsonItems);
    void setState(bool state);
    void fromStream(QDataStream *stream, QString path = QString());
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void widgetRunAction(int ioIndex);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void setStateSwitch(bool state, QString source);
    void setToggle(QString source);

signals:
    void changeStateSwitch(bool state, QString source);
    void onRM(QByteArray code);

private:
    bool filldb;
    bool switchState;
    bool confirmedEnable;
    QsnGlobalModules *mds;
    QString alias;
    QString noticeLabel;
    QString dbName;
    int fdbTimer;
    int numIcon;
    int confirmedWait;

    QsnWebWidgetCardSwitchMQTT *mqtt;
    QsnWebWidgetCardSwitchRM *rm;

    QString textState();
    QString textLabel();
    void turnOn(QString source);
    void turnOff(QString source);
    void turnSendSignal(bool state);
    void timeProcessing();
    void sendDB(bool state);
    void setLabelOnOff(QByteArray *data);
    void readFromMQTT(QByteArray *data);
    void writeToMQTTPSN(bool state);

    QString getLabel();
    QString getLink();

};

#endif // QSNWEBWIDGETCARDSWITCH_H
