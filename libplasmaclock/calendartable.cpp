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

#include "calendartable.h"

//Qt
#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QStyleOptionGraphicsItem>

//KDECore
#include <kglobal.h>
#include <kdebug.h>

//Plasma
#include <plasma/svg.h>
#include <plasma/theme.h>

namespace Plasma
{

class CalendarCellBorder
{
public:
    CalendarCellBorder(int c, int w, int d, CalendarTable::CellTypes t, QDate dt)
        : cell(c),
          week(w),
          weekDay(d),
          type(t),
          date(dt)
    {
    }

    int cell;
    int week;
    int weekDay;
    CalendarTable::CellTypes type;
    QDate date;
};

class CalendarTablePrivate
{
    public:
        CalendarTablePrivate(CalendarTable *q, const QDate &cDate = QDate::currentDate())
        {
            svg = new Svg();
            svg->setImagePath("widgets/calendar");
            svg->setContainsMultipleImages(true);

            calendar = KGlobal::locale()->calendar();

            date = cDate;

            QDate currentDate = QDate::currentDate();
            month = calendar->month(currentDate);
            year = calendar->year(currentDate);

            setupThemedElements();
            QObject::connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
                             q, SLOT(setupThemedElements()));
        }

        ~CalendarTablePrivate()
        {
            delete svg;
        }

        void setupThemedElements()
        {
            QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::HighlightColor);
            c.setAlpha(180);
            highlightBrush = QBrush(c);
        }

        int firstMonthDayIndex(int y, int m)
        {
            QDate myDate;
            calendar->setYMD(myDate, y, m, 1);

            return (((calendar->dayOfWeek(myDate) - 1) + (calendar->daysInWeek(date) - (calendar->weekStartDay() - 1))) % calendar->daysInWeek(date)) + 1;
        }

        QRectF hoveredCellRect(CalendarTable *q, const QPointF &hoverPoint)
        {
            hoverDay = -1;
            hoverWeek = -1;

            if (hoverPoint.isNull()) {
                return QRectF();
            }

            if (hoverPoint.x() < centeringSpace + cellW + weekBarSpace) {
                // skip the weekbar
                return QRectF();
            }

            int x = (hoverPoint.x() - centeringSpace) / (cellW + cellSpace);
            int y = (hoverPoint.y() - headerHeight - headerSpace) / (cellH + cellSpace);

            if (x < 1 || x > 7 || y < 0 || y > 5) {
                return QRectF();
            }

            //FIXME: this should be a hint or something somewhere
            hoverDay = x - 1;
            hoverWeek = y;
            //kDebug () << x << y;
            return QRectF(q->cellX(x - 1) - glowRadius, q->cellY(y) - glowRadius,
                          cellW + glowRadius * 2, cellH + glowRadius * 2);
        }

        void updateHoveredPainting(CalendarTable *q, const QPointF &hoverPoint)
        {
            QRectF newHoverRect = hoveredCellRect(q, hoverPoint);

            // now update what is needed, and only what is needed!
            if (newHoverRect.isValid() && newHoverRect != hoverRect) {
                if (hoverRect.isValid()) {
                    q->update(hoverRect);
                }
                q->update(newHoverRect);
            }

            hoverRect = newHoverRect;
        }

        int cell(int week, int weekDay, CalendarTable::CellTypes *type, QDate &cellDate)
        {
            QDate myDate;

            if ((week == 0) && (weekDay < firstMonthDayIndex(year, month))){
                int prevMonth = (month == 1) ? 12 : month - 1;
                calendar->setYMD(myDate, year, prevMonth, 1);

                if (type) {
                    (*type) |= CalendarTable::NotInCurrentMonth;
                }

                calendar->setYMD(cellDate, (prevMonth == 12) ? year - 1 : year, prevMonth, calendar->daysInMonth(myDate) - (firstMonthDayIndex(year, month) - 1 - weekDay));
                return calendar->daysInMonth(myDate) - (firstMonthDayIndex(year, month) - 1 - weekDay);
            } else {
                calendar->setYMD(myDate, year, month, 1);
                int day = (week * calendar->daysInWeek(date) + weekDay) - firstMonthDayIndex(year, month) + 1;

                if (day <= calendar->daysInMonth(myDate)) {
                    if (type) {
                        (*type) &= ~CalendarTable::NotInCurrentMonth;
                    }

                    calendar->setYMD(cellDate, year, month, day);
                    return day;
                } else {
                    if (type) {
                        (*type) |= CalendarTable::NotInCurrentMonth;
                    }
                    int nextMonth = (month == 12) ? 1 : month + 1;
                    calendar->setYMD(cellDate, (nextMonth == 1) ? year + 1 : year, nextMonth, day - calendar->daysInMonth(myDate));
                    return day - calendar->daysInMonth(myDate);
                }
            }
        }

        Plasma::Svg *svg;
        const KCalendarSystem *calendar;
        QBrush highlightBrush;
        QDate date;
        QRectF hoverRect;
        int month;
        int year;

        int hoverWeek;
        int hoverDay;
        int centeringSpace;
        int cellW;
        int cellH;
        int cellSpace;
        int headerHeight;
        int headerSpace;
        int weekBarSpace;
        int glowRadius;
};

