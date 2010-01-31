/*
    Copyright (C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stackedlayout.h"

#include <QGraphicsWidget>
#include <QDebug>

StackedLayout::StackedLayout(QGraphicsLayoutItem *parent)
    : QObject(0),
      QGraphicsLayout(parent),
      m_currentWidgetIndex(-1)
{
}

StackedLayout::~StackedLayout()
{
}

void StackedLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);

    const QRectF effectiveRect = geometry();

    for(int i = 0; i < items.size(); i++) {
        itemAt(i)->setGeometry(effectiveRect);
    }
}

QSizeF StackedLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    if (m_currentWidgetIndex <= 0 || !itemAt(m_currentWidgetIndex)) {
        return QSizeF();
    }

    QSizeF currentWidgetSize = itemAt(m_currentWidgetIndex)->effectiveSizeHint(which, constraint);

    return QSizeF( left + right + currentWidgetSize.width(), right + bottom + currentWidgetSize.height());
}

int StackedLayout::count() const
{
    return items.count();
}

QGraphicsLayoutItem *StackedLayout::itemAt(int i) const
{
    return items.at(i);
}

void StackedLayout::insertWidget(QGraphicsLayoutItem *item, int pos)
{
    if(!pos && (m_currentWidgetIndex == -1)) {
        m_currentWidgetIndex = 0;
    } else {
        item->graphicsItem()->hide();
    }

    items.insert(pos, item);
    activate();
}

void StackedLayout::addWidget(QGraphicsLayoutItem *item)
{
    insertWidget(item, items.size());
}

void StackedLayout::removeAt(int index)
{
    items.removeAt(index);
}

void StackedLayout::setCurrentWidgetIndex(qint32 index)
{
    QGraphicsItem *currentWidget = itemAt(m_currentWidgetIndex)->graphicsItem();
    QGraphicsItem *hiddenWidget = itemAt(index)->graphicsItem();

    currentWidget->hide();
    hiddenWidget->show();

    m_currentWidgetIndex = index;
}

qint32 StackedLayout::currentWidgetIndex() const
{
    return m_currentWidgetIndex;
}
