#ifndef QSNREGISTRY_H
#define QSNREGISTRY_H

#include <QObject>
#include <QDateTime>
#include <QDebug>
#include "qsnshapes.h"
//#include "qsnwebwidget.h"
#include <QStringList>

class QsnRegistry : public QObject
{
    Q_OBJECT
public:
    explicit QsnRegistry(QObject *parent = Q_NULLPTR);
    QVariant value(QString key);

signals:
    // left-hand side
    void snBUSOutput(QSNContainer container, QObject *sender);

public slots:
    void clear();
    void setValue(QString key, QVariant value);

private:
    QMap <QString, QVariant> reg;
    void log(quint8 warningLevel, QString text, QString fname, QString title);
    void changes(QString key, QVariant vol);
};

#endif // QSNREGISTRY_H
