#ifndef QSNWEBITEMMESSAGE_H
#define QSNWEBITEMMESSAGE_H

#include <QObject>
#include <QDebug>
#include <QCryptographicHash>
#include "qsnweb.h"

class QsnWebItemMessage : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebItemMessage(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);
    void setText(QString stext);
    QString getText();
    void setLevel(quint8 level);
    quint8 getLevel();
    QString key();
    bool isThisKey(QString key);
    void writeSetting(QXmlStreamWriter *stream);
    void readSetting(QXmlStreamAttributes *attrib);

signals:
    void sendMessage(QString text);

private:
    QsnGlobalModules *mds;
    QString mtext;
    quint8 mlevel;
    QByteArray lshashe;

    QByteArray hashCalc(QString text);

};


#endif // QSNWEBITEMMESSAGE_H
