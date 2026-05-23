#include "qsnwebauthorization.h"

QsnWebAuthorization::QsnWebAuthorization(QObject *parent) : QObject(parent)
{
    experienceDays = 180;
    WEBAuthorization = false;
    TCPAuthorization = false;
    isLog = false;
    admin.login = "administrator";
    admin.password = "signalnet";
    admin.name = tr("System");
    admin.ID = -1;
    admin.accessRights = 2;
    setObjectName(QString(tr("Authorization")));
    loadSetting();
    loadAUid();
}

void QsnWebAuthorization::fromStream(QDataStream *stream)
{
    int Count;
    QMap<QString, QVariant> optionsMap;
    QString opName;
    QVariant opValue;
    QString sig;
    //    int id;
    *stream >> sig;
    *stream >> Count;

    for (int i = 0; i < Count; i ++) {
        *stream >> opName;
        opValue.load(*stream);
        optionsMap.insert(opName, opValue);
    }

    *stream >> Count; //io
    *stream >> Count; //items

    if (optionsMap.contains("ExpD")) {
        experienceDays = optionsMap.value("ExpD").toInt();//===========================================
    }

    verificationAIDs();
}

int QsnWebAuthorization::accountIndex(QString login, QString password)
{
    emit snBUSOutput(QSNLogToContainer(
                         BUSSERV_LOG_LEVEL_information,
                         QString("%1:  %2").arg(tr("Authorization user"), login),
                         BUSSERV_LOG_CATEGORY_core,
                         QString("authorization"),
                         false), this);
    if (login == admin.login && password == admin.password) return 65535;
    for (int i = 0; i < accounts.count(); i ++)
        if (accounts[i].login == login && accounts[i].password == password) return i;
    return -1;
}

void QsnWebAuthorization::authorizationAttempt(QString login, QString password, int *aindex, QString *error)
{
    QString src = *error;
    *error = tr("Incorrect Login or Password!");
    *aindex = -1;

    // admin authorization
    if (admin.login == login) {
        if (QDateTime::currentDateTime() >= admin.lastAccessDenied.addSecs(600)) admin.countAccessDenied = 0;
        if (admin.password != password && admin.countAccessDenied < 3) {
            admin.lastAccessDenied = QDateTime::currentDateTime();
            admin.countAccessDenied = 3;
            for (int p = 0; p < 3 - admin.countAccessDenied; p ++) *error += ".";
            *aindex = -1;
            emit snBUSOutput(QSNLogToContainer(
                                 BUSSERV_LOG_LEVEL_warning,
                                 tr("The administrator's password is incorrect! Source:%2 ").arg(src),
                                 BUSSERV_LOG_CATEGORY_core,
                                 QString("authorization"),
                                 false), this);
        }

        if (admin.countAccessDenied >= 3 && QDateTime::currentDateTime() < admin.lastAccessDenied.addSecs(600)) {
            *error += "!";
            *aindex = -1;
        } else if (admin.password == password) {
            *aindex = 65535;
            admin.lastAccessAllowed = QDateTime::currentDateTime();
            admin.countAccessDenied = 0;
            emit snBUSOutput(QSNLogToContainer(
                                 BUSSERV_LOG_LEVEL_warning,
                                 tr("Access on behalf of the administrator. source:%2 ").arg(src),
                                 BUSSERV_LOG_CATEGORY_core,
                                 QString("authorization"),
                                 false), this);
        }
        return;
    }

    // user authorization
    for (int i = 0; i < accounts.count(); i ++)
        if (accounts[i].login == login) {
            if (QDateTime::currentDateTime() >= accounts[i].lastAccessDenied.addSecs(600)) accounts[i].countAccessDenied = 0;
            if (accounts[i].password != password && accounts[i].countAccessDenied < 3) {
                //if (accounts[i].countAccessDenied >= 3) accounts[i].countAccessDenied = 0;
                accounts[i].lastAccessDenied = QDateTime::currentDateTime();
                accounts[i].countAccessDenied ++;
                *error = tr("Password is not correct! Login:%1, %2 attempts left! Source:%3").arg(login).arg(3 - accounts[i].countAccessDenied).arg(src);
                emit snBUSOutput(QSNLogToContainer(
                                     BUSSERV_LOG_LEVEL_warning,
                                     *error,
                                     BUSSERV_LOG_CATEGORY_core,
                                     QString("authorization"),
                                     true), this);

                *aindex = -1;
            }

            if (accounts[i].countAccessDenied >= 3 && QDateTime::currentDateTime() < accounts[i].lastAccessDenied.addSecs(600)) {
                *error = QString(tr("Account %1 is locked until %2!")).arg(login, accounts[i].lastAccessDenied.addSecs(600).toString("hh:mm:ss"));
                *aindex = -1;
                emit snBUSOutput(QSNLogToContainer(
                                     BUSSERV_LOG_LEVEL_warning,
                                     *error,
                                     BUSSERV_LOG_CATEGORY_core,
                                     QString("authorization"),
                                     true), this);
                if (notifyBlock) {
                    QSNContainer container = newContainer();
                    container.role = QSNContainer::service;
                    container.Command = BUSSERV_MESSAGE;
                    container.Signal = 3;
                    container.Sender = 0;
                    container.info = "(!)" + *error + "{(" + src + ")}";
                    emit snBUSOutput(container, this);
                }
            } else if (accounts[i].password == password) {
                *aindex = i;
                accounts[i].lastAccessAllowed = QDateTime::currentDateTime();
                accounts[i].countAccessDenied = 0;
            }
            return;
        }
    if (*aindex == -1) {
        emit snBUSOutput(QSNLogToContainer(
                             BUSSERV_LOG_LEVEL_warning,
                             tr("Login and password is not correct! Login:%1, source:%2").arg(login, src),
                             BUSSERV_LOG_CATEGORY_core,
                             QString("authorization"),
                             true), this);
    }
}

