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

#ifndef EVENTDATADECORATOR_H
#define EVENTDATADECORATOR_H

#include <QObject>
#include <QDateTime>
#include <QString>

#include <CalendarEvents/CalendarEventsPlugin>

class EventDataDecorator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDateTime startDateTime READ startDateTime NOTIFY eventDataChanged)
    Q_PROPERTY(QDateTime endDateTime READ endDateTime NOTIFY eventDataChanged)
    Q_PROPERTY(bool isAllDay READ isAllDay NOTIFY eventDataChanged)
    Q_PROPERTY(bool isMinor READ isMinor NOTIFY eventDataChanged)
    Q_PROPERTY(QString title READ title NOTIFY eventDataChanged)
    Q_PROPERTY(QString description READ description NOTIFY eventDataChanged)
    Q_PROPERTY(QString eventColor READ eventColor NOTIFY eventDataChanged)
    Q_PROPERTY(QString eventType READ eventType NOTIFY eventDataChanged)

public:
    EventDataDecorator(const CalendarEvents::EventData &data, QObject *parent = nullptr);

    QDateTime startDateTime() const;
    QDateTime endDateTime() const;
    bool isAllDay() const;
    bool isMinor() const;
    QString title() const;
    QString description() const;
    QString eventType() const;
    QString eventColor() const;

Q_SIGNALS:
    void eventDataChanged();

private:
    CalendarEvents::EventData m_data;
};

#endif
