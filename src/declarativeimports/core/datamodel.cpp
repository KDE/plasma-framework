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
#include "datasource.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QTimer>

namespace Plasma
{

SortFilterModel::SortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setObjectName(QStringLiteral("SortFilterModel"));
    setDynamicSortFilter(true);
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()),
            this, SIGNAL(countChanged()));
    connect(this,  SIGNAL(countChanged()), this, SLOT(syncRoleNames()));
}

SortFilterModel::~SortFilterModel()
{
}

void SortFilterModel::syncRoleNames()
{
    if (!sourceModel()) {
        return;
    }

    m_roleIds.clear();
    const QHash<int, QByteArray> rNames = roleNames();
    m_roleIds.reserve(rNames.count());
    for (auto i = rNames.constBegin(); i != rNames.constEnd(); ++i) {
        m_roleIds[QString::fromUtf8(i.value())] = i.key();
    }

    setRoleNames(sourceModel()->roleNames());
    setFilterRole(m_filterRole);
    setSortRole(m_sortRole);
}

int SortFilterModel::roleNameToId(const QString &name) const
{
    return m_roleIds.value(name, Qt::DisplayRole);
}

void SortFilterModel::setModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }

    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(syncRoleNames()));
    }

    QSortFilterProxyModel::setSourceModel(model);

    if (model) {
        connect(model, SIGNAL(modelReset()), this, SLOT(syncRoleNames()));
        syncRoleNames();
    }

    emit sourceModelChanged(model);
}

bool SortFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_filterCallback.isCallable()) {
        QJSValueList args;
        args << QJSValue(source_row);

        const QModelIndex idx = sourceModel()->index(source_row, filterKeyColumn(), source_parent);
        QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
        args << engine->toScriptValue<QVariant>(idx.data(m_roleIds.value(m_filterRole)));

        return const_cast<SortFilterModel *>(this)->m_filterCallback.call(args).toBool();
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void SortFilterModel::setFilterRegExp(const QString &exp)
{
    if (exp == filterRegExp()) {
        return;
    }
    QSortFilterProxyModel::setFilterRegExp(QRegExp(exp, Qt::CaseInsensitive));
    Q_EMIT filterRegExpChanged(exp);
}

QString SortFilterModel::filterRegExp() const
{
    return QSortFilterProxyModel::filterRegExp().pattern();
}

void SortFilterModel::setFilterString(const QString &filterString)
{
    if (filterString == m_filterString) {
        return;
    }
    m_filterString = filterString;
    QSortFilterProxyModel::setFilterFixedString(filterString);
    Q_EMIT filterStringChanged(filterString);
}

QString SortFilterModel::filterString() const
{
    return m_filterString;
}

QJSValue SortFilterModel::filterCallback() const
{
    return m_filterCallback;
}

void SortFilterModel::setFilterCallback(const QJSValue& callback)
{
    if (m_filterCallback.strictlyEquals(callback)) {
        return;
    }

    if (!callback.isNull() && !callback.isCallable()) {
        return;
    }

    m_filterCallback = callback;
    invalidateFilter();

    emit filterCallbackChanged(callback);
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
    m_sortRole = role;
    if (role.isEmpty()) {
        sort(-1, Qt::AscendingOrder);
    } else if (sourceModel()) {
        QSortFilterProxyModel::setSortRole(roleNameToId(role));
        sort(0, sortOrder());
    }
}

QString SortFilterModel::sortRole() const
{
    return m_sortRole;
}

void SortFilterModel::setSortOrder(const Qt::SortOrder order)
{
    sort(0, order);
}

QVariantMap SortFilterModel::get(int row) const
{
    QModelIndex idx = index(row, 0);
    QVariantMap hash;

    const QHash<int, QByteArray> rNames = roleNames();
    for (auto i = rNames.begin(); i != rNames.end(); ++i) {
        hash[QString::fromUtf8(i.value())] = data(idx, i.key());
    }

    return hash;
}

int SortFilterModel::mapRowToSource(int row) const
{
    QModelIndex idx = index(row, 0);
    return mapToSource(idx).row();
}

int SortFilterModel::mapRowFromSource(int row) const
{
    if (!sourceModel()) {
        qWarning() << "No source model defined!";
        return -1;
    }
    QModelIndex idx = sourceModel()->index(row, 0);
    return mapFromSource(idx).row();
}

DataModel::DataModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_dataSource(0),
      m_maxRoleId(Qt::UserRole + 1)
{
    //There is one reserved role name: DataEngineSource
    m_roleNames[m_maxRoleId] = QByteArrayLiteral("DataEngineSource");
    m_roleIds[QStringLiteral("DataEngineSource")] = m_maxRoleId;
    ++m_maxRoleId;

    setObjectName(QStringLiteral("DataModel"));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()),
            this, SIGNAL(countChanged()));
}

DataModel::~DataModel()
{
}

