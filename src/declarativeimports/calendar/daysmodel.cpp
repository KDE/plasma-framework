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

#include "daysmodel.h"
#include <QDebug>
#include <QByteArray>

DaysModel::DaysModel(QObject *parent) :
    QAbstractListModel(parent),
    m_daysBeforeCurrent(0),
    m_daysAfterCurrent(0)
{

}

void DaysModel::setSourceData(QList<DayData> *data)
{
    if (m_data != data) {
        m_data = data;
        reset();
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

        DayData currentData = m_data->at(index.row());

        switch (role) {
        case isCurrent:
            return currentData.isCurrent;
        //      case containsHolidayItems:
        //          return currentData.containsHolidayItems;
        /* case containsEventItems:
             return currentData.containsEventItems;
         case containsTodoItems:
             return currentData.containsTodoItems;
         case containsJournalItems:
             return currentData.containsJournalItems;*/
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

    // We always have 42 items (or weeks * num of days in week) so we only have to tell the view that the data changed.
    layoutChanged();
}

void DaysModel::setDaysBeforeCurrent(int daysBeforeCurrent)
{
    qDebug() << "SET DAYS AFTER CURRENT";
    if (m_daysBeforeCurrent != daysBeforeCurrent) {
        m_daysBeforeCurrent = daysBeforeCurrent;
        emit daysBeforeCurrentChanged();
    }
}

void DaysModel::setDaysAfterCurrent(int daysAfterCurrent)
{
    if (m_daysBeforeCurrent != daysAfterCurrent) {
        m_daysBeforeCurrent = daysAfterCurrent;
        emit daysAfterCurrentChanged();
    }
}

QHash<int, QByteArray> DaysModel::roleNames() const
{
    // TODO use initializer list once we're allowed to
    QHash<int, QByteArray> roleNames;

    roleNames.insert(isCurrent, "isCurrent");
    //roleNames.insert(containsHolidayItems,   "containsHolidayItems");
    //roleNames.insert(containsEventItems,     "containsEventItems");
    // roleNames.insert(containsTodoItems,      "containsTodoItems");
    // roleNames.insert(containsJournalItems,   "containsJournalItems");
    roleNames.insert(dayNumber,              "dayNumber");
    roleNames.insert(monthNumber,            "monthNumber");
    roleNames.insert(yearNumber,             "yearNumber");

    return roleNames;
}
