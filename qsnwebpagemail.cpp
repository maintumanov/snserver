#include "qsnwebpagemail.h"

#define CHECK_QUEUE_INDEX \
    if (currentItemIndex < 0 || currentItemIndex >= mailqueue.size()) { \
    closeSocket(); \
    return; \
    }

QsnWebPageMail::QsnWebPageMail(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    sendState = 100; // 100 = idle
    SendTimeout = 360; // seconds
    widgetUrl = "/mail";
    widgetIcon = "subicon-mail";
    setObjectName(tr("Mail"));
    widgetAccessRights = 2; // requires authenticated user
    clientSocket = Q_NULLPTR;
    clientSocketSSL = Q_NULLPTR;
    mailCurOp = tr("Nothing was sent");
    mailState = tr("Undefined");
    currentItemIndex = -1;

    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));

    // Register permissions for email notifications
    mds->auth->addPermission("mailinfo", tr("Mail (info)"), false);
    mds->auth->addPermission("mailincident", tr("Mail (incidents)"), false);
}

QsnWebPageMail::~QsnWebPageMail()
{
    closeSocket();
}

void QsnWebPageMail::getFunctions(QStringList *functions, int accountIndex, QString )
{   
    if (SSL && !mds->supportSSL) return;
    if (mds->auth->atAccount(accountIndex)->email.isEmpty()) return;
    *functions << QString("$(\"#sendb\").click(function(){"
                          "sendState('action', 'send', true);"
                          "}); ");

    *functions << QString("$(\"#sendtest\").click(function(){"
                          "sendState('action', 'sendtest', true);"
                          "}); ");
}

void QsnWebPageMail::getFunctionsJSON(QStringList *fjson, int )
{
    if (SSL && !mds->supportSSL) return;
    *fjson << QString(" $('#queue').prop('checked', data.queue);");
    *fjson << QString(" $('#mailstate').text(data.mailstate);");
    *fjson << QString(" $('#operation').text(data.operation);");
    *fjson << QString(" $('#recipients').text(data.recipients);");
}

void QsnWebPageMail::getContents(QStringList *contents, int accountIndex)
{
    *contents << QsnBsPanelTitle(objectName());
    *contents << QsnBsFormBegin();

    if (SSL && !mds->supportSSL) {
        *contents << QsnBsAlertDanger(tr("Messages will not be sent because SSL is not supported!"));
    } else {
        if (mds->auth->atAccount(accountIndex)->email.isEmpty())
            *contents << QsnBsAlertWarning(tr("The current user does not have an email address!"));

        *contents << QsnBsFormLabel(tr("State"), mailState, "mailstate");
        *contents << QsnBsFormLabel(tr("Operation"), mailCurOp, "operation");
        *contents << QsnBsFormLabel(tr("In the queue"), QString::number(getQueueItems()), "queue");
        *contents << QsnBsFormLabel(tr("Recipients"), QString::number(getRecipientsCount()), "recipients");
        if (!mds->auth->atAccount(accountIndex)->email.isEmpty()) {
            *contents << QsnBsFormGroupButtonBegin();
            *contents << QsnBsFormGroupButton(tr("Send now"), QLatin1String("sendb"), QString("btn-info"));
            *contents << QsnBsFormGroupButton(tr("Send test"), QLatin1String("sendtest"), QString("btn-warning"));
            *contents << QsnBsFormGroupButtonEnd();
        }
    }
    *contents << QsnBsFormEnd();
}

void QsnWebPageMail::actionItem(QString , QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (SSL && !mds->supportSSL) return;
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("action") && options->value("id",QString()) == QLatin1String("send"))  sendNow();

        if (state == QLatin1String("action") && options->value("id", QString()) == QLatin1String("sendtest"))  sendTest();
    }
    *returnItems << QString("\"mailstate\": \"%1\"").arg(mailState);
    *returnItems << QString("\"operation\": \"%1\"").arg(mailCurOp);
    *returnItems << QString("\"queue\": \"%1\"").arg(getQueueItems());
    *returnItems << QString("\"recipients\": \"%1\"").arg(getRecipientsCount());
}

