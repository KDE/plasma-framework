/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
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

#ifndef PLASMA_APPLETHANDLE_P_H
#define PLASMA_APPLETHANDLE_P_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>
#include <QTimer>

#include "animator.h"
#include "svg.h"

namespace Plasma
{
class Applet;
class Containment;
class View;

class AppletHandle : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        enum FadeType { FadeIn, FadeOut };
        enum ButtonType { NoButton, MoveButton, RotateButton, ConfigureButton, RemoveButton, ResizeButton };

        AppletHandle(Containment *parent, Applet *applet);
        virtual ~AppletHandle();

        Applet *applet() const;

        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        void startFading(FadeType anim);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

    Q_SIGNALS:
       void disappearDone(AppletHandle *self);

    private Q_SLOTS:
        void fadeAnimation(qreal progress);
        void appletDestroyed();
        void appletResized();
        void fadeIn();

    private:
        static const int HANDLE_WIDTH = 5;
        static const int ICON_SIZE = 16;
        static const int ICON_MARGIN = 8;

        void calculateSize();
        ButtonType mapToButton(const QPointF &point) const;
        void forceDisappear();
        /**
         * move our applet to another containment
         * @param containment the containment to move to
         * @param pos the (scene-relative) position to place it at
         */
        void switchContainment(Containment *containment, const QPointF &pos);
        bool goTopLevel(const QPoint & pos);

        QRectF m_rect;
        ButtonType m_pressedButton;
        Containment *m_containment;
        Applet *m_applet;
        qreal m_opacity;
        FadeType m_anim;
        int m_animId;
        qreal m_angle;
        qreal m_tempAngle;
        qreal m_scaleWidth;
        qreal m_scaleHeight;
        QColor m_gradientColor;
        QTimer *m_hoverTimer;
        bool m_buttonsOnRight;
        bool m_pendingFade;
        View *m_topview;

        QPoint m_mousePos;  //mousepos relative to applet
        QRect m_screenRect; //boudingrect of applet in screencoords
        QPointF m_pos;      //current position of applet in sceneCoords
};

}

#endif // multiple inclusion guard
