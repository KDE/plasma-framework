/*
 *   Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "calendar.h"

//Qt
#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsProxyWidget>

//KDECore
#include <kglobal.h>
#include <kdebug.h>

//Plasma
#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/widgets/label.h>

#include "toolbutton.h"

namespace Plasma
{

class CalendarPrivate
{
    public:
        ToolButton *back;
        Plasma::Label *spacer0;
        Plasma::Label *month;
        #ifdef COOL_SPINBOX
            SpinBox *year;
        #else
            Plasma::Label *year;
        #endif
        Plasma::Label *spacer1;
        ToolButton *forward;
        Plasma::CalendarTable *calendarTable;
};

//TODO
Calendar::Calendar(const QDate &, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{

}

Calendar::Calendar(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarPrivate())
{
    QGraphicsLinearLayout *m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    QGraphicsLinearLayout *m_hLayout = new QGraphicsLinearLayout(m_layout);

    d->calendarTable = new Plasma::CalendarTable(this);
    d->calendarTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(d->calendarTable, SIGNAL(displayedMonthChanged(int, int)), this, SLOT(displayedMonthChanged(int, int)));

    QGraphicsProxyWidget *backProxy = new QGraphicsProxyWidget(this);
    d->back = new ToolButton();
    d->back->setText("<");
    d->back->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->back, SIGNAL(clicked()), this, SLOT(prevMonth()));
    backProxy->setWidget(d->back);
    m_hLayout->addItem(backProxy);

    d->spacer0 = new Plasma::Label(this);
    d->spacer0->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_hLayout->addItem(d->spacer0);

    d->month = new Plasma::Label(this);
    d->month->setText(d->calendarTable->calendar()->monthName(d->calendarTable->calendar()->month(d->calendarTable->date()), d->calendarTable->calendar()->year(d->calendarTable->date())));
    d->month->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_hLayout->addItem(d->month);

    #ifdef COOL_SPINBOX
        QGraphicsProxyWidget *yearProxy = new QGraphicsProxyWidget(this);
        d->year = new SpinBox();
        d->year->setRange(d->calendarTable->calendar()->year(d->calendarTable->calendar()->earliestValidDate()), d->calendarTable->calendar()->year(d->calendarTable->calendar()->latestValidDate()));
        d->year->setValue(d->calendarTable->calendar()->year(d->calendarTable->date()));
        yearProxy->setWidget(d->year);
        m_hLayout->addItem(yearProxy);
    #else
        d->year = new Plasma::Label;
        d->year->setText(QString::number(d->calendarTable->calendar()->year(d->calendarTable->date())));
        d->year->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_hLayout->addItem(d->year);
    #endif

    d->spacer1 = new Plasma::Label(this);
    d->spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_hLayout->addItem(d->spacer1);

    QGraphicsProxyWidget *forwardProxy = new QGraphicsProxyWidget(this);
    d->forward = new ToolButton();
    d->forward->setText(">");
    d->forward->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(d->forward, SIGNAL(clicked()), this, SLOT(nextMonth()));
    forwardProxy->setWidget(d->forward);
    m_hLayout->addItem(forwardProxy);

    m_layout->addItem(m_hLayout);

    m_layout->addItem(d->calendarTable);
}

Calendar::~Calendar()
{
   delete d;
}

bool Calendar::setCalendar(KCalendarSystem *calendar)
{
    return d->calendarTable->setCalendar(calendar);
}

bool Calendar::setDate(const QDate &date)
{
    return d->calendarTable->setDate(date);
}

const QDate& Calendar::date() const
{
    return d->calendarTable->date();
}

void Calendar::displayedMonthChanged(int calendarSystemYear, int calendarSystemMonth)
{
    d->month->setText(d->calendarTable->calendar()->monthName(calendarSystemMonth, calendarSystemYear));
    
    #ifdef COOL_SPINBOX
        d->year->setValue(calendarSystemYear);
    #else
        d->year->setText(QString::number(calendarSystemYear));
    #endif
}

void Calendar::prevMonth()
{
    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int month = d->calendarTable->calendar()->month(tmpDate);
    int year = d->calendarTable->calendar()->year(tmpDate);

    if (month == 1){
        month = 12;
        year--;
    }else{
        month--;
    }

    if (d->calendarTable->calendar()->setYMD(newDate, year, month, d->calendarTable->calendar()->day(tmpDate))){
        d->calendarTable->setDate(newDate);
    }else if (d->calendarTable->calendar()->setYMD(newDate, year, month, 1)){
        d->calendarTable->setDate(newDate);
    }
}

void Calendar::nextMonth()
{
    QDate tmpDate = d->calendarTable->date();
    QDate newDate;

    int month = d->calendarTable->calendar()->month(tmpDate);
    int year = d->calendarTable->calendar()->year(tmpDate);

    if (month == 12){
        month = 1;
        year++;
    }else{
        month++;
    }

    if (d->calendarTable->calendar()->setYMD(newDate, year, month, d->calendarTable->calendar()->day(tmpDate))){
        d->calendarTable->setDate(newDate);
    }else if (d->calendarTable->calendar()->setYMD(newDate, year, month, 1)){
        d->calendarTable->setDate(newDate);
    }
}

CalendarTable *Calendar::calendarTable() const
{
    return d->calendarTable;
}
}

#include "calendar.moc"
