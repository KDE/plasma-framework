/***************************************************************************
 *   Copyright 2009 by Alessandro Diaferia <alediaferia@gmail.com>         *
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2, or       *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ITEMBACKGROUND_H
#define ITEMBACKGROUND_H

#include <QtGui/QGraphicsWidget>

#include <plasma/plasma_export.h>

/**
 * @class ItemBackground plasma/widgets/itembackground.h
 *
 * @short a background for QGraphicsWidget based item views with animation effects
 *
 * @since 4.4
 */
namespace Plasma {

class FrameSvg;
class ItemBackgroundPrivate;

class PLASMA_EXPORT ItemBackground : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QRectF target READ target WRITE setTarget)
    Q_PROPERTY(QGraphicsItem* targetItem READ targetItem WRITE setTargetItem)

public:
    ItemBackground(QGraphicsWidget *parent = 0);
    ~ItemBackground();

    /**
     * Sets a new target geometry we want at the end of animation
     *
     * @arg newGeometry the final geometry target
     */
    void setTarget(const QRectF &newGeometry);

    /**
     * @return the current target rect; may be empty if there is no target currently set
     */
    QRectF target() const;

    /**
     * set the ItemBackground geometry to be the target geometry, plus the ItemBackground margins 
     */
    void setTargetItem(QGraphicsItem *target);

    /**
     * @return the target item, if any
     */
    QGraphicsItem *targetItem() const;

    /**
     * @reimp from QGraphicsWidget
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

Q_SIGNALS:
    /**
     * Emitted when the visual layout and appearance changes. Useful for synchronizing
     * content margins.
     */
    void appearanceChanged();

    /**
     * Emitted at each animation frame. Useful for synchronizing item animations
     */
    void animationStep(qreal progress);

    /**
     * Emitted when the target has been reached. Useful to consider this instead of
     * the corresponding hoverEnterEvent; 
     */
    void targetReached(QRectF);

    /**
     * Emitted when the target has been reached. Useful to consider this instead of
     * the corresponding hoverEnterEvent; 
     */
    void targetItemReached(QGraphicsItem *);

protected:
    /**
     * @reimp from QGraphicsWidget
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    /**
     * @reimp from QObject
     */
    bool eventFilter(QObject *watched, QEvent *event);

    /**
     * @reimp from QGraphicsItem
     */
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

    /**
     * @reimp from QGraphicsItem
     */
    void resizeEvent(QGraphicsSceneResizeEvent *);

private:
    Q_PRIVATE_SLOT(d, void animationUpdate(qreal progress))
    Q_PRIVATE_SLOT(d, void targetDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void frameSvgChanged())
    Q_PRIVATE_SLOT(d, void refreshCurrentTarget())

    friend class ItemBackgroundPrivate;
    ItemBackgroundPrivate * const d;
};
}

#endif
