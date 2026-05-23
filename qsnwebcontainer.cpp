#include "qsnwebcontainer.h"

QsnWebContainer::QsnWebContainer(QObject *parent) : QsnWeb(0, Q_NULLPTR, parent)
{
    setObjectName(QObject::tr("Server SignalNet"));
    timeInterval = 10000;
    tcpServer = Q_NULLPTR;
    udpServer = Q_NULLPTR;
    udpClient = Q_NULLPTR;
    uartCan = Q_NULLPTR;

    devId = 0;
    portWeb = 8080;
    portSSLWeb = 4433;
    Address = QHostAddress::Any;
    serverState = false;
    Server = new QTcpServer(this);
    ServerSSL = new QsnWebSSLServer(this ,this);
    serverIsStart = true;

    versionID = QDateTime::currentMSecsSinceEpoch();

    //#ifdef Q_OS_WIN
    //    QTextCodec *codec = QTextCodec::codecForName("cp866");
    //    QTextCodec::setCodecForLocale(codec);
    //#endif
    eventDispalyLog(QString("SignalNet WEB server. Version %1").arg(QCoreApplication::applicationVersion()));

    modules.auth = new QsnWebAuthorization(this);
    modules.timeOut = 40000;
    modules.iconTheme = 0;
    modules.interfaceTheme = 5;
    modules.interface = new QsnInterface(this);
    modules.interface->setMemorySize(64);
    modules.interface->setDeviceTypeIndex(1000);
    modules.interface->setDeviceName(objectName());
    modules.latitude = 58.583439;
    modules.longitude = 49.656228;
    modules.timezone = 3;
    modules.supportMQTT = false;

    modules.io = new QsnIOTable(modules.interface, this);

    modules.settings = new QSettings(QSNHomeSubPath("server", "settings").absoluteFilePath(QLatin1String("webitemsettings.ini")),
                                     QSettings::IniFormat);
    modules.db = new QsnDB(this);
    modules.locations = new QsnLocations(this);

    modules.alert_status = 0;
    modules.alarm_status = -1;

    modules.S_PTWR_MAX = 0;
    modules.S_PTWR_MIN = 1;
    modules.S_PTWR_COUNT = 0;
    modules.S_TM_BEGIN = QDateTime::currentDateTime();

    checkSupportSSL();
    loadSettings();

    connect(Server,SIGNAL(newConnection()),this,SLOT(newConnection()));
    connect(modules.interface, SIGNAL(eventAddressChange()), this, SLOT(saveSettings()));
    connect(modules.interface, SIGNAL(eventMemoryLoaded()), this, SLOT(updateConfiguration()));
    connect(modules.interface, SIGNAL(eventFailureMemoryLoad()), this, SLOT(setDefaultConfiguration()));
    connect(modules.db, SIGNAL(snBUSOutput(QSNContainer,QObject*)), modules.interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(modules.auth, SIGNAL(snBUSOutput(QSNContainer,QObject*)), modules.interface, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(modules.interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), modules.auth, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(modules.interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
    connect(this, SIGNAL(snBUSOutput(QSNContainer,QObject*)), modules.interface, SLOT(snBUSInput(QSNContainer,QObject*)));

    pageInfo = new QsnWebPage404(0, &modules, this);
    constWidgets.append(pageInfo);
    pageAbout = new QsnWebPageAbout(0, &modules, this);
    constWidgets.append(pageAbout);
    pageLog = new QsnWebPageLog(0, &modules, this);
    constWidgets.append(pageLog);
    pageGraph = new QsnWebPageGraph(0, &modules, this);
    constWidgets.append(pageGraph);
    pageUsers = new QsnWebPageUsers(0, &modules, this);
    constWidgets.append(pageUsers);
    pageSettings = new QsnWebPageControlPanel(0, &modules, this);
    constWidgets.append(pageSettings);
    startPage = new QsnWebWidgetStartPage(0, &modules, &tcpServer, this);
    constWidgets.append(startPage);
    LogCon = new QsnWebPageLogConnector(0, &modules, this);
    constWidgets.append(LogCon);

    connect(pageAbout, SIGNAL(reload()), this, SLOT(updateConfiguration()));

    modules.interface->setMemoryFileName(QSNHomeSubPath("server", "settings").absoluteFilePath(QLatin1String("webappconf.wac")));

    modules.isEnableLog = modules.settings->value(QString("logEnable"), false).toBool();


    //    QByteArray tb;
    //    qreal v = 15;
    //    qDebug() << "QSNFixedPointToRAW" << v << v * (pow(10,2));
    //    QSNFixedPointToRAW(&tb, 1, v, 2);
    //    qDebug() << QSNRAWtoInt32(&tb, 1) << tb;
    //    qDebug() << "QSNRAWtoFixedPoint" << QSNRAWtoFixedPoint(&tb, 1, 2);

}

QsnWebContainer::~QsnWebContainer()
{

}

void QsnWebContainer::authorizationAttempt(QString login, QString password, int *aindex, QString *error)
{
    *aindex = -1;
    if (!modules.auth->isWEBAuthorization()) return;
    return modules.auth->authorizationAttempt(login, password, aindex, error);
}

QString QsnWebContainer::authorizationCooke(int accountIndex, bool isSession)
{
    if (!modules.auth->isWEBAuthorization()) return QString();
    return modules.auth->getCooke(accountIndex, isSession);
}

int QsnWebContainer::accountIndexFromCookeID(qint64 cookeID)
{
    if (!modules.auth->isWEBAuthorization()) return -1;
    return modules.auth->accountIndexFromCookeID(cookeID);
}

bool QsnWebContainer::isAuthorizationEnable()
{
    return (modules.auth->isWEBAuthorization());
}

quint16 QsnWebContainer::webPort()
{
    return portWeb;
}

void QsnWebContainer::snBUSInput(QSNContainer , QObject *)
{
    //qDebug() << QSNContainerToLogText(container, true);
    //    if (container.role == QSNContainer::information && container.Command > 4) {
    //        if (container.Sender == 0) {
    //            QStringList options = QString::fromUtf8(container.Data).split("/");
    //            if (options.count() == 1)
    //                modules.db->writeLog(QSNWarningLevelToText(container.Command) + container.info,
    //                                     options[0], sender->objectName());
    //            else
    //                modules.db->writeLog(QSNWarningLevelToText(container.Command) + container.info,
    //                                     options[0], options[1]);
    //        } else {
    //            QString dlabel = QString::fromUtf8(container.Data);
    //            if (dlabel.isEmpty()) dlabel = tr("Unknown device");
    //            modules.db->writeLog(QSNWarningLevelToText(container.Command) + container.info,
    //                                 QString("devicelog_a%1").arg(container.Sender), dlabel);
    //        }
    //    }

}

void QsnWebContainer::updateConfiguration()
{
    QByteArray data;
    QSNContainer container;
    bool tcpServerLoaded = false;
    bool udpServerLoaded = false;
    bool udpClientLoaded = false;
    bool uartCanLoaded = false;
    configureClear();

    container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_CONFIGUPDATE;
    container.Signal = BUSSERV_CONFIGUPDATE_BEGIN;
    modules.interface->snBUSInput(container, this);

    data = qUncompress(modules.interface->exportMemory());
    if (data.count() < 12) return;

    QDataStream In(&data, QIODevice::ReadOnly);
    In.setVersion(QDataStream::Qt_4_7);
    QMap<QString, QVariant> optionsMap;
    int count;
    QString opName;
    QVariant opValue;
    QString sig;
    quint32 id;

    In >> sig;

    if (sig != QString(QLatin1String("SrvCfg"))) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("Missing configuration signature"),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);
        serverIsStart = false;
        return;
    }
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Configuration load begin "),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("server"),
                         false), this);


    In >> sig;
    In >> devId;
    In >> sig;

    if (!sig.isEmpty()) setObjectName(sig);
    modules.interface->setDeviceName(objectName());

    // load options
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Configuration load modules"),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("server"),
                         false), this);
    In >> count;
    for (int i = 0; i < count; i ++) {
        In >> opName;
        opValue.load(In);
        optionsMap.insert(opName, opValue);
    }

    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Configuration load IO"),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("server"),
                         false), this);

    modules.io->loadIOFromStream(&In, this);

    // load objects
    In >> count;
    for (int i = 0; i < count; i ++) {
        In >> sig;
        In >> id;

        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             tr("Config load module") + " - " + sig,
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);

        if (sig == QString(QLatin1String("PG"))) {
            QsnWebPage *page = new QsnWebPage(id, &modules, this);
            page->fromStream(&In);
            widgets.append(page);
        }

        if (sig == QString(QLatin1String("ACAN5"))) {
            QsnWebAdapterCANv5 *adaptercan = new QsnWebAdapterCANv5(id, &modules, this);
            widgets.append(adaptercan);
            adaptercan->fromStream(&In);
            pageSettings->addSetting(adaptercan);
        }

        if (sig == QString(QLatin1String("AUC"))) {
            if (uartCan == Q_NULLPTR) uartCan = new QsnWebAdapterUartCan(id, &modules, this);
            uartCan->fromStream(&In);
            pageSettings->addSetting(uartCan);
            uartCanLoaded = true;
        }

        if (sig == QString(QLatin1String("ARG"))) {
            QsnWebAdapterRing *adapterring = new QsnWebAdapterRing(id, &modules, this);
            widgets.append(adapterring);
            adapterring->fromStream(&In);
            pageSettings->addSetting(adapterring);
        }

        if (sig == QString(QLatin1String("ATCPS"))) {
            if (tcpServer == Q_NULLPTR) tcpServer = new QsnWebAdapterTCPserver(id, &modules, this);
            tcpServer->fromStream(&In);
            tcpServer->adapterConnect();
            pageSettings->addSetting(tcpServer);
            tcpServerLoaded = true;
        }

        if (sig == QString(QLatin1String("AUDP"))) {
            if (udpClient == Q_NULLPTR) udpClient = new QsnWebAdapterUDP(id, &modules, this);
            udpClient->fromStream(&In);
            pageSettings->addSetting(udpClient);
            udpClientLoaded = true;
        }

        if (sig == QString(QLatin1String("AUDPS"))) {
            if (udpServer == Q_NULLPTR) udpServer = new QsnWebAdapterUDPserver(id, &modules, this);
            udpServer->fromStream(&In);
            udpServer->adapterConnect();
            pageSettings->addSetting(udpServer);
            udpServerLoaded = true;
        }

        if (sig == QString(QLatin1String("ALR"))) {
            QsnWebPageAlert *alertpage = new QsnWebPageAlert(id, &modules, this);
            widgets.append(alertpage);
            alertpage->fromStream(&In);
            pageSettings->addSetting(alertpage);
        }

        if (sig == QString(QLatin1String("SECAL"))) {
            QsnWebPageSecurityAlarm *alarmpage = new QsnWebPageSecurityAlarm(id, &modules, this);
            widgets.append(alarmpage);
            alarmpage->fromStream(&In);
            pageSettings->addSetting(alarmpage);
        }

        if (sig == QString(QLatin1String("MAIL"))) {
            QsnWebPageMail *pageMail = new QsnWebPageMail(id, &modules, this);
            widgets.append(pageMail);
            pageMail->fromStream(&In);
            pageSettings->addSetting(pageMail);
        }


        if (sig == QString(QLatin1String("PRS"))) {
            QsnWebPagePresence *pagePresence = new QsnWebPagePresence(id, &modules, this);
            widgets.append(pagePresence);
            pagePresence->fromStream(&In);
            pageSettings->addSetting(pagePresence);
        }

        if (sig == QString(QLatin1String("ELC"))) {
            QsnWebPageEnergy *pageEnergy = new QsnWebPageEnergy(id, &modules, this);
            widgets.append(pageEnergy);
            pageEnergy->fromStream(&In);
            pageSettings->addSetting(pageEnergy);
        }

        if (sig == QString(QLatin1String("SCH"))) {
            QsnWebPageSchedule *pageSchedule = new QsnWebPageSchedule(id, &modules, this);
            widgets.append(pageSchedule);
            pageSchedule->fromStream(&In);
            pageSettings->addSetting(pageSchedule);
        }

        if (sig == QString(QLatin1String("NT"))) {
            QsnWebPageNighttime *nightTime = new QsnWebPageNighttime(id, &modules, this);
            widgets.append(nightTime);
            nightTime->fromStream(&In);
            pageSettings->addSetting(nightTime);
        }

        if (sig == QString(QLatin1String("DT"))) {
            QsnWebPageDaytime *dayTime = new QsnWebPageDaytime(id, &modules, this);
            widgets.append(dayTime);
            dayTime->fromStream(&In);
            pageSettings->addSetting(dayTime);
        }

        if (sig == QString(QLatin1String("MSGS"))) {
            QsnWebPageMessages *msgs = new QsnWebPageMessages(id, &modules, this);
            widgets.append(msgs);
            msgs->fromStream(&In);
            pageSettings->addSetting(msgs);
        }

        if (sig == QString(QLatin1String("GSM"))) {
            QsnWebPageGSM *pageGSM = new QsnWebPageGSM(id, &modules, this);
            widgets.append(pageGSM);
            pageGSM->fromStream(&In);
            pageSettings->addSetting(pageGSM);
        }

        if (sig == QString(QLatin1String("PZEM"))) {
            QsnWebPagePZEM *pagePZEM = new QsnWebPagePZEM(id, &modules, this);
            widgets.append(pagePZEM);
            pagePZEM->fromStream(&In);
            pageSettings->addSetting(pagePZEM);
        }

        if (sig == QString(QLatin1String("DBL"))) {
            QsnWebPageDoorBell *pageDBell = new QsnWebPageDoorBell(id, &modules, this);
            widgets.append(pageDBell);
            pageDBell->fromStream(&In);
            pageSettings->addSetting(pageDBell);
        }

        if (sig == QString(QLatin1String("OWT"))) {
            QsnWebPageOwnTracks *pageOT = new QsnWebPageOwnTracks(id, &modules, this);
            widgets.append(pageOT);
            pageOT->fromStream(&In);
            pageSettings->addSetting(pageOT);
        }

        if (sig == QString(QLatin1String("KNT"))) {
            QsnWebPageKeenetic *pageK = new QsnWebPageKeenetic(id, &modules, this);
            widgets.append(pageK);
            pageK->fromStream(&In);
            pageSettings->addSetting(pageK);
        }

        if (sig == QString(QLatin1String("YLG"))) {
            QsnWebPageYeelight *pageYeelight = new QsnWebPageYeelight(id, &modules, this);
            widgets.append(pageYeelight);
            pageYeelight->fromStream(&In);
            pageSettings->addSetting(pageYeelight);
        }

        if (sig == QString(QLatin1String("NRM"))) {
            QsnWebPageNarodmon *pageNM = new QsnWebPageNarodmon(id, &modules, this);
            widgets.append(pageNM);
            pageNM->fromStream(&In);
            pageSettings->addSetting(pageNM);
        }

        if (sig == QString(QLatin1String("NTF"))) {
            QsnWebPageNotification *pageNt = new QsnWebPageNotification(id, &modules, this);
            widgets.append(pageNt);
            pageNt->fromStream(&In);
            pageSettings->addSetting(pageNt);
        }

        if (sig == QString(QLatin1String("SLG"))) {
            QsnWebPageSyslogConnector *pageSyslog = new QsnWebPageSyslogConnector(id, &modules, this);
            widgets.append(pageSyslog);
            pageSyslog->fromStream(&In);
            pageSettings->addSetting(pageSyslog);
        }

        if (sig == QString(QLatin1String("MQTT"))) {
            QsnWebAdapterMQTT *adapterMQTT = new QsnWebAdapterMQTT(id, &modules, this);
            widgets.append(adapterMQTT);
            adapterMQTT->fromStream(&In);
            adapterMQTT->adapterConnect();
            pageSettings->addSetting(adapterMQTT);
        }

        if (sig == QString(QLatin1String("WFSN"))) {
            QsnWebPageWiFiSniffer *pageWFS = new QsnWebPageWiFiSniffer(id, &modules, this);
            widgets.append(pageWFS);
            pageWFS->fromStream(&In);
            pageSettings->addSetting(pageWFS);
        }

        if (sig == QString(QLatin1String("SNC"))) {
            QsnWebAdapterSynch *adapterSynch = new QsnWebAdapterSynch(id, &modules, this);
            widgets.append(adapterSynch);
            adapterSynch->fromStream(&In);
            adapterSynch->adapterConnect();
            pageSettings->addSetting(adapterSynch);
        }

        if (sig == QString(QLatin1String("SM"))) {
            QsnWebPageSNIRF *snirf = new QsnWebPageSNIRF(id, &modules, this);
            widgets.append(snirf);
            snirf->fromStream(&In);
            snirf->adapterConnect();
            pageSettings->addSetting(snirf);
        }
    }

    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Configuration applying settings"),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("server"),
                         false), this);

    modules.io->loadSignalsStream(&In);
    modules.locations->loadFromStream(&In);

    timeInterval = optionsMap.value(QLatin1String("TI"), 10000).toUInt();
    modules.timeOut = optionsMap.value(QLatin1String("TO"), 4000).toInt();

    modules.latitude = optionsMap.value(QLatin1String("LAT"), static_cast<double>(58.583439)).toDouble();
    modules.longitude = optionsMap.value(QLatin1String("LON"), static_cast<double>(49.656228)).toDouble();
    modules.timezone = static_cast<qint8>(optionsMap.value(QLatin1String("TMZ"), 3).toInt());

    modules.auth->setAdminLogin(optionsMap.value(QLatin1String("LN"), "administrator").toString());
    modules.auth->setAdminPassword(optionsMap.value(QLatin1String("PW"), "signalnet").toString());
    modules.auth->setEnableWEBAuthorization(optionsMap.value(QLatin1String("WA"), false).toBool());
    modules.auth->setEnableTCPAuthorization(optionsMap.value(QLatin1String("TCPA"), false).toBool());
    modules.auth->setLogEnable(optionsMap.value(QLatin1String("LG"), false).toBool());
    modules.auth->setEnableNotifyBlock(optionsMap.value(QLatin1String("MB"), true).toBool());
    notyfyLaunch = optionsMap.value(QLatin1String("ML"), true).toBool();
    modules.auth->loadSetting();

    setListenWebPorts(static_cast<quint16>(optionsMap.value(QLatin1String("PRW"), 80).toUInt()),
                      static_cast<quint16>(optionsMap.value(QLatin1String("PRSW"), 443).toUInt()));

    if (!tcpServerLoaded && (tcpServer != Q_NULLPTR)) {
        tcpServer->adapterDisconnect();
        delete tcpServer;
        tcpServer = Q_NULLPTR;
    }

    if (!udpServerLoaded && (udpServer != Q_NULLPTR)) {
        udpServer->adapterDisconnect();
        delete udpServer;
        udpServer = Q_NULLPTR;
    }

    if (!udpClientLoaded && (udpClient != Q_NULLPTR)) {
        udpClient->adapterDisconnect();
        delete udpClient;
        udpClient = Q_NULLPTR;
    }

    if (!uartCanLoaded && (uartCan != Q_NULLPTR)) {
        uartCan->adapterDisconnect();
        delete uartCan;
        uartCan = Q_NULLPTR;
    }

    modules.interface->actionConnect();

    for (int i = 0; i < widgets.count(); i ++) widgets[i]->endConfiguration();
    if (tcpServer != Q_NULLPTR) tcpServer->endConfiguration();
    if (udpServer != Q_NULLPTR) udpServer->endConfiguration();
    if (udpClient != Q_NULLPTR) udpClient->endConfiguration();
    if (uartCan != Q_NULLPTR)   uartCan->endConfiguration();

    QTimer::singleShot(optionsMap.value(QLatin1String("IDL"), 10000).toInt(), this, SLOT(timerInitDelay()));

    pageSettings->addSetting(pageUsers);
    pageSettings->addSetting(pageGraph);
    pageSettings->addSetting(pageLog);
    pageSettings->addSetting(LogCon);
    pageSettings->addSetting(pageAbout);

    versionID = QDateTime::currentMSecsSinceEpoch();

    container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_CONFIGUPDATE;
    container.Signal = BUSSERV_CONFIGUPDATE_END;
    modules.interface->snBUSInput(container, this);

    if (serverIsStart && notyfyLaunch) {
        container = newContainer();
        container.role = QSNContainer::service;
        container.Command = BUSSERV_MESSAGE;
        container.Signal = 3;
        container.Sender = 0;
        container.info = "(i)" + tr("The server is running");
        modules.interface->snBUSInput(container, this);
        serverIsStart = false;
    }

    modules.db->tempSettings.clear();
}

