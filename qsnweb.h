#ifndef QSNWEB_H
#define QSNWEB_H

#include <QObject>
#include <QTextStream>
#include <QMap>
#include "qsnglobalmodules.h"

#define WL_CAUTION 0
#define WL_WARNING 1
#define WL_INFORMATION 2
#define WL_NOTE 3
#define WL_DEBUG 5

//class QsnGlobalModules;

class QsnWeb : public QObject
{
    Q_OBJECT
public:
    QVariant widgetData;
    QString widgetUrl;
    QString widgetAlias;
    QString widgetIcon;
    bool widgetMenu;
    quint8 widgetAccessRights;
    quint8 widgetIntegrated; //deprecated
    bool widgetColPage;

    explicit QsnWeb(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    quint32 itemID();
    void setInterface(QsnInterface *interface);
    QsnInterface *interface();
    void setModules(QsnGlobalModules *modules);
    QsnGlobalModules *modules();
    void treatmentUrl(QString url, int accountIndex = -1);
    bool isKey(QString key);
    QString getValue(QString key, QString defVol = QString());

    void log(quint8 warningLevel, QString text, QString fname, QString title);
    QString getPath(bool noName = false);
    void setPath(QString path);

    virtual QString widgetState();
    virtual char widgetNotifState();
    virtual void adapterDisconnect();
    virtual void adapterConnect();
    virtual void widgetRunAction(int ioIndex);
    virtual void getJavaScript(QStringList *script, int accountIndex);
    virtual void getJSDepending(QStringList *depending, int accountIndex);
    virtual void getCSSDepending(QStringList *depending, int accountIndex);
    virtual void getHTMLOnLoad(QStringList *functions, int accountIndex);
    virtual void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    virtual void getFunctionsAsItem(QStringList *functions, int accountIndex, QString pageURL = "");
    virtual void getChartFunctions(QStringList *functions, QStringList *depending, QStringList *argument, int accountIndex);
    virtual void getFunctionsJSON(QStringList *fjson, int accountIndex);
    virtual void getFunctionsJSONAsItem(QStringList *functions, int accountIndex, QString pageURL = "");
    virtual void getDialogs(QStringList *dialogs, int accountIndex);
    virtual void getDialogsAsItem(QStringList *dialogs, int accountIndex);
    virtual void getContents(QStringList *contents, int accountIndex);
    virtual void getContentsToolBar(QStringList *contents, int accountIndex);
    virtual void getContentsAsItem(QStringList *contents, int accountIndex);
    virtual void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    virtual void actionItemAsItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    virtual void getItemJSON(QStringList *jsonItem);
    virtual void getItemJSONAsItem(QStringList *jsonItem);
    virtual void receiveSignalIOIndex(int indexIO, QByteArray *data);
    virtual void endConfiguration();
    virtual void actionJSON(QByteArray *INjson, QByteArray *outJSON);
    virtual void actionRMCode(QByteArray code);

    QMap<QString, QVariant> getOptionsFromStream(QDataStream *stream);

protected:
    virtual void urlChanged(int accountIndex);


private:
    struct urlArgument {
        QString key;
        QString value;
    };

    QList<urlArgument> arguments;
    quint32 itemid;
    QString wPath;
    QsnGlobalModules *gmodules;

};


#endif //QSNWEB_H

