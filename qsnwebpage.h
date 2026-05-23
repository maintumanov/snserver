#ifndef QSNWEBPAGE_H
#define QSNWEBPAGE_H

#include <QObject>
#include <QDebug>
#include "qsnweb.h"
#include "qsnwebwidgetcard.h"
#include "qsnwebwidgetrmcodeinput.h"

class QsnWebPage : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPage(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void getJavaScript(QStringList *script, int accountIndex);
    void getJSDepending(QStringList *depending, int accountIndex);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getChartFunctions(QStringList *functions, QStringList *depending, QStringList *argument, int accountIndex);
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getDialogs(QStringList *dialogs, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void addWidget(QsnWeb *widget);
    void fromStream(QDataStream *stream);
    void urlChanged(int accountIndex);

public slots:
    void readRM(QByteArray code);

protected:

private:
    QList<QsnWeb *> widgets;
    QsnGlobalModules *mds;
    qint64 versionID;
    quint8 iconNum;
    bool wEnable;
    QsnWebWidgetRMCodeInput *rm;
};

#endif // QSNWEBPAGE_H
