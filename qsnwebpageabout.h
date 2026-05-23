#ifndef QSNWEBPAGEABOUT_H
#define QSNWEBPAGEABOUT_H

#include <QObject>
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebPageAbout : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageAbout(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream);
    QString widgetState();

signals:
    void reload();


private:
    QsnGlobalModules *mds;
    QString workTime();
    QString statToJSON();

};

#endif // QSNWEBPAGEABOUT_H
