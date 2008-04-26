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

#include "kcategorizeditemsview_p.h"
#include "kcategorizeditemsviewdelegate_p.h"

#include <KIcon>
#include <KDebug>
#include <KAction>
#include <KStandardAction>

#define UNIVERSAL_PADDING 6

KCategorizedItemsView::KCategorizedItemsView(QWidget * parent, Qt::WindowFlags f)
        : QWidget(parent, f), m_modelCategories(NULL), m_modelFilters(NULL),
        m_modelItems(NULL), m_modelFilterItems(NULL), m_delegate(NULL),
        m_viewWidth(0)
{
    setupUi(this);
    itemsView->m_view = this;

    textSearch->setClickMessage(i18n("Enter search phrase here"));
    
    textSearch->setFocus();
    
    connect(textSearch, SIGNAL(textChanged(QString)),
            this, SLOT(searchTermChanged(QString)));
    connect(comboFilters, SIGNAL(currentIndexChanged(int)),
            this, SLOT(filterChanged(int)));

    // we filter "activated" signals to re-emit them only when wanted
    connect (itemsView, SIGNAL(activated(const QModelIndex &)),
                  this, SLOT(itemActivated(const QModelIndex &)));

    connect (itemsView, SIGNAL(clicked(const QModelIndex &)),
                  this, SIGNAL(clicked(const QModelIndex &)));
    connect (itemsView, SIGNAL(entered(const QModelIndex &)),
                  this, SIGNAL(entered(const QModelIndex &)));
    connect (itemsView, SIGNAL(pressed(const QModelIndex &)),
                  this, SIGNAL(pressed(const QModelIndex &)));

    itemsView->header()->setVisible(false);

    itemsView->setItemDelegate(m_delegate = new KCategorizedItemsViewDelegate(this));
    //itemsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect (m_delegate, SIGNAL(destroyApplets(const QString)),
                  parent, SLOT(destroyApplets(const QString)));

    comboFilters->setItemDelegate(new KCategorizedItemsViewFilterDelegate(this));

    itemsView->viewport()->setAttribute(Qt::WA_Hover);
    itemsView->setAlternatingRowColors(true);

    QAction * find = KStandardAction::find(textSearch, SLOT(setFocus()), this);
    addAction(find);
}

KCategorizedItemsView::~KCategorizedItemsView()
{
    delete m_modelFilterItems;
    delete m_delegate;
}

void KCategorizedItemsView::resizeEvent ( QResizeEvent * event )
{
    updateColumnsWidth();

    QWidget::resizeEvent(event);
}

bool KCategorizedItemsView::event ( QEvent * event )
{
    switch (event->type()) {
        case QEvent::PolishRequest:
        case QEvent::Polish:
            updateColumnsWidth(true);
            break;
        default:
            break;
    }

    return QWidget::event(event);
}

void KCategorizedItemsView::setFilterModel(QStandardItemModel * model)
{
    comboFilters->setModel(model);
    m_modelFilters = model;
}

void KCategorizedItemsView::setItemModel(QStandardItemModel * model)
{
    if (!m_modelFilterItems) {
        m_modelFilterItems = new DefaultItemFilterProxyModel(this);
    }

    m_modelItems = model;
    m_modelFilterItems->setSourceModel(m_modelItems);
    m_modelFilterItems->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_modelFilterItems->sort(0);

    itemsView->setModel(m_modelFilterItems);

    if (m_modelFilterItems->rowCount()) {
        itemsView->verticalScrollBar()->setSingleStep(itemsView->sizeHintForRow(0));
    }
}

void KCategorizedItemsView::searchTermChanged(const QString & text)
{
    kDebug() << "EVENT\n" << text;
    if (m_modelFilterItems) {
        m_modelFilterItems->setSearch(text);
    }
}

void KCategorizedItemsView::filterChanged(int index)
{
    if (m_modelFilterItems) {
        QVariant data = m_modelFilters->item(index)->data();
        m_modelFilterItems->setFilter(qVariantValue<KCategorizedItemsViewModels::Filter>(data));
    }
}

void KCategorizedItemsView::itemActivated( const QModelIndex& index )
{
    // don't emit activated signal for "favicon" and "remove applet"
    // columns so double clicking on these columns won't unexpectedly
    // add an applet to the containment
    if ( index.column() == 1 || index.column() == 2 ) {
        return;
    }

    emit activated(index);
}

void KCategorizedItemsView::updateColumnsWidth(bool force)
{
    m_viewWidth = itemsView->viewport()->width();

    if (force) {
        m_viewWidth -= style()->pixelMetric(QStyle::PM_ScrollBarExtent) + UNIVERSAL_PADDING;
    }

    itemsView->setColumnWidth(0, m_delegate->columnWidth(0, m_viewWidth));
    itemsView->setColumnWidth(1, m_delegate->columnWidth(1, m_viewWidth));
    itemsView->setColumnWidth(2, m_delegate->columnWidth(2, m_viewWidth));
}

void KCategorizedItemsView::addEmblem(const QString & title, QIcon * icon, const Filter & filter) {
    m_emblems[title] = QPair<Filter, QIcon *>(filter, icon);
}

void KCategorizedItemsView::clearEmblems() {
    m_emblems.clear();
}

AbstractItem * KCategorizedItemsView::getItemByProxyIndex(const QModelIndex & index) const {
    return (AbstractItem *) m_modelItems->itemFromIndex(
        m_modelFilterItems->mapToSource(index)
    );
}


QList < AbstractItem * > KCategorizedItemsView::selectedItems() const {
    QList < AbstractItem * > items;
    foreach (const QModelIndex &index, itemsView->selectionModel()->selectedIndexes()) {
        if (index.column() == 0) {
            items << getItemByProxyIndex(index);
        }
    }
    return items;
}

#include "kcategorizeditemsview_p.moc"