void QsnWebContainer::setDefaultConfiguration()
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_warning,
                         tr("Set default configuration."),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("server"),
                         false), this);
    QByteArray conf;
    createDefaulConfiguration(&conf);
    modules.interface->loadMemoryFromByteArray(&conf);
    updateConfiguration();
    serverIsStart = false;
}

void QsnWebContainer::createDefaulConfiguration(QByteArray *conf)
{
    QVariant value;
    QDataStream Out(conf, QIODevice::WriteOnly);
    Out.setVersion(QDataStream::Qt_4_7);
    Out << QString(QLatin1String("SrvCfg"));
    // Server
    Out << QString("Device");
    Out << static_cast<int>(0);
    Out << QString(tr("Server"));

    // Server - options
    Out << static_cast<int>(6);
    Out << QString("PRW");
    value = static_cast<int>(8080);
    value.save(Out);
    Out << QString("TI");
    value = static_cast<int>(5000);
    value.save(Out);
    Out << QString("LN");
    value = "administrator";
    value.save(Out);
    Out << QString("PW");
    value = "signalnet";
    value.save(Out);
    Out << QString("WA");
    value = false;
    value.save(Out);
    Out << QString("TCPA");
    value = false;
    value.save(Out);

    // Server - io (count 0)
    Out << static_cast<int>(0);

    // Server - save children
    Out << static_cast<int>(1);

    // Server - save children tcp - server
    Out << QString("ATCPS");
    Out << static_cast<int>(1);
    Out << QString(tr("Adapter TCP server"));

    // Server - save children tcp - options
    Out << static_cast<int>(1);
    Out << QString("PR");
    value = static_cast<int>(8888);
    value.save(Out);

    // Server - save children tcp - server - io (count 0)
    Out << static_cast<int>(0);

    // Server - save children tcp - server - childrens count
    Out << static_cast<int>(0);

    //========================================
    // Server - io
    Out << static_cast<int>(0);
    // Server - locations
    Out << static_cast<int>(0);
    *conf = qCompress(*conf, 9);

}

