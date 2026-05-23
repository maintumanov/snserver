#ifndef QSNWEBWIDGETSYSLOGCON_H
#define QSNWEBWIDGETSYSLOGCON_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QUdpSocket>
#include "qsnweb.h"
#include "qsnbsshapes.h"


class QsnWebPageSyslogConnector : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageSyslogConnector(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageSyslogConnector();
    void endConfiguration();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    QString widgetState();

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void sendToLog(quint8 wlevel, QString source, QString text);

private slots:

private:
    struct valueItem {
        quint8 type;
        QString value;
        QDateTime shtamp;
        bool issend;
    };

    bool connector_enable;

    bool connector_debug_enable;
    QUdpSocket serverSocket;
    QHostAddress syslogserver_address;
    quint16 syslogserver_port;

    quint32 send_count;

    QString MAC;
    QString Address;

    QString nameDevice;

    QsnGlobalModules *mds;
    QString msgName(quint8 type);


};

#endif // QSNWEBWIDGETSYSLOGCON_H