void DataModel::dataUpdated(const QString &sourceName, const QVariantMap &data)
{
    if (!m_sourceFilter.isEmpty() && m_sourceFilterRE.isValid() && !m_sourceFilterRE.exactMatch(sourceName)) {
        return;
    }

    if (m_keyRoleFilter.isEmpty()) {
        //an item is represented by a source: keys are roles m_roleLevel == FirstLevel
        QVariantList list;

        if (!m_dataSource->data()->isEmpty()) {
            foreach (const QString &key, m_dataSource->data()->keys()) {
                if (!m_sourceFilter.isEmpty() && m_sourceFilterRE.isValid() && !m_sourceFilterRE.exactMatch(key)) {
                    continue;
                }
                QVariant value = m_dataSource->data()->value(key);
                if (value.isValid() && value.canConvert<Plasma::DataEngine::Data>()) {
                    Plasma::DataEngine::Data data = value.value<Plasma::DataEngine::Data>();
                    data[QStringLiteral("DataEngineSource")] = key;
                    list.append(data);
                }
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
            QVariantMap::const_iterator i;
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
        qWarning() << "Error: DataSource type expected";
        return;
    }
    if (m_dataSource == source) {
        return;
    }

    if (m_dataSource) {
        disconnect(m_dataSource, 0, this, 0);
    }

    m_dataSource = source;

    foreach (const QString &key, m_dataSource->data()->keys()) {
        dataUpdated(key, m_dataSource->data()->value(key).value<Plasma::DataEngine::Data>());
    }

    connect(m_dataSource, &DataSource::newData,
            this, &DataModel::dataUpdated);
    connect(m_dataSource, &DataSource::sourceRemoved,
            this, &DataModel::removeSource);
    connect(m_dataSource, &DataSource::sourceDisconnected,
            this, &DataModel::removeSource);
}

QObject *DataModel::dataSource() const
{
    return m_dataSource;
}

void DataModel::setKeyRoleFilter(const QString &key)
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

void DataModel::setSourceFilter(const QString &key)
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
    const int oldLength = m_items.value(sourceName).count();
    const int delta = list.length() - oldLength;
    const bool firstRun = m_items.isEmpty();

    //At what row number the first item associated to this source starts
    int sourceIndex = 0;
    QMap<QString, QVector<QVariant> >::const_iterator i;
    for (i = m_items.constBegin(); i != m_items.constEnd(); ++i) {
        if (i.key() == sourceName) {
            break;
        }
        sourceIndex += i.value().count();
    }
    //signal as inserted the rows at the end, all the other rows will signal a dataupdated.
    //better than a model reset because doesn't cause deletion and re-creation of every list item on a qml ListView, repeaters etc.
    //the first run it gets reset because otherwise setRoleNames gets broken
    if (firstRun) {
        beginResetModel();
    } else if (delta > 0) {
        beginInsertRows(QModelIndex(), sourceIndex + oldLength, sourceIndex + list.length() - 1);
    } else if (delta < 0) {
        beginRemoveRows(QModelIndex(), sourceIndex + list.length(), sourceIndex + oldLength - 1);
    }
    //convert to vector, so data() will be O(1)
    m_items[sourceName] = list.toVector();

    if (!list.isEmpty()) {
        if (list.first().canConvert<QVariantMap>()) {
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
    }

    setRoleNames(m_roleNames);

    if (firstRun) {
        endResetModel();
    } else if (delta > 0) {
        endInsertRows();
    } else if (delta < 0) {
        endRemoveRows();
    }
    emit dataChanged(createIndex(sourceIndex, 0),
                     createIndex(sourceIndex + qMin(list.length(), oldLength), 0));
}

void DataModel::removeSource(const QString &sourceName)
{
    //FIXME: find a way to remove only the proper things also in the case where sources are items

    if (m_keyRoleFilter.isEmpty()) {
        //source name in the map, linear scan
        for (int i = 0; i < m_items.value(QString()).count(); ++i) {
            if (m_items.value(QString())[i].value<QVariantMap>().value(QStringLiteral("DataEngineSource")) == sourceName) {
                beginRemoveRows(QModelIndex(), i, i);
                m_items[QString()].remove(i);
                endRemoveRows();
                break;
            }
        }
    } else {
        if (m_items.contains(sourceName)) {
            //At what row number the first item associated to this source starts
            int sourceIndex = 0;
            for (auto i = m_items.constBegin(); i != m_items.constEnd(); ++i) {
                if (i.key() == sourceName) {
                    break;
                }
                sourceIndex += i.value().count();
            }

            //source name as key of the map

            int count = m_items.value(sourceName).count();
            if (count > 0) {
                beginRemoveRows(QModelIndex(), sourceIndex, sourceIndex + count - 1);
            }
            m_items.remove(sourceName);
            if (count > 0) {
                endRemoveRows();
            }
        }
    }
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() > 0 ||
            index.row() < 0 || index.row() >= countItems()) {
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
    if (!m_keyRoleFilter.isEmpty() && m_roleNames.value(role) == "DataEngineSource") {
        return source;
    } else {
        return m_items.value(source).value(actualRow).value<QVariantMap>().value(QString::fromUtf8(m_roleNames.value(role)));
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

    return createIndex(row, column);
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

QVariantMap DataModel::get(int row) const
{
    QModelIndex idx = index(row, 0);
    QVariantMap map;

    const QHash<int, QByteArray> rNames = roleNames();
    for (auto i = rNames.constBegin(); i != rNames.constEnd(); ++i) {
        map[QString::fromUtf8(i.value())] = data(idx, i.key());
    }

    return map;
}

}