void QsnWebContainer::eventDispalyLog(QString text)
{
#ifdef Q_OS_WIN
    QTextCodec *codec = QTextCodec::codecForName("cp866");
    QTextCodec::setCodecForLocale(codec);
#endif
    qDebug() << QString("[%1] %2").arg(QTime::currentTime().toString("hh:mm:ss"), text);
#ifdef Q_OS_WIN
    codec  = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
#endif
}

void QsnWebContainer::actionConnect()
{
    if (serverState == true) return;
    if (Server->listen(Address, portWeb)) {
        serverState = true;
        eventDispalyLog(tr("HTTP port: %1").arg(portWeb));
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             tr("HTTP port: %1").arg(portWeb),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);
    }
    if (modules.supportSSL) {
        if (ServerSSL->listen(Address, portSSLWeb)) {
            serverState = true;
            eventDispalyLog(tr("HTTPS port: %1").arg(portSSLWeb));
            emit snBUSOutput(QSNLogToContainer(
                                 BUSSERV_LOG_LEVEL_information,
                                 tr("HTTPS port: %1").arg(portSSLWeb),
                                 BUSSERV_LOG_CATEGORY_core,
                                 QString("server"),
                                 false), this);
        }
    } else {
        eventDispalyLog(tr("HTTPS not supported!"));
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("HTTPS not supported!"),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);
    }
}

