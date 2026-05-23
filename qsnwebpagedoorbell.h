#ifndef QSNWEBPAGEDOORBELL_H
#define QSNWEBPAGEDOORBELL_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QSound>
#include "qsnweb.h"
#include "qsnimageconvert.h"
#include "qsnbsshapes.h"

class QsnWebPageDoorBell : public QsnWeb
{
    Q_OBJECT
public:
    QsnWebPageDoorBell(quint32 iID, QsnGlobalModules *modules, QObject *parent = Q_NULLPTR);
    ~QsnWebPageDoorBell();
    void getFunctions(QStringList *functions, int accountIndex, QString pageURL = "");
    void getFunctionsJSON(QStringList *fjson, int accountIndex);
    void getContents(QStringList *contents, int accountIndex);
    void actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid);
    void fromStream(QDataStream *stream);
    void receiveSignalIOIndex(int indexIO, QByteArray *data);


public slots:
    //----BM
    void snBUSInput(QSNContainer container, QObject *sender = Q_NULLPTR);
    void sendMessage(QString theme, QString text);

private slots:

signals:
    void snBUSOutput(QSNContainer container, QObject *sender);

private:
    QsnGlobalModules *mds;
    QSound *sound;
    QString soundfilename;
    QString soundfilenamenight;
    int count;
    int countNight;
    bool isNight;
    QDateTime lastBell;
    int countCalls;
    int countCallsNight;

    void playBell();
    QString getFileNameFromSoundName(QString name);
    QString lastBellDate();
    QString switchNightState();
    bool isNewBell();
    void globalBell();
    bool nightBell();


};

#endif // QSNWEBPAGEDOORBELL_H
