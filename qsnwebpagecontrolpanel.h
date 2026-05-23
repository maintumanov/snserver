#ifndef QSNWEBPAGECONTROLPANEL_H
#define QSNWEBPAGECONTROLPANEL_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"
#include "qsnimageconvert.h"

class QsnWebPageControlPanel : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageControlPanel(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void getContents(QStringList *contents, int accountIndex);
//    void addSetting(QString category, QString name, QsnWebWidget *wgt);
    void addSetting(QsnWeb *wgt);
    void clearSettings();
    void urlChanged(int accountIndex);


protected:

private:
    struct settingItem {
        QString name;
        QString url;
        QString icon;
        QString note;
        quint8 level;
        QString id;
        QString category;
        QsnWeb *wgt;
    };

    QList<settingItem> sitems;
    QsnGlobalModules *mds;
    qint64 versionID;
    QString bool2text(bool state);
    QString wStateToClass(char wstate);
};

#endif // QSNWEBPAGECONTROLPANEL_H
