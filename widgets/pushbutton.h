/*
 *   Copyright 2007 by Siraj Razick <siraj@kde.org>
 *   Copyright 2007 by Matias Valdenegro <mvaldenegro@informatica.utem.cl>
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
#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QLayoutItem>

#include <KDE/KIcon>

#include <plasma/dataengine.h>
#include <plasma/widgets/widget.h>
#include <plasma/plasma_export.h>

class QStyleOptionButton;
namespace Plasma
{

/**
 * Class that emulates a QPushButton inside Plasma
 *
 * @author Siraj Razick
 * @author Matias Valdenegro
 * @author Matt Broadstone
 *
 *
 */
class PLASMA_EXPORT PushButton : public Plasma::Widget
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QSizeF iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( KIcon icon READ icon WRITE setIcon )
    Q_PROPERTY( bool flat READ isFlat WRITE setFlat )
    Q_PROPERTY( bool checkable READ isCheckable WRITE setCheckable )
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked)
public:
    /**
    * Creates a new Plasma::PushButton.
    * @param parent the Widge this button is parented to.
    */
    explicit PushButton(Widget *parent = 0);

    /**
    * Creates a new Plasma::PushButton with a text label.
    * @param text the text to display next to the button.
    * @param parent the QGraphicsItem this icon is parented to.
    */
    explicit PushButton(const QString &text, Widget *parent = 0);

    /**
    * Creates a new Plasma::PushButton with an icon and text
    * @param icon the icon to display with this button.
    * @param text the text to display with this button.
    * @param parent the QGraphicsItem this icon is parented to.
    */
    explicit PushButton(const KIcon &icon, const QString &text, Widget *parent = 0);

    /**
    * Destroys this Plasma::PushButton.
    */
    virtual ~PushButton();

    /**
    * @return text associated with this Plasma::PushButton
    */
    QString text() const;

    /**
    * Sets the text to be displayed by this button.
    * @param text the text to display
    */
    void setText(const QString &text);

    /**
    * @return the icon displayed by this button.
    */
    KIcon icon() const;

    /**
    * Sets the icon to be displayed by this Plasma::Icon.
    * @param icon the KIcon to display.
    */
    void setIcon(const KIcon &icon);

    /**
    * Convenience method to set the icon of this Plasma::PushButton
    * based on the icon path, or name.
    * @see KIconLoader
    * @param path the path to, or name of the icon to display.
    */
    Q_INVOKABLE void setIcon(const QString& path);

    /**
    * @return the size of the icon displayed by this Plasma::PushButton.
    */
    QSizeF iconSize() const;

    /**
    * Sets the size of icon shown by this button.
    * @param size size of the icon.
    */
    void setIconSize(const QSizeF &size);

    /**
    * @return whether this button is currently in a down/pressed state.
    */
    bool isDown() const;

    /**
    * @return whether this button is drawn flat.
    */
    bool isFlat() const;

    /**
    * Sets whether the button is drawn flat.
    * @param flat whether to draw it flat or not.
    */
    void setFlat(bool flat);

    /**
    * @return whether this button is checkable.
    */
    bool isCheckable() const;

    /**
    * Sets whether the button is checkable.
    * @param checkable whether button is checkable or not.
    */
    void setCheckable(bool checkable);

    /**
    * @return whether this button is checked.
    */
    bool isChecked() const;

    /**
    * Sets whether the button is checked.
    * @param checked whether button is checked or not.
    */
    void setChecked(bool checked);

    // NOTE: bogus
    QSizeF minimumSize() const;
    Qt::Orientations expandingDirections() const;
    #ifdef REMOVE
    QSizeF sizeHint() const;
    #endif

Q_SIGNALS:
    /**
    * Triggered when the button has been clicked.
    */
    void clicked();

    /**
    * Triggered when the checkable button has been toggled.
    */
    void toggled(bool checked);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    class Private ;
    Private *  const d;

};

} // namespace Plasma

#endif
