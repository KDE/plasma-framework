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

#ifndef EVENTPLUGINSMANAGER_H
#define EVENTPLUGINSMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>

namespace CalendarEvents {
class CalendarEventsPlugin;
class EventData;
}
class EventPluginsModel;
class QAbstractListModel;

class EventPluginsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractListModel *model READ pluginsModel NOTIFY pluginsChanged)
    Q_PROPERTY(QStringList enabledPlugins READ enabledPlugins WRITE setEnabledPlugins NOTIFY pluginsChanged)

public:
    EventPluginsManager(QObject *parent = 0);
    ~EventPluginsManager();

    QList<CalendarEvents::CalendarEventsPlugin*> plugins() const;
    QAbstractListModel* pluginsModel() const;

    // This is a helper function to set which plugins
    // are enabled without needing to go through setEnabledPlugins
    // which also loads the plugins; from the Applet config
    // the plugins are not required to be actually loaded
    Q_INVOKABLE void populateEnabledPluginsList(const QStringList &pluginsList);

    void setEnabledPlugins(QStringList &pluginsList);
    QStringList enabledPlugins() const;

Q_SIGNALS:
    void pluginsChanged();

    // These three signals below are used for relaying the
    // plugin signals so that the EventPluginsManager don't
    // have to worry about connecting to newly loaded plugins
    void dataReady(const QMultiHash<QDate, CalendarEvents::EventData> &data);
    void eventModified(const CalendarEvents::EventData &modifiedEvent);
    void eventRemoved(const QString &uid);

private:
    void loadPlugin(const QString &absolutePath);

    friend class EventPluginsModel;
    EventPluginsModel *m_model;
    QList<CalendarEvents::CalendarEventsPlugin*> m_plugins;
    struct PluginData {
        QString name;
        QString desc;
        QString icon;
        QString configUi;
    };
    QMap<QString, PluginData> m_availablePlugins;
    QStringList m_enabledPlugins;
};

#endif

