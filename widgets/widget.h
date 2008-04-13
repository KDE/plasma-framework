/*
 *   Copyright 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
 *                      and Matias Valdenegro <mvaldenegro@informatica.utem.cl>
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

#ifndef WIDGET_H_
#define WIDGET_H_

#include <QtGui/QGraphicsWidget>
#include <QtGui/QPixmap>

#include <QtCore/QRectF>
#include <QtCore/QSizeF>
#include <QtCore/QString>

#include <plasma/layouts/layoutitem.h>
#include <plasma/plasma_export.h>

class QGraphicsView;
class QGraphicsSceneHoverEvent;

namespace Plasma
{

struct PLASMA_EXPORT ToolTipData
{
    ToolTipData() : windowToPreview( 0 ) {}
    QString mainText; //Important information
    QString subText; //Elaborates on the Main Text
    QPixmap image; // Icon to show;
    WId windowToPreview; // Id of window to show preview
};

class Layout;


/**
 * Base class for all Widgets in Plasma.
 *
 * @author Alexander Wiedenbruch
 * @author Matias Valdenegro
 *
 * Widgets are the basis for User Interfaces inside Plasma.
 * Widgets are rectangular, but can be in any visible shape by just using transparency to mask
 * out non-rectangular areas.
 *
 * To implement a Widget, just subclass Plasma::Widget and implement at minimum,
 * sizeHint() and paintWidget()
 */
class PLASMA_EXPORT Widget  : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY( Qt::Orientations expandingDirections READ expandingDirections )
    Q_PROPERTY( QRectF geometry READ geometry WRITE setGeometry )
    Q_PROPERTY( QSizeF size READ size WRITE resize )
    Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )

public:
    enum CachePaintMode {
        NoCacheMode,
        ItemCoordinateCacheMode,
        DeviceCoordinateCacheMode
    };


    /**
     * Creates a new Plasma::Widget.
     * @param parent the QGraphicsItem this icon is parented to.
     */
    explicit Widget(QGraphicsItem *parent = 0 , QObject *parentObject = 0);

    /**
     * Destroys a Plasma::Widget.
     */
    virtual ~Widget();

    /**
     * This method is used by Plasma::Layout to determine which directions the
     * widget naturally expands.
     * @return bitmask with the directions that this Widget can be expanded.
     */
    virtual Qt::Orientations expandingDirections() const;

    /**
     * This method is used by Plasma::Layout to determine whether this widget
     * can provide a height value given a width value.
     * @return whether or not this Widget has heightForWidth.
     */
    virtual bool hasHeightForWidth() const;

    /**
     * This method is used by Plasma::Layout to determine a height value
     * given a width value.
     * @param width the width to use to determine height.
     * @return height calculated using width given.
     */
    virtual qreal heightForWidth(qreal width) const;

    /**
     * This method is used by Plasma::Layout to determine whether this widget
     * can provide a width value given a height value.
     * @return whether or not this Widget has widthForHeight.
     */
    virtual bool hasWidthForHeight() const;

    /**
     * This method is used by Plasma::Layout to determine a width value
     * given a height value.
     * @param height the width to use to determine width.
     * @return width calculated using height given.
     */
    virtual qreal widthForHeight(qreal h) const;

    /**
     * @return geometry of this widget.
     */
    QRectF geometry() const;

    /**
     * @return the font currently set for this widget
     **/
    QFont font() const;

    /**
     * @return this Plasma::Widget's parent, returns a null pointer if
     *         none exist.
     */
    Q_INVOKABLE Widget *parent() const;

    /**
     * @return the Plasma::Widget parent for a given QGraphicsItem
     */
    static Widget *parent(const QGraphicsItem *item);

    /**
     * Add another Plasma::Widget as a child of this one.
     * @param widget the widget to reparent to this Plasma::Widget.
     */
    Q_INVOKABLE void addChild(Widget *widget);

    void setOpacity(qreal opacity);
    qreal opacity() const;

    /**
     * Sets the widget's cache paint mode and cache size.
     * @param mode the new cache paint mode
     * @param size the new cache size, only applies to ItemCoordinateCacheMode
     */
    void setCachePaintMode(CachePaintMode mode, const QSize &size = QSize());

    /**
     * The current cache paint mode.
     */
    CachePaintMode cachePaintMode() const;

    /**
     * Invalidates the widget's cache paint mode for a given item rectangle.
     * @param rect the optional invalidated rectangle; if null, defaults to boundingRect().
     */
    void update(const QRectF &rect = QRectF());
    inline void update(qreal _x, qreal _y, qreal w, qreal h)
    { update(QRectF(_x, _y, w, h)); }

    virtual QGraphicsItem* graphicsItem();

    /**
     * Returns the view this widget is visible on
     */
    QGraphicsView *view() const;

    /**
     * Maps a QRect from a view's coordinates to local coordinates.
     * @param view the view from which rect should be mapped
     * @param rect the rect to be mapped
     */
    QRectF mapFromView(const QGraphicsView *view, const QRect &rect) const;

    /**
     * Maps a QRectF from local coordinates to a view's coordinates.
     * @param view the view to which rect should be mapped
     * @param rect the rect to be mapped
     */
    QRect mapToView(const QGraphicsView *view, const QRectF &rect) const;

    /**
    * The Data from the tooltip
    * @returns A ToolTip::Data object with current information
    */
    const ToolTipData* toolTip() const;

    /**
    * Setter for data shown in tooltip
    * @param data a ToolTip::Data object containing icon and text
    */
    void setToolTip(const ToolTipData &dt);

    /**
     * Called when the tooltip is going to be shown or just after hiding
     * it. This lets updating data right before a tooltip is shown or
     * tracking current visibility. That allows e.g. tips that are more
     * expensive to create ahead of time to be set at the last possible
     * moment.
     * @param update visibility of tooltip
     */
    virtual void updateToolTip(bool update);

    /**
    * Recomended position for a popup window like a menu or a tooltip
    * given its size
    * @param s size of the popup
    * @returns recomended position
    */
    QPoint popupPosition(const QSize &s) const;

    /**
     * Reimplemented from QGraphicsItem
     */
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected:
    /**
     * Paints the widget
     * @param painter the QPainter to use to paint.
     * @param option the style option used to give specific info on the item being dawn.
     * @param widget the parent QWidget (most likely the Corona)
     */
    virtual void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    
    virtual bool sceneEvent(QEvent *event);

private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void setSize(const QSizeF &);


    class Private;
    Private *const d;
};

} // Plasma namespace

#endif