QString QsnWebAuthorization::getCooke(int accountIndex, bool isSession)
{
    _userId UID = newUID(accountIndex, isSession);
    if (isSession)  return QString("Set-Cookie: UID=%1; path=/; \r\n")
            .arg(UID.cookeID);

    return QString("Set-Cookie: UID=%1; expires=%2; path=/;\r\n")
            .arg(UID.cookeID)
            .arg(getExpiresString(UID.expires));
}

int QsnWebAuthorization::accountIndexFromCookeID(qint64 cookeID)
{
    checkUIDs();
    for (int i = 0; i < usersID.count(); i ++)  if (usersID[i].cookeID == cookeID) {
        for (int j = 0; j < accounts.count(); j ++)
            if (accounts[j].ID == usersID[i].accountID) return j;
        if (admin.ID == usersID[i].accountID) return 65535;
        return -1;
    }

    return -1;
}

qint64 QsnWebAuthorization::accountUIDfromIndex(int index)
{
    if (index == 65535) return atAccount(index)->ID;
    if (index < 0 || index >= accounts.count()) return -1;
    return atAccount(index)->ID;
}

int QsnWebAuthorization::accountIndexfromAID(qint64 AID)
{
    for (int j = 0; j < accounts.count(); j ++)
        if (accounts[j].ID == AID) return j;
    return -1;
}

bool QsnWebAuthorization::isWEBAuthorization()
{
    return WEBAuthorization;
}

bool QsnWebAuthorization::isTCPAuthorization()
{
    return TCPAuthorization;
}

void QsnWebAuthorization::setAdminLogin(QString login)
{
    admin.login = login;
}

void QsnWebAuthorization::setAdminPassword(QString pass)
{
    admin.password = pass;
}

void QsnWebAuthorization::setAdminEmail(QString email)
{
    admin.email = email;
}

void QsnWebAuthorization::setEnableWEBAuthorization(bool enable)
{
    WEBAuthorization = enable;
}

void QsnWebAuthorization::setEnableTCPAuthorization(bool enable)
{
    TCPAuthorization = enable;
}

