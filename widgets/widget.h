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
public:
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

    #ifdef TOOLTIPMANAGER
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

    #endif

protected:
    #ifdef TOOLTIPMANAGER
       virtual bool sceneEvent(QEvent *event);
    #endif
private:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    class Private;
    Private *const d;
};

} // Plasma namespace

#endif
