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

#define PIX_SIZE 64
#define MAX_OFFSET 16
#define MAX_COUNT 5

CustomDragTreeView::CustomDragTreeView(QWidget * parent)
    : QTreeView(parent) {}

void CustomDragTreeView::startDrag ( Qt::DropActions supportedActions )
{
    Q_UNUSED(supportedActions);

    // TODO: calculate real size for pixmap - using the icon sizes, not fixed
    //       like now

    if (!m_view) return;

    QModelIndexList indexes = selectedIndexes();
    if (indexes.count() > 0) {
        QMimeData *data = model()->mimeData(indexes);
        if (!data) {
            return;
        }

        int size = PIX_SIZE + (qMin(MAX_COUNT, indexes.count()) * MAX_OFFSET);
        int off  = MAX_OFFSET;
        if (indexes.count() > MAX_COUNT) {
            off = (MAX_OFFSET * MAX_COUNT) / indexes.count();
        }

        kDebug() << "Size: " << size << " Off: " << off << "\n";

        QPixmap pixmap(size, size);
        pixmap.fill(QColor(255, 255, 255, 0)); // TODO: Transparent. Now it flickers when it's transparent
        QPainter painter(&pixmap);
        QRect rect(0, 0, PIX_SIZE, PIX_SIZE);

        foreach (QModelIndex index, indexes) {
            if (index.column() != 0) continue;

            KCategorizedItemsViewModels::AbstractItem * item =
                m_view->getItemByProxyIndex(index);

            if (item) {
                rect.setSize(item->icon().actualSize(QSize(PIX_SIZE, PIX_SIZE)));
                //painter.fillRect(rect, QBrush(QColor(255, 255, 255))); // TODO: Use global palettes
                item->icon().paint(&painter, rect);
                rect.moveTopLeft(rect.topLeft() + QPoint(off, off));
            }
        }
        painter.end();


        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->start(supportedActions);
        //drag->setHotSpot(d->pressedPosition - rect.topLeft());
        //if (drag->start(supportedActions) == Qt::MoveAction)
        //    d->clearOrRemove();
    }
}

