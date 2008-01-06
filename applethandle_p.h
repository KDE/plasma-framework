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

#ifndef PLASMA_APPLETHANDLE
#define PLASMA_APPLETHANDLE

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

#include "phase.h"
#include "svg.h"

namespace Plasma
{
class Applet;
class Containment;

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

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    Q_SIGNALS:
       void disappearDone(AppletHandle *self);

    private Q_SLOTS:
        void fadeAnimation(qreal progress);
        void appletDestroyed();
        void appletResized();

    private:
        static const int HANDLE_WIDTH = 5;
        static const int ICON_SIZE = 16;
        static const int ICON_MARGIN = 8;

        void startFading(FadeType anim);
        void forceDisappear();
        void calculateSize();
        ButtonType mapToButton(const QPointF &point) const;

        QRectF m_rect;
        bool m_buttonsOnRight;
        ButtonType m_pressedButton;
        Containment *m_containment;
        Applet *m_applet;
        qreal m_opacity;
        FadeType m_anim;
        Phase::AnimId m_animId;
        qreal m_angle;
        qreal m_scaleWidth;
        qreal m_scaleHeight;
        qreal m_originalAngle;
        QColor m_gradientColor;
};

}

#endif // multiple inclusion guard
