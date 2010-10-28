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

#include <KDebug>

namespace Plasma
{

DataModel::DataModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setObjectName("DataModel");
    setDynamicSortFilter(true);
    m_internalDataModel = new InternalDataModel(this);
    setSourceModel(m_internalDataModel);
}

DataModel::~DataModel()
{
}

void DataModel::setDataSource(QObject *source)
{
    m_internalDataModel->setDataSource(source);
}

QObject *DataModel::dataSource() const
{
    return m_internalDataModel->dataSource();
}

void DataModel::setKey(const QString key)
{
    m_internalDataModel->setKey(key);
}

QString DataModel::key() const
{
    return m_internalDataModel->key();
}

void DataModel::setFilterRegExp(const QString &exp)
{
    QSortFilterProxyModel::setFilterRegExp(QRegExp(exp));
}

QString DataModel::filterRegExp() const
{
    return QSortFilterProxyModel::filterRegExp().pattern();
}

void DataModel::setFilterRole(const QString &role)
{
    QSortFilterProxyModel::setFilterRole(m_internalDataModel->roleNameToId(role));
    m_filterRole = role;
}

QString DataModel::filterRole() const
{
    return m_filterRole;
}

void DataModel::setSortRole(const QString &role)
{
    QSortFilterProxyModel::setSortRole(m_internalDataModel->roleNameToId(role));
    m_sortRole = role;
    sort(0, sortOrder());
}

QString DataModel::sortRole() const
{
    return m_sortRole;
}

void DataModel::setSortOrder(const Qt::SortOrder order)
{
    sort(0, order);
}




InternalDataModel::InternalDataModel(DataModel* parent)
    : QAbstractItemModel(parent),
      m_dataModel(parent),
      m_dataSource(0)
{
}

InternalDataModel::~InternalDataModel()
{
}

void InternalDataModel::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName);

    if (data.contains(m_key) && data.value(m_key).canConvert<QVariantList>()) {
        setItems(data.value(m_key).value<QVariantList>());
    } else {
        QRegExp regExp(m_key);
        if (!regExp.isValid()) {
            return;
        }

        QHash<QString, QVariant>::const_iterator i;
        QVariantList list;
        for (i = data.constBegin(); i != data.constEnd(); ++i) {
            if (regExp.exactMatch(i.key())) {
                list.append(i.value());
            }
        }
        setItems(list);
    }
}

void InternalDataModel::setDataSource(QObject *object)
{
    DataSource *source = qobject_cast<DataSource *>(object);
    if (!source) {
        kWarning() << "Error: DataSource type expected";
        return;
    }
    if (m_dataSource == source) {
        return;
    }

    disconnect(m_dataSource, 0, this, 0);
    m_dataSource = source;
    connect(m_dataSource, SIGNAL(newData(const QString &, const Plasma::DataEngine::Data &)),
            this, SLOT(dataUpdated(const QString &, const Plasma::DataEngine::Data &)));
}

QObject *InternalDataModel::dataSource() const
{
    return m_dataSource;
}

void InternalDataModel::setKey(const QString key)
{
    if (m_key == key) {
        return;
    }

    m_key = key;
}

QString InternalDataModel::key() const
{
    return m_key;
}

void InternalDataModel::setItems(const QVariantList &list)
{
    emit modelAboutToBeReset();

    //convert to vector, so data() will be O(1)
    m_items = list.toVector();

    if (!list.isEmpty()) {
        int role = Qt::UserRole;
        m_roleNames.clear();
        m_roleIds.clear();

        if (list.first().canConvert<QVariantHash>()) {
            foreach (const QString& roleName, list.first().value<QVariantHash>().keys()) {
                ++role;
                m_roleNames[role] = roleName.toLatin1();
                m_roleIds[roleName] = role;
            }
        } else {
            foreach (const QString& roleName, list.first().value<QVariantMap>().keys()) {
                ++role;
                m_roleNames[role] = roleName.toLatin1();
                m_roleIds[roleName] = role;
            }
        }

        setRoleNames(m_roleNames);

        if (m_dataModel) {
            m_dataModel->setRoleNames(m_roleNames);
            //FIXME: ugly, but since the filter role can be set before population it can be re setted afterwards
            m_dataModel->setFilterRole(m_dataModel->filterRole());
            m_dataModel->setSortRole(m_dataModel->sortRole());
        }

    }

    //make the declarative view reload everything,
    //would be nice an incremental update but is not possible
    emit modelReset();
}

QVariant InternalDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() > 0 ||
        index.row() < 0 || index.row() >= m_items.count()){
        return QVariant();
    }

    if (m_items.value(index.row()).canConvert<QVariantHash>()) {
        return m_items.value(index.row()).value<QVariantHash>().value(m_roleNames.value(role));
    } else {
        return m_items.value(index.row()).value<QVariantMap>().value(m_roleNames.value(role));
    }
}

QVariant InternalDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

QModelIndex InternalDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || column > 0 || row < 0 || row >= m_items.count()) {
        return QModelIndex();
    }

    return createIndex(row, column, 0);
}

QModelIndex InternalDataModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)

    return QModelIndex();
}

int InternalDataModel::rowCount(const QModelIndex &parent) const
{
    //this is not a tree
    //TODO: make it possible some day?
    if (parent.isValid()) {
        return 0;
    }

    return m_items.count();
}

int InternalDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 1;
}

int InternalDataModel::roleNameToId(const QString &name)
{
    if (!m_roleIds.contains(name)) {
        return -1;
    }
    return m_roleIds.value(name);
}

}
#include "datamodel.moc"
