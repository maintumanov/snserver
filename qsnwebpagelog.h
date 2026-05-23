#ifndef QSNWEBPAGELOG_H
#define QSNWEBPAGELOG_H

#include <QObject>
#include "qsnweb.h"
#include "qsnbsshapes.h"

class QsnWebPageLog : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageLog(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);

    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int);
    QString getListItem(QString fname, QDateTime *date);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void urlChanged(int accountIndex);
    qint64 logFileSize();
    QString logDifferenceJson(quint64 start);

public slots:
    void snBUSInput(QSNContainer container, QObject *sender);

private:
    QsnGlobalModules *mds;
    QString logname;

    struct QLF {
        QString content;
        QDateTime date;
    };

    struct QPairSecondComparer
    {
        template<typename T1, typename T2>
        bool operator()(const QPair<T1,T2> &a, const QPair<T1,T2> &b) const
        {
            return a.second > b.second;
        }
    };


};

#endif // QSNWEBPAGELOG_H
