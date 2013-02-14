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

#include "columnproxymodel.h"
#include <KDebug>

ColumnProxyModel::ColumnProxyModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_column(0)
    , m_sourceModel(0)
{}

void ColumnProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    if(sourceModel==m_sourceModel) {
        return;
    }
    
    beginResetModel();
    if(m_sourceModel) {
        disconnect(m_sourceModel, SIGNAL(destroyed(QObject*)),
                this, SLOT(sourceDestroyed(QObject*)));
        
        disconnect(m_sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(considerDataChanged(QModelIndex,QModelIndex)));
        disconnect(m_sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(considerRowsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(m_sourceModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(considerRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(m_sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(considerRowsAboutToBeRemoved(QModelIndex,int,int)));        
        disconnect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(considerRowsInserted(QModelIndex,int,int)));
        disconnect(m_sourceModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(considerRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(considerRowsRemoved(QModelIndex,int,int)));
        
        disconnect(m_sourceModel, SIGNAL(modelAboutToBeReset()),
                this, SIGNAL(modelAboutToBeReset()));
        disconnect(m_sourceModel, SIGNAL(modelReset()),
                this, SIGNAL(modelReset()));
        disconnect(m_sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
        disconnect(m_sourceModel, SIGNAL(layoutAboutToBeChanged()),
                this, SIGNAL(layoutAboutToBeChanged()));
        disconnect(m_sourceModel, SIGNAL(layoutChanged()),
                this, SIGNAL(layoutChanged()));
    }
    m_sourceModel = sourceModel;
    if(m_sourceModel) {
        setRoleNames(m_sourceModel->roleNames());
        connect(m_sourceModel, SIGNAL(destroyed(QObject*)),
                this, SLOT(sourceDestroyed(QObject*)));
        
        connect(m_sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(considerDataChanged(QModelIndex,QModelIndex)));
        connect(m_sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(considerRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(m_sourceModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(considerRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(m_sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(considerRowsAboutToBeRemoved(QModelIndex,int,int)));        
        connect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(considerRowsInserted(QModelIndex,int,int)));
        connect(m_sourceModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                this, SLOT(considerRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(considerRowsRemoved(QModelIndex,int,int)));
        
        connect(m_sourceModel, SIGNAL(modelAboutToBeReset()),
                this, SIGNAL(modelAboutToBeReset()));
        connect(m_sourceModel, SIGNAL(modelReset()),
                this, SIGNAL(modelReset()));
        connect(m_sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
        connect(m_sourceModel, SIGNAL(layoutAboutToBeChanged()),
                this, SIGNAL(layoutAboutToBeChanged()));
        connect(m_sourceModel, SIGNAL(layoutChanged()),
                this, SIGNAL(layoutChanged()));
    }
    endResetModel();
}

void ColumnProxyModel::setColumn(int col)
{
    beginResetModel();
    m_column = col;
    endResetModel();
}

int ColumnProxyModel::column() const
{
    return m_column;
}

QModelIndex ColumnProxyModel::rootIndex() const
{
    return m_index;
}

void ColumnProxyModel::setRootIndex(const QModelIndex& index)
{
    if(index.isValid()) {
        setSourceModel(const_cast<QAbstractItemModel*>(index.model()));
    }
    beginResetModel();
    m_index = index;
    endResetModel();
    
    emit rootIndexChanged();
}

QModelIndex ColumnProxyModel::indexFromModel(QAbstractItemModel* model, int row, int column, const QModelIndex& parent)
{
    return model ? model->index(row, column, parent) : QModelIndex();
}

QVariant ColumnProxyModel::data(const QModelIndex& index, int role) const
{
    return m_sourceModel ? m_sourceModel->data(sourceIndex(index), role) : QVariant();
}

QVariant ColumnProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return m_sourceModel ? m_sourceModel->headerData(section, orientation, role) : QVariant();
}

QModelIndex ColumnProxyModel::sourceIndex(const QModelIndex& proxyIndex) const
{
    return m_sourceModel ? m_sourceModel->index(proxyIndex.row(), m_column, m_index) : QModelIndex();
}

int ColumnProxyModel::rowCount(const QModelIndex& parent) const
{
    return (!m_sourceModel || parent.isValid()) ? 0 : m_sourceModel->rowCount(m_index);
}

QModelIndex ColumnProxyModel::proxyIndex(const QModelIndex& sourceIndex) const
{
    if(sourceIndex.parent()==m_index)
        return index(sourceIndex.row(), sourceIndex.column(), QModelIndex());
    
    return QModelIndex();
}

void ColumnProxyModel::sourceDestroyed(QObject* source)
{
    Q_ASSERT(source==m_sourceModel);
    
    beginResetModel();
    m_sourceModel = 0;
    endResetModel();
}

QModelIndex ColumnProxyModel::indexAt(int row, const QModelIndex& parent) const
{
    return m_sourceModel ? m_sourceModel->index(row, m_column, parent) : QModelIndex();
}

/////////////////

void ColumnProxyModel::considerDataChanged(const QModelIndex& idxA, const QModelIndex& idxB)
{
    if(idxA.parent()==m_index && idxB.parent()==m_index) {
        emit dataChanged(proxyIndex(idxA), proxyIndex(idxB));
    }
}

void ColumnProxyModel::considerRowsAboutToBeInserted(const QModelIndex& parent, int rA, int rB)
{
    if(parent==m_index) {
        beginInsertRows(QModelIndex(), rA, rB);
    }
}

void ColumnProxyModel::considerRowsAboutToBeMoved(const QModelIndex &sourceParent, int rA, int rB, const QModelIndex& destParent, int rD)
{
    if(sourceParent==m_index && destParent==m_index) {
        beginMoveRows(QModelIndex(), rA, rB, QModelIndex(), rD);
    } else if(sourceParent==m_index) {
        beginRemoveRows(sourceParent, rA, rB); 
    } else if(destParent==m_index) {
        beginInsertRows(destParent, rD, rD+(rB-rA));
    }
}

void ColumnProxyModel::considerRowsAboutToBeRemoved(const QModelIndex& parent, int rA, int rB)
{
    if(parent==m_index) {
        beginRemoveRows(QModelIndex(), rA, rB);
    }
}

void ColumnProxyModel::considerRowsInserted(const QModelIndex& parent, int , int )
{
    if(parent==m_index) {
        endInsertRows();
    }
}

void ColumnProxyModel::considerRowsMoved(const QModelIndex& sourceParent, int , int , const QModelIndex& destParent, int )
{
    if(sourceParent==m_index && destParent==m_index) {
        endMoveRows();
    } else if(sourceParent==m_index) {
        endRemoveRows();
    } else if(destParent==m_index) {
        endInsertRows();
    }
}

void ColumnProxyModel::considerRowsRemoved(const QModelIndex& parent, int , int )
{
    if(parent==m_index) {
        endInsertRows();
    }
}

#include "columnproxymodel.moc"
