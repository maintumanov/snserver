#ifndef QSNLOCATIONS_H
#define QSNLOCATIONS_H

#include <QObject>
#include <QDataStream>
#include <QDebug>

class QsnLocations : public QObject
{
    Q_OBJECT
public:
    struct itemDev {
        quint16 address;
        QString Name;
        QString Note;
    };

    struct itemLocation {
        QString name;
        QList<itemDev> dev;
    };

    explicit QsnLocations(QObject *parent = Q_NULLPTR);
    void loadFromStream(QDataStream *stream);
    QString location(quint16 address);
    QList<quint16> devAdresses();
    QString getLabelFromAddress(quint16 addr);
    QString getLabelDevFromIDP(quint16 idp);
    QString getLabelFromIDP(quint16 idp);
    QList<QString> getLocations();
    QList<quint16> devAdressesFromLocation(QString locationName);
    QString nameFromNote(QString note, int nname, quint16 IDP);

signals:


public slots:


private:
    QList<itemLocation> locations;

};

#endif // QSNLOCATIONS_H