void QsnWebAuthorization::setEnableNotifyBlock(bool enable)
{
    notifyBlock = enable;
}

void QsnWebAuthorization::loadSetting()
{
    QFile file(QSNHomeSubPath("server", "settings").absoluteFilePath(QLatin1String("webaccounts.xml")));
    if (!file.open(QIODevice::ReadOnly)) return;
    QXmlStreamReader::TokenType token;
    QXmlStreamReader xmlDoc(&file);
    QXmlStreamAttributes attrib;
    token = xmlDoc.readNext();
    accounts.clear();
    accountItem item;
    fillAccount(&item);

    while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
        token = xmlDoc.readNext();
        if (token == QXmlStreamReader::StartElement && xmlDoc.name() == QLatin1String("Accounts")) {
            while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
                token = xmlDoc.readNext();
                if (xmlDoc.isStartElement() && xmlDoc.name() == QLatin1String("account")) {
                    attrib = xmlDoc.attributes();
                    item.name = attrib.value("name").toString();
                    item.ID = attrib.value("id").toLongLong();
                    if (attrib.value("isguest").toString() == "true") item.accessRights = 0;
                    else if (attrib.value("isadmin").toString() == "true") item.accessRights = 2;
                    else item.accessRights = 1;
                    item.login = attrib.value("login").toString();
                    item.password = attrib.value("password").toString();
                    item.email = attrib.value("email").toString();
                    for (int p = 0; p < item.parameters.count(); p ++)
                        if (attrib.hasAttribute(item.parameters[p].name))
                            item.parameters[p].value = attrib.value(item.parameters[p].name).toString();
                    for (int p = 0; p < item.permissions.count(); p ++)
                        if (attrib.hasAttribute(item.permissions[p].name))
                            item.permissions[p].value = (attrib.value(item.permissions[p].name).toString() == "true"?true:false);
                    item.lastAccessDenied = QDateTime(QDate(2000,1,1), QTime(0,0,0));
                    accounts.append(item);

                    while (xmlDoc.isEndElement()) token = xmlDoc.readNext();
                }
            }
        }
    }
    file.close();
    verificationAIDs();
}

