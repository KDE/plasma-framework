/*
 *   Copyright 2010 by Marco Martin <mart@kde.org>

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

#include "datamodel.h"
#include "datasource_p.h"

#include <QTimer>

#include <KDebug>

namespace Plasma
{

SortFilterModel::SortFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setObjectName("SortFilterModel");
    setDynamicSortFilter(true);
    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()),
            this, SIGNAL(countChanged()));
}

SortFilterModel::~SortFilterModel()
{
}

void SortFilterModel::syncRoleNames()
{
    m_roleIds.clear();

    setRoleNames(sourceModel()->roleNames());
    QHash<int, QByteArray>::const_iterator i;
    for (i = roleNames().constBegin(); i != roleNames().constEnd(); ++i) {
        m_roleIds[i.value()] = i.key();
    }
    setFilterRole(m_filterRole);
    setSortRole(m_sortRole);
}

int SortFilterModel::roleNameToId(const QString &name)
{
    if (!m_roleIds.contains(name)) {
        return -1;
    }
    return m_roleIds.value(name);
}

void SortFilterModel::setModel(QObject *source)
{
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(source);
    if (!model) {
        kWarning() << "Error: QAbstractItemModel type expected";
        return;
    }

    connect(model, SIGNAL(modelReset()), this, SLOT(syncRoleNames()));
    QSortFilterProxyModel::setSourceModel(model);
}




void SortFilterModel::setFilterRegExp(const QString &exp)
{
    //FIXME: this delaying of the reset signal seems to make the views behave a bit better, i.e. less holes and avoids some crashes, in theory shouldn't be necessary
    beginResetModel();
    blockSignals(true);
    QSortFilterProxyModel::setFilterRegExp(QRegExp(exp, Qt::CaseInsensitive));
    blockSignals(false);
    endResetModel();
}

QString SortFilterModel::filterRegExp() const
{
    return QSortFilterProxyModel::filterRegExp().pattern();
}

void SortFilterModel::setFilterRole(const QString &role)
{
    QSortFilterProxyModel::setFilterRole(roleNameToId(role));
    m_filterRole = role;
}

QString SortFilterModel::filterRole() const
{
    return m_filterRole;
}

void SortFilterModel::setSortRole(const QString &role)
{
    QSortFilterProxyModel::setSortRole(roleNameToId(role));
    m_sortRole = role;
    sort(0, sortOrder());
}

QString SortFilterModel::sortRole() const
{
    return m_sortRole;
}

void SortFilterModel::setSortOrder(const Qt::SortOrder order)
{
    sort(0, order);
}




DataModel::DataModel(QObject* parent)
    : QAbstractItemModel(parent),
      m_dataSource(0),
      m_maxRoleId(Qt::UserRole+1)
{
    //There is one reserved role name: DataEngineSource
    m_roleNames[m_maxRoleId] = "DataEngineSource";
    m_roleIds["DataEngineSource"] = m_maxRoleId;
    ++m_maxRoleId;

    setObjectName("DataModel");
    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()),
            this, SIGNAL(countChanged()));
}

DataModel::~DataModel()
{
}

void DataModel::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    if (!m_sourceFilter.isEmpty() && m_sourceFilterRE.isValid() && !m_sourceFilterRE.exactMatch(sourceName)) {
        return;
    }

    if (m_keyRoleFilter.isEmpty()) {
        //an item is represented by a source: keys are roles m_roleLevel == FirstLevel
        QVariantList list;

        if (!m_dataSource->data().isEmpty()) {
            QVariantHash::const_iterator i = m_dataSource->data().constBegin();

            while (i != m_dataSource->data().constEnd()) {
                QVariant value = i.value();
                if (value.isValid() && value.canConvert<Plasma::DataEngine::Data>()) {
                    Plasma::DataEngine::Data data = value.value<Plasma::DataEngine::Data>();
                    data["DataEngineSource"] = i.key();
                    list.append(data);
                }
                ++i;
            }
        }
        setItems(QString(), list);
    } else {
        //a key that matches the one we want exists and is a list of DataEngine::Data
        if (data.contains(m_keyRoleFilter) &&
            data.value(m_keyRoleFilter).canConvert<QVariantList>()) {
            setItems(sourceName, data.value(m_keyRoleFilter).value<QVariantList>());
        } else if (m_keyRoleFilterRE.isValid()) {
            //try to match the key we want with a regular expression if set
            QVariantList list;
            QHash<QString, QVariant>::const_iterator i;
            for (i = data.constBegin(); i != data.constEnd(); ++i) {
                if (m_keyRoleFilterRE.exactMatch(i.key())) {
                    list.append(i.value());
                }
            }
            setItems(sourceName, list);
        }
    }
}

void DataModel::setDataSource(QObject *object)
{
    DataSource *source = qobject_cast<DataSource *>(object);
    if (!source) {
        kWarning() << "Error: DataSource type expected";
        return;
    }
    if (m_dataSource == source) {
        return;
    }

    if (m_dataSource) {
        disconnect(m_dataSource, 0, this, 0);
    }

    m_dataSource = source;

    const QHash<QString, QVariant> data = source->data();
    QHash<QString, QVariant>::const_iterator i = data.constBegin();
    while (i != data.constEnd()) {
        dataUpdated(i.key(), i.value().value<Plasma::DataEngine::Data>());
        ++i;
    }

    connect(m_dataSource, SIGNAL(newData(const QString &, const Plasma::DataEngine::Data &)),
            this, SLOT(dataUpdated(const QString &, const Plasma::DataEngine::Data &)));
    connect(m_dataSource, SIGNAL(sourceRemoved(const QString &)), this, SLOT(removeSource(const QString &)));
    connect(m_dataSource, SIGNAL(sourceDisconnected(const QString &)), this, SLOT(removeSource(const QString &)));
}

QObject *DataModel::dataSource() const
{
    return m_dataSource;
}

void DataModel::setKeyRoleFilter(const QString& key)
{
    // the "key role filter" can be used in one of three ways:
    //
    // 1) empty string -> all data is used, each source is one row in the model
    // 2) matches a key in the data exactly -> only that key/value pair is used, and the value is
    //    treated as a collection where each item in the collection becomes a row in the model
    // 3) regular expression -> matches zero or more keys in the data, and each matching key/value
    //    pair becomes a row in the model
    if (m_keyRoleFilter == key) {
        return;
    }

    m_keyRoleFilter = key;
    m_keyRoleFilterRE = QRegExp(m_keyRoleFilter);
}

QString DataModel::keyRoleFilter() const
{
    return m_keyRoleFilter;
}

void DataModel::setSourceFilter(const QString& key)
{
    if (m_sourceFilter == key) {
        return;
    }

    m_sourceFilter = key;
    m_sourceFilterRE = QRegExp(key);
    /*
     FIXME: if the user changes the source filter, it won't immediately be reflected in the
     available data
    if (m_sourceFilterRE.isValid()) {
        .. iterate through all items and weed out the ones that don't match ..
    }
    */
}

