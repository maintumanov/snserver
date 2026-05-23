#include "qsnwebpagedoorbell.h"

QsnWebPageDoorBell::QsnWebPageDoorBell(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Doorbell"));
    widgetUrl = "/doorbell";
    widgetIcon = "subicon-doorbell";
    sound = Q_NULLPTR;
    count = 3;
    countNight = 3;
    isNight = false;
    lastBell = QDateTime(QDate(2000,1,1), QTime(0, 0, 0));
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageDoorBell::~QsnWebPageDoorBell()
{
    sound->deleteLater();
}

void QsnWebPageDoorBell::getFunctions(QStringList *functions, int , QString)
{
    if (sound == Q_NULLPTR) return;

    *functions << QString("$(\"#nightswitch\").click(function(){"
                          "sendState('action', 'nightswitch', ($(this).prop('checked')));"
                          "}); ");

    *functions << QString("$(\"#belltest\").click(function(){"
                          "sendState('action', 'belltest', true);"
                          "}); ");

}

void QsnWebPageDoorBell::getFunctionsJSON(QStringList *fjson, int )
{   
    *fjson << QString(" $('#nightswitch').prop('checked', data.nightswitch);");
    *fjson << QString(" $('#lastbell').text(data.lastbell);");
}

void QsnWebPageDoorBell::getContents(QStringList *contents, int )
{

    if (sound == Q_NULLPTR) {
        //================================== no init =======================================
        *contents << QsnBsAlertDanger(tr("The module is not initialized"));
    } else {
        //================================== note =======================================
        *contents << QsnBsPanelTitle(objectName());
        *contents << QsnBsFormBegin();
        *contents << QsnBsFormSwitch(tr("Night time"), isNight, "nightswitch");
        *contents << QsnBsFormLabel(tr("Last bell"), lastBellDate(), "lastbell");
        *contents << QsnBsFormButton(tr("Bell"), QLatin1String("belltest"), QString("btn-info"));
        *contents << QsnBsFormEnd();
    }
}

void QsnWebPageDoorBell::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{  
    if (options->contains("action")) {
        QString state = options->value("action");

        if (state == QLatin1String("action"))  {
            if (options->value("id", "") == "nightswitch") {
                QString state = options->value("state");
                if (state == QLatin1String("false")) isNight = false;
                if (state == QLatin1String("true")) isNight = true;
            }
        }

        if (state == QLatin1String("action") && options->value("id", "") == "belltest") playBell();
    }

    *returnItems << QString("\"nightswitch\": %1").arg(QSNBoolToText(isNight));
    *returnItems << QString("\"lastbell\": \"%1\"").arg(lastBellDate());
}

void QsnWebPageDoorBell::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    // set options
    soundfilename = optionsMap.value("SN", QLatin1String("doorbell")).toString();
    soundfilename = getFileNameFromSoundName(soundfilename);
    if (!soundfilename.isEmpty()) sound = new QSound(soundfilename, this);

    soundfilenamenight = optionsMap.value("SNN", QLatin1String("nightbell")).toString();
    soundfilenamenight = getFileNameFromSoundName(soundfilenamenight);
    if (!soundfilenamenight.isEmpty() && sound == Q_NULLPTR) sound = new QSound(soundfilenamenight, this);

    countNight = optionsMap.value("CN", 5).toInt();
}

void QsnWebPageDoorBell::receiveSignalIOIndex(int indexIO, QByteArray *)
{
    if (indexIO == 0) playBell();
}

void QsnWebPageDoorBell::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    //    if (container.Command == 5 && container.Signal == 2) {
    //        QVariant night = mds->registry->value("state_nighttime");
    //        if (night.isValid()) isNight = night.toBool();
    //    }
}

void QsnWebPageDoorBell::sendMessage(QString theme, QString text)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 3;
    container.Signal = 0;
    container.Address = 2;
    container.info = theme;
    container.Sender = 0;
    container.Data = text.toUtf8();
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageDoorBell::playBell()
{
    if (isNewBell()) mds->interface->snBUSInput(QSNLogToContainer(
                                                    BUSSERV_LOG_LEVEL_information,
                                                    QString("%1 - %2").arg(objectName(), tr("Bell")),
                                                    BUSSERV_LOG_CATEGORY_module,
                                                    QString("doorbell"),
                                                    false), this);
    if(nightBell()) return;
    if (sound == Q_NULLPTR) return;
    if (!sound->isFinished()) return;

    if (isNight && !soundfilenamenight.isEmpty()) {
        sound->play(soundfilenamenight);
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       QString("%1 - %2").arg(objectName(), tr("Play night bell")),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("doorbell"),
                                       false), this);
    }
    if (!isNight && !soundfilename.isEmpty()) {
        sound->play(soundfilename);
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       QString("%1 - %2").arg(objectName(), tr("Play day bell")),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("doorbell"),
                                       false), this);
    }
    globalBell();
    lastBell = QDateTime::currentDateTime();
}

QString QsnWebPageDoorBell::getFileNameFromSoundName(QString name)
{
    QString fname = QSNHomeSubPath("server", "sounds").absoluteFilePath(QString("%1.wav").arg(name));
    if (QFile::exists(fname)) return fname;
    fname = QString(":/sounds/%1.wav").arg(name);
    if (QFile::exists(fname)) return fname;
    return QString();
}

QString QsnWebPageDoorBell::lastBellDate()
{
    if (lastBell.date().year() > 2010) return lastBell.toString("yyyy.MM.dd hh:mm:ss");
    return tr("not called");
}

QString QsnWebPageDoorBell::switchNightState()
{
    if (isNight) return QLatin1String("on");
    else return QLatin1String("off");
}

bool QsnWebPageDoorBell::isNewBell()
{
    return (lastBell < QDateTime::currentDateTime().addSecs(-60));
}

void QsnWebPageDoorBell::globalBell()
{
    if (!isNewBell()) countCalls ++;
    else countCalls = 1;
    if (countCalls < count) return;
    QByteArray data;
    mds->io->widgetReciveIndexSignal(1, &data, this);
}

bool QsnWebPageDoorBell::nightBell()
{
    if (!isNight) return false;
    if (!isNewBell()) countCallsNight ++;
    else countCallsNight = 1;
    lastBell = QDateTime::currentDateTime();
    return (countCallsNight < countNight);
}




