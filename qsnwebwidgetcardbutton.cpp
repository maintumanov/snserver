#include "qsnwebwidgetcardbutton.h"

QsnWebWidgetCardButton::QsnWebWidgetCardButton(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Button"));
    mds = modules;
    numIcon = 0;
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardButton::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$(\"#button-%1\").click(function(){"
                          "sendState('action', '%1', 'press');}); ").arg(itemID());
}

void QsnWebWidgetCardButton::getContents(QStringList *contents, int )
{
    // CARD ITEM
    *contents << QString("<li class=\"list-group-item d-flex active\" data-unit=\"button-%1\" >").arg(itemID());
    *contents << QsnBsIconNum(numIcon);
    *contents << QString("<p class=\"specs\">%1</p>").arg(objectName());
    *contents << QString("<div class=\"d-grid mt-1 ml-auto\">"
                         "<button id=\"button-%1\" class=\"btn btn-secondary btn-mk\" type=\"button\">%2</button>"
                         "</div></li>").arg(itemID()).arg(btnname);
    *contents << QString("</li>");
}

void QsnWebWidgetCardButton::actionItem(QString, QMap<QString, QString> *options, QStringList *, qint64 )
{
    quint32 id = 0;

    if (options->contains("id")) id = options->value("id").toUInt();
    if (id != this->itemID()) return;
    if (options->contains("state")) {
        QString state = options->value("state");
        if (state == QLatin1String("press")) buttonPress("WEB");
    }
}

void QsnWebWidgetCardButton::actionRMCode(QByteArray )
{

}

void QsnWebWidgetCardButton::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    btnname = optionsMap.value("BN", tr("AUTO")).toString();

    numIcon = optionsMap.value("IC", 0).toInt();
    mds->io->registrationAction(this, tr("press"), 0, numIcon?numIcon:77);

}

void QsnWebWidgetCardButton::widgetRunAction(int ioIndex)
{
    if (ioIndex == 0) buttonPress("ACTION");
}

void QsnWebWidgetCardButton::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
}


void QsnWebWidgetCardButton::buttonPress(QString )
{
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1 [%2] - %3").arg(objectName(), tr("Server"), tr("button press")),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("widgets"),
                                   false), this);
    writeToMQTTPSN();

    QByteArray data;
    mds->io->widgetReciveIndexSignal(0, &data, this);
}

void QsnWebWidgetCardButton::writeToMQTTPSN()
{
    if (!mds->supportMQTT) return;
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 1, 1);

    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_publication_sn;
    container.Sender = 0;
    container.info = getPath();
    container.Data = data;
    mds->interface->snBUSInput(container, this);
}



