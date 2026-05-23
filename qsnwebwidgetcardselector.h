#ifndef QSNWEBWIDGETCARDSELECTOR_H
#define QSNWEBWIDGETCARDSELECTOR_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"
#include "qsnwebwidgetcardselectoritem.h"


class QsnWebWidgetCardSelector : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardSelector(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void actionRMCode(QByteArray code);
    void getItemJSON(QStringList *jsonItems);
    void fromStream(QDataStream *stream, QString path = QString());
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
     void widgetRunAction(int ioIndex);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

signals:
//    void changeStateSwitch(bool state, QString source);
//    void onRM(QByteArray code);

private:

    QsnGlobalModules *mds;

    int numIcon;
    quint8 selectItem;
    QList<QsnWebWidgetCardSelectorItem *> items;

    void setSelect(quint8 item, QString source);
//    void turnOff(QString source);
//    void turnSendSignal(bool state);
//    void turnCheck();
    void sendDB(quint8 item);
    void setLabelOnOff(QByteArray *data);
    void readFromMQTT(QByteArray *data);
    void writeToMQTTPSN(quint8 state);

    QString getLink();

};

#endif // QSNWEBWIDGETCARDSELECTOR_H
