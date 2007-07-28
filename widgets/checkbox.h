/*
 *   Copyright (C) 2007 by Siraj Razick siraj@kdemail.net
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

#include <plasma/plasma_export.h>

#include <plasma/dataengine.h>
//TODO
//Please Document this class

namespace Plasma
{

/**
 * Class that emulates a QCheckBox inside plasma
 */
class PLASMA_EXPORT CheckBox : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:

        CheckBox(QGraphicsItem *parent = 0);
        virtual ~CheckBox();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        QRectF boundingRect() const;
         void setText(const QString&) ;
        QString text() const;
        QSize size() const;
        int height() const;
        int width() const;
        void setSize(const QSize &size);
        void setWidth(int width);
        void setHeight(int height);
        void setMaximumWidth(int maxwidth);
        Qt::CheckState checkState() const;
        void setChecked(bool checked);
        void setCheckState(Qt::CheckState state);


    public Q_SLOTS:
        void updated(const QString&, const DataEngine::Data&);
    Q_SIGNALS:
        void clicked();
    protected:
        //bool isDown();
        void mousePressEvent ( QGraphicsSceneMouseEvent * event );
        void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
        void mouseMoveEvent (QGraphicsSceneMouseEvent * event);
        void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
        void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
        void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    private:
        class Private ;
        Private *  const d;

};

} // namespace Plasma

#endif
