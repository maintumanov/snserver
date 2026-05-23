#include "qsnwebpageusers.h"

QsnWebPageUsers::QsnWebPageUsers(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Users"));
    widgetUrl = "/users";
    widgetIcon = "subicon-users";
}

QsnWebPageUsers::~QsnWebPageUsers()
{

}

void QsnWebPageUsers::getFunctions(QStringList *functions, int accountIndex, QString)
{
    if (AID >= -1) {
        *functions << QString("function itemRemove(){"
                              " $.ajax({"
                              "url: \"%1\","
                              "type: 'POST',"
                              "data: {action : 'remove', AID: '%2'},"
                              "dataType: 'json',"
                              "contentType: \"application/json; charset=utf-8\","
                              "success: function(data) {"
                              "if (data.remove == 'ok') {"
                              "location.replace('%1');"
                              //                              "history.back();"
                              "} else {"
                              "} }, "
                              "error: function (x, t, e) { } }); "
                              "};").arg(widgetUrl).arg(AID);


        *functions << QString("$(\"#personal-remove\").click(function(){itemRemove();});");


        *functions << QString("function save(){"
                              " $.ajax({"
                              "url: \"%1\","
                              "type: 'POST',"
                              "dataType: 'json', "
                              "contentType: 'application/json; charset=utf-8',"
                              "data: {"
                              "action : 'save', "
                              "AID: '%2',"
                              "fname: $('#full-name').val(),"
                              "email: $('#user-email').val(), "
                              "login: $('#user-name').val(), ").arg(widgetUrl).arg(AID);
        if (mds->auth->atAccount(accountIndex)->accessRights > 1)
            *functions << QString("admin: $('#admin').prop('checked'),");
        if (mds->auth->atAccount(accountIndex)->accessRights > 0)
            *functions << QString("guest: $('#guest').prop('checked'),");

        for (int p = 0; p < mds->auth->permissions.count(); p ++)
            *functions << QString("%1: $('#%1').prop('checked'),").arg(mds->auth->permissions[p].name);

        for (int p = 0; p < mds->auth->parameters.count(); p ++) {
            if (mds->auth->parameters[p].type == "bool")
                *functions << QString("%1: $('#%1').prop('checked'),").arg(mds->auth->parameters[p].name);
            else
                *functions << QString("%1: $('#%1').val(),").arg(mds->auth->parameters[p].name);
        }

        *functions << QString("password: $('#user-password').val(),"
                              "password_c: $('#user-password-confirm').val()"
                              "},"
                              "contentType: \"application/json; charset=utf-8\","
                              "success: function(data) {"
                              "if (data.savestate == 1) $(\"#user-name\").addClass(\"is-invalid\");"
                              "else $(\"#user-name\").removeClass(\"is-invalid\");"
                              "if (data.savestate == 2) {$(\"#user-password\").addClass(\"is-invalid\");"
                              "$(\"#user-password-confirm\").addClass(\"is-invalid\");}"
                              "else {$(\"#user-password\").removeClass(\"is-invalid\");"
                              "$(\"#user-password-confirm\").removeClass(\"is-invalid\");}"
                              "if (data.savestate == 0) document.location.href=document.referrer;"
                              " }, "
                              "error: function (x, t, e) { } }); "
                              "};");//.arg(AID == -1?QString("location.replace('%1');").arg(widgetUrl()):"");

        //===============================================================================
        *functions << QString("$(\"#personal-save\").click(function(){save();});");

        *functions << QString("$(\"#permissions-save\").click(function(){save();});");

        if (mds->auth->atAccount(accountIndex)->accessRights > 1) {
            *functions << QString("$(\"#admin\").click(function(){$('#guest').prop('checked', false)});");
            *functions << QString("$(\"#guest\").click(function(){$('#admin').prop('checked', false)});");
        }
    } else {

    }
}

void QsnWebPageUsers::getFunctionsJSON(QStringList *fjson, int )
{

}

