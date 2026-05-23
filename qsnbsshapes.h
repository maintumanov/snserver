#ifndef QSNBSSHAPES_H
#define QSNBSSHAPES_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QStringList>
#include "qsnshapes.h"


QString QsnBsItalic(QString text, bool conditional = true);

QString QsnBsAccessRightsTextAddNote(QString text, quint8 accessRights);

QString QsnBsJsonItem(QString type, QString id, QString varname, QString value);
QString QsnBsJsonItemText(QString type, QString id, QString varname, QString value);

QString QsnBsNavItem(QString name, QString icon, QString link, bool select);


QString QsnBsFormHiddenBlockBegin(QString id, bool hidden = false);
QString QsnBsFormHiddenBlockEnd();
QString QsnBsFormTextEdit(QString name, QString value, QString id, QString type, bool isInvalid = false);
QString QsnBsFormTextArea(QString name, QString value, QString id);
QString QsnBsFormCheckboxL(QString name, bool checked, QString id);
QString QsnBsFormLabel(QString name, QString value, QString id);
QString QsnBsFormCheckbox(QString name, bool checked, QString id);
QString QsnBsFormRadio(QString name, QString group, bool checked, QString id);
QString QsnBsFormSwitch(QString name, bool checked, QString id);
QString QsnBsFormButton(QString name, QString id, QString type = QString("btn-info"));
QString QsnBsFormWeeks(QString name, quint8 days, QString id);
QString QsnBsFormSelectorBegin(QString name, QString id);
QString QsnBsFormSelectorOption(QString name, QString value, bool selected);
QString QsnBsFormSelectorEnd();

QString QsnBsFormGroupButtonBegin();
QString QsnBsFormGroupButton(QString name, QString id, QString type = QString("btn-info"));
QString QsnBsFormGroupButtonLink(QString name, QString id, QString link, QString type = QString("btn-info"));
QString QsnBsFormGroupButtonEnd();

QString QsnBsAlertDanger(QString text);
QString QsnBsAlertWarning(QString text);

QString QsnBsCategory(QString title);

QString QsnBsPanelWidgetsBegin(QString title);
QString QsnBsPanelWidgetsEnd();

QString QsnBsPanelCardBegin(quint32 id, QString title);
QString QsnBsPanelCardEnd();

QString QsnBsPanelCardItemSwitchBegin(quint8 integated, quint32 id, bool active);
QString QsnBsPanelCardItemSwitchEnd(quint8 integated);
QString QsnBsPanelCardItemSwitchSwitch(quint32 id, bool checked);
QString QsnBsPanelCardItemSwitchFunJSON(quint32 id);
QString QsnBsPanelCardItemSwitchFunClick(quint32 id);

QString QsnBsPanelCardItemLabelBegin(quint8 integated, quint32 id, bool active, QString link = QString());
QString QsnBsPanelCardItemLabelEnd(quint8 integated, QString link = QString());
QString QsnBsPanelCardItemLabelBadge(quint32 id, QString value);
QString QsnBsPanelCardItemLabelFunJSON(quint32 id);

QString QsnBsPanelCardItemLinkBegin(quint32 id, bool active, QString link = QString());
QString QsnBsPanelCardItemLinkEnd();
QString QsnBsPanelCardItemLinkFunJSON(quint32 id);

QString QsnBsIconNum(int num, QString par = QString());
QString QsnBsIconSys(QString name);
QString QsnBsIconLink();

QString QsnBsPanelCardItemName(quint8 integated, QString name);

QString QsnBsPanelCardItemEnd();

QString QsnBsTapsBegin();
QString QsnBsTapsTabAdd(QString id, QString name, bool active = false);
QString QsnBsTapsPanesBegin();
QString QsnBsTapsPanelBegin(QString id, bool active = false);
QString QsnBsTapsPanelEnd();
QString QsnBsTapsPanesEnd();
QString QsnBsTapsEnd();

QString QsnBsFormBegin();
QString QsnBsFormEnd();

QString QsnBsFormTableBegin();
QString QsnBsFormTableTheadBegin();
QString QsnBsFormTableTheadAdd(QString title, QString width = QString());
QString QsnBsFormTableTheadAddMD(QString title, QString width = QString());
QString QsnBsFormTableTheadAddCl(QString title, QString addclass = QString());
QString QsnBsFormTableTheadEnd();
QString QsnBsFormTableBodyBegin(QString id);
QString QsnBsFormTableBodyRowLogAdd(int level, QString time, QString text);
QString QsnBsFormTableBodyRowAlertAdd(bool alert, QString time, QString text, QString locales);
QString QsnBsFormTableBodyRowAdd(QString c1);
QString QsnBsFormTableBodyRowAdd(QString c1, QString c2);
QString QsnBsFormTableBodyRowAdd(QString c1, QString c2, QString c3);
QString QsnBsFormTableBodyRowAdd(QString c1, QString c2, QString c3, QString c4);
QString QsnBsFormTableBodyRowAdd(QString c1, QString c2, QString c3, QString c4, QString c5);
QString QsnBsFormTableBodyRowCustomBegin();
QString QsnBsFormTableBodyRowCustomCell(QString text);
QString QsnBsFormTableBodyRowCustomCellMD(QString text);
QString QsnBsFormTableBodyRowCustomEnd();
QString QsnBsFormTableBodyEnd();
QString QsnBsFormTableEnd();

QString QsnBsAlertBegin(QString level = QString());
QString QsnBsAlertEnd();

QString QsnBsLink(QString text, QString link);

QString QsnBsPageTitle(QString text);
QString QsnBsPageTitleBegin(QString text);
QString QsnBsPageTitleEnd();
QString QsnBsPanelTitle(QString text);
QString QsnBsPanelTitleButton(QString title, QString name, QString id, QString type, QString link);
QString QsnBsButton(QString name, QString id, QString type);

#endif // QSNBSSHAPES_H
