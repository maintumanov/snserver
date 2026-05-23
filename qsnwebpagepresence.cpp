#include "qsnwebpagepresence.h"

QsnWebPagePresence::QsnWebPagePresence(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Presence"));
    widgetUrl = "/presence";
    widgetIcon = "subicon-absence";
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPagePresence::~QsnWebPagePresence()
{

}

void QsnWebPagePresence::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("function tablepresence(data) {"
                          "var table = $(\"#table_users_p\");"
                          "table.empty();"
                          "$.each(data, function (i, item) {"
                          "table.append(\"<tr><td>\" + item.user + \"</td>\" +"
                          "\"<td>\" + item.presence + \"</td>\" +"
                          "\"<td>\" + item.ldet + \"</td>\" +"
                          "\"<td>\" + item.src + \"</td>\" +"
                          "\"<td>\" + item.eabs + \"</td>"
                          "</tr>\");"
                          "});}");
}

void QsnWebPagePresence::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString(" $('#state').text(data.state);");
    *fjson << QString(" tablepresence(data.usersp);");
}

void QsnWebPagePresence::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsTapsBegin();
    *contents << QsnBsTapsTabAdd("presence", tr("PRESENCE"), true);
    *contents << QsnBsTapsTabAdd("users", tr("USERS"), false);
    *contents << QsnBsTapsPanesBegin();

    *contents << QsnBsTapsPanelBegin("presence", true);
    *contents << QsnBsFormBegin();
    //    *contents << QsnBsFormSwitch(tr("Enable"), isEnable, "swenable");
    *contents << QsnBsFormLabel(tr("State"), mds->auth->presenceLabel(), "state");
    //    *contents << QsnBsFormFieldlabel(tr("Timer"), QTime::fromMSecsSinceStartOfDay(timerAbsence * 1000).toString("mm:ss"), "timer");
    *contents << QsnBsFormEnd();
    *contents << QsnBsTapsPanelEnd();

    *contents << QsnBsTapsPanelBegin("users", false);
    *contents << QsnBsFormTableBegin();
    *contents << QsnBsFormTableTheadBegin();
    *contents << QsnBsFormTableTheadAddCl(tr("User name"));
    *contents << QsnBsFormTableTheadAddCl(tr("Presence"));
    *contents << QString("<th class=\"d-none d-md-table-cell\" scope=\"col\">%1</th>").arg(tr("Last detect"));
    *contents << QString("<th class=\"d-none d-md-table-cell\" scope=\"col\">%1</th>").arg(tr("Source detect"));
    *contents << QString("<th class=\"d-none d-md-table-cell\" scope=\"col\">%1</th>").arg(tr("Absence through"));
    *contents << QsnBsFormTableTheadEnd();

    *contents << QsnBsFormTableBodyBegin(QString("table_users_p"));

    bool presence;
    QDateTime lastDetect;
    QString sourceDetect;
    QDateTime endAbsence;
    QsnWebAuthorization::accountItem *aitem;
    for (int i = -1; i < mds->auth->countAccounts(); i ++) {
        aitem = mds->auth->atAccount(i);
        mds->auth->getAccountPresenceLastConsolidated(aitem, &presence, &lastDetect, &sourceDetect, &endAbsence);
        *contents << QString("<tr>");
        *contents << QString("<td>%1</td>").arg(i==-1?tr("System"):aitem->name);
        *contents << QString("<td>%1</td>").arg(presence?tr("presence"):tr("absence"));
        *contents << QString("<td class=\"d-none d-md-table-cell\">%1</td>").arg(QSNDateTimeToString(lastDetect));
        *contents << QString("<td class=\"d-none d-md-table-cell\">%1</td>").arg(sourceDetect);
        *contents << QString("<td class=\"d-none d-md-table-cell\">%1</td>").arg(QSNToTimeString(endAbsence));
        *contents << QString("</tr>");
    }

    *contents << QsnBsFormTableBodyEnd();
    *contents << QsnBsFormTableEnd();
    *contents << QsnBsTapsPanelEnd();
}

void QsnWebPagePresence::actionItem(QString , QMap<QString, QString> *, QStringList *returnItems, qint64 )
{
    *returnItems << QString("\"state\": \"%1\"").arg(widgetState());
    *returnItems << QString("\"usersp\": %1").arg(userspToJSON());
}

QString QsnWebPagePresence::userspToJSON()
{
    bool presence;
    QDateTime lastDetect;
    QString sourceDetect;
    QDateTime endAbsence;
    QsnWebAuthorization::accountItem *aitem;

    QString ret = "[";
    for (int i = -1; i < mds->auth->countAccounts(); i ++) {
        aitem = mds->auth->atAccount(i);
        mds->auth->getAccountPresenceLastConsolidated(aitem, &presence, &lastDetect, &sourceDetect, &endAbsence);
        if (ret.count() > 1) ret += ",";
        ret += "{";
        ret += QString("\"user\": \"%1\",").arg(i==-1?tr("System"):aitem->name);
        ret += QString("\"presence\": \"%1\",").arg(presence?tr("presence"):tr("absence"));
        ret += QString("\"ldet\": \"%1\",").arg(QSNDateTimeToString(lastDetect));
        ret += QString("\"src\": \"%1\",").arg(sourceDetect);
        ret += QString("\"eabs\": \"%1\"").arg(QSNToTimeString(endAbsence));
        ret += "}";
    }
    ret += "]";
    return ret;
}

void QsnWebPagePresence::presenceAddDB()
{
    if (mds->auth->isPresence() < 0) return;
    QByteArray d;
    d.append(1);
    d.append(1);
    d.append(10);
    QSNBoolToRAW(&d, 1, mds->auth->isPresence());
    mds->db->writeRAWtoYBD("presence", &d, QDateTime::currentDateTime(), tr("Presence"), this);
}

void QsnWebPagePresence::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("MS"))) {
            QsnWebItemPresenceTime *presencetime = new QsnWebItemPresenceTime(static_cast<quint32>(id), modules(), this);
            presences.append(presencetime);
            presencetime->fromStream(stream);
            connect(presencetime, SIGNAL(presenceAction(QString, qint64, qint64)), mds->auth, SLOT(setPresenceSystem(QString, qint64, qint64)));
        }
    }

    presencetime = optionsMap.value("PT", 84600).toInt();
    presencetimealt = optionsMap.value("PTA", 300).toInt();

}

void QsnWebPagePresence::receiveSignalIOIndex(int indexIO, QByteArray *)
{
    if (indexIO == 2) mds->auth->setAbsence();
    if (indexIO == 3 &&  mds->auth->isPresence() != 1) {
        mds->auth->setAccountPresenceItem(mds->auth->atAccount(-1), tr("System"), true, presencetime, presencetimealt);
    }
}

QString QsnWebPagePresence::widgetState()
{
    return mds->auth->presenceLabel();
}

void QsnWebPagePresence::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;

    if (container.Command == BUSSERV_MODULE_STATE_CHANGE_absence) {
        QByteArray data;
            mds->io->widgetReciveIndexSignal(container.Signal, &data, this);
            mds->interface->snBUSInput(QSNLogToContainer(
                                           BUSSERV_LOG_LEVEL_information,
                                           container.Signal?tr("Presence"):tr("Absence"),
                                           BUSSERV_LOG_CATEGORY_module,
                                           QString("Presence"),
                                           false), this);
        presenceAddDB();
    }

    if (container.Command == BUSSERV_TIME_SEC_EVENT) {
        quint8 min = QDateTime::currentDateTime().time().minute();
        if (oldMinutes != min) {
            presenceAddDB();
            oldMinutes = min;
        }
    }

}
