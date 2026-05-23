#ifndef QSNWEBPAGENARODMON_H
#define QSNWEBPAGENARODMON_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QUdpSocket>
#include <QDnsLookup>
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebPageNarodmon : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageNarodmon(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageNarodmon();
    void endConfiguration();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    QString widgetState();
    char widgetNotifState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);

private slots:
    void lookupFinished();

private:
    struct valueItem {
        quint8 type;
        QString value;
        QDateTime shtamp;
        bool issend;
    };

    QUdpSocket *clientSocket;
    QList<valueItem> values;
    QDateTime lastSendTime;

    QString MAC;
    QString Address;
    quint16 Port;
    QString nameDevice;
    QDnsLookup dnslookup;
    QHostAddress ipaddress;

    QsnGlobalModules *mds;

    QString sendToJSON();
    void sendToNarodmon();

    void addValue(QByteArray *data);
    int indexValueFromType(quint8 type);
    QString nmTypeFromType(quint8 type);
    void configUpdateBegin();
    void configUpdateEnd();
};

#endif // QsnWebWidgetNarodmon_H
