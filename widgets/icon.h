/*
*   Copyright (C) 2007 by Siraj Razick siraj@kde.org
*   Copyright (C) 2007 by Matt Broadstone <mbroadst@gmail.com>
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

#ifndef ICON_H
#define ICON_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsTextItem>

#include <plasma/dataengine.h>
#include <plasma/plasma_export.h>
#include <plasma/widgets/layoutitem.h>

class KUrl;

//TODO
//Please Document this class

namespace Plasma
{

/**
* Icon class, for URIs and menu popups in panels
*/
class PLASMA_EXPORT Icon : public QObject, public QGraphicsItem, public LayoutItem
{
    Q_OBJECT
    public:
        Icon(QGraphicsItem *parent = 0);
        Icon(const QString &text, QGraphicsItem *parent = 0);
        Icon(const QIcon & icon, const QString &text, QGraphicsItem *parent = 0);
        virtual ~Icon();

        QString text() const;
        void setText(const QString &name);

        void setIcon(const QString& icon);
        void setIcon(const QIcon& icon);

        QSizeF iconSize() const;
        void setIconSize(const QSizeF& size);
        void setIconSize(int height, int width);

/*
        enum Position
        {
            TopLeft = 0,
            TopRight,
            BottomLeft,
            BottomRight
        };
        void setCornerAction(Position pos, QAction *action);
*/

        // Layout stuff
        Qt::Orientations expandingDirections() const;

        QSizeF minimumSize() const;
        QSizeF maximumSize() const;

        bool hasHeightForWidth() const;
        qreal heightForWidth(qreal w) const;

        bool hasWidthForHeight() const;
        qreal widthForHeight(qreal h) const;

        QRectF geometry() const;
        void setGeometry(const QRectF& r);

        QSizeF sizeHint() const;

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        QRectF boundingRect() const;

    Q_SIGNALS:
        void pressed(bool down);
        void clicked();

    protected:
        bool isDown();
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    private:
        QPixmap buttonPixmap();
        void init();
        void calculateSize();

        class Private;
        Private * const d;
};

} // namespace Plasma


/*
    // Add these to UrlIcon
    void setUrl(const KUrl& url);
    KUrl url() const;
*/

#endif
