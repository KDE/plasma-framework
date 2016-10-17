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

#include "eventpluginsmanager.h"
#include "debug_p.h"

#include <CalendarEvents/CalendarEventsPlugin>

#include <QCoreApplication>
#include <QAbstractListModel>
#include <QJsonObject>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>

class EventPluginsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    EventPluginsModel(EventPluginsManager *manager) : QAbstractListModel(manager)
    {
        m_manager = manager;
        m_roles = QAbstractListModel::roleNames();
        m_roles.insert(Qt::EditRole, QByteArrayLiteral("checked"));
        m_roles.insert(Qt::UserRole, QByteArrayLiteral("configUi"));
    };

    // make these two available to the manager
    void beginResetModel()
    {
        QAbstractListModel::beginResetModel();
    }

    void endResetModel()
    {
        QAbstractListModel::endResetModel();
    }

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE
    {
        return m_roles;
    }

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE
    {
        Q_UNUSED(parent);
        return m_manager->m_availablePlugins.size();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE
    {
        if (!index.isValid() && !m_manager) {
            return QVariant();
        }

        const QString currentPlugin = m_manager->m_availablePlugins.keys().at(index.row());
        const QJsonObject metadata = m_manager->m_availablePlugins.value(currentPlugin).value(QStringLiteral("MetaData")).toObject();

        switch (role) {
            case Qt::DisplayRole:
                return metadata.value(QStringLiteral("Name"));
            case Qt::DecorationRole:
                return metadata.value(QStringLiteral("Icon"));
            case Qt::UserRole:
            {
                // The currentPlugin path contains the full path including
                // the plugin filename, so it needs to be cut off from the last '/'
                const QStringRef pathRef = currentPlugin.leftRef(currentPlugin.lastIndexOf('/'));
                const QString qmlFilePath = metadata.value(QStringLiteral("ConfigUi")).toString();
                return QString(pathRef % '/' % qmlFilePath);
            }
            case Qt::EditRole:
                return m_manager->m_enabledPlugins.contains(currentPlugin);
        }

        return QVariant();

    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE
    {
        if (role != Qt::EditRole || !index.isValid()) {
            return false;
        }

        bool enabled = value.toBool();
        const QString pluginPath = m_manager->m_availablePlugins.keys().at(index.row());

        if (enabled) {
            if (!m_manager->m_enabledPlugins.contains(pluginPath)) {
                m_manager->m_enabledPlugins << pluginPath;
            }
        } else {
            m_manager->m_enabledPlugins.removeOne(pluginPath);
        }

        emit dataChanged(index, index);

        return true;
    }

    Q_INVOKABLE QVariant get(int row, const QByteArray &role)
    {
        return data(createIndex(row, 0), roleNames().key(role));
    }

private:
    EventPluginsManager *m_manager;
    QHash<int, QByteArray> m_roles;
};

EventPluginsManager::EventPluginsManager(QObject *parent)
    : QObject(parent)
{
    // First of all get a list of available plugins
    // and get their metadata. This alone is enough
    // for the applet config to work
    const QStringList paths = QCoreApplication::libraryPaths();
    Q_FOREACH (const QString &libraryPath, paths) {
        const QString path(libraryPath + QStringLiteral("/plasmacalendarplugins"));
        QDir dir(path);

        if (!dir.exists()) {
            continue;
        }

        QStringList entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);

        Q_FOREACH (const QString &fileName, entryList) {
            const QString absolutePath = dir.absoluteFilePath(fileName);
            QPluginLoader loader(absolutePath);
            // Load only our own plugins
            if (loader.metaData().value(QStringLiteral("IID")) == QLatin1String("org.kde.CalendarEventsPlugin")) {
                m_availablePlugins.insert(absolutePath, loader.metaData());
            }
        }
    }

    m_model = new EventPluginsModel(this);
    Q_EMIT pluginsChanged();
}

EventPluginsManager::~EventPluginsManager()
{
    qDeleteAll(m_plugins);
}

void EventPluginsManager::populateEnabledPluginsList(const QStringList &pluginsList)
{
    m_model->beginResetModel();
    m_enabledPlugins = pluginsList;
    m_model->endResetModel();
}

void EventPluginsManager::setEnabledPlugins(QStringList &pluginsList)
{
    m_model->beginResetModel();
    m_enabledPlugins = pluginsList;

    // Remove all already loaded plugins from the pluginsList
    // and unload those plugins that are not in the pluginsList
    auto i = m_plugins.begin();
    while (i != m_plugins.end()) {
        const QString pluginPath = (*i)->property("pluginPath").toString();
        if (pluginsList.contains(pluginPath)) {
            pluginsList.removeAll(pluginPath);
            ++i;
        } else {
            (*i)->deleteLater();
            i = m_plugins.erase(i);
        }
    }

    // Now load all the plugins left in pluginsList
    Q_FOREACH (const QString &pluginPath, pluginsList) {
        loadPlugin(pluginPath);
    }

    m_model->endResetModel();
    Q_EMIT pluginsChanged();
}

QStringList EventPluginsManager::enabledPlugins() const
{
    return m_enabledPlugins;
}

void EventPluginsManager::loadPlugin(const QString &absolutePath)
{
    QPluginLoader loader(absolutePath);

    if (!loader.load()) {
        qCWarning(LOG_PLASMACALENDAR) << "Could not create Plasma Calendar Plugin: " << absolutePath;
        qCWarning(LOG_PLASMACALENDAR) << loader.errorString();
        return;
    }

    QObject *obj = loader.instance();
    if (obj) {
        CalendarEvents::CalendarEventsPlugin *eventsPlugin = qobject_cast<CalendarEvents::CalendarEventsPlugin*>(obj);
        if (eventsPlugin) {
            qCDebug(LOG_PLASMACALENDAR) << "Loading Calendar plugin" << eventsPlugin;
            eventsPlugin->setProperty("pluginPath", absolutePath);
            m_plugins << eventsPlugin;

            // Connect the relay signals
            connect(eventsPlugin, &CalendarEvents::CalendarEventsPlugin::dataReady,
                    this, &EventPluginsManager::dataReady);
            connect(eventsPlugin, &CalendarEvents::CalendarEventsPlugin::eventModified,
                    this, &EventPluginsManager::eventModified);
            connect(eventsPlugin, &CalendarEvents::CalendarEventsPlugin::eventRemoved,
                    this, &EventPluginsManager::eventRemoved);
        } else {
            // not our/valid plugin, so unload it
            loader.unload();
        }
    } else {
        loader.unload();
    }
}

QList<CalendarEvents::CalendarEventsPlugin*> EventPluginsManager::plugins() const
{
    return m_plugins;
}

QAbstractListModel* EventPluginsManager::pluginsModel() const
{
    return m_model;
}

#include "eventpluginsmanager.moc"
