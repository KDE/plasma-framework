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

#include "calendarplugin.h"
#include "calendardata.h"
#include "calendar.h"
#include <QtQml>
#include <QAbstractItemModel>
#include <QAbstractListModel>

void CalendarPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.calendar"));
    qmlRegisterType<CalendarData>(uri, 2, 1, "CalendarData");
    qmlRegisterType<Calendar>(uri, 2, 1, "Calendar");
    qmlRegisterType<QAbstractItemModel>();
    qmlRegisterType<QAbstractListModel>();
}

//Q_EXPORT_PLUGIN2(calendarplugin, CalendarPlugin)
#include "moc_calendarplugin.cpp"
