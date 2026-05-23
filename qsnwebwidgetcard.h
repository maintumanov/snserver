#ifndef QSNWEBWIDGETCARD_H
#define QSNWEBWIDGETCARD_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"
#include "qsnwebwidgetcardswitch.h"
#include "qsnwebwidgetcardindicator.h"
#include "qsnwebwidgetcardtitle.h"
#include "qsnwebwidgetcardtitleswitch.h"
#include "qsnwebwidgetrmcodeinput.h"
#include "qsnwebwidgetcardthermostatcontrol.h"
#include "qsnwebwidgetcardtwobuttons.h"
#include "qsnwebwidgetcardbutton.h"
#include "qsnwebwidgetcardselector.h"

class QsnWebWidgetCard : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCard(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getJavaScript(QStringList *script, int accountIndex);
    void getJSDepending(QStringList *depending, int accountIndex);
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getDialogs(QStringList *dialogs, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void actionRMCode(QByteArray code);
    void fromStream(QDataStream *stream, QString path = "");
    void getItemJSON(QStringList *jsonItems);

public slots:
    void readRM(QByteArray code);

protected:

private:
    QList<QsnWeb *> widgets;

    QsnGlobalModules *mds;
    quint8 paneltype;

    QsnWeb  *titleWidget;
    QsnWebWidgetRMCodeInput *rm;

};

#endif // QSNWEBWIDGETCARD_H
