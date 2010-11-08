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

#include "datasource_p.h"

#include "qdeclarativeengine.h"
#include "qdeclarativecontext.h"


#include <Plasma/Applet>


namespace Plasma
{
DataSource::DataSource(QObject* parent)
    : QObject(parent),
      m_interval(1000),
      m_dataEngine(0)
{
    setObjectName("DataSource");

    connect(this, SIGNAL(engineChanged()),
            this, SLOT(setupData()));
    connect(this, SIGNAL(connectedSourcesChanged()),
            this, SLOT(setupData()));
    connect(this, SIGNAL(intervalChanged()),
            this, SLOT(setupData()));
}

void DataSource::setConnectedSources(const QStringList &sources)
{
    foreach (const QString &source, sources) {
        if (!m_connectedSources.contains(source)) {
            m_newSources.append(source);
        }
    }
    foreach (const QString &source, m_connectedSources) {
        if (!m_connectedSources.contains(source)) {
            m_oldSources.append(source);
        }
    }

    if (!m_newSources.isEmpty() || !m_oldSources.isEmpty()) {
        m_connectedSources = sources;
        m_changes |= SourcesChanged;
        emit connectedSourcesChanged();
    }
}

void DataSource::setEngine(const QString &e)
{
    if (e == m_engine) {
        return;
    }
    m_engine = e;

    m_changes |= DataEngineChanged;
    emit engineChanged();
}

void DataSource::setInterval(const int interval)
{
    if (interval == m_interval) {
        return;
    }
    m_interval = interval;
    m_changes |= DataEngineChanged;
    emit intervalChanged();
}

void DataSource::setupData()
{

    if (m_changes & DataEngineChanged) {
        if (m_dataEngine) {
            foreach (const QString &source, m_connectedSources) {
                m_dataEngine->disconnectSource(source, this);
            }
        }
        //FIXME: delete all?
        m_services.clear();

        m_dataEngine = dataEngine(m_engine);
        if (!m_dataEngine) {
            kWarning() << "DataEngine" << m_engine << "not found";
            return;
        }
        connect(m_dataEngine, SIGNAL(sourceAdded(const QString&)), this, SIGNAL(allSourcesChanged()));
        connect(m_dataEngine, SIGNAL(sourceRemoved(const QString&)), this, SIGNAL(allSourcesChanged()));

        connect(m_dataEngine, SIGNAL(sourceAdded(const QString&)), this, SIGNAL(sourceAdded(const QString&)));
        connect(m_dataEngine, SIGNAL(sourceRemoved(const QString&)), this, SLOT(removeSource(const QString&)));

        if (!(m_changes & SourcesChanged)) {
            foreach (const QString &source, m_connectedSources) {
                m_dataEngine->connectSource(source, this, m_interval);
            }
        }
    }

    if (m_changes & SourcesChanged) {
        if (m_dataEngine) {
            foreach (const QString &source, m_oldSources) {
                m_dataEngine->disconnectSource(source, this);
            }
            foreach (const QString &source, m_newSources) {
                m_dataEngine->connectSource(source, this, m_interval);
            }
            m_oldSources.clear();
            m_newSources.clear();
        }
    }
    m_changes = NoChange;
}

void DataSource::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    m_data.insert(sourceName.toLatin1(), data);

    emit dataChanged();
    emit newData(sourceName, data);
}

void DataSource::removeSource(const QString &source)
{
    m_data.remove(source);

    if (m_connectedSources.contains(source)) {
        emit connectedSourcesChanged();
    }
    if (m_dataEngine) {
        m_connectedSources.removeAll(source);
        m_newSources.removeAll(source);
        m_oldSources.removeAll(source);
        //TODO: delete it?
        m_services.remove(source);
        emit connectedSourcesChanged();
    }
}

QStringList DataSource::keysForSource(const QString &source) const
{
    if (!m_data.contains(source)) {
        return QStringList();
    }
    return m_data.value(source).value<Data>().keys();
}

Plasma::Service *DataSource::serviceForSource(const QString &source)
{
    if (!m_services.contains(source)) {
        m_services[source] = m_dataEngine->serviceForSource(source);
    }

    return m_services.value(source);
}

void DataSource::connectSource(const QString &source)
{
    m_newSources.append(source);
    m_connectedSources.append(source);
    m_changes |= SourcesChanged;
    emit connectedSourcesChanged();
}

void DataSource::disconnectSource(const QString &source)
{
    m_oldSources.append(source);
    m_connectedSources.removeAll(source);
    m_changes |= SourcesChanged;
    emit connectedSourcesChanged();
}

}
#include "datasource_p.moc"
