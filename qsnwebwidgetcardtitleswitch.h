#ifndef QSNWEBWIDGETCARDTITLESWITCH_H
#define QSNWEBWIDGETCARDTITLESWITCH_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

class QsnWebWidgetCardTitleSwitch : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardTitleSwitch(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getItemJSON(QStringList *jsonItems);
    void getContents(QStringList *contents, int accountIndex);
    void fromStream(QDataStream *stream, QString path = QString());


public slots:
    //----BM

private:

    QsnGlobalModules *mds;

};

#endif // QSNWEBWIDGETCARDTITLESWITCH_H
