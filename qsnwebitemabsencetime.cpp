#include "qsnwebitemabsencetime.h"

QsnWebItemAbsenceTime::QsnWebItemAbsenceTime(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Absence time"));
    mds = modules;
    isMonday = true;
    isTuesday = true;
    isWednesday = true;
    isThursday = true;
    isFriday = true;
    isSaturday = true;
    isSunday = true;
    beginTime = QTime(7, 00);
    endTime = QTime(18, 00);
    stateAbsence = false;

    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebItemAbsenceTime::getContents(QStringList *contents, int )
{

}

void QsnWebItemAbsenceTime::thisContents(QStringList *contents)
{
    *contents << QString("<li id=\"%1bt\" data-dojo-type=\"dojox.mobile.ListItem\" style=\"font-weight: normal;\"  "
                         "data-dojo-props='variableHeight:false, rightText:\"%3\"'>"
                         "%2</li>")
                 .arg(itemID())
                 .arg(tr("Beginning"))
                 .arg(beginTime.toString("hh:mm"));

    *contents << QString("<li id=\"%1et\" data-dojo-type=\"dojox.mobile.ListItem\" style=\"font-weight: normal;\"  "
                         "data-dojo-props='variableHeight:false, rightText:\"%3\"'>"
                         "%2</li>")
                 .arg(itemID())
                 .arg(tr("Ending"))
                 .arg(endTime.toString("hh:mm"));

      *contents << QString("<li data-dojo-type=\"dojox.mobile.ListItem\" style=\"font-weight: normal;\" "
                           ">%1</li>")
                   .arg(days());
}

void QsnWebItemAbsenceTime::fromStream(QDataStream *stream)
{
    int count;
    QString Name;
    *stream >> Name;
    setObjectName(Name);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items
    if (optionsMap.contains("Mn")) isMonday = optionsMap.value("Mn").toBool();
    if (optionsMap.contains("Tu")) isTuesday = optionsMap.value("Tu").toBool();
    if (optionsMap.contains("Wd")) isWednesday = optionsMap.value("Wd").toBool();
    if (optionsMap.contains("Th")) isThursday = optionsMap.value("Th").toBool();
    if (optionsMap.contains("Fr")) isFriday = optionsMap.value("Fr").toBool();
    if (optionsMap.contains("St")) isSaturday = optionsMap.value("St").toBool();
    if (optionsMap.contains("Sn")) isSunday = optionsMap.value("Sn").toBool();
    if (optionsMap.contains("Begin")) beginTime = optionsMap.value("Begin").toTime();
    if (optionsMap.contains("End")) endTime = optionsMap.value("End").toTime();
}

void QsnWebItemAbsenceTime::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 2) timeUpdate();
}

void QsnWebItemAbsenceTime::setAbsenceState(bool state)
{
    QTime curTime = QTime::currentTime();
    if (stateAbsence && !state && curTime > beginTime.addSecs(5) && curTime < endTime) {
        sendMessage(QString(tr("Unauthorized presence [%1]")).arg(QTime::currentTime().toString("hh:mm:ss")));
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       tr("Unauthorized presence"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("absence"),
                                       false), this);
    }
    stateAbsence = state;
}

void QsnWebItemAbsenceTime::timeUpdate()
{
    QTime curTime = QTime::currentTime();
    if (curTime > beginTime && curTime < beginTime.addSecs(5) && !stateAbsence) absanceState(true);
    if (curTime > endTime && stateAbsence) absanceState(false);
}

QString QsnWebItemAbsenceTime::days()
{
    QString ret;
    if (isMonday) ret = tr("Mo") + QLatin1String(", ");
    if (isTuesday) ret +=  tr("Tu") + QLatin1String(", ");
    if (isWednesday) ret +=  tr("We") + QLatin1String(", ");
    if (isThursday) ret +=  tr("Th") + QLatin1String(", ");
    if (isFriday) ret +=  tr("Fr") + QLatin1String(", ");
    if (isSaturday) ret += tr("Sa") + QLatin1String(", ");
    if (isSunday) ret += tr("Su") + QLatin1String(", ");
    ret.remove(ret.length() - 2, 2);
    return ret;
}

void QsnWebItemAbsenceTime::sendMessage(QString text)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 3;
    container.Signal = 3;
    container.info = QString(tr("Absence"));
    container.Sender = 0;
    QString msg = text;
    container.Data = msg.toUtf8();
    mds->interface->snBUSInput(container, this);
}





