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

#ifndef CALENDAREVENTSPLUGIN_H
#define CALENDAREVENTSPLUGIN_H

#include <QObject>
#include <QDateTime>
#include <QMultiHash>
#include <QSharedDataPointer>

#include "plasmacalendarintegration_export.h"

namespace Plasma {

class PLASMACALENDARINTEGRATION_EXPORT EventData
{
public:
    enum EventType {
        Holiday,    // Any holiday
        Event,      // General event
        Todo        // A Todo item
    };

    EventData();
    EventData(const EventData &other);
    ~EventData();

    EventData &operator=(const EventData &other);

    /**
     * The start date and time of this event
     */
    QDateTime startDateTime() const;

    /**
     * Set the start date-time of this event
     *
     * @param startDateTime the date-time of when the event is starting
     */
    void setStartDateTime(const QDateTime &startDateTime);

    /**
     * The end date and time of this event
     */
    QDateTime endDateTime() const;

    /**
     * Set the end date-time of this event
     *
     * @param endDateTime the date-time of when the event is ending
     */
    void setEndDateTime(const QDateTime &endDateTime);

    /**
     * If true, this event goes on the whole day (eg. a holiday)
     */
    bool isAllDay() const;

    /**
     * If set to true, it will be displayed in the Calendar agenda
     * without any time besides it, marked as "going on all day"
     *
     * This is useful for single-day events only, for multiple-day
     * events, leave to false (default)
     *
     * @param isAllDay set to true if the event takes all day, false otherwise
     *                 (defaults to false)
     */
    void setIsAllDay(bool isAllDay);

    /**
     * If true, this event won't mark the day in the calendar grid
     * The main purpose for this flag is to support
     * namedays, where in some countries the calendars have
     * different name in them every day. This is just a minor holiday
     * and as such should not mark the calendar grid, otherwise
     * the whole grid would be in a different color.
     */
    bool isMinor() const;

    /**
     * If set to true, it won't be marked in the calendar grid
     *
     * @param isMinor true if it's a minor event (like a nameday holiday),
     *                false otherwise (defaults to false)
     */
    void setIsMinor(bool isMinor);

    /**
     * Event title
     */
    QString title() const;

    /**
     * Sets the title of the event
     *
     * @param title The event title
     */
    void setTitle(const QString &title);

    /**
     * Event description, can provide more details about the event
     */
    QString description() const;

    /**
     * Sets the event description, which allows to add more details
     * about this event
     *
     * @param description The description
     */
    void setDescription(const QString &description);

    /**
     * Type of the current event, eg. a holiday, an event or a todo item
     */
    EventType type() const;

    /**
     * Sets the event type, eg. a holiday, an event or a todo item
     *
     * @param type The event type,
     */
    void setEventType(EventType type);

    /**
     * The color that should be used to mark this event with
     * It comes in the HTML hex format, eg. #AARRGGBB or #RRGGBB
     */
    QString eventColor() const;

    /**
     * This is to support various calendar colors the user might
     * have configured elsewhere
     *
     * @param color The color for this event in the HTML hex format
     *              eg. #AARRGGBB or #RRGGBB (this is passed directly
     *              to QML)
     */
    void setEventColor(const QString &color);

    /**
     * Unique ID of the event
     */
    QString uid() const;

    /**
     * Sets the uid of the event
     *
     * This is a mandatory field only if you want to use
     * the eventModified/eventRemoved signals, otherwise
     * setting it is optional
     *
     * @param uid A unique id, recommended is to use the plugin name as prefix (to keep it unique)
     */
    void setUid(const QString &uid);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

class PLASMACALENDARINTEGRATION_EXPORT CalendarEventsPlugin : public QObject
{
    Q_OBJECT

public:
    explicit CalendarEventsPlugin(QObject *parent = Q_NULLPTR);
    virtual ~CalendarEventsPlugin();

    /**
     * When this is called, the plugin should load all events data
     * between those two date ranges. Once the data are ready, it should
     * just emit the dataReady() signal. The range is usually one month
     *
     * @param startDate the start of the range
     * @param endDate the end of the range
     */
    virtual void loadEventsForDateRange(const QDate &startDate, const QDate &endDate) = 0;

Q_SIGNALS:
    /**
     * Emitted when the plugin has loaded the events data
     *
     * @param data A hash containing a QDate key for the event
     *             in the value, Plasma::EventData, which holds all
     *             the details for the given event
     *             It's a multihash as there can be multiple events
     *             in the same day
     *             For multi-day events, insert just one with the key
     *             being the startdate of the event
     */
    void dataReady(const QMultiHash<QDate, Plasma::EventData> &data);

    /**
     * Should be emitted when there is a modification of an event
     * that was previously returned via the dataReady() signal
     *
     * @param event The modified event data
     */
    void eventModified(const Plasma::EventData &modifiedEvent);

    /**
     * Should be emitted when the plugin removes some event
     * from its collection
     *
     * @param uid The uid of the event that was removed
     */
    void eventRemoved(const QString &uid);
};

}

Q_DECLARE_INTERFACE(Plasma::CalendarEventsPlugin, "org.kde.plasma.CalendarEventsPlugin")

#endif
