#include "qsnweb.h"


QsnWeb::QsnWeb(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QObject(parent)
{
    itemid = iID;
    gmodules = modules;
    widgetAccessRights = 1;
    wPath = QString();
    widgetAlias = QString();
    widgetIcon = QString();
    widgetIntegrated = 0;
    widgetMenu = false;
    widgetColPage = false;
    widgetData = QVariant();
}

quint32 QsnWeb::itemID()
{
    return itemid;
}

void QsnWeb::setInterface(QsnInterface *interface)
{
    gmodules->interface = interface;
}

QsnInterface *QsnWeb::interface()
{
    return gmodules->interface;
}

void QsnWeb::setModules(QsnGlobalModules *modules)
{
    gmodules = modules;
}

QsnGlobalModules *QsnWeb::modules()
{
    return gmodules;
}

void QsnWeb::treatmentUrl(QString url, int accountIndex)
{
    arguments.clear();
    QString u = url + '&';
    urlArgument a;
    bool iskey = true;
    int i = u.indexOf('?', 0);
    if (i != -1) {
        i ++;
        while (i < u.count()) {
            if (u[i] == '=') iskey = false;
            else if (u[i] == '&') {
                iskey = true;
                if (!a.key.isEmpty()) arguments.append(a);
                a.key.clear();
                a.value.clear();
            } else if (iskey) a.key += u[i];
            else a.value += u[i];
            i ++;
        }
    }
    urlChanged(accountIndex);
}

bool QsnWeb::isKey(QString key)
{
    QListIterator<urlArgument> i(arguments);
    while (i.hasNext()) if (i.next().key == key) return true;
    return false;
}

QString QsnWeb::getValue(QString key, QString defVol)
{
    QListIterator<urlArgument> i(arguments);
    urlArgument a;
    while (i.hasNext()) {
        a = i.next();
        if (a.key == key) return a.value;
    }
    return QString(defVol);
}

void QsnWeb::log(quint8 warningLevel, QString text, QString fname, QString title)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::information;
    container.Command = warningLevel;
    container.Address = 16;
    container.Sender = 0;
    container.info = text;
    container.Data = QString("%1/%2").arg(fname, title).toUtf8();
    container.Signal = 0;
    gmodules->interface->snBUSInput(container, this);
}

QString QsnWeb::getPath(bool noName)
{
    if (noName) return wPath;
    if (wPath.isEmpty()) return objectName();
    return wPath + '.' + objectName();
}

void QsnWeb::setPath(QString path)
{
    wPath = path;
}

void QsnWeb::adapterDisconnect()
{

}

void QsnWeb::adapterConnect()
{

}

void QsnWeb::widgetRunAction(int )
{

}

QString QsnWeb::widgetState()
{
    return QString();
}

char QsnWeb::widgetNotifState()
{
    return 'n';
}

void QsnWeb::getJavaScript(QStringList *, int )
{

}

void QsnWeb::getJSDepending(QStringList *, int)
{

}

void QsnWeb::getCSSDepending(QStringList *, int )
{

}

void QsnWeb::getHTMLOnLoad(QStringList *, int )
{

}

void QsnWeb::getFunctions(QStringList *, int , QString)
{

}

void QsnWeb::getFunctionsAsItem(QStringList *, int, QString )
{

}

void QsnWeb::getChartFunctions(QStringList *, QStringList *, QStringList *, int)
{

}

void QsnWeb::getFunctionsJSON(QStringList *, int)
{

}

void QsnWeb::getFunctionsJSONAsItem(QStringList *, int, QString )
{

}

void QsnWeb::getDialogs(QStringList *, int)
{

}

void QsnWeb::getDialogsAsItem(QStringList *, int)
{

}

void QsnWeb::getContents(QStringList *, int)
{

}

void QsnWeb::getContentsToolBar(QStringList *, int )
{

}

void QsnWeb::getContentsAsItem(QStringList *, int)
{

}

void QsnWeb::actionItem(QString, QMap<QString, QString> *, QStringList *, qint64 )
{

}

void QsnWeb::actionItemAsItem(QString , QMap<QString, QString> *, QStringList *, qint64)
{

}

void QsnWeb::getItemJSON(QStringList *)
{

}

void QsnWeb::actionJSON(QByteArray *, QByteArray *)
{

}

void QsnWeb::actionRMCode(QByteArray )
{

}

void QsnWeb::getItemJSONAsItem(QStringList *)
{

}


void QsnWeb::receiveSignalIOIndex(int , QByteArray *)
{

}

QMap<QString, QVariant> QsnWeb::getOptionsFromStream(QDataStream *stream)
{
    int optionsCount;
    QMap<QString, QVariant> optionsMap;
    QString opName;
    QVariant opValue;
    *stream >> optionsCount;
    for (int i = 0; i < optionsCount; i ++) {
        *stream >> opName;
        opValue.load(*stream);
        optionsMap.insert(opName, opValue);
    }
    return optionsMap;
}

void QsnWeb::urlChanged(int )
{

}

void QsnWeb::endConfiguration()
{

}

