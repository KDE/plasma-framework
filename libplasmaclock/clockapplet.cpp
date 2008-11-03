/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2008 by Sebastian Kuegler <sebas@kde.org>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "clockapplet.h"

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QSpinBox>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsView>
#include <QtCore/QDate>

#include <KColorScheme>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KDatePicker>
#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>

#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/dataengine.h>
#include <plasma/dialog.h>
#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/theme.h>

#include "calendar.h"

#include "ui_timezonesConfig.h"

class ClockApplet::Private
{
public:
    Private()
        : timezone(ClockApplet::localTimezone())
    {}

    Ui::timezonesConfig ui;
    QString timezone;
    QPoint clicked;
    QStringList selectedTimezones;
    QString prettyTimezone;

    void addTzToTipText(QString &subText, QString tz) 
    {
        Plasma::Applet applet;
        Plasma::DataEngine::Data data = applet.dataEngine("time")->query(tz);
        if (tz == "UTC")  {
            subText += "<br><b>UTC</b> ";
        }
        else  {
            subText += "<br><b>" + data["Timezone City"].toString().replace("_", " ")+"</b> ";
        }
        subText += KGlobal::locale()->formatTime(data["Time"].toTime(), false) + ", ";
        subText += KGlobal::locale()->formatDate(data["Date"].toDate());
    }
};

ClockApplet::ClockApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      d(new Private)
{
    setPopupIcon(QIcon());
}

ClockApplet::~ClockApplet()
{
    delete d;
}

void ClockApplet::toolTipAboutToShow()
{
    updateContent();
}

void ClockApplet::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}

void ClockApplet::updateContent()
{
    Plasma::ToolTipContent tipData;

    {
        // the main text contains the current timezone's time and date
        Plasma::DataEngine::Data data = dataEngine("time")->query(currentTimezone());
        QString mainText = d->prettyTimezone + " ";
        mainText += KGlobal::locale()->formatTime(data["Time"].toTime(), false) + "<br>";
        mainText += KGlobal::locale()->formatDate(data["Date"].toDate());
        tipData.setMainText(mainText);
    }

    QString subText;
    if (!isLocalTimezone()) { 
        d->addTzToTipText(subText, localTimezone()); 
    }

    foreach (const QString &tz, getSelectedTimezones()) {
        if (tz == currentTimezone()) {
            continue;
        }
        d->addTzToTipText(subText, tz);
    }

    tipData.setSubText(subText);

    // query for custom content
    Plasma::ToolTipContent customContent = toolTipContent();
    if (customContent.image().isNull()) {
        tipData.setImage(KIcon("chronometer").pixmap(IconSize(KIconLoader::Desktop)));
    } else {
        tipData.setImage(customContent.image());
    }

    if (!customContent.mainText().isEmpty()) {
        // add their main text
        tipData.setMainText(customContent.mainText() + "<br>" + tipData.mainText());
    }

    if (!customContent.subText().isEmpty()) {
        // add their sub text
        tipData.setSubText(customContent.subText() + "<br>" + tipData.subText());
    }

    tipData.setAutohide(false);
    Plasma::ToolTipManager::self()->setContent(this, tipData);
}

Plasma::ToolTipContent ClockApplet::toolTipContent()
{
    return Plasma::ToolTipContent();
}

void ClockApplet::createConfigurationInterface(KConfigDialog *parent)
{
    createClockConfigurationInterface(parent);

    QWidget *widget = new QWidget();
    d->ui.setupUi(widget);

    parent->addPage(widget, i18n("Time Zones"), Applet::icon());

    foreach (const QString &tz, d->selectedTimezones) {
        d->ui.timeZones->setSelected(tz, true);
    }

    updateClockDefaultsTo();
    int defaultSelection = d->ui.clockDefaultsTo->findText(currentTimezone());
    if (defaultSelection >= 0) {
        d->ui.clockDefaultsTo->setCurrentIndex(defaultSelection);
    }

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(d->ui.timeZones, SIGNAL(itemSelectionChanged()), this, SLOT(updateClockDefaultsTo()));

#if 0
#ifdef CLOCK_APPLET_CONF
    ui.localTimeZone->setChecked(isLocalTimezone());
    ui.timeZones->setEnabled(!isLocalTimezone());
    foreach (const QString &str, selectedTimezones) {
        ui.timeZones->setSelected(str, true);
    }
#endif
#endif
}

void ClockApplet::createClockConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent)
}

void ClockApplet::clockConfigAccepted()
{

}