void QsnWebPageUsers::getContents(QStringList *contents, int accountIndex)
{
    if (AID < -1) { //users list
        *contents << QsnBsPanelTitle(objectName());
        *contents << QString("<div class=\"col-12\">");
        *contents << QsnBsFormTableBegin();
        *contents << QsnBsFormTableTheadBegin();
        *contents << QsnBsFormTableTheadAddCl(tr("User name"));
        *contents << QsnBsFormTableTheadAddCl(tr("Full name"));
        *contents << QString("<th class=\"d-none d-md-table-cell\" scope=\"col\">%1</th>").arg(tr("Last access"));
        *contents << QString("<th class=\"d-none d-md-table-cell\" scope=\"col\">%1</th>").arg(tr("Last denied"));

        *contents << QString("<th class=\"text-right p-1 pe-2 align-middle col-1\" >");
        *contents << QString("<button type=\"button\" class=\"btn btn-primary btn-sm p-0\" "
                             "onclick=\"window.location.href='%1?AID=-1';\">").arg(widgetUrl);
        *contents << QString("<svg class=\"icon-sprite icon-05x \"><use xlink:href=\"assets/images/icons-sprite.svg#subicon-add\" /></svg>");
        *contents << QString("</button>");
        *contents << QString("</th>");
        *contents << QsnBsFormTableTheadEnd();

        *contents << QsnBsFormTableBodyBegin(QString("table_users"));

        QsnWebAuthorization::accountItem *aitem;
        for (int i = 0; i < mds->auth->countAccounts(); i ++) {
            aitem = mds->auth->atAccount(i);

            *contents << QString("<tr%1>").arg((aitem->countAccessDenied >= 3 && QDateTime::currentDateTime() < aitem->lastAccessDenied.addSecs(600))?" class=\"caution\"":"");
            *contents << QString("<td>%1</td>").arg(aitem->login);
            *contents << QString("<td>%1</td>").arg(aitem->name);
            *contents << QString("<td class=\"d-none d-md-table-cell\">%1</td>").arg(QSNDateTimeToString(aitem->lastAccessAllowed));
            *contents << QString("<td class=\"d-none d-md-table-cell\">%1</td>").arg(QSNDateTimeToString(aitem->lastAccessDenied));
            *contents << QString("<td class=\"td-actions text-right p-0 pe-2 align-middle\">");
            *contents << QString("<button type=\"button\" class=\"btn btn-secondary btn-ms p-0\""
                                 "onclick=\"window.location.href='%1?AID=%2';\">").arg(widgetUrl).arg(aitem->ID);
            *contents << QString("<svg class=\"icon-sprite icon-05x\">"
                                 "<use xlink:href=\"assets/images/icons-sprite.svg#subicon-edit\" />"
                                 "</svg>");
            *contents << QString("</button></td>");
            *contents << QString("</tr>");
        }
        *contents << QsnBsFormTableBodyEnd();
        *contents << QsnBsFormTableEnd();
        *contents << QString("</div>");
    } else {
        //================================== =======================================
        QsnWebAuthorization::accountItem aitem;
        bool isExistAID = mds->auth->checkAID(AID);
        if (isExistAID) {
            aitem = *mds->auth->atAccount(mds->auth->accountIndexfromAID(AID));
            *contents << QsnBsPanelTitle(tr("Editing a user"));
        } else {
            aitem.email = "";
            aitem.name = tr("User");
            aitem.login = "user";
            aitem.password = "password";
            aitem.accessRights = 1;
            if (isKey("guest")) aitem.accessRights = 0;
            mds->auth->fillAccount(&aitem);
            *contents << QsnBsPanelTitle(tr("Adding a new user"));
        }

        *contents << QsnBsTapsBegin();
        *contents << QsnBsTapsTabAdd("personal", tr("PERSONAL"), true);
        *contents << QsnBsTapsTabAdd("permissions", tr("PERMISSIONS"), false);
        *contents << QsnBsTapsPanesBegin();

        // ============== personal ====================
        *contents << QsnBsTapsPanelBegin("personal", true);

        *contents << QsnBsFormTextEdit(tr("User name"), aitem.login, "user-name", "text", false);
        *contents << QsnBsFormTextEdit(tr("Full name"), aitem.name, "full-name", "text", false);
        *contents << QsnBsFormTextEdit(tr("Email"), aitem.email, "user-email", "email", false);
        //        *contents << FormFieldText(tr("Phone"), aitem.email, "user-phone", "tel");
        if (aitem.parameters.count() > 0) {
            for (int p = 0; p < aitem.parameters.count(); p ++) {
                if (aitem.parameters[p].type != "bool") {
                    if (isKey(aitem.parameters[p].name))
                        *contents << QsnBsFormTextEdit(aitem.parameters[p].title, getValue(aitem.parameters[p].name), aitem.parameters[p].name, "text", false);
                    else
                        *contents << QsnBsFormTextEdit(aitem.parameters[p].title, aitem.parameters[p].value, aitem.parameters[p].name, "text", false);
                }
            }
        }
        *contents << QsnBsFormTextEdit(tr("Password"), aitem.password, "user-password", "password", false);
        *contents << QsnBsFormTextEdit(tr("Confirm password"), aitem.password, "user-password-confirm", "password", false);

        *contents << QsnBsFormGroupButtonBegin();
        *contents << QsnBsFormGroupButton(isExistAID?tr("Save"):tr("Add"), "personal-save", "btn-primary");
        *contents << QsnBsFormGroupButtonLink(tr("Cancel"), "personal-cancel","document.location.href=document.referrer;" , "btn-secondary");
        if (isExistAID) *contents << QsnBsFormGroupButton(tr("Remove"), "personal-remove", "btn-danger");
        *contents << QsnBsFormGroupButtonEnd();

        *contents << QsnBsTapsPanelEnd();

        // ============== permissions ====================
        *contents << QsnBsTapsPanelBegin("permissions", false);

        //bool wEnable = mds->auth->checkAcces(accountIndex, widgetAccessRights());
        if (mds->auth->atAccount(accountIndex)->accessRights > 1)
            *contents << QsnBsFormCheckbox(tr("Administrator"), aitem.accessRights == 2, "admin");
        if (mds->auth->atAccount(accountIndex)->accessRights > 0)
            *contents << QsnBsFormCheckbox(tr("Guest"), aitem.accessRights == 0, "guest");


        if (aitem.permissions.count() > 0) {
            for (int p = 0; p < aitem.permissions.count(); p ++) {
                *contents << QsnBsFormCheckbox(aitem.permissions[p].title, aitem.permissions[p].value, aitem.permissions[p].name);
            }
        }

        if (aitem.parameters.count() > 0) {
            for (int p = 0; p < aitem.parameters.count(); p ++) {
                if (aitem.parameters[p].type == "bool") {
                    *contents << QsnBsFormCheckbox(aitem.parameters[p].title, aitem.parameters[p].value == "true"?true:false, aitem.parameters[p].name);
                }
            }
        }
        *contents << QsnBsTapsPanelEnd();
        *contents << QsnBsTapsPanesEnd();
    }
}

