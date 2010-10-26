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
#include <QVector>

#include <Plasma/DataEngine>

namespace Plasma
{

class DataSource;

class DataModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QObject *dataSource READ dataSource WRITE setDataSource)
    Q_PROPERTY(QString key READ key WRITE setKey)

public:
    DataModel(QObject* parent=0);
    ~DataModel();

    void setDataSource(QObject *source);
    QObject *dataSource() const;

    void setKey(const QString key);
    QString key() const;

    void setItems(const QVariantList &list);

    //Reimplemented
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

Q_SIGNALS:
    void modelAboutToBeReset();
    void modelReset();

private Q_SLOTS:
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

private:
    DataSource *m_dataSource;
    QString m_key;
    QVector<QVariant> m_items;
    QHash<int, QByteArray> m_roleNames;
};

}
#endif
