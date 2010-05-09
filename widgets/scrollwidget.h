/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_SCROLLWIDGET_H
#define PLASMA_SCROLLWIDGET_H

#include <QtCore/QAbstractAnimation>
#include <QtGui/QGraphicsWidget>

#include <plasma/plasma_export.h>


namespace Plasma
{

class ScrollWidgetPrivate;

/**
 * @class ScrollWidget plasma/widgets/ScrollWidget.h <Plasma/Widgets/ScrollWidget>
 *
 * @short A container of widgets that can have scrollbars
 *
 * A container of widgets that can have horizontal and vertical scrollbars if the content is bigger than the widget itself
 *
 * @since 4.3
 */
class PLASMA_EXPORT ScrollWidget : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QGraphicsWidget *widget READ widget WRITE setWidget)
    Q_PROPERTY(Qt::ScrollBarPolicy horizontalScrollBarPolicy READ horizontalScrollBarPolicy WRITE setHorizontalScrollBarPolicy)
    Q_PROPERTY(Qt::ScrollBarPolicy verticalScrollBarPolicy READ verticalScrollBarPolicy WRITE setVerticalScrollBarPolicy)
    Q_PROPERTY(QPointF scrollPosition READ scrollPosition WRITE setScrollPosition)
    Q_PROPERTY(QSizeF contentsSize READ contentsSize)
    Q_PROPERTY(QRectF viewportGeometry READ viewportGeometry)
    Q_PROPERTY(QSizeF snapSize READ snapSize WRITE setSnapSize)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool overShoot READ hasOverShoot WRITE setOverShoot)

public:

    /**
     * Constructs a new ScrollWidget
     *
     * @arg parent the parent of this widget
     */
    explicit ScrollWidget(QGraphicsWidget *parent = 0);
    explicit ScrollWidget(QGraphicsItem *parent);
    ~ScrollWidget();

    /**
     * Sets the widget this ScrollWidget will contain
     * ownership is transferred to this scrollwidget,
     * if an old one was already in, it will be deleted.
     * If the widget size policy allows for horizontal and/or vertical expansion,
     * it will be resized when possible, otherwise it will be kept to whichever
     * width the widget resizes itself.
     *
     * @arg widget the new main sub widget
     */
    void setWidget(QGraphicsWidget *widget);

    /**
     * @return the main widget
     */
    QGraphicsWidget *widget() const;

    /**
     * Sets the alignment for the inner widget.
     * It is only meaningful if the inner widget is smaller
     * than the viewport.
     * @since 4.5
     */
    void setAlignment(Qt::Alignment align);

    /**
     * @return currently set alignment for the inner widget
     * @since 4.5
     */
    Qt::Alignment alignment() const;

    /**
     * Tells the scrollwidget whether the widget can scroll a little
     * beyond its boundaries and then automatically snap back or
     * whether the widget scrolling always stops at the edges.
     * @since 4.5
     */
    void setOverShoot(bool enable);

    /**
     * @return true if overshoot is enabled
     * @since 4.5
     */
    bool hasOverShoot() const;

    /**
     * Sets the horizontal scrollbar policy
     *
     * @arg policy desired policy
     */
    void setHorizontalScrollBarPolicy(const Qt::ScrollBarPolicy policy);

    /**
     * @return the horizontal scrollbar policy
     */
    Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;

    /**
     * Sets the vertical scrollbar policy
     *
     * @arg policy desired policy
     */
    void setVerticalScrollBarPolicy(const Qt::ScrollBarPolicy policy);

    /**
     * @return the vertical scrollbar policy
     */
    Qt::ScrollBarPolicy verticalScrollBarPolicy() const;

    /**
     * Scroll the view until the given rectangle is visible
     *
     * @param rect rect we want visible, in coordinates mapped to the inner widget
     * @since 4.4
     */
    Q_INVOKABLE void ensureRectVisible(const QRectF &rect);

    /**
     * Scroll the view until the given item is visible
     *
     * @param item item we want visible
     * @since 4.4
     */
    Q_INVOKABLE void ensureItemVisible(QGraphicsItem *item);

    /**
     * Register an item as a drag handle, it means mouse events will pass trough it
     * and will be possible to drag the view by dragging the item itself.
     * The item will still receive mouse clicks if the mouse didn't move
     * between press and release.
     *
     * This function is no more necessary, since it's the authomatic behaviour
     * for all children items, the implementation has now no effect
     *
     * @param item the drag handle item. widget() must be an ancestor if it in
     *             the parent hierarchy. if item doesn't accept mose press events
     *             it's not necessary to call this function.
     * @since 4.4
     */
    KDE_DEPRECATED Q_INVOKABLE void registerAsDragHandle(QGraphicsWidget *item);

    /**
     * Unregister the given item as drag handle (if it was registered)
     *
     * This function is no more necessary, since it's the authomatic behaviour
     * for all children items, the implementation has now no effect
     *
     * @since 4.4
     */
    KDE_DEPRECATED Q_INVOKABLE void unregisterAsDragHandle(QGraphicsWidget *item);

    /**
     * The geometry of the viewport.
     * @since 4.4
     */
    QRectF viewportGeometry() const;


    /**
     * @return the size of the internal widget
     * @since 4.4
     */
    QSizeF contentsSize() const;

    /**
     * Sets the position of the internal widget relative to this widget
     * @since 4.4
     */
    void setScrollPosition(const QPointF &position);

    /**
     * @return the position of the internal widget relative to this widget
     * @since 4.4
     */
    QPointF scrollPosition() const;

    /**
     * Set the nap size of the kinetic scrolling:
     * the scrolling will always stop at multiples of that size.
     *
     * @arg the desired snap size
     * @since 4.5
     */
    void setSnapSize(const QSizeF &size);

    /**
     * @return the snap size of the kinetic scrolling
     * @since 4.5
     */
    QSizeF snapSize() const;

    /**
     * Sets the stylesheet used to control the visual display of this ScrollWidget
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet() const;

    /**
     * @return the native widget wrapped by this ScrollWidget
     */
    QWidget *nativeWidget() const;

Q_SIGNALS:
    /**
     * The widget started or stopped an animated scroll
     * @since 4.4
     */
    void scrollStateChanged(QAbstractAnimation::State newState,
            QAbstractAnimation::State oldState);

    /**
     * The viewport geomety changed, for instance due a widget resize
     *
     * @since 4.5
     */
    void viewportGeometryChanged(const QRectF &geomety);

protected:

    void resizeEvent(QGraphicsSceneResizeEvent *event);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void focusInEvent(QFocusEvent *event);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint) const;
    bool sceneEventFilter(QGraphicsItem *i, QEvent *e);

private:
    ScrollWidgetPrivate * const d;

    Q_PRIVATE_SLOT(d, void verticalScroll(int value))
    Q_PRIVATE_SLOT(d, void horizontalScroll(int value))
    Q_PRIVATE_SLOT(d, void makeRectVisible())
    Q_PRIVATE_SLOT(d, void makeItemVisible())
    Q_PRIVATE_SLOT(d, void adjustScrollbars())
    Q_PRIVATE_SLOT(d, void fixupX())
    Q_PRIVATE_SLOT(d, void fixupY())
    Q_PRIVATE_SLOT(d, void setScrollX())
    Q_PRIVATE_SLOT(d, void setScrollY())

    friend class ScrollWidgetPrivate;
};

} // namespace Plasma

#endif // multiple inclusion guard
