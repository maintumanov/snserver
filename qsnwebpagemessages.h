#ifndef QSNWEBPAGEMESSAGES_H
#define QSNWEBPAGEMESSAGES_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "qsnweb.h"
#include "qsnwebitemmessage.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

class QsnWebPageMessages : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageMessages(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageMessages();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);

public slots:
    //----BM
    void sendMessage(QString text);

private slots:

signals:
    void snBUSOutput(QSNContainer container, QObject *sender);

private:
    QList<QsnWebItemMessage*> messages;
    QsnGlobalModules *mds;

    //html parametrs
    bool isMessageEdit;
    int itemMessageEdit;

    void saveSetting();
    void loadSetting();
    int IDtoItem(quint32 id);

};

#endif // QSNWEBPAGEMESSAGES_H
