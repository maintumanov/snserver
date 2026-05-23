#ifndef QSNWEBWIDGETCARDBUTTON_H
#define QSNWEBWIDGETCARDBUTTON_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"


class QsnWebWidgetCardButton : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardButton(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void actionRMCode(QByteArray code);
    void fromStream(QDataStream *stream, QString path = QString());
    void widgetRunAction(int ioIndex);

public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);


private:

    QsnGlobalModules *mds;
    QString btnname;
    QString dbName;

    int numIcon;

    void buttonPress(QString source);
    void writeToMQTTPSN();


};

#endif // QSNWEBWIDGETCARDBUTTON_H