void QsnWebAuthorization::saveSetting()
{
    QFile file(QSNHomeSubPath("server", "settings").absoluteFilePath(QLatin1String("webaccounts.xml")));
    if (!file.open(QIODevice::WriteOnly)) return;
    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement(QLatin1String("Accounts"));
    for (int i = 0; i < accounts.count(); i ++) {
        stream.writeStartElement("account");
        stream.writeAttribute("name", accounts[i].name);
        stream.writeAttribute("id", QString::number(accounts[i].ID));
        stream.writeAttribute("isadmin", accounts[i].accessRights == 2?"true":"false");
        stream.writeAttribute("isguest", accounts[i].accessRights == 0?"true":"false");
        stream.writeAttribute("login", accounts[i].login);
        stream.writeAttribute("password", accounts[i].password);
        stream.writeAttribute("email", accounts[i].email);
        for (int p = 0; p < accounts[i].parameters.count(); p ++)
            stream.writeAttribute(accounts[i].parameters[p].name, accounts[i].parameters[p].value);
        for (int p = 0; p < accounts[i].permissions.count(); p ++)
            stream.writeAttribute(accounts[i].permissions[p].name, accounts[i].permissions[p].value?"true":"false");
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
}

int QsnWebAuthorization::countAccounts()
{
    return accounts.count();
}

QsnWebAuthorization::accountItem *QsnWebAuthorization::atAccount(int index)
{
    if (index == 65535 || index < 0) return &admin;
    return &accounts[index];
}

bool QsnWebAuthorization::isAdmin(int index)
{
    return atAccount(index)->accessRights > 1;
}

bool QsnWebAuthorization::isUser(int index)
{
    return atAccount(index)->accessRights == 1;
}

void QsnWebAuthorization::deleteAccount(qint64 AID)
{
    int index = accountIndexfromAID(AID);
    if (index == -1) return;
    accounts.removeAt(index);
    saveSetting();
    loadSetting();
}

quint8 QsnWebAuthorization::applyAccount(accountItem item)
{
    int index = accountIndexfromAID(item.ID);
    if (index == -1) {
        for (int i = 0; i < accounts.count(); i ++) {
            if (accounts[i].login == item.login) return 1;
            if (item.password.isEmpty()) return 2;
        }
        accounts.append(item);
        accounts[accounts.count() - 1].ID = QDateTime::currentMSecsSinceEpoch();
        accounts[accounts.count() - 1].lastAccessDenied = QDateTime(QDate(2000,1,1), QTime(0,0,0));
    } else accounts[index] = item;
    saveSetting();
    return 0;
}

bool QsnWebAuthorization::checkAccountMSGType(int index, int type)
{
    if (!atAccount(index)->alertNotification && type == QsnWebAuthorization::alert) return false;
    if (!atAccount(index)->alarmNotification && type == QsnWebAuthorization::alarm) return false;
    return true;
}

void QsnWebAuthorization::addTag(int index, QsnWebAuthorization::tagItem item)
{
    if (index >= accounts.count()) return;
    for (int i = 0; i < atAccount(index)->tags.count(); i ++)
        if (atAccount(index)->tags[i].tag == item.tag) {
            atAccount(index)->tags[i] = item;
            accountChanged();
            return;
        }
    atAccount(index)->tags.append(item);
    accountChanged();
}


void QsnWebAuthorization::setTagState(int accountIndex, QString tag, QString tagState, QString label)
{
    tagItem l;
    l.label = label;
    l.tag = tag;
    l.tagState = tagState;
    l.shtamp = QDateTime::currentDateTime();
    addTag(accountIndex, l);
    checkLocationTag();
}

void QsnWebAuthorization::accountChanged()
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = 10;
    container.Signal = 0;
    container.Address = 0;
    container.Sender = 0;
    emit snBUSOutput(container, this);
}

bool QsnWebAuthorization::checkAcces(int accountIndex, quint8 accessRights)
{
    if (accountIndex == -1) return true;
    return (atAccount(accountIndex)->accessRights >= accessRights);
}

bool QsnWebAuthorization::checkAccesFromUID(qint64 uid,  quint8 accessRights)
{
    if (!WEBAuthorization) return true;
    if (uid == -1) return false;
    return checkAcces(accountIndexFromCookeID(uid), accessRights);
}

bool QsnWebAuthorization::checkCookeID(qint64 cookeID)
{
    for (int j = 0; j < usersID.count(); j ++)
        if (usersID[j].cookeID == cookeID) return true;
    return false;
}

bool QsnWebAuthorization::checkAID(qint64 AID)
{
    for (int i = 0; i < accounts.count(); i ++)
        if (accounts[i].ID == AID) return true;
    return false;
}

bool QsnWebAuthorization::logout(qint64 cookeID)
{
    for (int j = 0; j < usersID.count(); j ++)
        if (usersID[j].cookeID == cookeID) {
            usersID.removeAt(j);
            saveUIDs();
            return true;
        }
    return false;
}

void QsnWebAuthorization::addParametr(QString name, QString title, QString value, QString ptype)
{
    parameterItem pi;
    pi.name = name;
    pi.title = title;
    pi.value = value;
    pi.type = ptype;
    bool e = false;
    for (int i = 0; i < accounts.count(); i ++) {
        e = false;
        for (int p = 0; p < accounts[i].parameters.count(); p ++)
            if (accounts[i].parameters[p].name == name) {
                e = true;
                accounts[i].parameters[p] = pi;
            }
        if (!e) accounts[i].parameters.append(pi);
    }
    e = false;
    for (int p = 0; p < parameters.count(); p ++)
        if (parameters[p].name == name) {
            parameters[p] = pi;
            e = true;
        }
    if (!e) parameters.append(pi);
}

