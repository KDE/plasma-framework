/*
 *   Copyright 2012 by Antonis Tsiapaliokas <kok3rs@gmail.com>
 *   Copyright 2012 by Giorgos Tsiapaliwkas <terietor@gmail.com>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "localebindingsplugin.h"
#include <QtDeclarative/qdeclarative.h>
#include "locale_p.h"
#include "calendarsystem.h"

void LocaleBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.locale"));

    qmlRegisterType<Locale>(uri, 0, 1, "Locale");
    qmlRegisterType<CalendarSystem>(uri, 0, 1, "CalendarSystem");
}

#include "localebindingsplugin.moc"
