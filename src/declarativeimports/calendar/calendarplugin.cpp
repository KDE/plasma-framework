/*
    SPDX-FileCopyrightText: 2013 Mark Gaiser <markg85@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "calendarplugin.h"
#include "calendardata.h"
#include "calendar.h"
#include "eventdatadecorator.h"
#include "eventpluginsmanager.h"

#include <QtQml>
#include <QQmlEngine>
#include <QAbstractListModel>

static QObject *event_plugins_manager_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new EventPluginsManager();
}

void CalendarPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.calendar"));
    qmlRegisterType<CalendarData>(uri, 2, 0, "CalendarData");
    qmlRegisterType<Calendar>(uri, 2, 0, "Calendar");
    qmlRegisterAnonymousType<QAbstractItemModel>(uri, 1);
    qmlRegisterAnonymousType<QAbstractListModel>(uri, 1);
    qmlRegisterSingletonType<EventPluginsManager>(uri, 2, 0, "EventPluginsManager", event_plugins_manager_provider);
    qmlRegisterUncreatableType<EventDataDecorator>(uri, 2, 0, "EventDataDecorator",
                                                   QStringLiteral("Unabel to create EventDataDecorator from QML"));
}