CalendarTable::CalendarTable(const QDate &date, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarTablePrivate(this, date))
{
}

CalendarTable::CalendarTable(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarTablePrivate(this))
{
    setAcceptHoverEvents(true);
}

CalendarTable::~CalendarTable()
{
    delete d;
}

const KCalendarSystem *CalendarTable::calendar() const
{
    return d->calendar;
}

bool CalendarTable::setCalendar(KCalendarSystem *calendar)
{
    d->calendar = calendar;
    return false;
}

bool CalendarTable::setDate(const QDate &date)
{
    int oldYear = d->year;
    int oldMonth = d->month;
    QDate oldDate = d->date;
    d->date = date;
    d->year = d->calendar->year(date);
    d->month = d->calendar->month(date);
    bool fullUpdate = false;

    if (oldYear != d->year){
        emit displayedYearChanged(d->year, d->month);
        fullUpdate = true;
    }

    if (oldMonth != d->month){
        emit displayedMonthChanged(d->year, d->month);
        fullUpdate = true;
    }

    d->updateHoveredPainting(this, QPointF());

    if (fullUpdate) {
        update();
    } else {
        // only update the old and the new areas
        int offset = d->firstMonthDayIndex(d->year, d->month);
        int daysInWeek = d->calendar->daysInWeek(d->date);

        int day = d->calendar->day(oldDate);
        int x = ((offset + day - 1) % daysInWeek);
        if (x == 0) {
            x = daysInWeek;
        }
        int y = (offset + day - 2) / daysInWeek;
        update(cellX(x - 1) - d->glowRadius, cellY(y) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);

        day = d->calendar->day(date);
        x = (offset + day - 1) % daysInWeek;
        if (x == 0) {
            x = daysInWeek;
        }
        y = (offset + day - 2) / daysInWeek;
        update(cellX(x - 1) - d->glowRadius, cellY(y) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);
    }

    return false;
}

const QDate& CalendarTable::date() const
{
    return d->date;
}

int CalendarTable::cellX(int weekDay)
{
    return boundingRect().x() + d->centeringSpace +
           d->weekBarSpace + d->cellW +
           ((d->cellW + d->cellSpace) * (weekDay));
}

int CalendarTable::cellY(int week)
{
    return (int) boundingRect().y() + (d->cellW + d->cellSpace) * (week) + d->headerHeight + d->headerSpace;
}

void CalendarTable::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    Q_UNUSED(event);

    if (event->delta() < 0) {
        if (d->month == 12) {
            d->month = 1;
            d->year++;
            emit displayedYearChanged(d->year, d->month);
        } else {
            d->month++;
        }

        emit displayedMonthChanged(d->year, d->month);
    } else if (event->delta() > 0) {
        if (d->month == 1) {
            d->month = 12;
            d->year--;
            emit displayedYearChanged(d->year, d->month); 
        } else {
            d->month--;
        }

        emit displayedMonthChanged(d->year, d->month);
    }

    update();
}

void CalendarTable::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();

    if ((event->pos().x() >= cellX(0)) && (event->pos().x() <= cellX(d->calendar->daysInWeek(d->date)) - d->cellSpace) &&
        (event->pos().y() >= cellY(0)) && (event->pos().y() <= cellY(5) - d->cellSpace)){

        int week = -1;
        int weekDay = -1;
        QDate cellDate;

        for (int i = 0; i < d->calendar->daysInWeek(d->date); i++) {
            if ((event->pos().x() >= cellX(i)) && (event->pos().x() <= cellX(i + 1) - d->cellSpace))
                weekDay = i;
        }

        for (int i = 0; i < 5; i++) {
            if ((event->pos().y() >= cellY(i)) && (event->pos().y() <= cellY(i + 1) - d->cellSpace))
                week = i;
        }

        if ((week >= 0) && (weekDay >= 0)) {
            d->hoverDay = -1;
            d->hoverWeek = -1;
            QDate tmpDate;
            QDate oldDate = d->date;
            d->cell(week, weekDay + 1, 0, tmpDate);

            if (tmpDate == oldDate) {
                return;
            }

            setDate(tmpDate);
            emit dateChanged(tmpDate, oldDate);
            emit dateChanged(tmpDate);
        }
    }
}

void CalendarTable::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mousePressEvent(event);
}

void CalendarTable::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    emit tableClicked();
}

void CalendarTable::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    d->updateHoveredPainting(this, event->pos());
}

void CalendarTable::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    Q_UNUSED(event);

    QRectF r = contentsRect();
    int rectSize = int(qMin(r.width() / 8, r.height() / 6));

    //Using integers to help to keep things aligned to the grid
    //kDebug() << r.width() << rectSize;
    d->cellSpace = qMax(1, qMin(4, rectSize / 20));
    d->headerSpace = d->cellSpace * 2;
    d->weekBarSpace = d->cellSpace * 2 + 1;
    d->cellH = rectSize - d->cellSpace;
    d->cellW = rectSize - d->cellSpace;
    d->glowRadius = d->cellW * .1;
    d->headerHeight = (int) (d->cellH / 1.5); 
    d->centeringSpace = qMax(0, int((r.width() - (rectSize * 8) - (d->cellSpace * 7)) / 2));
}

