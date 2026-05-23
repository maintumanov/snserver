#include "qsnwebsocket.h"

QsnWebSocket::QsnWebSocket(QTcpSocket* Socket, QsnWebContainer *container, QObject *parent) : QObject(parent)
{
    clientSocket = Socket;
    webContainer = container;
    connect(Socket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
    connect(Socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    inputState = 0;
    inputDataSize = 0;
    xRequestedWith = "";
    dataMap.clear();
}

QsnWebSocket::~QsnWebSocket()
{
    delete clientSocket;
}

void QsnWebSocket::slotDisconnect()
{
    clientSocket->close();
}

void QsnWebSocket::checkData()
{

}

void QsnWebSocket::writeData(QByteArray data)
{
    clientSocket->write(data);
}

void QsnWebSocket::slotReadClient()
{
    inputStream.append(clientSocket->readAll());
    switch (inputState) {
    case 0: readHeader(); break;
    case 1: readPostData(); break;
    }
}

void QsnWebSocket::slotDisconnected()
{
    closeConnected(this);
    this->deleteLater();
}

void QsnWebSocket::readHeader()
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
    if (!isGET() && !isPOST()) slotDisconnect();
    getInputDataSize();
    getXrequestedWith();
    parseCookies();
    if (inputDataSize == 0) parseHeader();
    else { inputState = 1; readPostData(); };
}

void QsnWebSocket::readPostData()
{
    if (inputStream.size() < inputDataSize) return;
    inputData = inputStream;
    inputStream.clear();
    parseHeader();
}

QString QsnWebSocket::findHeaderOption(QString opionName)
{
    int icn = inputHeader.indexOf(opionName, 0);
    if (icn == -1) return QString();
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

void QsnWebSocket::findDataOption(QString *option, QString *value)
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

bool QsnWebSocket::isPOST()
{
    int icn = inputHeader.indexOf(QLatin1String("POST"), 0);
    return (icn == 0);
}

bool QsnWebSocket::isGET()
{
    int icn = inputHeader.indexOf(QLatin1String("GET"), 0);
    return (icn == 0);
}

bool QsnWebSocket::isJSON(QString *url)
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

void QsnWebSocket::getInputDataSize()
{
    QString rcl = findHeaderOption(QLatin1String("Content-Length"));
    if (rcl == QString("")) inputDataSize = 0;
    else inputDataSize = rcl.toInt();
}

void QsnWebSocket::getXrequestedWith()
{
    QString rcl = findHeaderOption(QLatin1String("X-Requested-With"));
    if (rcl == QString("")) xRequestedWith = "";
    else xRequestedWith = rcl;
}

void QsnWebSocket::getURL()
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

bool QsnWebSocket::checkAuthorized()
{
    if (!webContainer->isAuthorizationEnable()) return true;
    if (cookiesMap.contains("UID"))
        if (webContainer->accountIndexFromCookeID(cookiesMap["UID"].toLongLong()) != -1) return true;
    return false;
}

void QsnWebSocket::parseXHRdata()
{
    QString option;
    QString value;
    while (inputData.count() > 0) {
        findDataOption(&option, &value);
        dataMap.insert(option, value);
    }
}

void QsnWebSocket::parseCookies()
{
    QString option;
    QString value;
    QString cookes = findHeaderOption(QLatin1String("Cookie"));
    if (cookes == QString()) return;
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

bool QsnWebSocket::isUrlFileName(QString name)
{
    return (getContentTypeFromName(name) != QString(""));
}

QString QsnWebSocket::getContentTypeFromName(QString name)
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
    if (ex == QLatin1String("jpg")) return QLatin1String("image/jpeg");
    if (ex == QLatin1String("png")) return QLatin1String("image/png");
    if (ex == QLatin1String("svg")) return QLatin1String("image/svg+xml");
    if (ex == QLatin1String("log")) return QLatin1String("text/plain");
    if (ex == QLatin1String("json")) return QLatin1String("application/json");
    if (ex == QLatin1String("gpx")) return QLatin1String("application/gpx+xml");
    if (ex == QLatin1String("ttf")) return QLatin1String("application/x-font-ttf");
    if (ex == QLatin1String("woff")) return QLatin1String("application/x-font-woff");
    if (ex == QLatin1String("woff2")) return QLatin1String("application/x-font-woff2");

    return QString();
}

void QsnWebSocket::parseHeader()
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

void QsnWebSocket::replyFile(QString fileName)
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
    QTextStream *htextStream = new QTextStream(clientSocket);
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
        clientSocket->write(file.readAll());
        file.close();
    } else {
        emit webContainer->snBUSOutput(QSNLogToContainer(
                                      BUSSERV_LOG_LEVEL_information,
                                      QString(tr("WEB[%1] '%2' - file not found!")).arg(clientSocket->peerAddress().toString(), fileName),
                                      BUSSERV_LOG_CATEGORY_core,
                                      QString(),
                                      true), this);
    }
}