void QsnWebContainer::actionDisconnect()
{
    if (serverState == false) return;
    for (int i = ServerClients.count() - 1; i >= 0; i --)
        ServerClients[i]->slotDisconnect();
    Server->close();
    ServerSSL->close();
    serverState = false;
    eventDispalyLog(tr("Web server stoped."));
}

void QsnWebContainer::setListenWebPorts(quint16 port, quint16 sslport)
{
    if ((port == portWeb && sslport == portSSLWeb) && serverState) return;
    if (port != portWeb || sslport != portSSLWeb) actionDisconnect();
    portWeb = port;
    portSSLWeb = sslport;
    actionConnect();
}

QsnGlobalModules *QsnWebContainer::gmodules()
{
    return &modules;
}

void QsnWebContainer::getHtml(QString url, QByteArray *content, int accountIndex)
{
    QString pageURL = url;
    if (url.indexOf('?', 0)) pageURL = url.left(url.indexOf('?', 0));
    QsnWeb *w = getWidgetFromUrl(pageURL);
    QTextStream *htextStream = new QTextStream(content);
    htextStream->setAutoDetectUnicode(true);
    QStringList jsdepending;
    QStringList cssdepending;
    QStringList functions;
    QStringList funonload;
    QStringList fjson;
    QStringList dialogs;
    QStringList contents;
    QStringList contentsToolbar;
    QStringList js;

    bool wEnable = modules.auth->checkAcces(accountIndex, w->widgetAccessRights);
    if (wEnable) w->treatmentUrl(url, accountIndex);
    *htextStream << QString("<!DOCTYPE html><html><head><title>");
    *htextStream << objectName();
    *htextStream << QString("</title>"
                            "<meta charset=\"utf-8\">"
                            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, shrink-to-fit=no\"/>"
                            "<meta name=\"description\" content=\"\">"
                            "<meta name=\"author\" content=\"Stanislav Tumanov - signalnet.ru\">"
                            "<link href=\"assets/css/iot-theme-bundle.min.css\" rel=\"stylesheet\">");
    if (wEnable) w->getCSSDepending(&cssdepending, accountIndex);
    cssdepending.removeDuplicates();
    for (int i = 0; i < cssdepending.count(); i ++) *htextStream << QString("<link href=\"%1\" rel=\"stylesheet\">").arg(cssdepending.at(i));
    *htextStream << QString("</head>");

    //  QString fad; //===========================================================================================================!!!!!!!!!!!!!!!!!!!!!

    *htextStream << QString("<body>");
    // -- Preloader --
    *htextStream << QString("<div id=\"iot-preloader\">"
                            "<div class=\"center-preloader d-flex align-items-center\">"
                            "<div class=\"spinners\">"
                            "<div class=\"spinner01\"></div>"
                            "<div class=\"spinner02\"></div>"
                            "</div></div></div>");

    // Модальное окно оповещения==============================================================
    //    // -- Alerts Modal --
    //    *htextStream << QString("<div class=\"modal modal-nobg centered fade\" id=\"alertsModal\" tabindex=\"-1\" role=\"dialog\" aria-label=\"Alerts\" aria-hidden=\"true\">");
    //    *htextStream << QString("<div class=\"modal-dialog\" role=\"document\">");
    //    *htextStream << QString("<div class=\"modal-content\">");
    //    *htextStream << QString("<div class=\"modal-body\">");
    //    // -- Alerts Modal - message begin --
    //    *htextStream << QString("<div class=\"alert alert-danger alert-dismissible fade show border-0\" role=\"alert\">");
    //    *htextStream << QString("<button type=\"button\" class=\"close\" data-bs-dismiss=\"alert\" aria-label=\"Close\">");
    //    *htextStream << QString("<span aria-hidden=\"true\">&times;</span>");
    //    *htextStream << QString("</button> Security SW update available");
    //    *htextStream << QString("</div>");
    //    // -- Alerts Modal - message end --
    //    *htextStream << QString("</div></div></div>");
    //    *htextStream << QString("<button type=\"button\" class=\"close close-modal\" data-bs-dismiss=\"modal\" aria-label=\"Close\">");
    //    *htextStream << QString("<span aria-hidden=\"true\">&times;</span>");
    //    *htextStream << QString("</button></div>");

    // -- Arming Modal --
    //    *htextStream << QString("<div class=\"modal modal-warning centered fade\" id=\"armModal\" tabindex=\"-1\" role=\"dialog\" aria-label=\"Arming\" aria-hidden=\"true\">");

    //    *htextStream << QString("<div class=\"modal-dialog modal-sm\" role=\"document\">");
    //    *htextStream << QString("<div class=\"modal-content\">"
    //                            "<div class=\"modal-body\">"
    //                            "<div id=\"armTimer\">");
    //    *htextStream << QString("<h3 class=\"font-weight-bold\">EXIT NOW! <span class=\"timer font-weight-normal\"></span></h3>");
    //    *htextStream << QString("</div></div></div></div>");
    //    *htextStream << QString("<button type=\"button\" class=\"close close-modal\" data-bs-dismiss=\"modal\" aria-label=\"Close\">");
    //    *htextStream << QString("<span aria-hidden=\"true\">&times;</span>");
    //    *htextStream << QString("</button></div>");


    // -- Alarm Modal --
    //    *htextStream << QString("<div class=\"modal modal-danger centered fade\" id=\"alarmModal\" tabindex=\"-1\" "
    //                            "role=\"dialog\" aria-label=\"ALARM\" aria-hidden=\"true\" data-backdrop=\"static\">");
    //    *htextStream << QString("<div class=\"modal-dialog\" role=\"document\">"
    //                            "<div class=\"modal-content\" data-bs-dismiss=\"modal\">"
    //                            "<div class=\"modal-body d-flex\">");
    //    *htextStream << QString("<svg class=\"icon-sprite icon-2x icon-pulse\"><use xlink:href=\"assets/images/icons-sprite.svg#alarm\"/></svg>");
    //    *htextStream << QString("<h3 class=\"text-right font-weight-bold ms-auto align-self-center\">MOTION DETECTED!</h3>");
    //    *htextStream << QString("</div></div>");
    //    *htextStream << QString("<p class=\"mt-2 text-center text-danger\">Click the red area to accept/close message</p>");
    //    *htextStream << QString("</div></div>");

    //-- Wrapper START --
    *htextStream << QString("<div id=\"wrapper\" class=\"hidden\">");

    //-- Top navbar START --
    *htextStream << QString("<nav class=\"navbar navbar-expand fixed-top d-flex flex-row justify-content-start\">");
    *htextStream << QString("<div class=\"d-none d-lg-block\"><form><div id=\"menu-minifier\"><label>");
    *htextStream << QString("<svg width=\"32\" height=\"32\" viewBox=\"0 0 32 32\">"
                            "<rect x=\"2\" y=\"8\" width=\"4\" height=\"3\" class=\"menu-dots\"></rect>"
                            "<rect x=\"2\" y=\"15\" width=\"4\" height=\"3\" class=\"menu-dots\"></rect>"
                            "<rect x=\"2\" y=\"22\" width=\"4\" height=\"3\" class=\"menu-dots\"></rect>"
                            "<rect x=\"8\" y=\"8\" width=\"21\" height=\"3\" class=\"menu-lines\"></rect>"
                            "<rect x=\"8\" y=\"15\" width=\"21\" height=\"3\" class=\"menu-lines\"></rect>"
                            "<rect x=\"8\" y=\"22\" width=\"21\" height=\"3\" class=\"menu-lines\"></rect>"
                            "</svg>");
    *htextStream << QString("<input id=\"minifier\" type=\"checkbox\"></label>");
    *htextStream << QString("<div class=\"info-holder info-rb\">");
    *htextStream << QString("<div data-bs-toggle=\"popover-all\" data-content=\"Checkbox element using localStorage to remember the last status.\" data-original-title=\"Side menu narrowing\" data-placement=\"right\"></div>");
    *htextStream << QString("</div></div></form></div>");

    *htextStream << QString("<a class=\"navbar-brand px-lg-3 px-1 me-0\" id=\"brand\" href=\"/\">%1</a>").arg(objectName());
    *htextStream << QString("<div class=\"ms-auto\">"
                            "<div class=\"navbar-nav flex-row navbar-icons\">");
    //Пиктограмма оповещения на панели==============================================
    //    *htextStream << QString("<div class=\"nav-item\">");
    //    *htextStream << QString("<button id=\"alerts-toggler\" class=\"btn btn-link nav-link\" title=\"Alerts\" type=\"button\" data-alerts=\"1\" "); //data-alerts количество сообщений
    //    *htextStream << QString("data-bs-toggle=\"modal\" data-bs-target=\"#alertsModal\">");
    //    *htextStream << QString("<svg class=\"icon-sprite\"><use xlink:href=\"assets/images/icons-sprite.svg#alert\"/>");
    //    *htextStream << QString("<svg class=\"text-danger\"><use class=\"icon-dot\" xlink:href=\"assets/images/icons-sprite.svg#icon-dot\"/></svg>");
    //    *htextStream << QString("</svg></button></div>");

    *htextStream << QString("<div id=\"user-menu\" class=\"nav-item dropdown\">");
    *htextStream << QString("<button class=\"btn btn-link nav-link dropdown-toggle\" title=\"User\" type=\"button\" data-bs-toggle=\"dropdown\" aria-haspopup=\"true\" aria-expanded=\"false\">");
    *htextStream << QString("<svg class=\"icon-sprite\"><use xlink:href=\"assets/images/icons-sprite.svg#user\"/></svg></button>");
    *htextStream << QString("<div class=\"dropdown-menu dropdown-menu-right\">");

    if (accountIndex > -1 && accountIndex < 65535) {
        *htextStream << QString("<a class=\"dropdown-item\" href=\"users?AID=%2\">%1</a>").arg(tr("Profile")).arg(modules.auth->accountUIDfromIndex(accountIndex));
        *htextStream << QString("<div class=\"dropdown-divider\"></div>");
    }
    // qDebug() << accountIndex << modules.auth->accountUIDfromIndex(accountIndex);

    *htextStream << QString("<a class=\"dropdown-item\" href=\"\" id=\"logout\">%1</a>").arg(tr("Logout"));
    *htextStream << QString("</div></div>");
    *htextStream << QString("<div class=\"nav-item d-lg-none\">"
                            "<button id=\"sidebar-toggler\" type=\"button\" class=\"btn btn-link nav-link\" data-bs-toggle=\"offcanvas\">");
    *htextStream << QString("<svg class=\"icon-sprite\"><use xlink:href=\"assets/images/icons-sprite.svg#menu\"/></svg>");
    *htextStream << QString("</button></div></div></div></nav>");

    //-- wrapper-offcanvas START --
    *htextStream << QString("<div class=\"wrapper-offcanvas\">");

    //-- row-offcanvas START --
    *htextStream << QString("<div class=\"row-offcanvas row-offcanvas-left\">");

    //-- Side menu START --
    *htextStream << QString("<div id=\"sidebar\" class=\"sidebar-offcanvas\">"
                            "<ul class=\"nav flex-column nav-sidebar\">");
    //-- Side menu - home menu item

    // *htextStream << QsnBsNavItem(tr("Home"), "home", "/", pageURL == QLatin1String("/"));// Исправить на название страницы
    //-- Side menu - next menu item
    bool isCurrentAdd = false;
    for(int i = 0; i < widgets.count(); i ++)
        if (widgets[i]->widgetMenu && widgets[i]->widgetUrl == QLatin1String("/")) {
            *htextStream << QsnBsNavItem(widgets[i]->objectName(), widgets[i]->widgetIcon.isEmpty()?"home":widgets[i]->widgetIcon, widgets[i]->widgetUrl, pageURL == widgets[i]->widgetUrl);
        }

    for(int i = 0; i < widgets.count(); i ++)
        if (widgets[i]->widgetMenu && widgets[i]->widgetUrl != QLatin1String("/")) {
            if (widgets[i] == w) isCurrentAdd = true;
            *htextStream << QsnBsNavItem(widgets[i]->objectName(), widgets[i]->widgetIcon , widgets[i]->widgetUrl, pageURL == widgets[i]->widgetUrl);
        }

    for(int i = 0; i < constWidgets.count(); i ++)
        if (constWidgets[i]->widgetMenu) {
            if (constWidgets[i] == w) isCurrentAdd = true;
            *htextStream << QsnBsNavItem(constWidgets[i]->objectName(), constWidgets[i]->widgetIcon, constWidgets[i]->widgetUrl, pageURL == constWidgets[i]->widgetUrl);
        }

    if (!isCurrentAdd && pageURL != QLatin1String("/"))
        *htextStream << QsnBsNavItem(w->objectName(), w->widgetIcon, w->widgetUrl, true);

    //-- Side menu - settings menu item
    //    *htextStream << QString("<li class=\"nav-item\">");
    //    *htextStream << QString("<a class=\"nav-link %1\" href=\"settings\">").arg(pageURL == QLatin1String("/settings")?"active":"");
    //    *htextStream << QString("<svg class=\"icon-sprite\"><use xlink:href=\"assets/images/icons-sprite.svg#settings\"/></svg> %1</a></li>").arg(tr("Settings"));
    // *htextStream << QsnBsNavItem(tr("Settings"), "settings", "settings", pageURL == QLatin1String("/settings"));

    // ---
    *htextStream << QString("</ul></div>");
    //-- Side menu END --
    //-- Main content START --
    *htextStream << QString("<div id=\"main\">");
    if (w->widgetColPage)  *htextStream << QString("<div class=\"container\"><div class=\"card-columns\">");
    else *htextStream << QString("<div class=\"container-fluid\"><div class=\"row\">");
    //-- this content --


    if (wEnable) w->getContents(&contents, accountIndex);
    for(int i = 0; i < contents.count(); i ++) *htextStream << contents.at(i);


    *htextStream << QString("</div></div>");
    //-- Main content overlay when side menu appears  --
    *htextStream << QString("<div class=\"cover-offcanvas\" data-bs-toggle=\"offcanvas\"></div></div></div></div></div>");
    //-- Wrapper END --
    //-- FAB button - bottom right on large screens --
    //    *htextStream << QString("<button id=\"info-toggler\" type=\"button\" class=\"btn btn-primary btn-fab btn-fixed-br d-none d-lg-inline-block\">");
    //    *htextStream << QString("<svg class=\"icon-sprite\"><use xlink:href=\"assets/images/icons-sprite.svg#info\"/></svg></button>");

    //-- jQuery --
    *htextStream << QString("<script src=\"assets/js/jquery-3.4.1.min.js\"></script>");

    //-- Bootstrap bundle --
    *htextStream << QString("<script src=\"assets/js/bootstrap.bundle.min.js\"></script>");
    *htextStream << QString("<link rel=\"stylesheet\" href=\"assets/css/bootstrap.min.css\">");

    //-- Cross browser support for SVG icon sprites --
    *htextStream << QString("<script src=\"assets/js/svg4everybody.min.js\"></script>");

    //-- jQuery countdown timer plugin (Exit modal, Garage doors, Washing machine) --
    *htextStream << QString("<script src=\"assets/js/iot-timer.min.js\"></script>");

    //-- Basic theme functionality (arming, garage doors, switches ...) - using jQuery --
    *htextStream << QString("<script src=\"assets/js/iot-functions.min.js\"></script>");
    if (wEnable) w->getJSDepending(&jsdepending, accountIndex);
    jsdepending.removeDuplicates();
    for (int i = 0; i < jsdepending.count(); i ++) *htextStream << QString("<script src=\"%1\"></script>").arg(jsdepending.at(i));


    // -- Local scripts
    *htextStream << QString("<script>");

    //==== widjet script begin ===========
    if (timeInterval < 200) timeInterval = 200;

    functions << QString("var pageVersion = '%1';").arg(versionID);
    functions << QString("var requestTimeOut = '%1';").arg(timeInterval / 10);

    //if (wEnable || (pageURL == QLatin1String("/settings"))) w->getFunctions(&functions, accountIndex, url);
    if (wEnable) w->getFunctions(&functions, accountIndex, url);
    for (int i = 0; i < functions.count(); i ++)  *htextStream << functions.at(i);

    fjson << QString("if (pageVersion != data.pageversion) { document.location.reload();}");

    if (wEnable) w->getFunctionsJSON(&fjson, accountIndex);

    if (fjson.count() > 0) {

        // статус соединения
        *htextStream << QString("function setConnectedState(state){");
        *htextStream << QString("if (state == false) {$('#brand').addClass(\"navbar-disconnected\"); } "
                                "else { $('#brand').removeClass(\"navbar-disconnected\"); }");
        *htextStream << QString("}; ");

        // обработка json
        *htextStream << QString("function requestProcess(data){");
        for(int i = 0; i < fjson.count(); i ++) *htextStream << fjson.at(i);
        *htextStream << QString("requestTimeOut = %1;").arg(timeInterval / 10);
        *htextStream << QString("setConnectedState(true);");
        *htextStream << QString("}; ");

        // опрос сотояния
        *htextStream << QString("function requestState(){"
                                " $.ajax({"
                                "url: \"%1\","
                                "type: 'POST',"
                                "data: {'action':'request'},"
                                "dataType: 'json',"
                                "contentType: \"application/json; charset=utf-8\","
                                "success: function(data) { requestProcess(data);").arg(url);
        *htextStream << QString("}, error: function (x, t, e) { setConnectedState(false);");
        *htextStream << QString("}");
        *htextStream << QString("}); }; ");

        *htextStream << QString("function sendState(vaction, vid, vstate){"
                                " $.ajax({"
                                "url: \"%1\","
                                "type: 'POST',"
                                "data: {action: vaction, id: vid, state: vstate},"
                                "dataType: 'json',"
                                "contentType: \"application/json; charset=utf-8\","
                                "success: function(data) { requestProcess(data);").arg(url);
        *htextStream << QString("}, error: function (x, t, e) { setConnectedState(false);");
        *htextStream << QString("}");
        *htextStream << QString("}); }; ");

        *htextStream << QString("$('#logout').click(function() {"
                                " $.ajax({"
                                "url: \"%1\","
                                "type: 'POST',"
                                "data: {action : 'action' , id : 'logout', state : 'logout'},"
                                "dataType: 'json',"
                                "contentType: \"application/json; charset=utf-8\","
                                "success: function(data) {if (data.reload === true) location.replace('/');"
                                "").arg(url);
        *htextStream << QString("}, error: function (x, t, e) { setConnectedState(false);");
        *htextStream << QString("}");
        *htextStream << QString("}); }); ");

        *htextStream << QString("function requestTimeCheck(){");
        *htextStream << QString("if (requestTimeOut > 0) requestTimeOut --;");
        *htextStream << QString("if (requestTimeOut == 0) {requestTimeOut = %1;").arg(timeInterval / 10);
        *htextStream << QString(" requestState();};");
        *htextStream << QString("};");


        //периодический запрос
        //        *htextStream << QString("window.setInterval(requestState,%1);") .arg(timeInterval);
        *htextStream << QString("window.setInterval(requestTimeCheck, %1);") .arg(10);
    }

    //==== widjet script end ===========

    *htextStream << QString("$(document).ready(function() {");
    // Get checkbox statuses from localStorage if available (IE)
    *htextStream << QString("if (localStorage) {");

    // Menu minifier status (Contract/expand side menu on large screens)
    *htextStream << QString("var checkboxValue = localStorage.getItem('minifier');"
                            "if (checkboxValue === 'true') {"
                            "$('#sidebar,#menu-minifier').addClass('mini');"
                            "$('#minifier').prop('checked', true);"
                            "} else {"
                            "if ($('#minifier').is(':checked')) {"
                            "$('#sidebar,#menu-minifier').addClass('mini');"
                            "$('#minifier').prop('checked', true);"
                            "} else {"
                            "$('#sidebar,#menu-minifier').removeClass('mini');"
                            "$('#minifier').prop('checked', false);}}");
    *htextStream << QString("} ");

    // Contract/expand side menu on click. (only large screens)
    *htextStream << QString("$('#minifier').click(function() {"
                            "$('#sidebar,#menu-minifier').toggleClass('mini');"
                            "if (localStorage) {"
                            "checkboxValue = this.checked;"
                            "localStorage.setItem('minifier', checkboxValue);}});");
    // Side menu toogler for medium and small screens
    *htextStream << QString("$('[data-bs-toggle=\"offcanvas\"]').click(function() {"
                            "$('.row-offcanvas').toggleClass('active');});");

    // Switch (checkbox element) toogler
    *htextStream << QString("$('.switch input[type=\"checkbox\"]').on(\"change\", function(t) {"
                            "iot.chekboxHandler(this.id);});");

    // Reposition to center when a modal is shown
    *htextStream << QString("$('.modal.centered').on('show.bs.modal', iot.centerModal);");
    // Reset/Stop countdown timer (EXIT NOW)
    *htextStream << QString("$('#armModal').on('hide.bs.modal', iot.clearCountdown);");
    // Alerts \"Close\" callback - hide modal and alert indicator dot when user closes all alerts
    *htextStream << QString("$('#alertsModal .alert').on('close.bs.alert', function() {"
                            "var sum = $('#alerts-toggler').attr('data-alerts');"
                            "sum = sum - 1;"
                            "$('#alerts-toggler').attr('data-alerts', sum);"
                            "if (sum === 0) {"
                            "$('#alertsModal').modal('hide');"
                            "$('#alerts-toggler').attr('data-bs-toggle', 'none');}});");
    // Show/hide tips (popovers) - FAB button (right bottom on large screens)
    *htextStream << QString("$('#info-toggler').click(function() {"
                            "if ($('body').hasClass('info-active')) {"
                            "$('[data-bs-toggle=\"popover-all\"]').popover('hide');"
                            "$('body').removeClass('info-active');");
    *htextStream << QString("} else {"
                            "$('[data-bs-toggle=\"popover-all\"]').popover('show');"
                            "$('body').addClass('info-active');"
                            "}});");

    // Hide tips (popovers) by clicking outside
    *htextStream << QString("$('body').on('click', function(pop) {"
                            "if (pop.target.id !== 'info-toggler' && $('body').hasClass('info-active')) {"
                            "$('[data-bs-toggle=\"popover-all\"]').popover('hide');"
                            "$('body').removeClass('info-active');"
                            "}});");

    *htextStream << QString("});");

    // Apply necessary changes, functionality when content is loaded
    *htextStream << QString("$(window).on('load', function() {");

    // This script is necessary for cross browsers icon sprite support (IE9+, ...)
    *htextStream << QString("svg4everybody();");

    // \"Timeout\" function is not neccessary - important is to hide the preloader overlay
    *htextStream << QString("setTimeout(function() {");

    // Hide preloader overlay when content is loaded
    *htextStream << QString("$('#iot-preloader,.card-preloader').fadeOut();"
                            "$(\"#wrapper\").removeClass(\"hidden\");");

    if (wEnable) w->getHTMLOnLoad(&funonload, accountIndex);
    for (int i = 0; i < funonload.count(); i ++)  *htextStream << funonload.at(i);
    *htextStream << QString("iot.positionFab();"// Check for Main contents scrollbar visibility and set right position for FAB button
                            "}, 800);});");

    // Apply necessary changes if window resized
    *htextStream << QString("$(window).on('resize', function() {"
                            "$('.modal.centered:visible').each(iot.centerModal);"
                            "iot.positionFab();});");

    *htextStream << QString("</script></body></html>");

    delete htextStream;
}