void QsnWebPageMail::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);
    *stream >> count;  // items (unused in current version)
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
    }
    // Set options
    SenderName = optionsMap.value("SN", QLatin1String("Smart home")).toString();
    SenderEmail = optionsMap.value("SM", QLatin1String()).toString();
    Login = optionsMap.value("LG", tr("Login")).toString();
    Password = optionsMap.value("PS", QLatin1String()).toString();
    Server = optionsMap.value("SRV", QLatin1String()).toString();
    Port = static_cast<quint16>(optionsMap.value("PRT", 25).toUInt());
    SendTimeout = optionsMap.value("TO", 360).toUInt();
    SSL = optionsMap.value("SSL", false).toBool();
    mds->auth->setAdminEmail(optionsMap.value("AM", QLatin1String()).toString());

    if (SSL && !mds->supportSSL) wState = tr("does not work");
    else wState = tr("waiting for messages");
}

void QsnWebPageMail::urlChanged(int accountIndex)
{
    currentAccountIndex = accountIndex;
}

QString QsnWebPageMail::widgetState()
{
    return wState;
}

void QsnWebPageMail::snBUSInput(QSNContainer container, QObject *)
{
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) queueCheck(); // periodic queue processing

    if (container.Command == BUSSERV_MESSAGE) {
        for (int i = 0; i < mds->auth->countAccounts(); i ++) {
            if (mds->auth->atAccount(i)->email.isEmpty()) continue;
            quint8 level = QSNWarningLevelFromText(container.info);

            // Info-level logs → users with "mailinfo" permission
            if (mds->auth->permissionValue(mds->auth->atAccount(i), "mailinfo") && level == 2) addToQueue(container.info, i);

            // Warnings/errors → users with "mailincident" permission
            if (mds->auth->permissionValue(mds->auth->atAccount(i), "mailincident") && level < 2) addToQueue(container.info, i);
        }
    }
}

void QsnWebPageMail::addToQueue(QString text, int user)
{
    if (SSL && !mds->supportSSL) return;

    QString email = mds->auth->atAccount(user)->email;

    // Try to append to existing pending message for this email
    for (int i = 0; i < mailqueue.count(); i ++)
        if (mailqueue[i].email == email && mailqueue[i].state < 2) {
            mailqueue[i].content.append(contentFromText(text));
            if (mailqueue[i].content.last().color == QLatin1String("red")) mailqueue[i].waitingUntil = QDateTime::currentDateTime();
            return;
        }

    // Create new queue entry
    queueItem msg;
    msg.name = mds->auth->atAccount(user)->name;
    msg.content.append(contentFromText(text));
    msg.email = email;
    msg.waitingUntil = QDateTime::currentDateTime();
    msg.state = 0; // ready
    mailqueue.append(msg);
}

QsnWebPageMail::contentItem QsnWebPageMail::contentFromText(QString text)
{
    contentItem item;
    item.time = QDateTime::currentDateTime();
    item.content = QSNGetWarninMSGBody(text);
    switch (QSNWarningLevelFromText(text)) {
    case 0:  // Caution
        item.color = "red";
        item.color_border = "bf3128";
        item.color_background = "ff4136";
        item.color_note = "f2c3bf";
        break;
    case 1: // Warning
        item.color = "yellow";
        item.color_border = "b6750d";
        item.color_background = "f39c12";
        item.color_note = "fad8a2";
        break;
    case 2: // Info
        item.color = "blue";
        item.color_border = "0057A3";
        item.color_background = "0074D9";
        item.color_note = "75bfff";
        break;
    case 3: // Note
        item.color = "purple";
        item.color_border = "660066";
        item.color_background = "800080";
        item.color_note = "ff99ff";
        break;
    case 4: // Tip / neutral
        item.color = "green";
        item.color_border = "229930";
        item.color_background = "2ecc40";
        item.color_note = "a7ebaf";
        break;
    }

    item.note = QSNGetWarninMSGNote(text);
    return item;
}

// MAIL QUEUE
void QsnWebPageMail::queueCheck()
{
    if (mailqueue.isEmpty()) {
        mailCurOp = tr("Waiting for messages");
        return;
    }

    if (sendState != 100) return;  // already sending

    // Find first message whose waiting period has expired
    for (int i = mailqueue.count() - 1; i >= 0; i --)
        if (QDateTime::currentDateTime() >= mailqueue[i].waitingUntil) {
            beginSend(i);
            return;
        }
    mailCurOp = tr("Waiting for the time of sending");
}

void QsnWebPageMail::clearState()
{
    if (sendState == 100) return;
    sendState = 100;
}

