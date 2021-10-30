/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractItemModel>
#include <QJSValue>
#include <QRegExp>
#include <QSortFilterProxyModel>
#include <QVector>

#include <Plasma/DataEngine>

class QTimer;

// All classes here will hopefully be removed in KF6 (along with DataEngines in general)

namespace Plasma
{
class DataSource;
class DataModel;

/**
 * @class SortFilterModel
 * @short Filter and sort an existing QAbstractItemModel
 */
class SortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The source model of this sorting proxy model. It has to inherit QAbstractItemModel (ListModel is not supported)
     */
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)

    /**
     * The regular expression for the filter, only items with their filterRole matching filterRegExp will be displayed
     */
    Q_PROPERTY(QString filterRegExp READ filterRegExp WRITE setFilterRegExp NOTIFY filterRegExpChanged)

    /**
     * The string for the filter, only items with their filterRole matching filterString will be displayed
     */
    Q_PROPERTY(QString filterString READ filterString WRITE setFilterString NOTIFY filterStringChanged REVISION 1)

    /**
     * A JavaScript callable that is passed the source model row index as first argument and the value
     * of filterRole as second argument. The callable's return value is evaluated as boolean to determine
     * whether the row is accepted (true) or filtered out (false). It overrides the default implementation
     * that uses filterRegExp or filterString; while filterCallable is set those two properties are
     * ignored. Attempts to write a non-callable to this property are silently ignored, but you can set
     * it to null.
     */
    Q_PROPERTY(QJSValue filterCallback READ filterCallback WRITE setFilterCallback NOTIFY filterCallbackChanged REVISION 1)

    /**
     * The role of the sourceModel on which filterRegExp must be applied.
     */
    Q_PROPERTY(QString filterRole READ filterRole WRITE setFilterRole)

    /**
     * The role of the sourceModel that will be used for sorting. if empty the order will be left unaltered
     */
    Q_PROPERTY(QString sortRole READ sortRole WRITE setSortRole)

    /**
     * One of Qt.Ascending or Qt.Descending
     */
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder)

    /**
     * Specify which column should be used for sorting
     */
    Q_PROPERTY(int sortColumn READ sortColumn WRITE setSortColumn NOTIFY sortColumnChanged)

    /**
     * How many items are in this model
     */
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    friend class DataModel;

public:
    explicit SortFilterModel(QObject *parent = nullptr);
    ~SortFilterModel() override;

    void setModel(QAbstractItemModel *source);

    void setFilterRegExp(const QString &exp);
    QString filterRegExp() const;

    void setFilterString(const QString &filterString);
    QString filterString() const;

    void setFilterCallback(const QJSValue &callback);
    QJSValue filterCallback() const;

    void setFilterRole(const QString &role);
    QString filterRole() const;

    void setSortRole(const QString &role);
    QString sortRole() const;

    void setSortOrder(const Qt::SortOrder order);

    void setSortColumn(int column);

    int count() const
    {
        return QSortFilterProxyModel::rowCount();
    }

    /**
     * Returns the item at index in the list model.
     * This allows the item data to be accessed (but not modified) from JavaScript.
     * It returns an Object with a property for each role.
     *
     * @param i the row we want
     */
    Q_INVOKABLE QVariantMap get(int i) const;

    Q_INVOKABLE int mapRowToSource(int i) const;

    Q_INVOKABLE int mapRowFromSource(int i) const;

Q_SIGNALS:
    void countChanged();
    void sortColumnChanged();
    void sourceModelChanged(QObject *);
    void filterRegExpChanged(const QString &);
    Q_REVISION(1) void filterStringChanged(const QString &);
    Q_REVISION(1) void filterCallbackChanged(const QJSValue &);

protected:
    int roleNameToId(const QString &name) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    QHash<int, QByteArray> roleNames() const override;

protected Q_SLOTS:
    void syncRoleNames();

private:
    QString m_filterRole;
    QString m_sortRole;
    QString m_filterString;
    QJSValue m_filterCallback;
    QHash<QString, int> m_roleIds;
};

/**
 * @class DataModel
 * @short DataSource data as a model
 */
class DataModel : public QAbstractItemModel
{
    Q_OBJECT

    /**
     * The instance of DataSource to construct this model on
     */
    Q_PROPERTY(QObject *dataSource READ dataSource WRITE setDataSource)

    /**
     * It's a regular expression. Only data with keys that match this filter
     * expression will be inserted in the model
     */
    Q_PROPERTY(QString keyRoleFilter READ keyRoleFilter WRITE setKeyRoleFilter)

    /**
     * It's a regular expression. If the DataSource is connected to more than one source,
     * only inserts data from sources matching this filter expression in the model. If we
     * want to have a source watch all sources beginning with say "name:", the required
     * regexp would be sourceFilter: "name:.*"
     */
    Q_PROPERTY(QString sourceFilter READ sourceFilter WRITE setSourceFilter)

    /**
     * How many items are in this model
     */
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    DataModel(QObject *parent = nullptr);
    ~DataModel() override;

    void setDataSource(QObject *source);
    QObject *dataSource() const;

    /**
     * Include only items with a key that matches this regexp in the model
     */
    void setKeyRoleFilter(const QString &key);
    QString keyRoleFilter() const;

    /**
     * Include only sources that matches this regexp in the model
     */
    void setSourceFilter(const QString &key);
    QString sourceFilter() const;

    // Reimplemented
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int count() const
    {
        return countItems();
    }

    /**
     * Returns the item at index in the list model.
     * This allows the item data to be accessed (but not modified) from JavaScript.
     * It returns an Object with a property for each role.
     *
     * @param i the row we want
     */
    Q_INVOKABLE QVariantMap get(int i) const;

protected:
    void setItems(const QString &sourceName, const QVariantList &list);
    inline int countItems() const;
    QHash<int, QByteArray> roleNames() const override;
Q_SIGNALS:
    void countChanged();
    void sourceModelChanged(QObject *);
    void filterRegExpChanged(const QString &);

private Q_SLOTS:
    void dataUpdated(const QString &sourceName, const QVariantMap &data);
    void removeSource(const QString &sourceName);

private:
    DataSource *m_dataSource;
    QString m_keyRoleFilter;
    QRegExp m_keyRoleFilterRE;
    QString m_sourceFilter;
    QRegExp m_sourceFilterRE;
    QMap<QString, QVector<QVariant>> m_items;
    QHash<int, QByteArray> m_roleNames;
    QHash<QString, int> m_roleIds;
    int m_maxRoleId;
};

int DataModel::countItems() const
{
    int count = 0;
    for (const QVector<QVariant> &v : std::as_const(m_items)) {
        count += v.count();
    }
    return count;
}

}
#endif
