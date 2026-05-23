#include "qsnwebwidgetcardtwobuttons.h"

QsnWebWidgetCardTwoButtons::QsnWebWidgetCardTwoButtons(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Two buttons"));
    mds = modules;
    numIcon = 0;
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardTwoButtons::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$(\"#buttonsO-%1\").click(function(){"
                          "sendState('action', '%1', 'button1');}); ").arg(itemID());
    *functions << QString("$(\"#buttonsT-%1\").click(function(){"
                          "sendState('action', '%1', 'button2');}); ").arg(itemID());
}

void QsnWebWidgetCardTwoButtons::getContents(QStringList *contents, int )
{
    // CARD ITEM
    *contents << QString("<li class=\"list-group-item d-flex active\" data-unit=\"buttons-%1\" >").arg(itemID());
    *contents << QsnBsIconNum(numIcon);
    *contents << QString("<p class=\"specs\">%1</p>").arg(objectName());
    *contents << QString("<div class=\"d-grid mt-1 ms-auto\">"
                         "<button id=\"buttonsO-%1\" class=\"btn btn-secondary btn-mk\" type=\"button\">%2</button>"
                         "<button id=\"buttonsT-%1\" class=\"btn btn-secondary btn-mk ms-1\" type=\"button\">%3</button>"
                         "</div></li>").arg(itemID()).arg(btnname1, btnname2);
    *contents << QString("</li>");
}

void QsnWebWidgetCardTwoButtons::actionItem(QString, QMap<QString, QString> *options, QStringList *, qint64 )
{
    quint32 id = 0;

    if (options->contains("id")) id = options->value("id").toUInt();
    if (id != this->itemID()) return;
    if (options->contains("state")) {
        QString state = options->value("state");
        if (state == QLatin1String("button1")) buttonPress(0, "WEB");
        if (state == QLatin1String("button2")) buttonPress(1, "WEB");

    }
}

void QsnWebWidgetCardTwoButtons::actionRMCode(QByteArray )
{

}

void QsnWebWidgetCardTwoButtons::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    btnname1 = optionsMap.value("B1", tr("OPEN")).toString();
    btnname2 = optionsMap.value("B2", tr("CLOSE")).toString();

    numIcon = optionsMap.value("IC", 0).toInt();
    mds->io->registrationAction(this, tr("press 1"), 0, numIcon?numIcon:77);
    mds->io->registrationAction(this, tr("press 2"), 1, numIcon?numIcon:77);

}

void QsnWebWidgetCardTwoButtons::widgetRunAction(int ioIndex)
{
    buttonPress(ioIndex, "ACTION");
}

void QsnWebWidgetCardTwoButtons::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
}


void QsnWebWidgetCardTwoButtons::buttonPress(quint8 button, QString )
{
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1 [%2] - %3").arg(objectName(), tr("Server"), tr("button %1 press").arg(button + 1)),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("widgets"),
                                   false), this);
    writeToMQTTPSN(button);

    QByteArray data;
    mds->io->widgetReciveIndexSignal(button, &data, this);
}

void QsnWebWidgetCardTwoButtons::writeToMQTTPSN(quint8 button)
{
    if (!mds->supportMQTT) return;
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, button);

    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_publication_sn;
    container.Sender = 0;
    container.info = getPath();
    container.Data = data;
    mds->interface->snBUSInput(container, this);
}