void QsnWebPageMail::senderror(QString info, QString operation)
{
    if (currentItemIndex < 0 || currentItemIndex >= mailqueue.size()) {
        closeSocket();
        return;
    }
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_caution,
                                   tr("Error sending %2 %3 {%1}").arg(mailqueue[currentItemIndex].email, info, operation),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("mail"),
                                   false), this);

    if (operation.isEmpty()) mailCurOp = QString("ready");
    else mailCurOp = operation;
    if (!info.isEmpty()) {
        mailState = tr("Error") + ":" + info;
        wState = tr("error");
    } else wState = mailCurOp;

    // Retry after timeout
    mailqueue[currentItemIndex].waitingUntil = QDateTime::currentDateTime().addSecs(SendTimeout);
    mailqueue[currentItemIndex].state = 0;
    closeSocket();
}

void QsnWebPageMail::slotReadyRead()
{
    switch (sendState)  {
    case 0:connect_answer(); break;
    case 1:helo_answer(); break;
    case 2:mail_from_answer(); break;
    case 3:rcpt_to_answer(); break;
    case 4:data_answer(); break;
    case 5:data_send_answer(); break;
    case 10:ehlo_answer(); break;
    case 11:auth_login_answer(); break;
    case 12:auth_username_answer(); break;
    case 13:auth_password_answer(); break;
    }
}

void QsnWebPageMail::disconnected()
{
    clearState();
}

void QsnWebPageMail::slotError(QAbstractSocket::SocketError )
{
    CHECK_QUEUE_INDEX // ← безопасный доступ

            mailState = tr("Socket error");
    mailCurOp = tr("error sending");
    wState = mailCurOp;
    mailqueue[currentItemIndex].waitingUntil = QDateTime::currentDateTime().addSecs(SendTimeout);
    mailqueue[currentItemIndex].state = 0;
    closeSocket();
}

void QsnWebPageMail::slotConnected()
{
    if (SSL && mds->supportSSL) localAddress = clientSocketSSL->localAddress().toString();
    else localAddress = clientSocket->localAddress().toString();
    sendState = 0;
    mailqueue[currentItemIndex].state = 1;  // connected
}

void QsnWebPageMail::readAnswer(QString *answer)
{
    QIODevice *dev = socket();
    //    if (dev == Q_NULLPTR) return;
    //    QTextStream htextStream(dev);
    //    htextStream.setAutoDetectUnicode(true);
    //    htextStream.setCodec("UTF-8");
    //    htextStream.setAutoDetectUnicode(true);
    //    while(!htextStream.atEnd()) *answer += htextStream.readLine();

    if (!dev || !dev->bytesAvailable()) return;
    answer->clear();
    while (dev->canReadLine()) {
        QByteArray line = dev->readLine().trimmed();
        if (!line.isEmpty()) *answer += QString::fromUtf8(line) + "\n";
        // Остановка на строке, начинающейся с кода + пробел (не '-')
        if (line.size() >= 4 && line[3] == ' ') break;
    }
    if (!answer->isEmpty()) answer->chop(1); // убрать последний \n
}

void QsnWebPageMail::sendRequest(QString request)
{
    QIODevice *dev = socket();
    if (dev == Q_NULLPTR) return;
    QTextStream htextStream(dev);
    htextStream.setAutoDetectUnicode(true);
    htextStream.setCodec("UTF-8");
    htextStream.setAutoDetectUnicode(true);
    htextStream << request;
    htextStream.flush();
}
//------------------------------------

void QsnWebPageMail::connect_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("220") == 0) {
        mailCurOp = tr("Sending");
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_information,
                                       tr("Sending"),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString(),
                                       true), this);
        if (Login.isEmpty()) helo();
        else ehlo();

    } else senderror(answer, tr("connect"));
}

void QsnWebPageMail::helo()
{
    sendState = 1;
    sendRequest(QString("HELO %1\r\n").arg(localAddress));
}

void QsnWebPageMail::helo_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("250") == 0) mail_from();
    else senderror(answer, "HELO");
}

void QsnWebPageMail::mail_from()
{
    sendState = 2;
    sendRequest(QString("MAIL FROM: <%1>\r\n").arg(SenderEmail));
}

void QsnWebPageMail::mail_from_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("250") == 0) rcpt_to();
    else senderror(answer, "MAIL FROM");
}

void QsnWebPageMail::rcpt_to()
{
    sendState = 3;
    sendRequest(QString("RCPT TO: <%1>\r\n").arg(mailqueue[currentItemIndex].email));
}

