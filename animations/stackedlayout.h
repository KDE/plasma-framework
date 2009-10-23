////////////////////////////////////////////////////////////////////////
// stackedlayout.h                                                     //
//                                                                     //
// Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>//
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#ifndef STACKEDLAYOUT_H
#define STACKEDLAYOUT_H

#include <QGraphicsLayout>
#include <QList>

class StackedLayout : public QGraphicsLayout {
    public:
        StackedLayout(QGraphicsLayoutItem *parent = 0);
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
