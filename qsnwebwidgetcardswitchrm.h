#ifndef QSNWEBWIDGETCCARDSWITCHRM_H
#define QSNWEBWIDGETCCARDSWITCHRM_H

#include <QObject>
#include <QDebug>
#include "qsnbsshapes.h"
#include "qsnweb.h"

class QsnWebWidgetCardSwitchRM : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebWidgetCardSwitchRM(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    void fromStream(QDataStream *stream);

public slots:
    void onRM(QByteArray code);

signals:
    void onStateChange(bool state, QString source);
    void onToggle(QString source);

private:
    QsnGlobalModules *mds;
    QByteArray rmOn;
    QByteArray rmOff;
    QByteArray rmSwitch;


};

#endif // QSNWEBWIDGETCCARDSWITCHRM_H
