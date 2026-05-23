#include "qsnwebwidgetcardselector.h"

QsnWebWidgetCardSelector::QsnWebWidgetCardSelector(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{
    setObjectName(tr("Selector"));
    selectItem = false;
    mds = modules;
    numIcon = 0;
    selectItem = 0;
    connect(modules->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}

void QsnWebWidgetCardSelector::getFunctions(QStringList *functions, int , QString )
{
    *functions << QString("$(\"#selector-%1\").on('change', function (e) {"
                          "sendState('action', '%1', this.value);}); ").arg(itemID());
}

void QsnWebWidgetCardSelector::getFunctionsJSON(QStringList *fjson, int )
{
    *fjson << QString("$('#selector-%1 option[value=' + data.selector%1item + ']').prop('selected', true);").arg(itemID());
}

void QsnWebWidgetCardSelector::getContents(QStringList *contents, int )
{
    // CARD ITEM
    *contents << QString("<li class=\"list-group-item d-flex %2\" "
                         "data-unit=\"selector-%1\">").arg(itemID()).arg(" active");
    *contents << QsnBsIconNum(numIcon);
    *contents << QString("<p class=\"specs\">%1</p>").arg(objectName());
    *contents << QString("<div class=\"ms-auto mt-1\">"
                         "<select id=\"selector-%1\" class=\"select_m inline-card\" "
                         "style=\"min-width: 6.5rem;\">").arg(itemID());
    for (int i = 0; i < items.count(); i ++)
        *contents << QString("<option value=\"%1\" %3>%2</option>")
                     .arg(i).arg(items[i]->objectName(), (selectItem == i?"selected":""));

    *contents << QString("</select></div></li>");
}

void QsnWebWidgetCardSelector::actionItem(QString, QMap<QString, QString> *options, QStringList *, qint64 )
{
    quint32 id = 0;
    if (options->contains("id")) id = options->value("id").toUInt();
    if (id != this->itemID()) return;
    if (options->contains("state")) {
        QString state = options->value("state");
        bool ok = false;
        quint8 newsel = state.toUInt(&ok);
        if (ok) setSelect(newsel, "WEB");
    }
}

void QsnWebWidgetCardSelector::actionRMCode(QByteArray )
{

}

void QsnWebWidgetCardSelector::getItemJSON(QStringList *jsonItems)
{
    *jsonItems << QsnBsJsonItem("selector", QString::number(itemID()), "item", QString::number(selectItem));
}

void QsnWebWidgetCardSelector::fromStream(QDataStream *stream, QString path)
{
    int count;
    QString sig;
    int id;
    setPath(path);
    *stream >> sig;
    if (!sig.isEmpty()) setObjectName(sig);

    QMap<QString, QVariant> optionsMap = getOptionsFromStream(stream);
    mds->io->loadIOFromStream(stream, this);

    *stream >> count; //items

    for (int i = 0; i < count; i ++) {
        *stream >> sig;
        *stream >> id;

        if (sig == QString(QLatin1String("SLI"))) {
            QsnWebWidgetCardSelectorItem *item = new QsnWebWidgetCardSelectorItem(static_cast<quint32>(id), modules(), this);
            item->fromStream(stream);
            items.append(item);
        }
    }
    numIcon = optionsMap.value("IC", 0).toInt();

    selectItem = mds->settings->value(QString("%1_selector").arg(itemID()), 0).toUInt();
    selectItem = mds->db->tempSettings.value(objectName()+QString::number(itemID()) + "lastdata", selectItem).toUInt();
    for (int i = 0; i < items.count(); i ++) mds->io->registrationAction(this, items[i]->objectName(), i, numIcon?numIcon:77);
}

void QsnWebWidgetCardSelector::receiveSignalIOIndex(int indexIO, QByteArray *data)
{
    if (indexIO == 1 && QSNRAWtoByte(data, 0) == 5) {
        setSelect(QSNRAWtoByte(data, 1), "signal");
    }
}

void QsnWebWidgetCardSelector::widgetRunAction(int ioIndex)
{
    if (ioIndex < items.count()) setSelect(ioIndex, "runAction");
}

void QsnWebWidgetCardSelector::snBUSInput(QSNContainer container, QObject *sender)
{
    if (sender == this) return;
    if (container.role != QSNContainer::service) return;
    if (container.Command == BUSSERV_MGTT) {
        if (container.Signal == BUSSERV_MGTT_reception_sn && container.info == getPath() && !container.Signal) readFromMQTT(&container.Data);
        if (container.Signal == BUSSERV_MGTT_request_sn && QSNMQTTTopicFilterCompare(getPath(), container.info)  && container.Signal) writeToMQTTPSN(selectItem);
    }
    //    if (container.Command == BUSSERV_DB) {
    //        if (!dbName.isEmpty() && container.info == dbName && !container.Data.isEmpty()) {
    //            if (container.Data.at(0) == 1) setStateSwitch(QSNRAWtoBool(&container.Data, 1), "BUSSERV_DB");
    //        }
    //    }
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        mds->db->tempSettings.insert(QString::number(itemID()) + "item", selectItem);
    }
}

void QsnWebWidgetCardSelector::setSelect(quint8 item, QString source)
{
    selectItem = item;
    mds->interface->snBUSInput(QSNLogToContainer(
                                   BUSSERV_LOG_LEVEL_information,
                                   QString("%1 [%2] - %3 %4").arg(objectName(), tr("Server"), tr("select item from %1").arg(source)).arg(selectItem),
                                   BUSSERV_LOG_CATEGORY_module,
                                   QString("widgets"),
                                   false), this);
    mds->settings->setValue(QString("%1_selector").arg(itemID()), selectItem);
    writeToMQTTPSN(selectItem);
    if (source != "signal") {
        QByteArray data;
        QSNByteToRAW(&data, 0, 5);
        QSNByteToRAW(&data, 1, selectItem);
        mds->io->widgetReciveIndexSignal(0, &data, this);
    }
    //    if (source != "BUSSERV_DB") sendDB(selectItem);
}

void QsnWebWidgetCardSelector::sendDB(quint8 item)
{
    QByteArray d;
    d.append(1);
    QSNBoolToRAW(&d, 1, item);
    //    mds->db->writeRAWtoYBD(dbName, &d, QDateTime::currentDateTime(), getLabel(), this);
}

void QsnWebWidgetCardSelector::readFromMQTT(QByteArray *data)
{
    if (data->at(0) != 5) return;
    setSelect(QSNRAWtoByte(data, 1), "BUSSERV_MGTT_SN");
}

void QsnWebWidgetCardSelector::writeToMQTTPSN(quint8 state)
{
    if (!mds->supportMQTT) return;
    QByteArray data;
    QSNByteToRAW(&data, 0, 1);
    QSNBoolToRAW(&data, 5, state);

    QSNContainer container = newContainer();
    container.role = QSNContainer::service;
    container.Command = BUSSERV_MGTT;
    container.Signal = BUSSERV_MGTT_publication_sn;
    container.Sender = 0;
    container.info = getPath();
    container.Data = data;
    mds->interface->snBUSInput(container, this);
}


QString QsnWebWidgetCardSelector::getLink()
{
    //    if (!dbName.isEmpty()) return QString("onclick=\"location.href='/graph?name=%1';\"").arg(dbName);
    return QString();
}

