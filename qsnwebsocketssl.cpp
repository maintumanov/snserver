#include "qsnwebsocketssl.h"

QsnWebSocketSSL::QsnWebSocketSSL(QsnWebContainer *container, QObject *parent) : QSslSocket(parent)
{
    //    V.webContainer = container;
    //    V.inputState = 0;
    //    V.inputDataSize = 0;
    //    V.xRequestedWith = "";
    //    V.dataMap.clear();
    //    V.socket = this;

    connect(this, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
    //
    webContainer = container;

    inputState = 0;
    inputDataSize = 0;
    xRequestedWith = "";
    dataMap.clear();

}

void QsnWebSocketSSL::slotReadClient()
{
    inputStream.append(readAll());
    switch (inputState) {
    case 0: readHeader(); break;
    case 1: readPostData(); break;
    }
}

void QsnWebSocketSSL::slotDisconnected()
{
    closeConnected(this);
    this->deleteLater();
}

void QsnWebSocketSSL::readHeader()
{
    // start_time = steady_clock::now();
    wtimer.start();
    QByteArray fs;
    fs.append(static_cast<char>(13));
    fs.append(static_cast<char>(10));
    fs.append(static_cast<char>(13));
    fs.append(static_cast<char>(10));
    int icn = inputStream.indexOf(fs, 0);
    if (icn == -1) return;
    inputHeader.resize(icn);
    for (int i = 0; i < icn; i ++) inputHeader[i] = inputStream[i];
    inputStream.remove(0,icn + 4);
    if (!isGET() && !isPOST()) disconnectFromHost();
    getInputDataSize();
    getXrequestedWith();
    parseCookies();
    if (inputDataSize == 0) parseHeader();
    else { inputState = 1; readPostData(); }
}

void QsnWebSocketSSL::readPostData()
{
    if (inputStream.size() < inputDataSize) return;
    inputData = inputStream;
    inputStream.clear();
    parseHeader();
}

QString QsnWebSocketSSL::findHeaderOption(QString opionName)
{
    int icn = inputHeader.indexOf(opionName, 0);
    if (icn == -1) return QString::null;
    icn += opionName.length();
    while (inputHeader[icn] == ' ') icn ++;
    while (inputHeader[icn] == ':') icn ++;
    while (inputHeader[icn] == ' ') icn ++;
    QString res;
    while (inputHeader[icn] != static_cast<char>(13) && icn < inputHeader.count()) {
        res.append(static_cast<char>(inputHeader[icn]));
        icn ++;
    }
    return res;
}

void QsnWebSocketSSL::findDataOption(QString *option, QString *value)
{
    int icn = 0;
    while (icn < inputData.count() && inputData[icn] != '=') icn ++;
    *option = inputData.left(icn);
    inputData.remove(0, icn + 1);
    icn = 0;
    while (icn < inputData.count() && inputData[icn] != '&') icn ++;
    *value = inputData.left(icn);
    inputData.remove(0, icn + 1);
}

bool QsnWebSocketSSL::isPOST()
{
    int icn = inputHeader.indexOf(QLatin1String("POST"), 0);
    return (icn == 0);
}

bool QsnWebSocketSSL::isGET()
{
    int icn = inputHeader.indexOf(QLatin1String("GET"), 0);
    return (icn == 0);
}

bool QsnWebSocketSSL::isJSON(QString *url)
{
    QString j = *url;
    if (j.left(1) == "/") j.remove(0, 1);
    if (j.left(4) == "json") {
        j.remove(0, 4);
        *url = j;
        return true;
    }
    return false;
}

void QsnWebSocketSSL::getInputDataSize()
{
    QString rcl = findHeaderOption(QLatin1String("Content-Length"));
    if (rcl == QString("")) inputDataSize = 0;
    else inputDataSize = rcl.toInt();
}

void QsnWebSocketSSL::getXrequestedWith()
{
    QString rcl = findHeaderOption(QLatin1String("X-Requested-With"));
    if (rcl == QString("")) xRequestedWith = "";
    else xRequestedWith = rcl;
}

void QsnWebSocketSSL::getURL()
{
    int icn = 0;
    URL.clear();
    while (inputHeader[icn] != ' ') icn ++;
    icn ++;
    while (inputHeader[icn] != ' ') {
        URL.append(static_cast<char>(inputHeader[icn]));
        icn ++;
    }
}

bool QsnWebSocketSSL::checkAuthorized()
{
    if (!webContainer->isAuthorizationEnable()) return true;
    if (cookiesMap.contains("UID"))
        if (webContainer->accountIndexFromCookeID(cookiesMap["UID"].toLongLong()) != -1) return true;
    return false;
}

void QsnWebSocketSSL::parseXHRdata()
{
    QString option;
    QString value;
    while (inputData.count() > 0) {
        findDataOption(&option, &value);
        dataMap.insert(option, value);
    }
}

void QsnWebSocketSSL::parseCookies()
{
    QString option;
    QString value;
    QString cookes = findHeaderOption(QLatin1String("Cookie"));
    if (cookes == QString::null) return;
    for (int i = cookes.count() - 1; i > 0; i -- )
        if (cookes[i] == ' ') cookes.remove(i,1);
    int icn;
    while (cookes.count() > 2) {
        icn = 0;
        while (icn < cookes.count() && cookes[icn] != '=') icn ++;
        option = cookes.left(icn);
        cookes.remove(0, icn + 1);
        icn = 0;
        while (icn < cookes.count() && cookes[icn] != ';') icn ++;
        value = cookes.left(icn);
        cookes.remove(0, icn + 1);
        cookiesMap.insert(option, value);
    }
}

bool QsnWebSocketSSL::isUrlFileName(QString name)
{
    return (getContentTypeFromName(name) != QString(""));
}

QString QsnWebSocketSSL::getContentTypeFromName(QString name)
{
    QString ex = name.section('.', -1);
    if (ex == name) return QString();
    ex = ex.toLower();
    if (ex == QLatin1String("html")) return QLatin1String("text/html");
    if (ex == QLatin1String("css")) return QLatin1String("text/css");
    if (ex == QLatin1String("js")) return QLatin1String("text/javascript");
    if (ex == QLatin1String("txt")) return QLatin1String("text/plain");
    if (ex == QLatin1String("ico")) return QLatin1String("image/x-icon");
    if (ex == QLatin1String("gif")) return QLatin1String("image/gif");
    if (ex == QLatin1String("jpeg")) return QLatin1String("image/jpeg");
    if (ex == QLatin1String("png")) return QLatin1String("image/png");
    if (ex == QLatin1String("svg")) return QLatin1String("image/svg+xml");
    if (ex == QLatin1String("log")) return QLatin1String("text/plain");
    if (ex == QLatin1String("json")) return QLatin1String("application/json");
    if (ex == QLatin1String("gpx")) return QLatin1String("application/gpx+xml");
    return QString();
}

void QsnWebSocketSSL::parseHeader()
{
    inputStream.clear();
    inputState = 2;
    getURL();

    if (xRequestedWith == QLatin1String("XMLHttpRequest")) {
        parseXHRdata();
        requestXHR();
    }
    else {
        if (isUrlFileName(URL)) replyFile(URL);
        else if (isJSON(&URL)) replyJSON();
        else replyHTTP();
    }

    inputState = 0;


    //  steady_clock::time_point clock_end = steady_clock::now();
    //  steady_clock::duration time_span = clock_end - start_time;
    //  double nseconds = double(time_span.count()) * steady_clock::period::num / steady_clock::period::den;
    int mseconds = wtimer.elapsed();
    if (webContainer->gmodules()->S_PTWR_MAX < mseconds) webContainer->gmodules()->S_PTWR_MAX = mseconds;
    if (webContainer->gmodules()->S_PTWR_MIN > mseconds) webContainer->gmodules()->S_PTWR_MIN = mseconds;
    webContainer->gmodules()->S_PTWR_COUNT ++;

    if (inputStream.size() > 0) slotReadClient();
}

void QsnWebSocketSSL::replyFile(QString fileName)
{
    qint64 fileSize = 0;
    QFile file(QLatin1String(":/") + fileName);
    if (fileName.contains(QLatin1String(".log"), Qt::CaseSensitive))
        file.setFileName(QSNHomeSubPath("server", "logs").absolutePath() + fileName);
    if (fileName.contains(QLatin1String(".gpx"), Qt::CaseSensitive))
        file.setFileName(QSNHomeSubPath("server", "owntracks").absolutePath() + fileName);
    if(file.open(QIODevice::ReadOnly)) fileSize = file.size();
    else {
        file.setFileName(QCoreApplication::applicationDirPath() + fileName);
        if(file.open(QIODevice::ReadOnly)) fileSize = file.size();
    }
    QTextStream *htextStream = new QTextStream(this);
    htextStream->setAutoDetectUnicode(true);
    *htextStream << QString(
                        "HTTP/1.0 200 Ok\r\n"
                        "Content-Type: %1\r\n"
                        "Server: snServer\r\n"
                        "Content-Length: %2\r\n"
                        "Connection: close\r\n"
                        "\r\n").arg(getContentTypeFromName(fileName)).arg(fileSize);
    htextStream->flush();
    if (file.isOpen()) {
        write(file.readAll());
        file.close();
    } else
        emit webContainer->snBUSOutput(QSNLogToContainer(
                                      BUSSERV_LOG_LEVEL_warning,
                                      QString(tr("WEBSSL [%1] '%2' - file not found!")).arg(peerAddress().toString(), fileName),
                                      BUSSERV_LOG_CATEGORY_core,
                                      QString(),
                                      true), this);
}

void QsnWebSocketSSL::replyHTTP()
{
    int aIndex = -1;
    if (cookiesMap.contains("UID")) aIndex = webContainer->accountIndexFromCookeID(cookiesMap["UID"].toLongLong());
    if (aIndex != -1 || !webContainer->isAuthorizationEnable()) webContainer->getHtml(URL, &outputContent, aIndex);
    else webContainer->getAuthorizationHtml(URL, &outputContent);
    QTextStream *htextStream = new QTextStream(this);
    htextStream->setAutoDetectUnicode(true);
    *htextStream << QString(
                        "HTTP/1.0 200 Ok\r\n"
                        "Content-Type: text/html; charset=\"utf-8\"\r\n"
                        "Server: snServer\r\n"
                        "Content-Length: %1\r\n"
                        "Connection: close\r\n"
                        "\r\n").arg(outputContent.size());
    htextStream->flush();
    write(outputContent);
    delete htextStream;
}

void QsnWebSocketSSL::replyJSON()
{
    webContainer->getJSON(URL, &inputData, &outputContent);

    QTextStream *htextStream = new QTextStream(this);
    htextStream->setAutoDetectUnicode(true);
    *htextStream << QString(
                        "HTTP/1.0 200 Ok\r\n"
                        "Content-Type: application/json; charset=\"utf-8\"\r\n"
                        "Server: snServer\r\n"
                        "Content-Length: %1\r\n"
                        "Connection: close\r\n"
                        "\r\n").arg(outputContent.size());
    htextStream->flush();
    this->write(outputContent);
    delete htextStream;
}

void QsnWebSocketSSL::requestXHR()
{
    QByteArray returnData;
    QTextStream *htextStream;
    QTextStream *retStream;

    if (dataMap.contains("username") && dataMap.contains("password")) {
        QString login = dataMap["username"];
        QString password = dataMap["password"];
        QString remember = dataMap["remember"];
        QString error = peerAddress().toString();
        int accountIndex;
        webContainer->authorizationAttempt(login, password, &accountIndex, &error);
        retStream = new QTextStream(&returnData, QIODevice::WriteOnly);
        retStream->setAutoDetectUnicode(true);
        if (accountIndex == -1)  {
            *retStream << error;
            emit webContainer->snBUSOutput(QSNLogToContainer(
                                          BUSSERV_LOG_LEVEL_warning,
                                          QString(tr("WEBSSL [%1] - '%2' %3!")).arg(peerAddress().toString(), login, error),
                                          BUSSERV_LOG_CATEGORY_core,
                                          QString(),
                                          true), this);
        } else  {
            *retStream << QString("allowed");
            emit webContainer->snBUSOutput(QSNLogToContainer(
                                          BUSSERV_LOG_LEVEL_warning,
                                          QString(tr("WEBSSL [%1] - '%2' Acces allowed!")).arg(peerAddress().toString(), login),
                                          BUSSERV_LOG_CATEGORY_core,
                                          QString(),
                                          true), this);
        }
        retStream->flush();
        delete retStream;

        htextStream = new QTextStream(this);
        htextStream->setAutoDetectUnicode(true);
        *htextStream << QString("HTTP/1.0 200 Ok\r\n");
        if (accountIndex > -1) *htextStream << webContainer->authorizationCooke(accountIndex, (remember == QLatin1String("false")));

        *htextStream << QString(
                            "Content-Type: application/x-www-form-urlencoded\r\n"
                            "Server: snServer\r\n"
                            "Content-Length: %1\r\n"
                            "Connection: close\r\n"
                            "\r\n"
                            ).arg(returnData.size());
        htextStream->flush();
        write(returnData);
        delete htextStream;
        return;
    }

    if (!checkAuthorized()) {
        retStream = new QTextStream(&returnData, QIODevice::WriteOnly);
        retStream->setAutoDetectUnicode(true);
        *retStream << QString("denied");
        retStream->flush();
        delete retStream;

        htextStream = new QTextStream(this);
        htextStream->setAutoDetectUnicode(true);
        *htextStream << QString("HTTP/1.0 200 Ok\r\n"
                                "Content-Type: application/x-www-form-urlencoded\r\n"
                                "Server: snServer\r\n"
                                "Content-Length: %1\r\n"
                                "Connection: close\r\n"
                                "\r\n"
                                ).arg(returnData.size());
        htextStream->flush();
        write(returnData);
        delete htextStream;
        return;
    }


    bool ok = false;
    qint64 uid = -1;
    if (cookiesMap.contains("UID")) uid = cookiesMap["UID"].toLongLong(&ok);
    if (!ok) uid = -1;

    QStringList jsonItems;
    retStream = new QTextStream(&returnData, QIODevice::WriteOnly);
    retStream->setAutoDetectUnicode(true);
    webContainer->actionItem(URL, &dataMap, &jsonItems, uid);
    if (jsonItems.count() > 0) {
        *retStream << QLatin1String("{");
        for(int i = 0; i < jsonItems.count() - 1; i ++) *retStream << jsonItems.at(i) << QLatin1String(",");
        *retStream << jsonItems.last();
        *retStream << QLatin1String("}");
    }
    retStream->flush();
    delete retStream;

    htextStream = new QTextStream(this);
    htextStream->setAutoDetectUnicode(true);
    *htextStream << QString(
                        "HTTP/1.0 200 Ok\r\n"
                        "Content-Type: application/x-www-form-urlencoded\r\n"
                        "Server: snServer\r\n"
                        "Content-Length: %1\r\n"
                        "Connection: close\r\n"
                        "\r\n").arg(returnData.size());
    htextStream->flush();
    write(returnData);
    delete htextStream;

}





