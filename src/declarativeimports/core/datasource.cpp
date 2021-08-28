/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "datasource.h"

namespace Plasma
{
DataSource::DataSource(QObject *parent)
    : QObject(parent)
    , m_ready(false)
    , m_interval(0)
    , m_intervalAlignment(Plasma::Types::NoAlignment)
{
    m_models = new QQmlPropertyMap(this);
    m_data = new QQmlPropertyMap(this);
    setObjectName(QStringLiteral("DataSource"));
}

void DataSource::classBegin()
{
}

void DataSource::componentComplete()
{
    m_ready = true;
    setupData();
}

void DataSource::setConnectedSources(const QStringList &sources)
{
    bool sourcesChanged = false;
    for (const QString &source : sources) {
        if (!m_connectedSources.contains(source)) {
            sourcesChanged = true;
            if (m_dataEngine) {
                m_connectedSources.append(source);
                m_dataEngine->connectSource(source, this, m_interval, m_intervalAlignment);
                Q_EMIT sourceConnected(source);
            }
        }
    }

    for (const QString &source : std::as_const(m_connectedSources)) {
        if (!sources.contains(source)) {
            m_data->clear(source);
            sourcesChanged = true;
            if (m_dataEngine) {
                m_dataEngine->disconnectSource(source, this);
                Q_EMIT sourceDisconnected(source);
            }
        }
    }

    if (sourcesChanged) {
        m_connectedSources = sources;
        Q_EMIT connectedSourcesChanged();
    }
}

void DataSource::setEngine(const QString &e)
{
    if (e == m_engine) {
        return;
    }

    m_engine = e;

    if (m_engine.isEmpty()) {
        Q_EMIT engineChanged();
        return;
    }

    m_dataEngineConsumer.reset(new Plasma::DataEngineConsumer());
    Plasma::DataEngine *engine = dataEngine(m_engine);
    if (!engine) {
        qWarning() << "DataEngine" << m_engine << "not found";
        Q_EMIT engineChanged();
        return;
    }

    if (m_dataEngine) {
        m_dataEngine->disconnect(this);
        // Deleting the consumer triggers the reference counting
        m_dataEngineConsumer.reset();
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
    connect(m_dataEngine, &DataEngine::sourceAdded, this, &DataSource::updateSources, Qt::QueuedConnection);
    connect(m_dataEngine, &DataEngine::sourceRemoved, this, &DataSource::updateSources);

    connect(m_dataEngine, &DataEngine::sourceAdded, this, &DataSource::sourceAdded, Qt::QueuedConnection);
    connect(m_dataEngine, &DataEngine::sourceRemoved, this, &DataSource::removeSource);
    connect(m_dataEngine, &DataEngine::sourceRemoved, this, &DataSource::sourceRemoved);

    updateSources();

    Q_EMIT engineChanged();
}

void DataSource::setInterval(const int interval)
{
    if (interval == m_interval) {
        return;
    }

    m_interval = interval;
    setupData();
    Q_EMIT intervalChanged();
}

void DataSource::setIntervalAlignment(Plasma::Types::IntervalAlignment intervalAlignment)
{
    if (intervalAlignment == m_intervalAlignment) {
        return;
    }

    m_intervalAlignment = intervalAlignment;
    setupData();
    Q_EMIT intervalAlignmentChanged();
}

void DataSource::setupData()
{
    if (!m_ready) {
        return;
    }

    //     qDebug() << " loading engine " << m_engine;
    // FIXME: should all services be deleted just because we're changing the interval, etc?
    qDeleteAll(m_services);
    m_services.clear();

    for (const QString &source : std::as_const(m_connectedSources)) {
        m_dataEngine->connectSource(source, this, m_interval, m_intervalAlignment);
        Q_EMIT sourceConnected(source);
    }
}

void DataSource::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    // it can arrive also data we don't explicitly connected a source
    if (m_connectedSources.contains(sourceName)) {
        m_data->insert(sourceName, data);
        Q_EMIT dataChanged();
        Q_EMIT newData(sourceName, data);
    } else if (m_dataEngine) {
        m_dataEngine->disconnectSource(sourceName, this);
    }
}

void DataSource::modelChanged(const QString &sourceName, QAbstractItemModel *model)
{
    if (!model) {
        m_models->clear(sourceName);
        return;
    }

    m_models->insert(sourceName, QVariant::fromValue(model));
    // FIXME: this will break in the case a second model is set
    connect(model, &QObject::destroyed, m_models, [=]() {
        m_models->clear(sourceName);
    });
}

void DataSource::removeSource(const QString &source)
{
    m_data->clear(source);
    m_models->clear(source);

    // TODO: emit those signals as last thing
    if (m_connectedSources.contains(source)) {
        m_connectedSources.removeAll(source);
        Q_EMIT sourceDisconnected(source);
        Q_EMIT connectedSourcesChanged();
    }

    if (m_dataEngine) {
        QHash<QString, Plasma::Service *>::iterator it = m_services.find(source);
        if (it != m_services.end()) {
            delete it.value();
            m_services.erase(it);
        }
    }
}

QObject *DataSource::serviceForSource(const QString &source)
{
    if (!m_services.contains(source)) {
        Plasma::Service *service = m_dataEngine->serviceForSource(source);
        if (!service) {
            return nullptr;
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
        m_dataEngine->connectSource(source, this, m_interval, m_intervalAlignment);
        Q_EMIT sourceConnected(source);
        Q_EMIT connectedSourcesChanged();
    }
}

void DataSource::disconnectSource(const QString &source)
{
    if (m_dataEngine && m_connectedSources.contains(source)) {
        m_connectedSources.removeAll(source);
        m_dataEngine->disconnectSource(source, this);
        Q_EMIT sourceDisconnected(source);
        Q_EMIT connectedSourcesChanged();
    }
}

void DataSource::updateSources()
{
    QStringList sources;
    if (m_dataEngine) {
        sources = m_dataEngine->sources();
    }

    if (sources != m_sources) {
        m_sources = sources;
        Q_EMIT sourcesChanged();
    }
}

}