QString QsnWebAuthorization::parametrValue(QsnWebAuthorization::accountItem *item, QString name)
{
    for (int p = 0; p < item->parameters.count(); p ++)
        if (item->parameters[p].name == name) return item->parameters[p].value;
    return QString();
}

void QsnWebAuthorization::addPermission(QString name, QString title, bool value)
{
    permissionItem pp;
    pp.name = name;
    pp.title = title;
    pp.value = value;
    bool e = false;
    for (int i = 0; i < accounts.count(); i ++) {
        e = false;
        for (int p = 0; p < accounts[i].permissions.count(); p ++)
            if (accounts[i].permissions[p].name == name) {
                e = true;
                accounts[i].permissions[p] = pp;
            }
        if (!e) accounts[i].permissions.append(pp);
    }
    e = false;
    for (int p = 0; p < permissions.count(); p ++)
        if (permissions[p].name == name) {
            permissions[p] = pp;
            e = true;
        }
    if (!e) permissions.append(pp);
}

bool QsnWebAuthorization::permissionValue(QsnWebAuthorization::accountItem *item, QString name)
{
    for (int p = 0; p < item->permissions.count(); p ++)
        if (item->permissions[p].name == name) return item->permissions[p].value;
    return false;
}

void QsnWebAuthorization::fillAccount(QsnWebAuthorization::accountItem *item)
{
    for (int p = 0; p < parameters.count(); p ++) item->parameters.append(parameters[p]);
    for (int p = 0; p < permissions.count(); p ++) item->permissions.append(permissions[p]);
}

void QsnWebAuthorization::cleanAccounts()
{
    parameters.clear();
    permissions.clear();
}

void QsnWebAuthorization::setLogEnable(bool enable)
{
    isLog = enable;
}

QString QsnWebAuthorization::getTypeAccountName(accountItem *item)
{
    if(item->accessRights == 0) return QObject::tr("Guest");
    if(item->accessRights == 2) return QObject::tr("Administrator");
    return QObject::tr("User");
}

bool QsnWebAuthorization::getAccountPresenceState(accountItem *item, QString source)
{
    QList<presenceItem>::iterator i;
    for (i = item->presence.begin(); i != item->presence.end(); ++i) {
        if ((*i).source == source) return (*i).presence;
    }
    return false;
}

QStringList QsnWebAuthorization::getAccountPresenceSources(accountItem *item)
{
    QStringList ret;
    for(auto &pres:item->presence) ret.append(pres.source);
    return ret;
}

QDateTime QsnWebAuthorization::getAccountPresenceLastDetect(accountItem *item, QString source)
{
    QList<presenceItem>::iterator i;
    for (i = item->presence.begin(); i != item->presence.end(); ++i) {
        if ((*i).source == source) return (*i).lastDetection;
    }
    return QDateTime(QDate(2000,1,1), QTime(0,0,0));
}

void QsnWebAuthorization::getAccountPresenceLastConsolidated(accountItem *item, bool *presence, QDateTime *lastDetect, QString *sourceDetect, QDateTime *endAbsence)
{
    QList<presenceItem>::iterator i;
    *lastDetect = QDateTime(QDate(2000,1,1), QTime(0,0,0));
    *endAbsence = QDateTime(QDate(2000,1,1), QTime(0,0,0));
    *presence = false;
    *sourceDetect = tr("----");
    for (i = item->presence.begin(); i != item->presence.end(); ++i) {
        if ((*i).lastDetection > *lastDetect) {
            *lastDetect = (*i).lastDetection;
            *sourceDetect = (*i).source;
        }
        if ((*i).endPresence > *endAbsence) *endAbsence = (*i).endPresence;
        if ((*i).presence) *presence = true;
    }
}