void QsnWebPageMail::rcpt_to_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("250") == 0) data();
    else senderror(answer, "RCPT TO");
}

void QsnWebPageMail::data()
{
    sendState = 4;
    sendRequest(QString("DATA\r\n"));
}

void QsnWebPageMail::data_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("354") == 0) data_send();
    else senderror(answer, "DATA");
}

void QsnWebPageMail::data_send()
{
    CHECK_QUEUE_INDEX // ← защита от выхода за границы

    QString dt;
    sendState = 5;

    // Заголовки письма
    dt = QString("From: %1 <%2>\r\n")
            .arg(encodeHeader(SenderName), SenderEmail);
    dt += QString("To: %1 <%2>\r\n")
            .arg(encodeHeader(mailqueue[currentItemIndex].name), mailqueue[currentItemIndex].email);
    dt += QString("Subject: %1\r\n")
            .arg(encodeHeader(tr("Smart Home — Уведомления")));
    dt += QString("Content-Type: text/html; charset=\"UTF-8\"\r\n");
    dt += QString("\r\n");

    // Начало HTML
    dt += QString(
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
        "  <style>\n"
        "    @media (prefers-color-scheme: dark) {\n"
        "      .event-title { color: #ffffff !important; }\n"
        "      .event-note { color: #d0d0d0 !important; opacity: 0.9; }\n"
        "      .event-time { color: #aaaaaa !important; }\n"
        "    }\n"
        "  </style>\n"
        "</head>\n"
        "<body style=\"margin:0; padding:0; background:#f5f7fa; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\">\n"
        "  <table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\" bgcolor=\"#f5f7fa\">\n"
        "    <tr>\n"
        "      <td align=\"center\" style=\"padding:24px 12px;\">\n"
        "        <table width=\"100%\" style=\"max-width:600px; background:#ffffff; border-radius:12px; overflow:hidden; box-shadow:0 2px 8px rgba(0,0,0,0.08);\">\n"
    );

    // Заголовок письма
    dt += QString(
        "          <tr>\n"
        "            <td style=\"padding:20px 24px; text-align:center; background:#f8f9fa; border-bottom:1px solid #eaecef;\">\n"
        "              <h2 style=\"margin:0; font-size:20px; color:#2d3748;\">🔔 Уведомления системы Smart Home</h2>\n"
        "              <p style=\"margin:6px 0 0; font-size:13px; color:#718096;\">За последние события</p>\n"
        "            </td>\n"
        "          </tr>\n"
    );

    // События
    for (int i = 0; i < mailqueue[currentItemIndex].content.count(); ++i) {
        const contentItem &item = mailqueue[currentItemIndex].content[i];

        // Определяем иконку и цвета по уровню
        QString icon, titleColor, noteColor;
        if (item.color == "red") {
            icon = "⚠️";
            titleColor = "#c53030";
            noteColor = "#dd6b6b";
        } else if (item.color == "yellow") {
            icon = "🔔";
            titleColor = "#d97706";
            noteColor = "#f6ad55";
        } else if (item.color == "blue") {
            icon = "ℹ️";
            titleColor = "#2b6cb0";
            noteColor = "#90cdf4";
        } else if (item.color == "purple") {
            icon = "💡";
            titleColor = "#805ad5";
            noteColor = "#d6bcfa";
        } else if (item.color == "green") {
            icon = "✅";
            titleColor = "#38a169";
            noteColor = "#81e6d9";
        } else {
            icon = "📌";
            titleColor = "#4a5568";
            noteColor = "#a0aec0";
        }

        // Блок события
        dt += QString(
            "          <tr>\n"
            "            <td style=\"padding:16px 24px;\">\n"
            "              <table cellpadding=\"0\" cellspacing=\"0\" border=\"0\" width=\"100%\">\n"
            "                <tr>\n"
            "                  <td style=\"width:28px; vertical-align:top; padding-right:12px;\">\n"
            "                    <span style=\"font-size:18px;\">%1</span>\n"
            "                  </td>\n"
            "                  <td>\n"
            "                    <div class=\"event-title\" style=\"font-size:16px; font-weight:600; color:%2; margin-bottom:4px;\">%3</div>\n"
            "                    <div class=\"event-note\" style=\"font-size:14px; color:%4;\">%5</div>\n"
            "                    <div class=\"event-time\" style=\"font-size:12px; color:#a0aec0; margin-top:6px;\">[%6]</div>\n"
            "                  </td>\n"
            "                </tr>\n"
            "              </table>\n"
            "            </td>\n"
            "          </tr>\n"
        ).arg(icon)
         .arg(titleColor)
         .arg(item.content.toHtmlEscaped())
         .arg(noteColor)
         .arg(item.note.isEmpty() ? QString("") : item.note.toHtmlEscaped())
         .arg(item.time.toString("hh:mm:ss dd.MM.yyyy"));

        // Разделитель (кроме последнего)
        if (i < mailqueue[currentItemIndex].content.count() - 1) {
            dt += QString(
                "          <tr><td style=\"height:1px; background:#edf2f7;\"></td></tr>\n"
            );
        }
    }

    // Подвал
    dt += QString(
        "          <tr>\n"
        "            <td style=\"padding:20px 24px; text-align:center; font-size:12px; color:#a0aec0; border-top:1px solid #edf2f7;\">\n"
        "              Это автоматическое уведомление. Не отвечайте на него.<br>\n"
        "              <a href=\"https://smart.home.local\" style=\"color:#4299e1; text-decoration:none; margin-top:6px; display:inline-block;\">\n"
        "                Открыть панель управления\n"
        "              </a>\n"
        "            </td>\n"
        "          </tr>\n"
    );

    // Завершение HTML
    dt += QString(
        "        </table>\n"
        "      </td>\n"
        "    </tr>\n"
        "  </table>\n"
        "</body>\n"
        "</html>\r\n"
        "\r\n.\r\n"
    );

    // Отправка
    mailqueue[currentItemIndex].state = 2;
    sendRequest(dt);
}

