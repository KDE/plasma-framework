/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KCategorizedItemsView_H
#define KCategorizedItemsView_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>

#include "kcategorizeditemsviewmodels_p.h"

#include "ui_kcategorizeditemsviewbase.h"

using namespace KCategorizedItemsViewModels;
class KCategorizedItemsViewDelegate;

/**
 * QT4 View Widget for displaying a list of categorized items with special
 * filtering capabilities.
 *
 * To use it, you need to implement KCategorizedItemsView::AbstractItem
 * 
 * For convenience, there are default implementations of category model
 * (DefaultCategoryModel), filters model (DefaultFilterModel) and the
 * default item mode (DefaultItemModel), but you do not need to use them.
 * 
 * One constraint is that DefaultItemModel *must* hold the items that
 * are subclassed from KCategorizedItemsView::AbstractItem
 * 
 */
class KCategorizedItemsView: public QWidget, public Ui::KCategorizedItemsViewBase
{
    Q_OBJECT
public:

    explicit KCategorizedItemsView(QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~KCategorizedItemsView();

    void setFilterModel(QStandardItemModel * model);             ///< Sets the filters model
    void setItemModel(QStandardItemModel * model);               ///< Sets the item model, as mentioned items must implement AbstractItem class

    void addEmblem(const QString & title, QIcon * icon, const Filter & filter);
    void clearEmblems();

    QList < AbstractItem * > selectedItems() const;

protected:
    virtual void resizeEvent ( QResizeEvent * event );
    virtual bool event ( QEvent * event );

protected slots:
    void searchTermChanged(const QString &text);
    void filterChanged(int index);

Q_SIGNALS:
    void activated ( const QModelIndex & index );
    void clicked ( const QModelIndex & index );
    void doubleClicked ( const QModelIndex & index );
    void entered ( const QModelIndex & index );
    void pressed ( const QModelIndex & index );

private:
    void updateColumnsWidth(bool force = false);

    QStandardItemModel * m_modelCategories;
    QStandardItemModel * m_modelFilters;
    QStandardItemModel * m_modelItems;

    DefaultItemFilterProxyModel * m_modelFilterItems;
    KCategorizedItemsViewDelegate * m_delegate;

    int m_viewWidth;

    QMap < QString, QPair < Filter, QIcon * > > m_emblems;
    AbstractItem * getItemByProxyIndex(const QModelIndex & index) const;

    friend class KCategorizedItemsViewDelegate;
    friend class CustomDragTreeView;
};

//Q_DECLARE_METATYPE(KCategorizedItemsView::Filter)

#endif