void QsnWebAuthorization::setAccountPresenceItem(accountItem *item, QString source, bool state, quint64 presenceTime, quint64 altPresenceTime)
{
    QList<presenceItem>::iterator i;
    for (i = item->presence.begin(); i != item->presence.end(); ++i) {
        if ((*i).source == source) {
            (*i).presence = state;
            if (state) {
                (*i).lastDetection = QDateTime::currentDateTime();
                (*i).endPresence = QDateTime::currentDateTime().addSecs(presenceTime);
                (*i).endPresenceAlt = QDateTime::currentDateTime().addSecs(altPresenceTime);
            } else {
              (*i).endPresence = QSNNULLDateTime();
            }
            if (!state && item != atAccount(-1)) presenceClearSystemSources();
            presenceItemsCheck();
            return;
        }
    }
    presenceItem it;
    it.source = source;
    it.presence = state;
    it.lastDetection = QDateTime::currentDateTime();
    it.endPresence = QDateTime::currentDateTime().addSecs(presenceTime);
    it.endPresenceAlt = QDateTime::currentDateTime().addSecs(altPresenceTime);
    item->presence.append(it);
    if (!state && item != atAccount(-1)) presenceClearSystemSources();
    presenceItemsCheck();
}

QString QsnWebAuthorization::presenceLabel()
{
    if (presenceState == -1) return tr("It is not known");
    return presenceState == 1?tr("Presence"):tr("Absence");
}

int QsnWebAuthorization::isPresence()
{
    return presenceState;
}

void QsnWebAuthorization::setAbsence()
{
    if (presenceState != 1) return;
    for (int j = -1; j < accounts.count(); j ++)
        for (int i = 0; i < atAccount(j)->presence.count(); i ++) {
            atAccount(j)->presence[i].presence = false;
            atAccount(j)->presence[i].endPresence = QSNNULLDateTime();
        }
    presenceItemsCheck();
}

void QsnWebAuthorization::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_TIME_SEC_EVENT) {
        if (QDateTime::currentDateTime() > presenceNextCheck) presenceItemsCheck();
    }

    if (container.Command == BUSSERV_MODULE_STATE_CHANGE_absence) {
        if (container.Signal == 1) presenceClearSystemSources();// добавить реакцию на присутствие
    }

}

void QsnWebAuthorization::setPresenceSystem(QString source, qint64 presenceTime, qint64 altPresenceTime)
{
    setAccountPresenceItem(atAccount(-1), source, true,  presenceTime, altPresenceTime);
}

QsnWebAuthorization::_userId QsnWebAuthorization::newUID(int accountIndex, bool isSession)
{
    qint64 cookeID = 0;
    do cookeID = QDateTime::currentMSecsSinceEpoch();
    while (checkCookeID(cookeID));
    _userId UID;
    UID.accountID = atAccount(accountIndex)->ID;
    UID.cookeID = cookeID;
    if (isSession) UID.expires = QDateTime::currentDateTimeUtc().addDays(2);
    else UID.expires = QDateTime::currentDateTimeUtc().addDays(experienceDays);
    usersID.append(UID);
    saveUIDs();
    return UID;
}

void QsnWebAuthorization::verificationAIDs()
{
    bool changed = false;
    int j;
    for (int i = usersID.count() - 1; i >= 0; i --) {
        bool present = false;
        for (j = 0; j < accounts.count(); j ++)
            if (accounts[j].ID == usersID[i].accountID) present = true;
        if (atAccount(65535)->ID == usersID[i].accountID) present = true;
        if (!present) {
            usersID.removeAt(i);
            changed =true;
        }
    }
    if (changed) saveUIDs();
}

void QsnWebAuthorization::checkUIDs()
{
    QDateTime cdt = QDateTime::currentDateTimeUtc();
    bool changed = false;
    for (int i = usersID.count() - 1; i >= 0; i --)
        if (usersID[i].expires < cdt) {
            usersID.removeAt(i);
            changed =true;
        }
    if (changed) saveUIDs();
}