void QsnWebPageMail::data_send_answer()
{
    CHECK_QUEUE_INDEX // ← безопасный доступ

            QString answer;
    readAnswer(&answer);
    if (answer.indexOf("250") == 0) quit();
    else senderror(answer, "DATA SEND");
}

void QsnWebPageMail::quit()
{
    CHECK_QUEUE_INDEX // ← безопасный доступ

            mailState = QString("%1: %2").arg(tr("Last send"), QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"));
    mailCurOp = tr("waiting for messages");
    wState = mailCurOp;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Successful sent > %1 (%2)").arg(mailqueue[currentItemIndex].name,
                                                                       mailqueue[currentItemIndex].email),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("mail"),
                                   false), this);

    mailqueue[currentItemIndex].content.clear();
    mailqueue.removeAt(currentItemIndex);
    currentItemIndex = -1;

    sendRequest(QString("QUIT\r\n"));
    closeSocket();
}

void QsnWebPageMail::ehlo()
{
    sendState = 10;
    sendRequest(QString("EHLO %1\r\n").arg(localAddress));
}

void QsnWebPageMail::ehlo_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("250") == 0) auth_login();
    else senderror(answer, "EHLO");
}

void QsnWebPageMail::auth_login()
{
    sendState = 11;
    sendRequest(QString("AUTH LOGIN\r\n"));
}

void QsnWebPageMail::auth_login_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("334") == 0) auth_username();
    else senderror(answer, "LOGIN");
}

void QsnWebPageMail::auth_username()
{
    sendState = 12;
    QString login = Login;
    QByteArray ba;
    ba.append(login.toUtf8());
    login = ba.toBase64();
    sendRequest(QString("%1\r\n").arg(login));
}

void QsnWebPageMail::auth_username_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("334") == 0) auth_password();
    else senderror(answer, "USER NAME");
}

void QsnWebPageMail::auth_password()
{
    sendState = 13;
    QString pass = Password;
    QByteArray ba;
    ba.append(pass.toUtf8());
    pass = ba.toBase64();
    sendRequest(QString("%1\r\n").arg(pass));
}

void QsnWebPageMail::auth_password_answer()
{
    QString answer;
    readAnswer(&answer);
    if (answer.indexOf("235") == 0) mail_from();
    else senderror(answer, "PASSWORD");
}

