#ifndef QSNWEBPAGEUSERS_H
#define QSNWEBPAGEUSERS_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnwebauthorization.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

class QsnWebPageUsers : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageUsers(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageUsers();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);
    QString widgetState();

public slots:

private slots:

private:
    QsnGlobalModules *mds;
    qint64 AID;
    QString avatarBase64(QString name);
    QString userspToJSON();
};

#endif // QSNWEBPAGEUSERS_H
