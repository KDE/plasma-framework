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

#ifndef DAYSMODEL_H
#define DAYSMODEL_H

#include <QAbstractListModel>
#include "daydata.h"

class DaysModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        isPreviousMonth = Qt::UserRole + 1,
        isCurrentMonth,
        isNextMonth,
        //containsHolidayItems,
        //containsEventItems,
        //containsTodoItems,
        //containsJournalItems,
        dayNumber,
        monthNumber,
        yearNumber
    };

    explicit DaysModel(QObject *parent = 0);
    void setSourceData(QList<DayData>* data);
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    void update();

private:
    QList<DayData> *m_data;
};

#endif // DAYSMODEL_H