void ClockApplet::configAccepted()
{
    KConfigGroup cg = config();

    d->selectedTimezones = d->ui.timeZones->selection();
    cg.writeEntry("timeZones", d->selectedTimezones);

    QString newTimezone;

    if (d->ui.clockDefaultsTo->currentIndex() == 0) {
        //The first position in ui.clockDefaultsTo is "Local"
        newTimezone = localTimezone();
    } else {
        newTimezone = d->ui.clockDefaultsTo->currentText();
    }

    changeEngineTimezone(currentTimezone(), newTimezone);
    setCurrentTimezone(newTimezone);

    clockConfigAccepted();
    constraintsEvent(Plasma::SizeConstraint);
    update();

    emit configNeedsSaving();
}

void ClockApplet::updateClockDefaultsTo()
{
    QString oldSelection = d->ui.clockDefaultsTo->currentText();
    d->ui.clockDefaultsTo->clear();
    d->ui.clockDefaultsTo->addItems(d->ui.timeZones->selection());
    d->ui.clockDefaultsTo->insertItem(0, "Local");
    int newPosition = d->ui.clockDefaultsTo->findText(oldSelection);
    if (newPosition >= 0) {
        d->ui.clockDefaultsTo->setCurrentIndex(newPosition);
    }
    if (d->ui.clockDefaultsTo->count() > 1) {
        d->ui.clockDefaultsTo->setEnabled(true);
    }
    else {
        // Only "Local" in ui.clockDefaultsTo
        d->ui.clockDefaultsTo->setEnabled(false);
    }
}

void ClockApplet::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    // reimplemented by subclasses to get the new data
    Q_UNUSED(oldTimezone);
    Q_UNUSED(newTimezone);
}

void ClockApplet::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (d->selectedTimezones.count() < 1) {
        return;
    }

    QString newTimezone;

    if (isLocalTimezone()) {
        if (event->delta() > 0) {
            newTimezone = d->selectedTimezones.last();
        } else {
            newTimezone = d->selectedTimezones.first();
        }
    } else {
        int current = d->selectedTimezones.indexOf(currentTimezone());

        if (event->delta() > 0) {
            int previous = current - 1;
            if (previous < 0) {
                newTimezone = localTimezone();
            } else {
                newTimezone = d->selectedTimezones.at(previous);
            }
        } else {
            int next = current + 1;
            if (next > d->selectedTimezones.count() - 1) {
                newTimezone = localTimezone();
            } else {
                newTimezone = d->selectedTimezones.at(next);
            }
        }
    }

    changeEngineTimezone(currentTimezone(), newTimezone);
    setCurrentTimezone(newTimezone);

    update();
}

void ClockApplet::initExtenderItem(Plasma::ExtenderItem *item)
{
    Plasma::Calendar *calendar = new Plasma::Calendar;
    calendar->setMinimumSize(QSize(200, 180));

    Plasma::DataEngine::Data data = dataEngine("time")->query(currentTimezone());
    QDate date = data["Date"].toDate();
    if (date.isValid()) {
        calendar->setDate(date);
    }

    item->setWidget(calendar);
    item->setTitle(i18n("Calendar"));
}

void ClockApplet::init()
{
    KConfigGroup cg = config();
    d->selectedTimezones = cg.readEntry("timeZones", QStringList());
    d->timezone = cg.readEntry("timezone", d->timezone);
    if (d->timezone == "UTC")  {
        d->prettyTimezone = d->timezone;
    } else {
        if (!isLocalTimezone()) {
            QStringList tzParts = d->timezone.split("/");
            d->prettyTimezone = tzParts.value(1);
       } else {
           d->prettyTimezone = localTimezone();
       }
    }

    //avoid duplication
    if (!extender()->item("calendar")) {
        Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(extender());
        eItem->setName("calendar");
        initExtenderItem(eItem);
    }

    Plasma::ToolTipManager::self()->registerWidget(this);
}

void ClockApplet::setCurrentTimezone(const QString &tz)
{
    if (d->timezone == tz) {
        return;
    }

    d->timezone = tz;
    if (tz == "UTC") {
        d->prettyTimezone = tz;
    } else {
        QStringList tzParts = tz.split("/");
        d->prettyTimezone = tzParts.value(1);
    }
    KConfigGroup cg = config();
    cg.writeEntry("timezone", tz);
    emit configNeedsSaving();
}

QString ClockApplet::currentTimezone() const
{
    return d->timezone;
}

QString ClockApplet::prettyTimezone() const
{
    return d->prettyTimezone;
}

QStringList ClockApplet::getSelectedTimezones() const
{
    return d->selectedTimezones;
}

bool ClockApplet::isLocalTimezone() const
{
    return d->timezone == localTimezone();
}

QString ClockApplet::localTimezone()
{
    return "Local";
}

#include "clockapplet.moc"