void QsnWebSocket::replyHTTP()
{
    int aIndex = -1;
    if (cookiesMap.contains("UID")) aIndex = webContainer->accountIndexFromCookeID(cookiesMap["UID"].toLongLong());
    // qDebug() << "cooke" <<  cookiesMap["UID"].toLongLong() << aIndex;
    if (aIndex != -1 || !webContainer->isAuthorizationEnable()) webContainer->getHtml(URL, &outputContent, aIndex);
    else webContainer->getAuthorizationHtml(URL, &outputContent);
    QTextStream *htextStream = new QTextStream(clientSocket);
    htextStream->setAutoDetectUnicode(true);
    *htextStream << QString(
                        "HTTP/1.0 200 Ok\r\n"
                        "Content-Type: text/html; charset=\"utf-8\"\r\n"
                        "Server: snServer\r\n"
                        "Content-Length: %1\r\n"
                        "Connection: close\r\n"
                        "\r\n").arg(outputContent.size());
    htextStream->flush();
    clientSocket->write(outputContent);
    delete htextStream;
}

void QsnWebSocket::replyJSON()
{
    webContainer->getJSON(URL, &inputData, &outputContent);

    QTextStream *htextStream = new QTextStream(clientSocket);
    htextStream->setAutoDetectUnicode(true);
    *htextStream << QString(
                        "HTTP/1.0 200 Ok\r\n"
                        "Content-Type: application/json; charset=\"utf-8\"\r\n"
                        "Server: snServer\r\n"
                        "Content-Length: %1\r\n"
                        "Connection: close\r\n"
                        "\r\n").arg(outputContent.size());
    htextStream->flush();
    clientSocket->write(outputContent);
    delete htextStream;
}

void QsnWebSocket::requestXHR()
{
    QByteArray returnData;
    QTextStream *htextStream;
    QTextStream *retStream;

    if (dataMap.contains("username") && dataMap.contains("password")) {
        QString login = dataMap["username"];
        QString password = dataMap["password"];
        QString remember = dataMap["remember"];
        QString error = clientSocket->peerAddress().toString();
        int accountIndex;
        webContainer->authorizationAttempt(login, password, &accountIndex, &error);
        retStream = new QTextStream(&returnData, QIODevice::WriteOnly);
        retStream->setAutoDetectUnicode(true);
        if (accountIndex == -1)  {
            *retStream << error;
            emit webContainer->snBUSOutput(QSNLogToContainer(
                                          BUSSERV_LOG_LEVEL_information,
                                          QString(tr("WEB [%1] - '%2' %3!")).arg(clientSocket->peerAddress().toString(), login, error),
                                          BUSSERV_LOG_CATEGORY_core,
                                          QString(),
                                          true), this);
        } else  {
            *retStream << QString("allowed");
            emit webContainer->snBUSOutput(QSNLogToContainer(
                                          BUSSERV_LOG_LEVEL_caution,
                                          QString(tr("WEB [%1] - '%2' Acces allowed!")).arg(clientSocket->peerAddress().toString(), login),
                                          BUSSERV_LOG_CATEGORY_core,
                                          QString(),
                                          true), this);
        }
        retStream->flush();
        delete retStream;

        htextStream = new QTextStream(clientSocket);
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
        clientSocket->write(returnData);
        delete htextStream;
        return;
    }

    if (!checkAuthorized()) {
        retStream = new QTextStream(&returnData, QIODevice::WriteOnly);
        retStream->setAutoDetectUnicode(true);
        *retStream << QString("denied");
        retStream->flush();
        delete retStream;

        htextStream = new QTextStream(clientSocket);
        htextStream->setAutoDetectUnicode(true);
        *htextStream << QString("HTTP/1.0 200 Ok\r\n"
                                "Content-Type: application/x-www-form-urlencoded\r\n"
                                "Server: snServer\r\n"
                                "Content-Length: %1\r\n"
                                "Connection: close\r\n"
                                "\r\n"
                                ).arg(returnData.size());
        htextStream->flush();
        clientSocket->write(returnData);
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

    htextStream = new QTextStream(clientSocket);
    htextStream->setAutoDetectUnicode(true);
    *htextStream << QString(
                        "HTTP/1.0 200 Ok\r\n"
                        "Content-Type: application/x-www-form-urlencoded\r\n"
                        "Server: snServer\r\n"
                        "Content-Length: %1\r\n"
                        "Connection: close\r\n"
                        "\r\n").arg(returnData.size());
    htextStream->flush();
    clientSocket->write(returnData);
    delete htextStream;

}