void QsnWebPageUsers::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 accountIndex)
{
    if (options->contains("action")) {
        QString state = options->value("action");

        // ====== save =================
        if (state == QLatin1String("save"))  {
            QsnWebAuthorization::accountItem aitem;
            mds->auth->fillAccount(&aitem);
            aitem.ID = options->value("AID").toLongLong();
            aitem.name = QSNDecodeText(options->value("fname"));
            aitem.email = QSNDecodeText(options->value("email"));
            aitem.login = QSNDecodeText(options->value("login"));
            if (QSNDecodeText(options->value("guest")) == "true" && mds->auth->atAccount(mds->auth->accountIndexfromAID(accountIndex))->accessRights > 0) aitem.accessRights = 0;
            else if (QSNDecodeText(options->value("admin")) == "true" && mds->auth->atAccount(mds->auth->accountIndexfromAID(accountIndex))->accessRights > 1) aitem.accessRights = 2;
            else aitem.accessRights = 1;
            for (int p = 0; p < aitem.permissions.count(); p ++) {
                if (options->contains("" + aitem.permissions[p].name))
                    aitem.permissions[p].value = (QSNDecodeText(options->value("" + aitem.permissions[p].name)) == "true"?true:false);
            }
            for (int p = 0; p < aitem.parameters.count(); p ++) {
                if (options->contains(aitem.parameters[p].name))
                    aitem.parameters[p].value = QSNDecodeText(options->value( aitem.parameters[p].name));
            }

            aitem.password = QSNDecodeText(options->value("password"));
            QString pswc = QSNDecodeText(options->value("password_c"));
            if (aitem.password != pswc) *returnItems << QString("\"savestate\": 2");
            else *returnItems << QString("\"savestate\": %1").arg(mds->auth->applyAccount(aitem));
        }


        // ====== remove account =================
        if (state == QLatin1String("remove")) {
            mds->auth->deleteAccount(options->value("AID").toLongLong());
            *returnItems << QString("\"remove\": \"ok\"");
        }

        *returnItems << QString("\"usersp\": %1").arg(userspToJSON());
    }
}

void QsnWebPageUsers::fromStream(QDataStream *stream)
{
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);

    // set options
    if (optionsMap.contains("Url")) widgetUrl = optionsMap.value("Url").toString();

    mds->io->loadIOFromStream(stream, this);
    *stream >> count; //items

}

void QsnWebPageUsers::urlChanged(int )
{
    AID = -10;
    if (isKey("AID")) {
        AID = getValue("AID").toLongLong();
    }
}

QString QsnWebPageUsers::widgetState()
{
    return QString("%1 %2").arg(tr("accounts")).arg(mds->auth->countAccounts());
}

QString QsnWebPageUsers::avatarBase64(QString name)
{
    QFile img(QSNHomeSubPath("server", "photo").absoluteFilePath(QString("%2.jpeg").arg(name)));

    if (img.open(QIODevice::ReadOnly)) {
        QByteArray raw = img.readAll();
        img.close();
        return QString(raw.toBase64());
    }
    return QString();
}

QString QsnWebPageUsers::userspToJSON()
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
        ret += QString("\"ldet\": \"%1\",").arg(lastDetect.toString("dd.MM.yy hh:mm"));
        ret += QString("\"src\": \"%1\",").arg(sourceDetect);
        ret += QString("\"eabs\": \"%1\"").arg(endAbsence.toString("dd.MM.yy hh:mm"));
        ret += "}";
    }
    ret += "]";
    return ret;
}