void QsnWebPageMail::beginSend(int itemIndex)
{
    // Защита от некорректного индекса
    if (itemIndex < 0 || itemIndex >= mailqueue.size()) {
        mds->interface->snBUSInput(QSNLogToContainer(
                                       BUSSERV_LOG_LEVEL_caution,
                                       tr("Attempt to send email with invalid queue index: %1").arg(itemIndex),
                                       BUSSERV_LOG_CATEGORY_module,
                                       QString("mail"),
                                       false), this);
        return;
    }


    // Нечего отправлять — удаляем пустой элемент
    if (mailqueue[itemIndex].content.isEmpty()) {
        mailqueue.removeAt(itemIndex);
        return;
    }

    // Логируем начало подключения
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Initiating connection to SMTP server for user: %1").arg(mailqueue[itemIndex].name),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("mail"),
                                   true), this);

    mailCurOp = tr("Connection");
    mailState = tr("Sending");
    sendState = 0;
    currentItemIndex = itemIndex;

    // Устанавливаем время повторной попытки (на случай ошибки)
    mailqueue[currentItemIndex].waitingUntil = QDateTime::currentDateTime().addSecs(SendTimeout);

    if (SSL && mds->supportSSL) {
        clientSocketSSL = new QSslSocket(this);
        // Подключаем сигналы
        QObject::connect(clientSocketSSL, SIGNAL(readyRead()),this, SLOT(slotReadyRead()));
        QObject::connect(clientSocketSSL, SIGNAL(disconnected()),this, SLOT(disconnected()));
        QObject::connect(clientSocketSSL, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(slotError(QAbstractSocket::SocketError)));
        QObject::connect(clientSocketSSL, SIGNAL(connected()),this, SLOT(slotConnected()));

        // 🔐 Критически важный обработчик SSL-ошибок
        connect(clientSocketSSL, static_cast<void (QSslSocket::*)(const QList<QSslError>&)>(&QSslSocket::sslErrors),
                [this](const QList<QSslError> &errors) {
            // ваш обработчик
            for (const QSslError &e : errors)
                mds->interface->snBUSInput(QSNLogToContainer(
                                               BUSSERV_LOG_LEVEL_caution,
                                               tr("SSL error: %1").arg(e.errorString()),
                                               BUSSERV_LOG_CATEGORY_module,
                                               "mail", false), this);
            clientSocketSSL->ignoreSslErrors(errors);
        });

        clientSocketSSL->connectToHostEncrypted(Server, Port);

    } else {
        clientSocket = new QTcpSocket(this);
        QObject::connect(clientSocket, SIGNAL(readyRead()),this, SLOT(slotReadyRead()));
        QObject::connect(clientSocket, SIGNAL(disconnected()),this, SLOT(disconnected()));
        QObject::connect(clientSocket, SIGNAL(connected()),this, SLOT(slotConnected()));
        QObject::connect(clientSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(slotError(QAbstractSocket::SocketError)));
        clientSocket->connectToHost(Server, Port);
    }


}

void QsnWebPageMail::closeSocket()
{
    clearState();
    if (SSL && mds->supportSSL) {
        if (clientSocketSSL == Q_NULLPTR) return;
        clientSocketSSL->close();
        clientSocketSSL->disconnectFromHost();
        clientSocketSSL->deleteLater();
        clientSocketSSL = Q_NULLPTR;
    } else {
        if (clientSocket == Q_NULLPTR) return;
        clientSocket->disconnectFromHost();
        clientSocket->deleteLater();
        clientSocket = Q_NULLPTR;
    }
}

QIODevice *QsnWebPageMail::socket()
{
    if (SSL && mds->supportSSL) return clientSocketSSL;
    return clientSocket;
}

int QsnWebPageMail::getQueueItems()
{
    int count = 0;
    for(int i = 0; i < mailqueue.count(); i ++)
        count += mailqueue[i].content.count();
    return count;
}

int QsnWebPageMail::getRecipientsCount()
{
    int count = 0;
    for (int i = 0; i < mds->auth->countAccounts(); i ++)
        if (!mds->auth->atAccount(i)->email.isEmpty()) count ++;
    return count;
}

void QsnWebPageMail::sendNow()
{
    if (mds->auth->atAccount(currentAccountIndex)->email.isEmpty()) return;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Send mail manual"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString(),
                                   true), this);

    for (int i = mailqueue.count() - 1; i >= 0; i --)
        mailqueue[i].waitingUntil = QDateTime::currentDateTime();
    queueCheck();
}

void QsnWebPageMail::sendTest()
{
    if (mds->auth->atAccount(currentAccountIndex)->email.isEmpty()) return;

    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   tr("Send test mail"),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString(),
                                   true), this);
    addToQueue(tr("Test mail"), currentAccountIndex);
    sendNow();

}


QString QsnWebPageMail::encodeHeader(const QString &text)
{
    return "=?UTF-8?B?" + text.toUtf8().toBase64() + "?=";
}


