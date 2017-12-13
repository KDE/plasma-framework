/*
    Copyright (C) 2013 Mark Gaiser <markg85@gmail.com>

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

#ifndef CALENDARDATA_H
#define CALENDARDATA_H

#include <QObject>
#include <QFlags>
#include <QDate>

class QAbstractItemModel;

class CalendarData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDate endDate READ endDate WRITE setEndDate NOTIFY endDateChanged)
    //  Q_PROPERTY(int types READ types WRITE setTypes NOTIFY typesChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    //  Q_PROPERTY(QAbstractItemModel* model READ model CONSTANT)

public:
    enum Type {
        Holiday = 1,
        Event = 2,
        Todo = 4,
        Journal = 8
    };
    Q_ENUM(Type)
    Q_DECLARE_FLAGS(Types, Type)

    explicit CalendarData(QObject *parent = nullptr);

Q_SIGNALS:
    void startDateChanged();
    void endDateChanged();
    void typesChanged();
    void errorMessageChanged();
    void loadingChanged();

private:
    QDate startDate() const;
    void setStartDate(const QDate &dateTime);
    QDate endDate() const;
    void setEndDate(const QDate &dateTime);
    int types() const;
    // void setTypes(int types);
    QString errorMessage() const;
    bool loading() const;
    //  QAbstractItemModel* model() const;

    // void updateTypes();

    QDate m_startDate;
    QDate m_endDate;
    Types m_types;

    // Akonadi::ETMCalendar *m_etmCalendar;
    // Akonadi::EntityMimeTypeFilterModel *m_itemList;
    // DateTimeRangeFilterModel *m_filteredList;
};

#endif // CALENDARDATA_H