void QsnWebContainer::getJSON(QString url, QByteArray *input, QByteArray *output)
{
    QsnWeb *w = getWidgetFromUrl(url);
    w->actionJSON(input, output);
}

void QsnWebContainer::getAuthorizationHtml(QString , QByteArray *content)
{
    QTextStream *htextStream = new QTextStream(content);
    htextStream->setAutoDetectUnicode(true);

    *htextStream << QString("<!DOCTYPE html><html><head><title>");
    *htextStream << objectName();
    *htextStream << QString("</title>"
                            "<meta charset=\"utf-8\">"
                            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, shrink-to-fit=no\" />"
                            "<meta name=\"description\" content=\"\">"
                            "<meta name=\"author\" content=\"Stanislav Tumanov - signalnet.ru\">"
                            "<link href=\"assets/css/iot-theme-bundle.min.css\" rel=\"stylesheet\">"
                            "</head><body>");


    *htextStream << QString("<div id=\"login\" class=\"d-flex align-items-center pb-4\">"
                            "<div class=\"row mx-auto\" style=\"width:100%;max-width:24rem;\">"
                            "<div class=\"col-12 text-center\">");
    *htextStream << QString("<h3>Signal Net</h3>");
    *htextStream << QString("</div><div class=\"col-12\">"
                            "<div class=\"card px-4 pt-2\">"
                            "<form action=\"/\" class=\"p-2\">"
                            "<div class=\"form-group row\">");
    *htextStream << QString("<label for=\"username\" class=\"col-12 col-form-label\">%1</label>").arg(tr("Username"));
    *htextStream << QString("<div class=\"col-12\">");
    *htextStream << QString("<input id=\"username\" class=\"form-control custom-focus\" type=\"text\" value=\"\" >");
    *htextStream << QString("</div></div>");
    *htextStream << QString("<div class=\"form-group row\">");
    *htextStream << QString("<label for=\"password\" class=\"col-12 col-form-label\">%1</label>").arg(tr("Password"));
    *htextStream << QString("<div class=\"col-12\">");
    *htextStream << QString("<input id=\"password\" class=\"form-control custom-focus\" type=\"password\" value=\"\""
                            " data-placement=\"bottom\" data-content=\"%1\">").arg(tr("Incorrect Login or Password!"));
    *htextStream << QString("</div></div><div class=\"form-check\">");
    *htextStream << QString("<input id=\"remember\" type=\"checkbox\" class=\"form-check-input \">");
    *htextStream << QString("<label class=\"form-check-label\" for=\"remember\">%1</label>").arg(tr("Remember"));
    *htextStream << QString("</div><div class=\"form-group row\">");
    *htextStream << QString("<div class=\"col-12 mt-3 mb-2 text-center\">");
    *htextStream << QString("<button id=\"btnlogin\" type=\"button\" class=\"btn btn-primary btn-block\" "
                            ">%1</button >").arg(tr("Enter"));
    *htextStream << QString("</div></div></form></div></div></div></div>");

    *htextStream << QString("<script src=\"assets/js/jquery-3.4.1.min.js\"></script>");
    *htextStream << QString("<script src=\"assets/js/bootstrap.bundle.min.js\"></script>");

    //auth script
    *htextStream << QString("<script>");
    *htextStream << QString("$('#username').val(localStorage.getItem('username'));");
    *htextStream << QString("$('#remember').prop('checked', localStorage.getItem('remember'));");

    *htextStream << QString("function sendlogin(){"
                            "$(this).prop('disabled', true);"
                            " $.ajax({"
                            "url: \"/\","
                            "type: 'POST',"
                            "data: {username: $('#username').val(), password: $('#password').val(), remember: $('#remember').prop('checked')},"
                            "contentType: \"application/json; charset=utf-8\","
                            "success: function(data) {"
                            "if (data == 'allowed') {"
                            "localStorage.setItem('username', $('#username').val());"
                            "localStorage.setItem('remember', $('#remember').prop('checked'));"
                            "location.replace('/');"
                            "} else {"
                            "$('#password').popover('show');"
                            "setTimeout(function () { $('#password').popover('hide') }, 10000);"
                            "$('#btnlogin').prop('disabled', false);");
    *htextStream << QString("} }, error: function (x, t, e) { } }); "
                            "};");

    *htextStream << QString("$(document).keypress(function(e) { if(e.which == 13) { sendlogin();} });");

    *htextStream << QString("$(\"#btnlogin\").click(function(){sendlogin();})");

    *htextStream << QString("</script>");

    *htextStream << QString("</body></html>");

    delete htextStream;
}

