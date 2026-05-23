#include "qsnlocations.h"

QsnLocations::QsnLocations(QObject *parent) : QObject(parent)
{

}

void QsnLocations::loadFromStream(QDataStream *stream)
{
    locations.clear();
    itemLocation loc;
    int locationCount;
    int devCount;
    itemDev dev;
    *stream >> locationCount;
    for (int l = 0; l < locationCount; l ++) {
        loc.dev.clear();
        *stream >> loc.name;
        *stream >> devCount;
        for (int j = 0; j < devCount; j ++) {
            *stream >> dev.address;
            *stream >> dev.Name;
            *stream >> dev.Note;
            loc.dev.append(dev);
        }
        locations.append(loc);
    }
}

QString QsnLocations::location(quint16 address)
{
    for (int l = 0; l < locations.count(); l ++)
        for (int i = 0; i < locations[l].dev.count(); i ++)
            if (locations[l].dev[i].address == address) return locations[l].name;
    if (locations.count() == 1) return locations[0].name;
    return tr("The location is not defined");
}

QList<quint16> QsnLocations::devAdresses()
{
    QList<quint16> list;
    for (int l = 0; l < locations.count(); l ++)
        for (int i = 0; i < locations[l].dev.count(); i ++)
            list.append(locations[l].dev[i].address);
    return list;
}

QString QsnLocations::getLabelFromAddress(quint16 addr)
{
    for (int l = 0; l < locations.count(); l ++)
        for (int i = 0; i < locations[l].dev.count(); i ++)
            if (locations[l].dev[i].address == addr) {
                if (!locations[l].dev[i].Note.isEmpty()) return locations[l].dev[i].Note;
                return locations[l].dev[i].Name;
            }
    return QString();
}

QString QsnLocations::getLabelDevFromIDP(quint16 idp)
{
    return nameFromNote(getLabelFromAddress(idp & 0x0FFF), -1, idp);
}

QString QsnLocations::getLabelFromIDP(quint16 idp)
{
    return nameFromNote(getLabelFromAddress(idp & 0x0FFF), idp >> 12, idp);
}

QList<QString> QsnLocations::getLocations()
{
    QList<QString> list;
    for (int l = 0; l < locations.count(); l ++)
            list.append(locations[l].name);
    return list;
}

QList<quint16> QsnLocations::devAdressesFromLocation(QString locationName)
{
    QList<quint16> list;
    for (int l = 0; l < locations.count(); l ++)
        if (locations[l].name == locationName)
        for (int i = 0; i < locations[l].dev.count(); i ++)
            list.append(locations[l].dev[i].address);
    return list;
}

QString QsnLocations::nameFromNote(QString note, int nname, quint16 IDP)
{
    if (note.isEmpty()) return tr("unknown device [%1]").arg(IDP & 0x0FFF);
    int bnstr = note.indexOf('(');
    if (bnstr == -1) return note;
    int bnstp = note.indexOf(')');
    if (bnstp == -1) return note;
    QString name;
    if (bnstr == 0) name = note; else name = note.mid(0, bnstr);
    if (nname == -1) return name;
    QStringList namelist = note.mid(bnstr + 1, bnstp - bnstr - 1).split(',', QString::SkipEmptyParts);
    if (nname >= namelist.count() || nname < 0) return name;
    if (bnstr == 0) return namelist[nname].simplified();
    return name + " - " + namelist[nname].simplified();
}



