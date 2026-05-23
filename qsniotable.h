#ifndef QSNSIGNALTABLE_H
#define QSNSIGNALTABLE_H

#include <QObject>
#include "qsninterface.h"

class QsnWeb;

class QsnIOTable : public QObject
{
    Q_OBJECT
public:
    struct ioItem {
        int index;
        int indexItemIO;
        QsnWeb *widget;
    };

    struct pageWidgetLink {
        QString url;
        QsnWeb *webwidget;
    };

    struct signalItem {
        int index;
        quint16 signal;
    };

    struct actionItem {
        QString name;
        int iconIndex;
        int ioIndex;
        quint32 id;
        QsnWeb *webwidget;
    };

    QList<actionItem> actions;

    explicit QsnIOTable(QsnInterface *snInterface, QObject *parent = Q_NULLPTR);
    void registrationInput(int index, QsnWeb *widget);
    void registrationOutput(int index, QsnWeb *widget);
    void registrationSignal(int index, quint16 signal);
    void clearIOS();
    void loadIOFromStream(QDataStream *stream, QsnWeb *widget);
    void loadSignalsStream(QDataStream *stream);
    void registrationAction(QsnWeb *widget, QString name, int ioIndex, int icon = 0);
    void runAction(int actionIndex);

signals:
    void interfaceSendSignal(quint16 signal, QByteArray *data);

public slots:
    void interfaceReciveSignalData(QSNSignal signal);
    void interfaceReciveSignalData(quint16 signal, QByteArray *data);
    void widgetReciveIndexSignal(int indexItemIO, QByteArray *data, QsnWeb *widget);

private:
    QsnInterface *interface;
    QList<ioItem> inputs;
    QList<ioItem> outputs;
    QList<signalItem> signalsTable;
    QList<pageWidgetLink> ulinks;

    void sendSignalFromIndex(int index, QByteArray *data);
    void reciveSignalFromIndex(int index, QByteArray *data);

};

#endif // QSNSIGNALTABLE_H