void QsnWebAuthorization::saveUIDs()
{
    QFile file(QSNHomeSubPath("server", "settings").absoluteFilePath(QLatin1String("usersId.dat")));

    if (!file.open(QIODevice::WriteOnly)) return;
    QDataStream Out(&file);
    Out.setVersion(QDataStream::Qt_4_7);
    Out << QString(QLatin1String("UsersId"));
    for (int i = 0; i < usersID.count(); i ++)
    {
        Out << usersID[i].accountID;
        Out << usersID[i].cookeID;
        Out << usersID[i].expires;
    }
    file.close();
}

void QsnWebAuthorization::loadAUid()
{
    QString signature;
    QFile fileb(QSNHomeSubPath("server", "settings").absoluteFilePath(QLatin1String("usersId.dat")));
    if (!fileb.open(QIODevice::ReadOnly)) return;
    QDataStream Inb(&fileb);
    Inb.setVersion(QDataStream::Qt_4_7);
    _userId uid;
    usersID.clear();
    Inb >> signature;
    if (signature == QLatin1String("UsersId")) {
        while (!Inb.atEnd()) {
            Inb >> uid.accountID;
            Inb >> uid.cookeID;
            Inb >> uid.expires;
            usersID.append(uid);
        }
    }
    fileb.close();
}

QString QsnWebAuthorization::getExpiresString(QDateTime expires)
{
    QVector<QString> eDays;
    eDays << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" << "Sun";
    QVector<QString> eMothe;
    eMothe << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "June" << "July" << "Aug" << "Sept" << "Oct" << "Nov" << "Dec";
    return QString("%1, %2 %3 %4 %5")
            .arg(eDays.at(expires.date().dayOfWeek() - 1))
            .arg(expires.date().day())
            .arg(eMothe.at(expires.date().month() - 1))
            .arg(expires.date().year())
            .arg(expires.time().toString("hh:mm:ss 'GMT'"));
}

void QsnWebAuthorization::checkLocationTag() //????
{
    int home = 0;
    int nothome = 0;
    for (int j = 0; j < accounts.count(); j ++)
        for (int i = 0; i < atAccount(j)->tags.count(); i ++) {
            if (atAccount(j)->tags[i].tagState == "home") home ++;
            if (atAccount(j)->tags[i].tagState == "nothome") nothome ++;
        }

    if (!home && !nothome) return;
    if (!home && nothome) {emit isAllNotHome();}
    else {emit isHome();}
}

void QsnWebAuthorization::presenceItemsCheck()
{

    bool presence = false;
    int newPresence = presenceState;
    QDateTime currDate = QDateTime::currentDateTime();
    presenceNextCheck = currDate.addSecs(60);
    for (int j = -1; j < accounts.count(); j ++)
        for (int i = 0; i < atAccount(j)->presence.count(); i ++) {
            if (atAccount(j)->presence[i].endPresence >= currDate) {
               atAccount(j)->presence[i].presence = true;
               presence = true;
               if (presenceNextCheck > atAccount(j)->presence[i].endPresence) presenceNextCheck = atAccount(j)->presence[i].endPresence.addSecs(1);
            } else {
               atAccount(j)->presence[i].presence = false;
               atAccount(j)->presence[i].endPresence = QSNNULLDateTime();

            }
        }

    if (presence) newPresence = 1;
    else if (newPresence == 1) newPresence = 0;
    if (presenceState != newPresence) {
        presenceState = newPresence;
        QSNContainer container;
        container = newContainer();
        container.role = QSNContainer::service;
        container.Command = BUSSERV_MODULE_STATE_CHANGE_absence;
        if (newPresence == 1) container.Signal = 1;
        if (newPresence == 0) container.Signal = 0;
        container.Sender = 0;
        emit snBUSOutput(container, this);

    }
}

void QsnWebAuthorization::presenceClearSystemSources()
{
    QDateTime currDate = QDateTime::currentDateTime();
    QsnWebAuthorization::accountItem *aitem = atAccount(-1);
        for (int i = 0; i < aitem->presence.count(); i ++) {
            if (aitem->presence[i].endPresence >= currDate) aitem->presence[i].endPresence = QDateTime::currentDateTime().addSecs(180);
        }
}



