#include "qsnwebwidgetcardindicatorcomp.h"

QsnWebWidgetCardIndicatorComp::QsnWebWidgetCardIndicatorComp(quint32 iID, QsnGlobalModules *modules, QObject *parent) : QsnWeb(iID, modules, parent)
{

    setObjectName(tr("Comparison"));
    mds = modules;

    // Default state: normal
    isUpper = false;
    isLower = false;
    isNormal = true;
    currentImportance = 2; // Normal
    outputs = Q_NULLPTR;
    lastImportance = currentImportance;

    // Listen for global events (e.g., config save)
    connect(mds->interface, SIGNAL(snBUSOutput(QSNContainer,QObject*)), this, SLOT(snBUSInput(QSNContainer,QObject*)));
}


void QsnWebWidgetCardIndicatorComp::fromStream(QDataStream *stream, QString path)
{
    // Load child components (only "INCO" = outputs)
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

        if (sig == QString(QLatin1String("INCO"))) {
            QsnWebWidgetCardIndicatorCompOutputs *ioutputs = new QsnWebWidgetCardIndicatorCompOutputs(static_cast<quint32>(id), modules(), this);
            if (outputs != Q_NULLPTR) delete(outputs);
            outputs = ioutputs;
            outputs->fromStream(stream);
            outputs->setPath(getPath());
        }
    }

    // Apply configuration
    upperBound = optionsMap.value("UB", QString()).toString();
    lowerBound = optionsMap.value("LB", QString()).toString();
    upperImportance = static_cast<quint8>(optionsMap.value("UP", 1).toInt());   // Default: warning
    lowerImportance = static_cast<quint8>(optionsMap.value("LP", 1).toInt());   // Default: warning
    normalImportance = static_cast<quint8>(optionsMap.value("NP", 2).toInt());  // Default: normal
    boundHysteresis = optionsMap.value("BH", 0).toReal();

    // Restore last importance from temp settings
    currentImportance = normalImportance;
    currentImportance = mds->db->tempSettings.value(objectName() + QString::number(itemID()) + "importance", currentImportance).toUInt();
    lastImportance = currentImportance;
}

quint8 QsnWebWidgetCardIndicatorComp::importance(QByteArray data)
{
    if (data.isEmpty()) return 2; // Default to "normal" on empty
    compareUpper(data);
    compareLower(data);
    compareNormal();
    checkChange();
    return currentImportance;
}

void QsnWebWidgetCardIndicatorComp::snBUSInput(QSNContainer container, QObject *)
{
    // Persist state before config reload
    if (container.Command == BUSSERV_CONFIGUPDATE && container.Signal == BUSSERV_CONFIGUPDATE_BEGIN) {
        mds->db->tempSettings.insert(objectName() + QString::number(itemID()) + "importance", currentImportance);
    }
}

void QsnWebWidgetCardIndicatorComp::compareUpper(QByteArray data)
{
    if (data.count() < 3) return;
    bool ok = false;
    qreal cvol = upperBound.toDouble(&ok);
    if (!ok) return;
    qreal dvol = QSNRAWtoReal(&data, data.at(0), 1);
    if (dvol >= cvol) isUpper = true;
    if (isUpper && (dvol <= cvol - boundHysteresis)) isUpper = false;
    if (isUpper) {
        currentImportance = upperImportance;
        if (outputs != Q_NULLPTR) outputs->outputUpper();
    }
}

void QsnWebWidgetCardIndicatorComp::compareLower(QByteArray data)
{
    if (data.count() < 3) return;
    bool ok = false;
    qreal cvol = lowerBound.toDouble(&ok);
    if (!ok) return;
    qreal dvol = QSNRAWtoReal(&data, data.at(0), 1);
    if (dvol <= cvol) isLower = true;
    if (isLower && dvol >= cvol + boundHysteresis) isLower = false;
    if (isLower) {
        currentImportance = lowerImportance;
        if (outputs != Q_NULLPTR) outputs->outputLower();
    }
}

void QsnWebWidgetCardIndicatorComp::compareNormal()
{
    isNormal = !(isLower || isUpper);
    if (isNormal) {
        currentImportance = normalImportance;
        if (outputs != Q_NULLPTR) outputs->outputNormal();
    }
}

void QsnWebWidgetCardIndicatorComp::checkChange()
{
    if (currentImportance == lastImportance) return;
    lastImportance = currentImportance;
    emit onStateChange(currentImportance);
}

