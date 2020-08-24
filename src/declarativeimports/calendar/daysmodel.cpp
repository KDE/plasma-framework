/*
    SPDX-FileCopyrightText: 2013 Mark Gaiser <markg85@gmail.com>
    SPDX-FileCopyrightText: 2016 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "daysmodel.h"
#include "eventdatadecorator.h"
#include "eventpluginsmanager.h"

#include <QDebug>
#include <QByteArray>
#include <QDir>
#include <QMetaObject>

DaysModel::DaysModel(QObject *parent) :
    QAbstractListModel(parent),
    m_pluginsManager(nullptr),
    m_lastRequestedEventsStartDate(QDate()),
    m_agendaNeedsUpdate(false)
{

}

DaysModel::~DaysModel()
{
    qDeleteAll(m_eventPlugins);
}

void DaysModel::setSourceData(QList<DayData> *data)
{
    if (m_data != data) {
        beginResetModel();
        m_data = data;
        endResetModel();
    }
}

int DaysModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_data->size() <= 0) {
        return 0;
    } else {
        return m_data->size();
    }
}

QVariant DaysModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {

        const DayData &currentData = m_data->at(index.row());
        const QDate currentDate(currentData.yearNumber, currentData.monthNumber, currentData.dayNumber);

        switch (role) {
        case isCurrent:
            return currentData.isCurrent;
        case containsEventItems:
            return m_eventsData.contains(currentDate);
        case containsMajorEventItems:
            return hasMajorEventAtDate(currentDate);
        case containsMinorEventItems:
            return hasMinorEventAtDate(currentDate);
        case dayNumber:
            return currentData.dayNumber;
        case monthNumber:
            return currentData.monthNumber;
        case yearNumber:
            return currentData.yearNumber;
        }
    }
    return QVariant();
}

void DaysModel::update()
{
    if (m_data->size() <= 0) {
        return;
    }

    m_eventsData.clear();

    const QDate modelFirstDay(m_data->at(0).yearNumber, m_data->at(0).monthNumber, m_data->at(0).dayNumber);

    if (m_pluginsManager) {
        const auto plugins = m_pluginsManager->plugins();
        for (CalendarEvents::CalendarEventsPlugin *eventsPlugin : plugins) {
            eventsPlugin->loadEventsForDateRange(modelFirstDay, modelFirstDay.addDays(42));
        }
    }

    // We always have 42 items (or weeks * num of days in week) so we only have to tell the view that the data changed.
    emit dataChanged(index(0, 0), index(m_data->count() - 1, 0));
}

void DaysModel::onDataReady(const QMultiHash<QDate, CalendarEvents::EventData> &data)
{
    m_eventsData.reserve(m_eventsData.size() + data.size());
    m_eventsData += data;

    if (data.contains(QDate::currentDate())) {
        m_agendaNeedsUpdate = true;
    }

    // only the containsEventItems roles may have changed
    emit dataChanged(index(0, 0), index(m_data->count() - 1, 0),
                     {containsEventItems, containsMajorEventItems, containsMinorEventItems});

    Q_EMIT agendaUpdated(QDate::currentDate());
}

void DaysModel::onEventModified(const CalendarEvents::EventData &data)
{
    QList<QDate> updatesList;
    auto i = m_eventsData.begin();
    while (i != m_eventsData.end()) {
        if (i->uid() == data.uid()) {
            *i = data;
            updatesList << i.key();
        }

        ++i;
    }

    if (!updatesList.isEmpty()) {
        m_agendaNeedsUpdate = true;
    }

    for (const QDate date : qAsConst(updatesList)) {
        const QModelIndex changedIndex = indexForDate(date);
        if (changedIndex.isValid()) {
            Q_EMIT dataChanged(changedIndex, changedIndex,
                               {containsEventItems, containsMajorEventItems, containsMinorEventItems});
        }
        Q_EMIT agendaUpdated(date);
    }
}

void DaysModel::onEventRemoved(const QString &uid)
{
    QList<QDate> updatesList;
    auto i = m_eventsData.begin();
    while (i != m_eventsData.end()) {
        if (i->uid() == uid) {
            updatesList << i.key();
            i = m_eventsData.erase(i);
        } else {
            ++i;
        }
    }

    if (!updatesList.isEmpty()) {
        m_agendaNeedsUpdate = true;
    }

    for (const QDate date : qAsConst(updatesList)) {
        const QModelIndex changedIndex = indexForDate(date);
        if (changedIndex.isValid()) {
            Q_EMIT dataChanged(changedIndex, changedIndex,
                               {containsEventItems, containsMajorEventItems, containsMinorEventItems});
        }
        Q_EMIT agendaUpdated(date);
    }
}

QList<QObject*> DaysModel::eventsForDate(const QDate &date)
{
    if (m_lastRequestedAgendaDate == date && !m_agendaNeedsUpdate) {
        return m_qmlData;
    }

    m_lastRequestedAgendaDate = date;
    qDeleteAll(m_qmlData);
    m_qmlData.clear();

    QList<CalendarEvents::EventData> events = m_eventsData.values(date);
    m_qmlData.reserve(events.size());

    // sort events by their time and type
    std::sort(events.begin(), events.end(), [](const CalendarEvents::EventData &a, const CalendarEvents::EventData &b) {
        return b.type() > a.type() || b.startDateTime() > a.startDateTime();
    });

    for (const CalendarEvents::EventData &event : qAsConst(events)) {
        m_qmlData << new EventDataDecorator(event, this);
    }

    m_agendaNeedsUpdate = false;
    return m_qmlData;
}

QModelIndex DaysModel::indexForDate(const QDate &date)
{
    if (!m_data) {
        return QModelIndex();
    }

    const DayData &firstDay = m_data->at(0);
    const QDate firstDate(firstDay.yearNumber, firstDay.monthNumber, firstDay.dayNumber);

    qint64 daysTo = firstDate.daysTo(date);

    return createIndex(daysTo, 0);
}

bool DaysModel::hasMajorEventAtDate(const QDate &date) const
{
    auto it = m_eventsData.find(date);
    while (it != m_eventsData.end() && it.key() == date) {
        if (!it.value().isMinor()) {
            return true;
        }
        ++it;
    }
    return false;
}

bool DaysModel::hasMinorEventAtDate(const QDate &date) const
{
    auto it = m_eventsData.find(date);
    while (it != m_eventsData.end() && it.key() == date) {
        if (it.value().isMinor()) {
            return true;
        }
        ++it;
    }
    return false;
}

void DaysModel::setPluginsManager(QObject *manager)
{
    EventPluginsManager *m = qobject_cast<EventPluginsManager*>(manager);

    if (!m) {
        return;
    }

    if (m_pluginsManager) {
        m_pluginsManager->deleteLater();
        m_pluginsManager = nullptr;
    }

    m_pluginsManager = m;

    connect(m_pluginsManager, &EventPluginsManager::dataReady,
            this, &DaysModel::onDataReady);
    connect(m_pluginsManager, &EventPluginsManager::eventModified,
            this, &DaysModel::onEventModified);
    connect(m_pluginsManager, &EventPluginsManager::eventRemoved,
            this, &DaysModel::onEventRemoved);
    connect(m_pluginsManager, &EventPluginsManager::pluginsChanged,
            this, &DaysModel::update);

    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

QHash<int, QByteArray> DaysModel::roleNames() const
{
    return {
        {isCurrent, "isCurrent"},
        {containsEventItems, "containsEventItems"},
        {containsMajorEventItems, "containsMajorEventItems"},
        {containsMinorEventItems, "containsMinorEventItems"},
        {dayNumber, "dayNumber"},
        {monthNumber, "monthNumber"},
        {yearNumber, "yearNumber"}
    };
}
