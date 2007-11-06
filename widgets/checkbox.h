/*
 *   Copyright 2007 by Siraj Razick <siraj@kde.org>
 *   Copyright 2007 by Matt Broadstone <mbroadst@gmail.com>
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

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsItem>

#include <plasma/plasma_export.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/widget.h>

namespace Plasma
{

/**
 * This class provides a QCheckBox available as a Plasma::Widget, so it can be used
 * within Plasma::Applet's.
 *
 * @author Siraj Razick <siraj@kde.org>
 */
class PLASMA_EXPORT CheckBox : public Plasma::Widget
{
    Q_OBJECT
public:
    /**
     * Creates a new Plasma::CheckBox.
     * @param parent the QGraphicsItem this icon is parented to.
    */
    explicit CheckBox(QGraphicsItem *parent = 0);

    /**
     * Creates a new Plasma::Icon with default text.
     * @param text the text to display next to the checkbox.
     * @param parent the QGraphicsItem this icon is parented to.
    */
    explicit CheckBox(const QString &text, QGraphicsItem *parent = 0);

    /**
     * Destroys a Plasma::Icon.
    */
    virtual ~CheckBox();

    /**
     * @return whether this checkbox is currently checked.
    */
    bool isChecked() const;

    /**
     * Sets the checked state of this Plasma::CheckBox.
     * @param checked whether the Plasma::CheckBox is checked or not.
    */
    void setChecked(bool checked);

    /**
     * @return the checkstate of this Plasma::CheckBox.
    */
    Qt::CheckState checkState() const;

    /**
     * Sets the checkstate of this Plasma::CheckBox
     * @see <qt.h> for definition of Qt::CheckState
     * @param state the checkstate of this Plasma::CheckBox
    */
    void setCheckState(Qt::CheckState state);

    /**
     * @return the text associated with this Plasma::CheckBox
    */
    QString text() const;

    /**
     * Sets the text associated with this Plasma::CheckBox
     * @param text the text to associate with this Plasma::CheckBox.
    */
    void setText(const QString &text);

/*
    bool isTristate() const;
    void setTristate(bool triState = true);
*/

public Q_SLOTS:
    void dataUpdated(const QString&, const DataEngine::Data&);

Q_SIGNALS:
    /**
     * Indicates that this Plasma::CheckBox has been clicked, changing its state.
    */
    void clicked();

protected:
    //bool isDown();
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void mouseMoveEvent (QGraphicsSceneMouseEvent * event);
    void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
    void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );

    void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    void init();

    class Private ;
    Private *  const d;

};

} // namespace Plasma

#endif
