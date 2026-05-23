#include "qsnwebpagenotification.h"

QsnWebPageNotification::QsnWebPageNotification(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    isLog = false;
    timeout = 0;
    notifState = tr("Nothing was sent");
    notifStateWidget = QString();
    widgetUrl = QString("/notification%1").arg(iID);
    widgetIcon = "subicon-notification";
    setObjectName(tr("Notification"));
    widgetAccessRights = 2;
    widgetNID = QLatin1String("notification");

    connect(&manager, SIGNAL(finished( QNetworkReply*)), SLOT(onFinished( QNetworkReply*)));
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

QsnWebPageNotification::~QsnWebPageNotification()
{

}

void QsnWebPageNotification::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    *stream >> count; //io
    *stream >> count; //items

    //setTitle(title);
    rURL = optionsMap.value("UR", QLatin1String("https://notifymydevice.com/push?ApiKey=1234567898&PushTitle=%TITLE%&PushText=%TEXT%")).toString();
    rMethod = optionsMap.value("MT", 0).toInt();
    rPOSTdata = optionsMap.value("PS", 0).toString();

    widgetNID = QSNNameToFileName(objectName());
    widgetNID = widgetNID.replace('.','_');

    isUsePhone = false;
    if (rURL.indexOf("%PHONE%") >= 0) {mds->auth->addParametr("phoneNumber", tr("Phone"), QString()); isUsePhone = true;}


    if (isUsePhone) {
        mds->auth->addPermission(widgetNID+"info", QString("%1 (%2)").arg(objectName(), tr("info")), false);
        mds->auth->addPermission(widgetNID+"icident", QString("%1 (%2)").arg(objectName(), tr("icidents")), false);
    }

}

void QsnWebPageNotification::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$(\"#sendtest\").click(function(){"
                          "sendState('action', 'sendtest', true);"
                          "}); ");
}

void QsnWebPageNotification::getFunctionsJSON(QStringList *fjson, int )
{ 
    *fjson << QString(" $('#queue').text(data.queue);");
    *fjson << QString(" $('#state').text(data.state);");
}

void QsnWebPageNotification::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("action") && options->value("id", "") == "sendtest")  sendTest();
    }
    *returnItems << QString("\"state\": \"%1\"").arg(notifState);
    *returnItems << QString("\"queue\": \"%1\"").arg(notifqueue.count());

}

void QsnWebPageNotification::getContents(QStringList *contents, int )
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();
    *contents << QsnBsFormLabel(tr("State"), notifState, "state");
    *contents << QsnBsFormLabel(tr("In the queue"), QString::number(notifqueue.count()), "queue");
    *contents << QsnBsFormButton(tr("Send test"), QLatin1String("sendtest"), QString("btn-warning"));
    *contents << QsnBsFormEnd();
}

void QsnWebPageNotification::urlChanged(int )
{

}

QString QsnWebPageNotification::widgetState()
{
    return notifStateWidget;
}

void QsnWebPageNotification::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == 3) sendmessage(container.info);
    if (container.Command == 2) checkTime();
}

void QsnWebPageNotification::onFinished(QNetworkReply *reply)
{
    if( reply->error() == QNetworkReply::NoError ) {
        notifState = tr("Last send") + ": " + QDateTime::currentDateTime().toString("dd.MM.yy hh:mm");
        notifStateWidget = QString();
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       QString("%1 - %2 %3").arg(objectName(), tr("Last send"), notifState),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("widgets"),
                                       false), this);
        timeout = 0;
    } else {
        notifState = tr("Error sending") + ": " + reply->errorString();
        notifStateWidget = tr("Error");
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       QString("%1 - %2 %3").arg(objectName(), tr("Error sending"), notifState),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("widgets"),
                                       false), this);
        timeout = 60;
    }

    reply->deleteLater();
}

void QsnWebPageNotification::sendmessage(QString text, bool filter)
{
    queueItem msg;
    if (isUsePhone) {
        for (int i = 0; i < mds->auth->countAccounts(); i ++) {
            quint8 level = QSNWarningLevelFromText(text);
            if (filter && level >= 2 && !mds->auth->permissionValue(mds->auth->atAccount(i), widgetNID+"info")) continue;
            if (filter && level < 2 && !mds->auth->permissionValue(mds->auth->atAccount(i), widgetNID+"icident")) continue;
            if (isUsePhone) msg.phone = mds->auth->parametrValue(mds->auth->atAccount(i), "phoneNumber");
            if (msg.phone.isEmpty()) continue;
            msg.text = QSNGetWarninMSGBody(text);
            msg.note = QSNGetWarninMSGNote(text);
            notifqueue.append(msg);
        }
    } else {
        msg.text = QSNGetWarninMSGBody(text);
        msg.note = QSNGetWarninMSGNote(text);
        notifqueue.append(msg);
    }
}

void QsnWebPageNotification::checkTime()
{
    if (timeout > 0) timeout --;
    checkQueue();
}

void QsnWebPageNotification::checkQueue()
{
    if (notifqueue.isEmpty()) return;
    if (timeout > 0) return;
    timeout = 10;
    queueItem *msg = &notifqueue.first();
    QString url = rURL;
    QString pdata = rPOSTdata;
    QString pnote = msg->note;
    QString ptext = msg->text;
    if (pnote.isEmpty()) pnote = QLatin1String("-");
    if (ptext.isEmpty()) ptext = QLatin1String("-");
    url = url.replace("%NOTE%", pnote);
    url = url.replace("%TEXT%", ptext);
    url = url.replace("%PHONE%", msg->phone);

    pdata = pdata.replace("%NOTE%", pnote);
    pdata = pdata.replace("%TEXT%", ptext);
    pdata = pdata.replace("%PHONE%", msg->phone);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    if (rMethod == 0) manager.get(request);
    else manager.post(request, pdata.toUtf8());
    notifqueue.removeFirst();
}

void QsnWebPageNotification::sendTest()
{
    sendmessage(tr("Test Message {Test note}"), false);
}
