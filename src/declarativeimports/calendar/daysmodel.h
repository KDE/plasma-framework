/*
    Copyright (C) 2013 Mark Gaiser <markg85@gmail.com>
    Copyright (C) 2016 Martin Klapetek <mklapetek@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef DAYSMODEL_H
#define DAYSMODEL_H

#include <QAbstractListModel>

#include "daydata.h"
#include <CalendarEvents/CalendarEventsPlugin>

class EventPluginsManager;

class DaysModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        isCurrent = Qt::UserRole + 1,
        //containsHolidayItems,
        containsEventItems,
        containsMajorEventItems,
        containsMinorEventItems,
        //containsTodoItems,
        //containsJournalItems,
        dayNumber,
        monthNumber,
        yearNumber
    };

    explicit DaysModel(QObject *parent = nullptr);
    virtual ~DaysModel();
    void setSourceData(QList<DayData> *data);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void setPluginsManager(QObject *manager);

    Q_INVOKABLE QList<QObject*> eventsForDate(const QDate &date);

    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void agendaUpdated(const QDate &updatedDate);

public Q_SLOTS:
    void update();

private Q_SLOTS:
    void onDataReady(const QMultiHash<QDate, CalendarEvents::EventData> &data);
    void onEventModified(const CalendarEvents::EventData &data);
    void onEventRemoved(const QString &uid);

private:
    QModelIndex indexForDate(const QDate &date);
    bool hasMajorEventAtDate(const QDate &date) const;
    bool hasMinorEventAtDate(const QDate &date) const;

    EventPluginsManager *m_pluginsManager = nullptr;
    QList<DayData> *m_data = nullptr;
    QList<QObject*> m_qmlData;
    QDate m_lastRequestedAgendaDate;
    QList<CalendarEvents::CalendarEventsPlugin*> m_eventPlugins;
    QMultiHash<QDate, CalendarEvents::EventData> m_eventsData;
    QDate m_lastRequestedEventsStartDate; // this is always this+42 days
    bool m_agendaNeedsUpdate;
};

#endif // DAYSMODEL_H
