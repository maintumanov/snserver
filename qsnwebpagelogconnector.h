/***************************
1 - smBUS
2 - Adapters
3 - Pages
4 - Widgets
****************************/
#ifndef QSNWEBWIDGETDEBUGCON_H
#define QSNWEBWIDGETDEBUGCON_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QUdpSocket>
#include "qsnshapes.h"
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebPageLogConnector : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageLogConnector(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageLogConnector();
    void endConfiguration();
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    QString widgetState();
    char widgetNotifState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void sendToLog(QString text);

private slots:
    void readyRead();

private:
    struct valueItem {
        quint8 type;
        QString value;
        QDateTime shtamp;
        bool issend;
    };

    QUdpSocket *serverSocket;
    QHostAddress loger_address;

    QString MAC;
    QString Address;
    quint16 Port;
    QString nameDevice;
    quint8 loger_level;

    QsnGlobalModules *mds;

    void answerNetworkMode();
    QString msgName(quint8 type);
    QString getChannelsNames();

};

#endif // QSNWEBWIDGETDEBUGCON_H
