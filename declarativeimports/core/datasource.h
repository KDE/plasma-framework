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

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>


#include <QDeclarativePropertyMap>

#include "dataengineconsumer_p.h"
#include <Plasma/DataEngine>
#include <qdeclarative.h>


class QDeclarativePropertyMap;


namespace Plasma
{
class DataEngine;

class DataSource : public QObject, DataEngineConsumer
{
    Q_OBJECT
public:
    enum Change {
        NoChange = 0,
        DataEngineChanged = 1,
        SourcesChanged = 2
    };
    Q_DECLARE_FLAGS(Changes, Change)

    typedef QHash<QString, QVariant> Data;

    DataSource(QObject* parent=0);

    Q_PROPERTY(bool valid READ valid)
    bool valid() const {return m_dataEngine && m_dataEngine->isValid();}

    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    int interval() const {return m_interval;}
    void setInterval(const int interval);

    Q_PROPERTY(QString dataEngine READ engine WRITE setEngine NOTIFY engineChanged)
    Q_PROPERTY(QString engine READ engine WRITE setEngine NOTIFY engineChanged)
    QString engine() const {return m_engine;}
    void setEngine(const QString &e);

    Q_PROPERTY(QStringList connectedSources READ connectedSources WRITE setConnectedSources NOTIFY connectedSourcesChanged)
    QStringList connectedSources() const {return m_connectedSources;}
    void setConnectedSources(const QStringList &s);

    Q_PROPERTY(QStringList sources READ sources NOTIFY sourcesChanged)
    QStringList sources() const {if (m_dataEngine) return m_dataEngine->sources(); else return QStringList();}

    Q_PROPERTY(QVariantHash data READ data NOTIFY dataChanged);
    QVariantHash data() const {return m_data;}

    Q_INVOKABLE Plasma::Service *serviceForSource(const QString &source);

    Q_INVOKABLE void connectSource(const QString &source);
    Q_INVOKABLE void disconnectSource(const QString &source);

protected Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);
    void removeSource(const QString &source);
    void setupData();

Q_SIGNALS:
    void newData(const QString &sourceName, const Plasma::DataEngine::Data &data);
    void sourceAdded(const QString &source);
    void sourceRemoved(const QString &source);
    void sourceConnected(const QString &source);
    void sourceDisconnected(const QString &source);
    void intervalChanged();
    void engineChanged();
    void dataChanged();
    void connectedSourcesChanged();
    void sourcesChanged();

private:
    QString m_id;
    int m_interval;
    QString m_engine;
    QVariantHash m_data;
    Plasma::DataEngine* m_dataEngine;
    QStringList m_connectedSources;
    QStringList m_oldSources;
    QStringList m_newSources;
    Changes m_changes;
    QHash<QString, Plasma::Service *> m_services;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DataSource::Changes)
}
#endif
