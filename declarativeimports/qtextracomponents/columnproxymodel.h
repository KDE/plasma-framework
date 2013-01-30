/*
 *   Copyright 2012 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>
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

#ifndef COLUMNPROXYMODEL_H
#define COLUMNPROXYMODEL_H

#include <QAbstractListModel>

class ColumnProxyModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
//     Q_PROPERTY(QAbstractItemModel* sourceModel READ sourceModel WRITE setSourceModel) //rootIndex sets the model
    Q_PROPERTY(int column READ column WRITE setColumn)
    public:
        ColumnProxyModel(QObject* parent = 0);

        void setRootIndex(const QModelIndex& idx);
        QModelIndex rootIndex() const;

        void setSourceModel(QAbstractItemModel* sourceModel);
        QAbstractItemModel* sourceModel() const { return m_sourceModel; }

        int column() const;
        void setColumn(int col);

        Q_SCRIPTABLE static QModelIndex indexFromModel(QAbstractItemModel* model, int row, int column=0, const QModelIndex& parent=QModelIndex());
        Q_SCRIPTABLE QModelIndex indexAt(int row, const QModelIndex& parent = QModelIndex()) const;

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    signals:
        void rootIndexChanged();

    private:
        QModelIndex proxyIndex(const QModelIndex& sourceIndex) const;
        QModelIndex sourceIndex(const QModelIndex& proxyIndex) const;

        int m_column;
        QModelIndex m_index;
        QAbstractItemModel* m_sourceModel;

    private slots:
        void considerRowsAboutToBeInserted(const QModelIndex&,int,int);
        void considerRowsAboutToBeMoved(const QModelIndex& sourceParent, int rA, int rB, const QModelIndex& destParent, int rD);
        void considerRowsAboutToBeRemoved(const QModelIndex&,int,int);
        void considerRowsRemoved(const QModelIndex&,int,int);
        void considerRowsMoved(const QModelIndex&,int,int,const QModelIndex&,int);
        void considerRowsInserted(const QModelIndex&,int,int);
        void considerDataChanged(const QModelIndex& idxA, const QModelIndex& idxB);
        void sourceDestroyed(QObject* source);
};

#endif
