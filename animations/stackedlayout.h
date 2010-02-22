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

#ifndef STACKEDLAYOUT_H
#define STACKEDLAYOUT_H

/* TODO: document the methods */

#include <QGraphicsLayout>
#include <QList>
#include <QObject>

class StackedLayout : public QObject, public QGraphicsLayout
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsLayout)

public:
        explicit StackedLayout(QGraphicsLayoutItem *parent = 0);
        ~StackedLayout();

        void setGeometry(const QRectF &rect);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint=QSizeF()) const;
        int count() const;
        QGraphicsLayoutItem *itemAt(int i) const;

        void insertWidget(QGraphicsLayoutItem *item, int pos);
        void addWidget(QGraphicsLayoutItem *item);

        void removeAt(int index);

        qint32 currentWidgetIndex() const;
        void setCurrentWidgetIndex(qint32 index);

    private:
        QList<QGraphicsLayoutItem *> items;
        qint32 m_currentWidgetIndex;
};

#endif
