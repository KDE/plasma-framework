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

    Q_PROPERTY(int daysBeforeCurrent READ daysBeforeCurrent NOTIFY daysBeforeCurrentChanged)
    Q_PROPERTY(int daysAfterCurrent READ daysAfterCurrent NOTIFY daysAfterCurrentChanged)

public:
    enum Roles {
        isCurrent = Qt::UserRole + 1,
        //containsHolidayItems,
        //containsEventItems,
        //containsTodoItems,
        //containsJournalItems,
        dayNumber,
        monthNumber,
        yearNumber
    };

    explicit DaysModel(QObject *parent = 0);
    void setSourceData(QList<DayData> *data);
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    void update();

    int daysBeforeCurrent() const {
        return m_daysBeforeCurrent;
    }
    int daysAfterCurrent() const {
        return m_daysAfterCurrent;
    }

    void setDaysBeforeCurrent(int daysBeforeCurrent);
    void setDaysAfterCurrent(int daysAfterCurrent);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

Q_SIGNALS:
    void daysBeforeCurrentChanged();
    void daysAfterCurrentChanged();

private:
    QList<DayData> *m_data;
    int m_daysBeforeCurrent;
    int m_daysAfterCurrent;
};

#endif // DAYSMODEL_H
