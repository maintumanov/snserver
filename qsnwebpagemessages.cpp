#include "qsnwebpagemessages.h"

QsnWebPageMessages::QsnWebPageMessages(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    mds = modules;
    setObjectName(tr("Messages"));
    widgetUrl = "/messages";
    widgetIcon = "subicon-messages";
    isMessageEdit = false;
    itemMessageEdit = -1;
}

QsnWebPageMessages::~QsnWebPageMessages()
{

}

void QsnWebPageMessages::getFunctions(QStringList *functions, int, QString)
{
    if (itemMessageEdit == -1) return;

    *functions << QString(
                      "$('#savebtn').click(function() {"
                      " $.ajax({"
                      "url: \"%1\","
                      "type: 'POST',"
                      "data: {"
                      "action : 'save', "
                      "item: \"%2\","
                      "text: $('#texedit').val(),"
                      "type: $('#typeselect').find(\":selected\").val()"
                      "},"
                      "dataType: 'json',"
                      "contentType: \"application/json; charset=utf-8\","
                      "success: function(data) {"
                      "if (data.save == true) {document.location.href=\"%1\"};"
                      " }, "
                      "error: function (x, t, e) { } }); "
                      "});").arg(widgetUrl).arg(messages[itemMessageEdit]->itemID());
}

void QsnWebPageMessages::getContents(QStringList *contents, int )
{
    if (itemMessageEdit >= messages.count() || itemMessageEdit < 0) {
        //================================== Messages list =======================================

        *contents << QsnBsPanelTitle(objectName());
        for (int i = 0; i < messages.count(); i ++) {
            *contents << QString("<div class=\"col-12 col-sm-6 col-md-6 col-xl-3 col-lg-4 px-0 px-sm-1\">");
            *contents << QString("<a href=\"%1?itemmessage=%2\" class=\"mvh-150 card card-link my-2\" >").arg(widgetUrl).arg(messages[i]->itemID());
            *contents << QString("<div class=\"card-body\">");
            *contents << QString("<h5 class=\"card-title text-center\">%1</h5>").arg(messages[i]->objectName());
            *contents << QString("<h6 class=\"card-text\">%1</h6>").arg(messages[i]->getText());
            *contents << QString("</div></a></div>");
        }

    } else {
        //================================== Message edit =======================================

        *contents << QsnBsPanelTitle(objectName() + " - " + messages[itemMessageEdit]->objectName());
        *contents << QsnBsFormBegin();
        *contents << QsnBsFormSelectorBegin(tr("Type"), "typeselect");
        *contents << QsnBsFormSelectorOption(QSNWarningLevelToLabel(2), "info", (messages[itemMessageEdit]->getLevel() == 2));
        *contents << QsnBsFormSelectorOption(QSNWarningLevelToLabel(1), "warn", (messages[itemMessageEdit]->getLevel() == 1));
        *contents << QsnBsFormSelectorOption(QSNWarningLevelToLabel(0), "caut", (messages[itemMessageEdit]->getLevel() == 0));
        *contents << QsnBsFormSelectorEnd();
        *contents << QsnBsFormTextArea(tr("Message"), messages[itemMessageEdit]->getText(), "texedit");
        *contents << QsnBsFormGroupButtonBegin();
        *contents << QsnBsFormGroupButtonLink(tr("Cancel"), "personal-cancel", QString("document.location.href='%1'").arg(widgetUrl) , "btn-secondary");
        *contents << QsnBsFormGroupButton(tr("Save"), "savebtn", "btn-primary");
        *contents << QsnBsFormGroupButtonEnd();
        *contents << QsnBsFormEnd();
    }
}

void QsnWebPageMessages::actionItem(QString, QMap<QString, QString> *options, QStringList *returnItems, qint64 )
{
    if (options->contains("action")) {
        QString state = options->value("action");
        if (state == QLatin1String("save"))  {
            int item = IDtoItem(static_cast<quint32>(options->value("item").toUInt()));
            if (item < messages.count() && item >= 0){
                messages[item]->setText(QSNDecodeText(options->value("text")));
                if (options->value("type", "info") == "info") messages[item]->setLevel(2);
                if (options->value("type", "info") == "warn") messages[item]->setLevel(1);
                if (options->value("type", "info") == "caut") messages[item]->setLevel(0);

                saveSetting();
                *returnItems << QString("\"save\":true");
            }
        }
    }
}

void QsnWebPageMessages::fromStream(QDataStream *stream)
{
    int id;
    int count;
    QString sig;
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    getOptionsFromStream(stream);

    // set options
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items
    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;
        if (sig == QString(QLatin1String("MSG"))) {
            QsnWebItemMessage *msg = new QsnWebItemMessage(static_cast<quint32>(id), modules(), this);
            msg->fromStream(stream);
            connect(msg, SIGNAL(sendMessage(QString)), this, SLOT(sendMessage(QString)));
            messages.append(msg);
        }
    }

    loadSetting();
}

void QsnWebPageMessages::urlChanged(int )
{
    itemMessageEdit = -1;
    if (isKey("itemmessage")) {
        itemMessageEdit = IDtoItem(static_cast<quint32>(getValue("itemmessage").toUInt()));
    }
}

void QsnWebPageMessages::sendMessage(QString text)
{
    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MESSAGE;
    container.Signal = 3;
    container.Sender = 0;
    container.info = text;
    mds->interface->snBUSInput(container, this);
}

void QsnWebPageMessages::saveSetting()
{
    QFile file(QSNHomeSubPath("server", "messages").absoluteFilePath("messages.xml"));
    if (!file.open(QIODevice::WriteOnly)) return;
    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement(QLatin1String("messages"));
    for (int i = 0; i < messages.count(); i ++) {
        stream.writeStartElement("message");
        messages[i]->writeSetting(&stream);
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
}

void QsnWebPageMessages::loadSetting()
{
    QFile file(QSNHomeSubPath("server", "messages").absoluteFilePath("messages.xml"));
    if (!file.open(QIODevice::ReadOnly)) return;
    QXmlStreamReader::TokenType token;
    QXmlStreamReader xmlDoc(&file);
    QXmlStreamAttributes attrib;
    xmlDoc.readNext();
    QString key;
    while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
        token = xmlDoc.readNext();
        if (token == QXmlStreamReader::StartElement && xmlDoc.name() == QLatin1String("messages")) {
            while (!xmlDoc.atEnd() && !xmlDoc.hasError()) {
                xmlDoc.readNext();
                if (xmlDoc.isStartElement() && xmlDoc.name() == QLatin1String("message")) {
                    attrib = xmlDoc.attributes();
                    key = attrib.value("key").toString();
                    for (int i = 0; i < messages.count(); i ++)
                        if (messages[i]->key() == key) messages[i]->readSetting(&attrib);
                }
                while (xmlDoc.isEndElement()) xmlDoc.readNext();
            }
        }
    }
    file.close();
}

int QsnWebPageMessages::IDtoItem(quint32 id)
{
    for (int i = 0; i < messages.count(); i ++)
        if (messages[i]->itemID() == id) return i;
    return -1;
}



