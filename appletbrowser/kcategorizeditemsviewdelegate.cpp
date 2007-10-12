/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free 
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kcategorizeditemsviewdelegate_p.h"

#include <cmath>

#include <QtCore>

#include <KIconLoader>

#include "kcategorizeditemsview_p.h"

#define FAV_ICON_SIZE 24
#define EMBLEM_ICON_SIZE 16
#define UNIVERSAL_PADDING 6
#define FADE_LENGTH 32
#define MAIN_ICON_SIZE 48
#define DROPDOWN_PADDING 2
#define DROPDOWN_SEPARATOR_HEIGHT 32

KCategorizedItemsViewDelegate::KCategorizedItemsViewDelegate(QObject * parent)
        : QItemDelegate(parent), m_favoriteIcon("bookmark"), 
        m_favoriteAddIcon("edit-add"), m_favoriteRemoveIcon("edit-delete"),
        m_onFavoriteIconItem(NULL)
{
    m_parent = (KCategorizedItemsView *) parent;
}

void KCategorizedItemsViewDelegate::paint(QPainter *painter,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    // Preparing needed data for painting
    int left = option.rect.left();
    int top = option.rect.top();
    int width = option.rect.width();
    int height = option.rect.height();
    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);

    QIcon::Mode iconMode = QIcon::Normal;

    QColor backgroundColor = (option.state.testFlag(QStyle::State_Selected))?
            option.palette.color(QPalette::Highlight):option.palette.color(QPalette::Base);
    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
            option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);

    KCategorizedItemsViewModels::AbstractItem * item =
        getItemByProxyIndex(index);
    if (!item) return;

    // Base Background
    painter->fillRect(option.rect, QBrush(backgroundColor));

    if (index.column() == 0) {
        // Painting main column             
        QStyleOptionViewItem local_option_title(option);
        QStyleOptionViewItem local_option_normal(option);

        local_option_title.font.setBold(true);
        local_option_title.font.setPointSize(local_option_title.font.pointSize() + 2);

        QLinearGradient gradient;

        QString title = item->name();
        QString description = item->description();

        // Painting

        // Text
        int textInner = 2 * UNIVERSAL_PADDING + MAIN_ICON_SIZE;

        painter->setPen(foregroundColor);
        painter->setFont(local_option_title.font);
        painter->drawText(
            left + (leftToRight ? textInner : 0),
            top + UNIVERSAL_PADDING,
            width - textInner, MAIN_ICON_SIZE / 2,
            Qt::AlignBottom | Qt::AlignLeft, title);
        painter->setFont(local_option_normal.font);
        painter->drawText(
            left + (leftToRight ? textInner : 0),
            top + UNIVERSAL_PADDING + MAIN_ICON_SIZE / 2,
            width - textInner, MAIN_ICON_SIZE / 2,
            Qt::AlignTop | Qt::AlignLeft, description);

        // Main icon
        item->icon().paint(painter, 
                leftToRight ? left + UNIVERSAL_PADDING : left + width - UNIVERSAL_PADDING - MAIN_ICON_SIZE,
            top + UNIVERSAL_PADDING, 
            MAIN_ICON_SIZE, MAIN_ICON_SIZE, Qt::AlignCenter, iconMode);

        // Counting the number of emblems for this item
        int emblemCount = 0;
        QPair < Filter, QIcon * > emblem;
        foreach (emblem, m_parent->m_emblems) {
            if (item->passesFiltering(emblem.first)) ++emblemCount;
        }

        // Gradient part of the background - fading of the text at the end
        if (leftToRight) {
            gradient = QLinearGradient(left + width - UNIVERSAL_PADDING - FADE_LENGTH, 0, 
                left + width - UNIVERSAL_PADDING, 0);
            gradient.setColorAt(1, backgroundColor);
            backgroundColor.setAlpha(0);
            gradient.setColorAt(0, backgroundColor);
        } else {
            gradient = QLinearGradient(left + UNIVERSAL_PADDING, 0, 
                left + UNIVERSAL_PADDING + FADE_LENGTH, 0);
            gradient.setColorAt(0, backgroundColor);
            backgroundColor.setAlpha(0);
            gradient.setColorAt(1, backgroundColor);

        }

        QRect paintRect = option.rect;
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->fillRect(paintRect, gradient);

        if (leftToRight) {
            gradient.setStart(left + width
                - emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE) - FADE_LENGTH, 0);
            gradient.setFinalStop(left + width
                - emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE), 0);
        } else {
            gradient.setStart(left + UNIVERSAL_PADDING
                + emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE), 0);
            gradient.setFinalStop(left + UNIVERSAL_PADDING
                + emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE) + FADE_LENGTH, 0);
        }
        paintRect.setHeight(UNIVERSAL_PADDING + MAIN_ICON_SIZE / 2);
        painter->fillRect(paintRect, gradient);

        // Emblems icons
        int emblemLeft = leftToRight ? (left + width - EMBLEM_ICON_SIZE) : left; // - FAV_ICON_SIZE - 2 * UNIVERSAL_PADDING
        foreach (emblem, m_parent->m_emblems) {
            if (item->passesFiltering(emblem.first)) {
                emblem.second->paint(painter, 
                    emblemLeft, top + UNIVERSAL_PADDING, 
                    EMBLEM_ICON_SIZE, EMBLEM_ICON_SIZE, Qt::AlignCenter, iconMode);
                if (leftToRight) {
                    emblemLeft -= UNIVERSAL_PADDING + EMBLEM_ICON_SIZE;
                } else {
                    emblemLeft += UNIVERSAL_PADDING + EMBLEM_ICON_SIZE;
                }
            }
        }

    } else {
        // Painting favorite icon column

        KCategorizedItemsViewModels::AbstractItem * item =
            getItemByProxyIndex(index);

        if (! (option.state & QStyle::State_MouseOver) && m_onFavoriteIconItem == item)
            m_onFavoriteIconItem = NULL;

        QIcon::Mode iconMode = QIcon::Normal;
        if (!item->isFavorite()) {
          iconMode = QIcon::Disabled;
        } else if (option.state & QStyle::State_MouseOver) {
          iconMode = QIcon::Active;
        } 

        m_favoriteIcon.paint(painter, 
            left + width - FAV_ICON_SIZE - UNIVERSAL_PADDING, top + UNIVERSAL_PADDING, 
            FAV_ICON_SIZE, FAV_ICON_SIZE, Qt::AlignCenter, iconMode);

        const KIcon * icon = (item->isFavorite())? & m_favoriteRemoveIcon : & m_favoriteAddIcon;

        if ((option.state & QStyle::State_MouseOver) && (m_onFavoriteIconItem != item))
            icon->paint(painter, 
                left + width - EMBLEM_ICON_SIZE - UNIVERSAL_PADDING, top + UNIVERSAL_PADDING, 
                EMBLEM_ICON_SIZE, EMBLEM_ICON_SIZE, Qt::AlignCenter, iconMode);

    }

    // Dividing line 
    backgroundColor = option.palette.color(QPalette::Highlight);
    backgroundColor.setAlpha(100);
    painter->setPen(backgroundColor);
    painter->drawLine(left, top + height - 1, left + width, top + height - 1);

}