void CalendarTable::paintCell(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate)
{
    QString cellSuffix = cellSVGSuffix(cell, week, weekDay, type & NotInCurrentMonth, cellDate);
    d->svg->paint(p, QRectF(cellX(weekDay), cellY(week), d->cellW, d->cellH), cellSuffix);
}

void CalendarTable::paintBorder(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(cell);
    Q_UNUSED(cellDate);

    if (type & Hovered) {
        p->fillRect(QRect(cellX(weekDay), cellY(week), d->cellW, d->cellH), d->highlightBrush);
    }

    QString elementId;

    if (type & Today) {
        elementId = "today";
    } else if (type & Selected) {
        elementId = "selected";
    } else {
        return;
    }

    d->svg->paint(p, QRectF(cellX(weekDay) - 1, cellY(week) - 1,
                            d->cellW + 1, d->cellH + 2),
                  elementId);
}

QString CalendarTable::cellSVGSuffix(int cell, int week, int weekDay, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(week);
    Q_UNUSED(weekDay);
    Q_UNUSED(cellDate);
    return QString::number(cell) + (type & NotInCurrentMonth ? "-grayed" : "");
}

void CalendarTable::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    int daysInWeek = d->calendar->daysInWeek(d->date);

    // Draw weeks numbers column and day header
    QRectF r = boundingRect();
    d->svg->paint(p, QRectF(r.x() + d->centeringSpace, cellY(0), d->cellW,
                  cellY(5) - cellY(0) - d->cellSpace),  "weeksColumn");
    d->svg->paint(p, QRectF(r.x() + d->centeringSpace, r.y(),
                  cellX(daysInWeek) - r.x() - d->cellSpace - d->centeringSpace, d->headerHeight), "weekDayHeader");

    QList<CalendarCellBorder> borders;
    QList<CalendarCellBorder> hovers;
    QDate currentDate = QDate::currentDate(); //FIXME: calendar timezone

    //kDebug() << "exposed: " << option->exposedRect;
    for (int week = 0; week < 5; week++) {
        for (int weekDay = 0; weekDay < daysInWeek; weekDay++) {

            int x = cellX(weekDay);
            int y = cellY(week);

            QRectF cellRect(x, y, d->cellW, d->cellH);
            if (!cellRect.intersects(option->exposedRect)) {
                continue;
            }

            QDate cellDate(d->date.year(), d->date.month(), (week * 7) + (weekDay + 1));
            CalendarTable::CellTypes type(CalendarTable::NoType);
            // get cell info
            int cell = d->cell(week, weekDay + 1, &type, cellDate);

            // check what kind of cell we are
            if (cellDate == currentDate) {
                type |= Today;
            }

            if (cellDate == d->date) {
                type |= Selected;
            }

            if (type != CalendarTable::NoType && type != CalendarTable::NotInCurrentMonth) {
                borders.append(CalendarCellBorder(cell, week, weekDay, type, cellDate));
            }

            if (week == d->hoverWeek && weekDay == d->hoverDay) {
                type |= Hovered;
                hovers.append(CalendarCellBorder(cell, week, weekDay, type, cellDate));
            }

            paintCell(p, cell, week, weekDay, type, cellDate);

            if (weekDay == 0) {
                QRectF cellRect(r.x() + d->centeringSpace, y, d->cellW, d->cellH);
                d->svg->paint(p, cellRect, "week" + QString::number(d->calendar->weekNumber(cellDate)));
            }
        }
    } 

    // Draw days
    if (option->exposedRect.intersects(QRect(r.x(), r.y(), r.width(), d->headerHeight))) {
        p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        int weekStartDay = d->calendar->weekStartDay();
        for (int i = 0; i < daysInWeek; i++){
            int weekDay = ((i + weekStartDay - 1) % daysInWeek) + 1;
            QString dayName = d->calendar->weekDayName(weekDay, KCalendarSystem::ShortDayName);
            p->drawText(QRectF(cellX(i), r.y(), d->cellW, d->headerHeight), Qt::AlignCenter, dayName);
        }
    }

    // Draw hovers
    foreach (const CalendarCellBorder &border, hovers) {
        p->save();
        paintBorder(p, border.cell, border.week, border.weekDay, border.type, border.date);
        p->restore();
    }

    // Draw borders
    foreach (const CalendarCellBorder &border, borders) {
        p->save();
        paintBorder(p, border.cell, border.week, border.weekDay, border.type, border.date);
        p->restore();
    }

    /*
    p->save();
    p->setPen(Qt::red);
    p->drawRect(option->exposedRect.adjusted(1, 1, -2, -2));
    p->restore();
    */
}

} //namespace Plasma

#include "calendartable.moc"