QString DataModel::sourceFilter() const
{
    return m_sourceFilter;
}

void DataModel::setItems(const QString &sourceName, const QVariantList &list)
{
    beginResetModel();

    //convert to vector, so data() will be O(1)
    m_items[sourceName] = list.toVector();

    if (!list.isEmpty()) {
        if (list.first().canConvert<QVariantHash>()) {
            foreach (const QVariant &item, list) {
                const QVariantHash &vh = item.value<QVariantHash>();
                QHashIterator<QString, QVariant> it(vh);
                while (it.hasNext()) {
                    it.next();
                    const QString &roleName = it.key();
                    if (!m_roleIds.contains(roleName)) {
                        ++m_maxRoleId;
                        m_roleNames[m_maxRoleId] = roleName.toLatin1();
                        m_roleIds[roleName] = m_maxRoleId;
                    }
                }
            }
        } else {
            foreach (const QVariant &item, list) {
                const QVariantMap &vh = item.value<QVariantMap>();
                QMapIterator<QString, QVariant> it(vh);
                while (it.hasNext()) {
                    it.next();
                    const QString &roleName = it.key();
                    if (!m_roleIds.contains(roleName)) {
                        ++m_maxRoleId;
                        m_roleNames[m_maxRoleId] = roleName.toLatin1();
                        m_roleIds[roleName] = m_maxRoleId;
                    }
                }
            }
        }

        setRoleNames(m_roleNames);
    }

    setRoleNames(m_roleNames);

    //make the declarative view reload everything,
    //would be nice an incremental update but is not possible
    endResetModel();
}

void DataModel::removeSource(const QString &sourceName)
{
    //FIXME: this could be way more efficient by not resetting the whole model
    //FIXME: find a way to remove only the proper things also in the case where sources are items

    if (m_keyRoleFilter.isEmpty()) {
        //source name in the map, linear scan
        for (int i = 0; i < m_items.value(QString()).count(); ++i) {
            if (m_items.value(QString())[i].value<QVariantHash>().value("DataEngineSource") == sourceName) {
                beginResetModel();
                m_items[QString()].remove(i);
                endResetModel();
                break;
            }
        }
    } else {
        //source name as key of the map
        if (m_items.contains(sourceName)) {
            beginResetModel();
            m_items.remove(sourceName);
            endResetModel();
        }
    }
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() > 0 ||
        index.row() < 0 || index.row() >= countItems()){
        return QVariant();
    }

    int count = 0;
    int actualRow = 0;
    QString source;
    QMap<QString, QVector<QVariant> >::const_iterator i;
    for (i = m_items.constBegin(); i != m_items.constEnd(); ++i) {
        const int oldCount = count;
        count += i.value().count();

        if (index.row() < count) {
            source = i.key();
            actualRow = index.row() - oldCount;
            break;
        }
    }

    //is it the reserved role: DataEngineSource ?
    //also, if each source is an item DataEngineSource is a role between all the others, otherwise we know it from the role variable
    //finally, sub items are some times QVariantHash some times QVariantMaps
    if (!m_keyRoleFilter.isEmpty() && m_roleNames.value(role) == "DataEngineSource") {
        return source;
    } else if (m_items.value(source).value(actualRow).canConvert<QVariantHash>()) {
        return m_items.value(source).value(actualRow).value<QVariantHash>().value(m_roleNames.value(role));
    } else {
        return m_items.value(source).value(actualRow).value<QVariantMap>().value(m_roleNames.value(role));
    }
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

QModelIndex DataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || column > 0 || row < 0 || row >= countItems()) {
        return QModelIndex();
    }

    return createIndex(row, column, 0);
}

QModelIndex DataModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)

    return QModelIndex();
}

int DataModel::rowCount(const QModelIndex &parent) const
{
    //this is not a tree
    //TODO: make it possible some day?
    if (parent.isValid()) {
        return 0;
    }

    return countItems();
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 1;
}

int DataModel::roleNameToId(const QString &name)
{
    if (!m_roleIds.contains(name)) {
        return -1;
    }
    return m_roleIds.value(name);
}

}

#include "datamodel.moc"