void QsnWebContainer::actionItem(QString URL, QMap<QString, QString> *options, QStringList *returnItems, qint64 uid)
{
    QString pageURL = URL;

    //        QMap <QString, QString> :: iterator it; // перемещаемся по карте
    //               for ( it = options->begin(); it != options->end(); ++it )
    //               {
    //                   qDebug() << it.key() << it->data();
    //               }

    if (URL.indexOf('?', 0)) pageURL = URL.left(URL.indexOf('?', 0));


    // только с функцией авторизации
    if (options->contains("action")) {
        QString state = options->value("action");

        if (state == QLatin1String("action"))   {
            QString id;
            if (options->contains("id")) id = options->value("id");
            if (id == "logout")
                if (options->contains("state")) {
                    QString state = options->value("state");
                    if (state == QLatin1String("logout")) *returnItems << QString("\"reload\": %1").arg(modules.auth->logout(uid)?"true":"false");
                }
        }
    }
    //--------------------


    for(int i = 0; i < widgets.count(); i ++)
        if (widgets[i]->widgetUrl == pageURL) {
            widgets[i]->treatmentUrl(URL);
            widgets[i]->actionItem(pageURL, options, returnItems, uid);
        }

    *returnItems << QString("\"pageversion\":\"%1\"").arg(versionID);

    for(int i = 0; i < constWidgets.count(); i ++)
        if (constWidgets[i]->widgetUrl == pageURL) {
            constWidgets[i]->treatmentUrl(URL);
            constWidgets[i]->actionItem(pageURL, options, returnItems, uid);
        }

    if (tcpServer != Q_NULLPTR && tcpServer->widgetUrl == pageURL) {
        tcpServer->treatmentUrl(URL);
        tcpServer->actionItem(pageURL, options, returnItems, uid);
    }
    if (udpServer != Q_NULLPTR && udpServer->widgetUrl == pageURL) {
        udpServer->treatmentUrl(URL);
        udpServer->actionItem(pageURL, options, returnItems, uid);
    }

    if (udpClient != Q_NULLPTR && udpClient->widgetUrl == pageURL) {
        udpClient->treatmentUrl(URL);
        udpClient->actionItem(pageURL, options, returnItems, uid);
    }

    if (uartCan != Q_NULLPTR && uartCan->widgetUrl == pageURL) {
        uartCan->treatmentUrl(URL);
        uartCan->actionItem(pageURL, options, returnItems, uid);
    }

    if (pageURL != QLatin1String("/alert")) *returnItems << QString("\"lbalertstatus\": \"%1\"").arg(modules.alert_status);
    if (pageURL != QLatin1String("/alarm")) *returnItems << QString("\"lbalarmstatus\": \"%1\"").arg(modules.alarm_status);
}

