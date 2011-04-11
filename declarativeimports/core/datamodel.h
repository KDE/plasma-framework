/*
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

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QVector>

#include <Plasma/DataEngine>

class QTimer;

namespace Plasma
{

class DataSource;
class DataModel;


class SortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QObject *sourceModel READ sourceModel WRITE setModel)

    Q_PROPERTY(QString filterRegExp READ filterRegExp WRITE setFilterRegExp)
    Q_PROPERTY(QString filterRole READ filterRole WRITE setFilterRole)
    Q_PROPERTY(QString sortRole READ sortRole WRITE setSortRole)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    friend class DataModel;

public:
    SortFilterModel(QObject* parent=0);
    ~SortFilterModel();

    //FIXME: find a way to make QML understnd QAbstractItemModel
    void setModel(QObject *source);

    void setFilterRegExp(const QString &exp);
    QString filterRegExp() const;

    void setFilterRole(const QString &role);
    QString filterRole() const;

    void setSortRole(const QString &role);
    QString sortRole() const;

    void setSortOrder(const Qt::SortOrder order);

    int count() const {return QSortFilterProxyModel::rowCount();}

Q_SIGNALS:
    void countChanged();

protected:
    int roleNameToId(const QString &name);

protected Q_SLOTS:
    void syncRoleNames();

private:
    QString m_filterRole;
    QString m_sortRole;
    QHash<QString, int> m_roleIds;
};

class DataModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QObject *dataSource READ dataSource WRITE setDataSource)
    Q_PROPERTY(QString keyRoleFilter READ keyRoleFilter WRITE setKeyRoleFilter)
    Q_PROPERTY(QString sourceFilter READ sourceFilter WRITE setSourceFilter)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    DataModel(QObject* parent=0);
    ~DataModel();

    void setDataSource(QObject *source);
    QObject *dataSource() const;

    /**
     * Include only items with a key that matches this regexp in the model
     */
    void setKeyRoleFilter(const QString& key);
    QString keyRoleFilter() const;

    /**
     * Include only items with a source name that matches this regexp in the model
     * @since 4.7
     */
    void setSourceFilter(const QString& key);
    QString sourceFilter() const;

    int roleNameToId(const QString &name);

    //Reimplemented
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    int count() const {return countItems();}

protected:
    void setItems(const QString &sourceName, const QVariantList &list);
    inline int countItems() const;

Q_SIGNALS:
    void modelAboutToBeReset();
    void modelReset();
    void countChanged();

private Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);
    void removeSource(const QString &sourceName);
    void syncRoleNames();

private:
    DataSource *m_dataSource;
    QString m_keyRoleFilter;
    QString m_sourceFilter;
    QTimer *m_roleNamesTimer;
    QMap<QString, QVector<QVariant> > m_items;
    QHash<int, QByteArray> m_roleNames;
    QHash<QString, int> m_roleIds;
    int m_maxRoleId;
};

int DataModel::countItems() const
{
    int count = 0;
    foreach (const QVector<QVariant> &v, m_items) {
        count += v.count();
    }
    return count;
}

}
#endif