bool KCategorizedItemsViewDelegate::editorEvent(QEvent *event,
                               QAbstractItemModel *model,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index)
{
    if (index.column() == 1 && event->type() == QEvent::MouseButtonPress) {
        (m_onFavoriteIconItem = getItemByProxyIndex(index))
             ->setFavorite(!getItemByProxyIndex(index)->isFavorite());

        return true;
    }

    return QItemDelegate::editorEvent(event, model, option, index);
}

QSize KCategorizedItemsViewDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index ) const
{
    Q_UNUSED(option);

    //Q_UNUSED(index);
    //option.
    int width = (index.column() == 0) ? 0 : FAV_ICON_SIZE;
    return QSize(width, MAIN_ICON_SIZE + 2 * UNIVERSAL_PADDING);
}

int KCategorizedItemsViewDelegate::columnWidth (int column, int viewWidth) const {
    if (column == 1) {
        return FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING;
    } else return viewWidth - columnWidth(1, viewWidth);
}


KCategorizedItemsViewModels::AbstractItem * KCategorizedItemsViewDelegate::getItemByProxyIndex(const QModelIndex & index) const { 
    return (AbstractItem *) m_parent->m_modelItems->itemFromIndex(
        m_parent->m_modelFilterItems->mapToSource(index)
    );
}

//     KCategorizedItemsViewFilterDelegate


KCategorizedItemsViewFilterDelegate::KCategorizedItemsViewFilterDelegate(QObject *parent)
    : QItemDelegate(parent) {
    kDebug() << "KCategorizedItemsViewFilterDelegate(QObject *parent)\n";

}

void KCategorizedItemsViewFilterDelegate::paint(QPainter *painter,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.flags() & Qt::ItemIsEnabled) {
        QItemDelegate::paint(painter, option, index);
    } else {
        QStyleOptionViewItem separatorOption(option);
        int height = QItemDelegate::sizeHint(option, index).height() + 2 * DROPDOWN_PADDING;

        separatorOption.state &= ~(QStyle::State_Selected
                | QStyle::State_MouseOver | QStyle::State_HasFocus);
        separatorOption.rect.setTop(separatorOption.rect.top() + separatorOption.rect.height() - height);
        separatorOption.rect.setHeight(height);
        QItemDelegate::paint(painter, separatorOption, index);
        /*painter->drawLine(
                option.rect.left(), 
                option.rect.top() + 1,
                option.rect.left() + option.rect.width(),
                option.rect.top() + 1);*/
    }
}

QSize KCategorizedItemsViewFilterDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index ) const
{
    QSize size = QItemDelegate::sizeHint(option, index);
    if (index.flags() & Qt::ItemIsEnabled) {
        size.setHeight(size.height() + 2 * DROPDOWN_PADDING);
    } else {
        size.setHeight(DROPDOWN_SEPARATOR_HEIGHT);
    }
    return size;
}

#include "kcategorizeditemsviewdelegate_p.moc"