QsnWeb *QsnWebContainer::getWidgetFromUrl(QString url)
{
    for(int i = 0; i < constWidgets.count(); i ++)
        if (constWidgets[i]->widgetUrl == url) return constWidgets[i];

    for(int i = 0; i < widgets.count(); i ++)
        if (widgets[i]->widgetUrl == url) return widgets[i];

    if (tcpServer != Q_NULLPTR && tcpServer->widgetUrl == url) return tcpServer;
    if (udpServer != Q_NULLPTR && udpServer->widgetUrl == url) return udpServer;
    if (udpClient != Q_NULLPTR && udpClient->widgetUrl == url) return udpClient;
    if (uartCan != Q_NULLPTR && uartCan->widgetUrl == url) return uartCan;
    if (url == "/" ) return startPage;
    return pageInfo;
}

void QsnWebContainer::loadSettings()
{
    QSettings settings;
    modules.interface->loadSettings(&settings);
}

void QsnWebContainer::checkSupportSSL()
{
    modules.supportSSL = QSslSocket::supportsSsl();
    // Логирование информации о SSL поддержке для диагностики
    if (modules.supportSSL) {
        QString sslVersion = QSslSocket::sslLibraryVersionString();
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             QString(tr("SSL поддержка включена. Версия библиотеки: %1")).arg(sslVersion),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);

        // Проверка доступных протоколов
        QList<QSsl::SslProtocol> supportedProtocols;
        supportedProtocols << QSsl::TlsV1_2 << QSsl::TlsV1_3 << QSsl::TlsV1_2OrLater;

        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_information,
                             tr("Настроен безопасный TLS: минимальная версия TLS 1.2"),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);
    } else {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_caution,
                             tr("SSL поддержка недоступна. SSL сервер не будет работать."),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);
    }
}

