/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
    SPDX-FileCopyrightText: 2010 Marco MArtin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QQmlParserStatus>
#include <QQmlPropertyMap>
#include <QtQml>

#include <Plasma/DataEngine>
#include <Plasma/DataEngineConsumer>

class QQmlPropertyMap;

// This class will hopefully be removed in KF6 (along with DataEngines in general)

namespace Plasma
{
class DataEngine;

/**
 * @class DataSource
 * @short Provides data from a range of plugins
 */
class DataSource : public QObject, public QQmlParserStatus, DataEngineConsumer
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    enum Change {
        NoChange = 0,
        DataEngineChanged = 1,
        SourcesChanged = 2,
    };
    Q_DECLARE_FLAGS(Changes, Change)

    typedef QMap<QString, QVariant> Data;

    explicit DataSource(QObject *parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

    /**
     * True if the connection to the Plasma DataEngine is valid.
     */
    Q_PROPERTY(bool valid READ valid)
    bool valid() const
    {
        return m_dataEngine && m_dataEngine->isValid();
    }

    /**
     * Polling interval in milliseconds when the data will be fetched again. If 0, no polling will be done.
     */
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    int interval() const
    {
        return m_interval;
    }
    void setInterval(const int interval);

    /**
     * The interval to align polling to.
     */
    Q_PROPERTY(Plasma::Types::IntervalAlignment intervalAlignment READ intervalAlignment WRITE setIntervalAlignment NOTIFY intervalAlignmentChanged)
    Plasma::Types::IntervalAlignment intervalAlignment() const
    {
        return m_intervalAlignment;
    }
    void setIntervalAlignment(Plasma::Types::IntervalAlignment intervalAlignment);

    /**
     * Plugin name of the Plasma DataEngine
     */
    Q_PROPERTY(QString dataEngine READ engine WRITE setEngine NOTIFY engineChanged)
    Q_PROPERTY(QString engine READ engine WRITE setEngine NOTIFY engineChanged)
    QString engine() const
    {
        return m_engine;
    }
    void setEngine(const QString &e);

    /**
     * List of all the sources connected to the DataEngine.
     */
    Q_PROPERTY(QStringList connectedSources READ connectedSources WRITE setConnectedSources NOTIFY connectedSourcesChanged)
    QStringList connectedSources() const
    {
        return m_connectedSources;
    }
    void setConnectedSources(const QStringList &s);

    /**
     * Read-only list of all the sources available from the DataEngine (connected or not).
     */
    Q_PROPERTY(QStringList sources READ sources NOTIFY sourcesChanged)
    QStringList sources() const
    {
        return m_sources;
    }

    /**
     * All the data fetched by this dataengine.
     * This is a map of maps.
     *
     *   1. At the first level, there are the source names.
     *   2. At the second level, there are source-specific keys set by the DataEngine.
     *
     * Refer to a particular DataEngine implementation for available sources,
     * keys and expected value types.
     */
    Q_PROPERTY(QQmlPropertyMap *data READ data CONSTANT)
    QQmlPropertyMap *data() const
    {
        return m_data;
    }

    /**
     * All the models associated to this DataEngine, indexed by source.
     *
     * In order for a model to be available, besides being implemented in the
     * DataEngine, the user has to be connected to its source, i.e. the
     * source name has to be present in connectedSources list.
     */
    Q_PROPERTY(QQmlPropertyMap *models READ models CONSTANT)
    QQmlPropertyMap *models() const
    {
        return m_models;
    }

    /**
     * @returns a Plasma::Service given a source name
     * @param source source name we want a service of
     */
    Q_INVOKABLE QObject *serviceForSource(const QString &source);

    /**
     * Connects a new source and adds it to the connectedSources list.
     */
    Q_INVOKABLE void connectSource(const QString &source);

    /**
     * Disconnects from a DataEngine source and removes it from the connectedSources list.
     */
    Q_INVOKABLE void disconnectSource(const QString &source);

public Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);
    void modelChanged(const QString &sourceName, QAbstractItemModel *model);

protected Q_SLOTS:
    void removeSource(const QString &source);
    void setupData();
    void updateSources();

Q_SIGNALS:
    void newData(const QString &sourceName, const QVariantMap &data);
    void sourceAdded(const QString &source);
    void sourceRemoved(const QString &source);
    void sourceConnected(const QString &source);
    void sourceDisconnected(const QString &source);
    void intervalChanged();
    void intervalAlignmentChanged();
    void engineChanged();
    void dataChanged();
    void connectedSourcesChanged();
    void sourcesChanged();

private:
    bool m_ready;
    QString m_id;
    int m_interval;
    Plasma::Types::IntervalAlignment m_intervalAlignment;
    QString m_engine;
    QQmlPropertyMap *m_data = nullptr;
    QQmlPropertyMap *m_models = nullptr;
    Plasma::DataEngine *m_dataEngine = nullptr;
    std::unique_ptr<Plasma::DataEngineConsumer> m_dataEngineConsumer;
    QStringList m_sources;
    QStringList m_connectedSources;
    QStringList m_oldSources;
    QStringList m_newSources;
    Changes m_changes;
    QHash<QString, Plasma::Service *> m_services;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DataSource::Changes)
}
#endif
