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

#include "calendardata.h"



CalendarData::CalendarData(QObject *parent)
    : QObject(parent)
    , m_types(Holiday | Event | Todo | Journal)
{
 //   m_etmCalendar = new ETMCalendar();
 //   m_etmCalendar->setParent(this); //TODO: hit sergio

   // EntityTreeModel *model = m_etmCalendar->entityTreeModel();
   // model->setCollectionFetchStrategy(EntityTreeModel::InvisibleCollectionFetch);

  //  m_itemList = new EntityMimeTypeFilterModel(this);
  //  m_itemList->setSourceModel(model);

   // CalendarRoleProxyModel *roleModel = new CalendarRoleProxyModel(this);
  //  roleModel->setSourceModel(m_itemList);

  //  m_filteredList = new DateTimeRangeFilterModel(this);
  //  m_filteredList->setSourceModel(roleModel);

 //   updateTypes();
}



QDate CalendarData::startDate() const
{
    return m_startDate;
}

void CalendarData::setStartDate(const QDate &dateTime)
{
    if (m_startDate == dateTime)
        return;

    m_startDate = dateTime;
 //   m_filteredList->setStartDate(m_startDate);
    emit startDateChanged();
}

QDate CalendarData::endDate() const
{
    return m_endDate;
}

void CalendarData::setEndDate(const QDate &dateTime)
{
    if (m_endDate == dateTime)
        return;

    m_endDate = dateTime;
  //  m_filteredList->setEndDate(m_endDate);
    emit endDateChanged();
}

int CalendarData::types() const
{
    return m_types;
}

QString CalendarData::errorMessage() const
{
    return QString();
}

bool CalendarData::loading() const
{
    return false;
}