void QsnWebContainer::saveSettings()
{
    QSettings settings;
    modules.interface->saveSettings(&settings);
}

void QsnWebContainer::configureClear()
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_warning,
                         tr("Configuration clear begin"),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("server"),
                         false), this);
    modules.db->tempSettings.clear();
    modules.io->clearIOS();
    pageSettings->clearSettings();

    for (int i = widgets.count() - 1; i >= 0; i --){
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_warning,
                             tr("Configuration delete module") + " - " + widgets[i]->objectName(),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("server"),
                             false), this);
        widgets[i]->adapterDisconnect();
        widgets[i]->deleteLater();
        widgets.removeAt(i);
    }
    modules.auth->cleanAccounts();
    modules.alarm_status = -1;
    modules.alert_status = 0;
    modules.supportMQTT = false;
}

void QsnWebContainer::timerInitDelay()
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         tr("Initialization delay end."),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("server"),
                         false), this);
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 4;
    container.Sender = 0;
    container.Signal = 0;
    modules.interface->snBUSInput(container, this);
}

void QsnWebContainer::newConnection()
{
    QsnWebSocket *webSocket = new QsnWebSocket(Server->nextPendingConnection(), this, this);
    ServerClients.append(webSocket);
    connect(webSocket, SIGNAL(closeConnected(QsnWebSocket*)), this, SLOT(closeConnection(QsnWebSocket*)));
}

void QsnWebContainer::newConnectionSSL()
{

    //    QSslSocket *webSocketSSL = ServerSSL->nextPendingConnection();
    //qDebug() << "new ssl conection";
    //    QSslConfiguration a;
    //    QFile *f=new QFile("./server.crt");
    //    QFile *f1=new QFile("./server.key");
    //    f->open(QIODevice::ReadWrite);
    //    f1->open(QIODevice::ReadWrite);
    //    a.setCaCertificates(QSslCertificate::fromDevice(f));
    //    a.setPrivateKey(QSslKey(f1, QSsl::Rsa));
    //    f->close();
    //    f1->close();
    //    webSocketSSL->setSslConfiguration(a);
    //    webSocketSSL->setCiphers("DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA:EDH-RSA-DES-CBC3-SHA:EDH-DSS-DES-CBC3-SHA:DES-CBC3-SHA:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:AES128-SHA:RC4-SHA:RC4-MD5:EDH-RSA-DES-CBC-SHA:EDH-DSS-DES-CBC-SHA:DES-CBC-SHA:EXP-EDH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:EXP-DES-CBC-SHA:EXP-RC2-CBC-MD5:EXP-RC4-MD5");

    //   QObject::connect(serverSocket, SIGNAL(encrypted()), this, SLOT(socketReady()));
    //   QObject::connect(serverSocket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(socketSslErrors(const QList<QSslError> &)));
    //   QObject::connect(serverSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    //webSocketSSL->startServerEncryption();

    //QsnWebSocket *webSocket = new QsnWebSocket(Server->nextPendingConnection(), this, this);
    // ServerClients.append(webSocket);
    // connect(webSocket, SIGNAL(closeConnected(QsnWebSocket*)), this, SLOT(closeConnection(QsnWebSocket*)));
}

void QsnWebContainer::closeConnection(QsnWebSocket *socket)
{
    for (int i = ServerClients.count() - 1; i >= 0; i --)
        if (ServerClients[i] == socket) ServerClients.removeAt(i);
}

