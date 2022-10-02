/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>
    SPDX-FileCopyrightText: 2009 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the test suite of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1
*/

#ifndef DYNAMICTREEMODEL_H
#define DYNAMICTREEMODEL_H

#include <QAbstractItemModel>

#include <QHash>
#include <QList>

class DynamicTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DynamicTreeModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &index = QModelIndex()) const;
    int columnCount(const QModelIndex &index = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void clear();

protected Q_SLOTS:

    /**
    Finds the parent id of the string with id @p searchId.

    Returns -1 if not found.
    */
    qint64 findParentId(qint64 searchId) const;

private:
    QHash<qint64, QString> m_items;
    QHash<qint64, QList<QList<qint64>>> m_childItems;
    qint64 nextId;
    qint64 newId()
    {
        return nextId++;
    };

    QModelIndex m_nextParentIndex;
    int m_nextRow;

    int m_depth;
    int maxDepth;

    friend class ModelInsertCommand;
    friend class ModelMoveCommand;
    friend class ModelResetCommand;
    friend class ModelResetCommandFixed;
};

class ModelChangeCommand : public QObject
{
    Q_OBJECT
public:
    explicit ModelChangeCommand(DynamicTreeModel *model, QObject *parent = nullptr);

    virtual ~ModelChangeCommand()
    {
    }

    void setAncestorRowNumbers(QList<int> rowNumbers)
    {
        m_rowNumbers = rowNumbers;
    }

    QModelIndex findIndex(QList<int> rows);

    void setStartRow(int row)
    {
        m_startRow = row;
    }

    void setEndRow(int row)
    {
        m_endRow = row;
    }

    void setNumCols(int cols)
    {
        m_numCols = cols;
    }

    virtual void doCommand() = 0;

protected:
    DynamicTreeModel *m_model;
    QList<int> m_rowNumbers;
    int m_numCols;
    int m_startRow;
    int m_endRow;
};

typedef QList<ModelChangeCommand *> ModelChangeCommandList;

class ModelInsertCommand : public ModelChangeCommand
{
    Q_OBJECT

public:
    explicit ModelInsertCommand(DynamicTreeModel *model, QObject *parent = nullptr);
    virtual ~ModelInsertCommand()
    {
    }

    void doCommand() override;
};

class ModelMoveCommand : public ModelChangeCommand
{
    Q_OBJECT
public:
    ModelMoveCommand(DynamicTreeModel *model, QObject *parent);

    virtual ~ModelMoveCommand()
    {
    }

    virtual bool emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow);

    void doCommand() override;

    virtual void emitPostSignal();

    void setDestAncestors(QList<int> rows)
    {
        m_destRowNumbers = rows;
    }

    void setDestRow(int row)
    {
        m_destRow = row;
    }

protected:
    QList<int> m_destRowNumbers;
    int m_destRow;
};

/**
  A command which does a move and emits a reset signal.
*/
class ModelResetCommand : public ModelMoveCommand
{
    Q_OBJECT
public:
    explicit ModelResetCommand(DynamicTreeModel *model, QObject *parent = nullptr);

    virtual ~ModelResetCommand();

    bool emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow) override;
    void emitPostSignal() override;
};

/**
  A command which does a move and emits a beginResetModel and endResetModel signals.
*/
class ModelResetCommandFixed : public ModelMoveCommand
{
    Q_OBJECT
public:
    explicit ModelResetCommandFixed(DynamicTreeModel *model, QObject *parent = nullptr);

    virtual ~ModelResetCommandFixed();

    bool emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow) override;
    void emitPostSignal() override;
};

#endif
