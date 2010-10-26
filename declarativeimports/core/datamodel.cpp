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
    : QAbstractItemModel(parent),
      m_dataSource(0)
{
    setObjectName("DataModel");
}

DataModel::~DataModel()
{
}

void DataModel::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
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

    disconnect(m_dataSource, 0, this, 0);
    m_dataSource = source;
    connect(m_dataSource, SIGNAL(newData(const QString &, const Plasma::DataEngine::Data &)),
            this, SLOT(dataUpdated(const QString &, const Plasma::DataEngine::Data &)));
}

QObject *DataModel::dataSource() const
{
    return m_dataSource;
}

void DataModel::setKey(const QString key)
{
    if (m_key == key) {
        return;
    }

    m_key = key;
}

QString DataModel::key() const
{
    return m_key;
}

void DataModel::setItems(const QVariantList &list)
{
    emit modelAboutToBeReset();

    //convert to vector, so data() will be O(1)
    m_items = list.toVector();

    if (!list.isEmpty()) {
        int role = Qt::UserRole;
        m_roleNames.clear();
        foreach (QString roleName, list.first().value<QVariantMap>().keys()) {
            ++role;
            m_roleNames[role] = roleName.toLatin1();
        }
        setRoleNames(m_roleNames);
    }

    //make the declarative view reload everything,
    //would be nice an incremental update but is not possible
    emit modelReset();
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() > 0 ||
        index.row() < 0 || index.row() >= m_items.count()){
        return QVariant();
    }

    return m_items.value(index.row()).value<QVariantMap>().value(m_roleNames.value(role));
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
    if (parent.isValid() || column > 0 || row < 0 || row >= m_items.count()) {
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

    return m_items.count();
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 1;
}

}
#include "datamodel.moc"
