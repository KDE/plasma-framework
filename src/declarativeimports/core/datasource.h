/*
 *   Copyright 2009 by Alan Alpert <alan.alpert@nokia.com>
 *   Copyright 2010 by Ménard Alexis <menard@kde.org>
 *   Copyright 2010 by Marco MArtin <mart@kde.org>
 *   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
 *
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

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QtQml>
#include <QQmlPropertyMap>
#include <QQmlParserStatus>

#include <Plasma/DataEngineConsumer>
#include <Plasma/DataEngine>

class QQmlPropertyMap;

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
        SourcesChanged = 2
    };
    Q_DECLARE_FLAGS(Changes, Change)

    typedef QMap<QString, QVariant> Data;

    explicit DataSource(QObject *parent = nullptr);

    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

    /**
     * true if the connection to the Plasma DataEngine is valid
     */
    Q_PROPERTY(bool valid READ valid)
    bool valid() const
    {
        return m_dataEngine && m_dataEngine->isValid();
    }

    /**
     * Polling interval in milliseconds when the data will be fetched again. If 0 no polling will be done.
     */
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    int interval() const
    {
        return m_interval;
    }
    void setInterval(const int interval);

    /**
     * The interval to align polling to
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
     * String array of all the source names connected to the DataEngine
     */
    Q_PROPERTY(QStringList connectedSources READ connectedSources WRITE setConnectedSources NOTIFY connectedSourcesChanged)
    QStringList connectedSources() const
    {
        return m_connectedSources;
    }
    void setConnectedSources(const QStringList &s);

    /**
     * Read only string array of all the sources available from the DataEngine (connected or not)
     */
    Q_PROPERTY(QStringList sources READ sources NOTIFY sourcesChanged)
    QStringList sources() const
    {
        return m_sources;
    }

    /**
     * All the data fetched by this dataengine.
     * This is a map of maps. At the first level, there are the source names, at the second, they keys set by the DataEngine
     */
    Q_PROPERTY(QQmlPropertyMap *data READ data CONSTANT)
    QQmlPropertyMap *data() const
    {
        return m_data;
    }

    /**
     * All the models associated to this DataEngine, indexed by source.
     * In order for a model to be present, besides being implemented in the DataEngine,
     * The user has to be connected to its source, so the source name has to be present in the connectedSources property.
     */
    Q_PROPERTY(QQmlPropertyMap *models READ models CONSTANT)
    QQmlPropertyMap *models()  const
    {
        return m_models;
    }

    /**
     * @returns a Plasma::Service given a source name
     * @param source source name we want a service of
     */
    Q_INVOKABLE QObject *serviceForSource(const QString &source);

    /**
     * Connect a new source. It adds it to connectedSources
     */
    Q_INVOKABLE void connectSource(const QString &source);

    /**
     * Disconnects from a DataEngine Source. It also removes it from connectedSources
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
    Plasma::DataEngineConsumer *m_dataEngineConsumer = nullptr;
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
