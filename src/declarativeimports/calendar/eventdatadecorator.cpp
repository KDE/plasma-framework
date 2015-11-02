/*
    Copyright (C) 2015 Martin Klapetek <mklapetek@kde.org>

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

#include "eventdatadecorator.h"

#include <KLocalizedString>

EventDataDecorator::EventDataDecorator(const Plasma::EventData &data, QObject *parent)
    : QObject(parent),
      m_data(data)
{
}

QDateTime EventDataDecorator::startDateTime() const
{
    return m_data.startDateTime();
}

QDateTime EventDataDecorator::endDateTime() const
{
    return m_data.endDateTime();
}

bool EventDataDecorator::isAllDay() const
{
    return m_data.isAllDay();
}

bool EventDataDecorator::isMinor() const
{
    return m_data.isMinor();
}

QString EventDataDecorator::title() const
{
    return m_data.title();
}

QString EventDataDecorator::description() const
{
    return m_data.description();
}

QString EventDataDecorator::eventType() const
{
    switch (m_data.type()) {
        case Plasma::EventData::Holiday:
            return i18nc("Agenda listview section title", "Holidays");
        case Plasma::EventData::Event:
            return i18nc("Agenda listview section title", "Events");
        case Plasma::EventData::Todo:
            return i18nc("Agenda listview section title", "Todo");
    }
    return i18nc("Means 'Other calendar items'", "Other");
}

QString EventDataDecorator::eventColor() const
{
    return m_data.eventColor();
}
