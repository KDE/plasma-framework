/*
 *   Copyright 2009 by Alan Alpert <alan.alpert@nokia.com>
 *   Copyright 2010 by Ménard Alexis <menard@kde.org>
 *   Copyright 2010 by Marco MArtin <mart@kde.org>

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

#include "datasource.h"

#include "qdeclarativeengine.h"
#include "qdeclarativecontext.h"


#include <Plasma/Applet>


namespace Plasma
{
DataSource::DataSource(QObject* parent)
    : QObject(parent),
      m_interval(0),
      m_dataEngine(0)
{
    setObjectName("DataSource");
}

void DataSource::setConnectedSources(const QStringList &sources)
{
    bool sourcesChanged = false;
    foreach (const QString &source, sources) {
        if (!m_connectedSources.contains(source)) {
            sourcesChanged = true;
            if (m_dataEngine) {
                m_connectedSources.append(source);
                m_dataEngine->connectSource(source, this, m_interval);
                emit sourceConnected(source);
            }
        }
    }

    foreach (const QString &source, m_connectedSources) {
        if (!sources.contains(source)) {
            m_data.remove(source);
            sourcesChanged = true;
            if (m_dataEngine) {
                m_dataEngine->disconnectSource(source, this);
                emit sourceDisconnected(source);
            }
        }
    }

    if (sourcesChanged) {
        m_connectedSources = sources;
        emit connectedSourcesChanged();
    }
}

void DataSource::setEngine(const QString &e)
{
    if (e == m_engine) {
        return;
    }

    m_engine = e;
    setupData();
    emit engineChanged();
}

void DataSource::setInterval(const int interval)
{
    if (interval == m_interval) {
        return;
    }

    m_interval = interval;
    setupData();
    emit intervalChanged();
}

//TODO: event compression for this
void DataSource::setupData()
{
    //FIXME: should all services be deleted just because we're changing the interval, etc?
    qDeleteAll(m_services);
    m_services.clear();

    Plasma::DataEngine *engine = dataEngine(m_engine);
    if (!engine) {
        kWarning() << "DataEngine" << m_engine << "not found";
        return;
    }

    if (engine != m_dataEngine) {
        if (m_dataEngine) {
            m_dataEngine->disconnect(this);
            finishedWithEngine(m_dataEngine->pluginName());
        }

        /*
         * It is due little explanation why this is a queued connection:
         * If sourceAdded arrives immediately, in the case we have a datamodel
         * with items at source level we connect too soon (before setData for
         * all roles is done), having a dataupdated in the datamodel with only 
         * the first role, killing off the other roles.
         * Besides causing a model reset more, unfortunately setRoleNames can be done a single time, so is not possible adding new roles after the
         * first setRoleNames() is called.
         * This fixes engines that have 1 item per source like the
         * recommendations engine.
         */
        m_dataEngine = engine;
        connect(m_dataEngine, SIGNAL(sourceAdded(const QString&)), this, SIGNAL(sourcesChanged()), Qt::QueuedConnection);
        connect(m_dataEngine, SIGNAL(sourceRemoved(const QString&)), this, SIGNAL(sourcesChanged()));

        connect(m_dataEngine, SIGNAL(sourceAdded(const QString&)), this, SIGNAL(sourceAdded(const QString&)), Qt::QueuedConnection);
        connect(m_dataEngine, SIGNAL(sourceRemoved(const QString&)), this, SLOT(removeSource(const QString&)));
        connect(m_dataEngine, SIGNAL(sourceRemoved(const QString&)), this, SIGNAL(sourceRemoved(const QString&)));
    }

    foreach (const QString &source, m_connectedSources) {
        m_dataEngine->connectSource(source, this, m_interval);
        emit sourceConnected(source);
    }
}

void DataSource::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    //it can arrive also data we don't explicitly connected a source
    if (m_connectedSources.contains(sourceName)) {
        m_data.insert(sourceName.toLatin1(), data);

        emit dataChanged();
        emit newData(sourceName, data);
    } else if (m_dataEngine) {
        m_dataEngine->disconnectSource(sourceName, this);
    }
}

void DataSource::removeSource(const QString &source)
{
    m_data.remove(source);

    //TODO: emit those signals as last thing
    if (m_connectedSources.contains(source)) {
        m_connectedSources.removeAll(source);
        emit sourceDisconnected(source);
        emit connectedSourcesChanged();
    }

    if (m_dataEngine) {
        QHash<QString, Plasma::Service *>::iterator it = m_services.find(source);
        if (it != m_services.end()) {
            delete it.value();
            m_services.erase(it);
        }
    }
}

Plasma::Service *DataSource::serviceForSource(const QString &source)
{
    if (!m_services.contains(source)) {
        Plasma::Service *service = m_dataEngine->serviceForSource(source);
        if (!service) {
            return 0;
        }
        m_services[source] = service;
    }

    return m_services.value(source);
}

void DataSource::connectSource(const QString &source)
{
    if (m_connectedSources.contains(source)) {
        return;
    }

    m_connectedSources.append(source);
    if (m_dataEngine) {
        m_dataEngine->connectSource(source, this, m_interval);
        emit sourceConnected(source);
        emit connectedSourcesChanged();
    }
}

void DataSource::disconnectSource(const QString &source)
{
    if (m_dataEngine && m_connectedSources.contains(source)) {
        m_connectedSources.removeAll(source);
        m_dataEngine->disconnectSource(source, this);
        emit sourceDisconnected(source);
        emit connectedSourcesChanged();
    }
}

}
#include "datasource.moc"
