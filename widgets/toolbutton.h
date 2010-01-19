/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_TOOLBUTTON_H
#define PLASMA_TOOLBUTTON_H

#include <QtGui/QGraphicsProxyWidget>

class QToolButton;

#include <plasma/plasma_export.h>

namespace Plasma
{

class ToolButtonPrivate;

/**
 * @class ToolButton plasma/widgets/pushbutton.h <Plasma/Widgets/ToolButton>
 *
 * @short Provides a plasma-themed QToolButton.
 */
class PLASMA_EXPORT ToolButton : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsWidget *parentWidget READ parentWidget)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)
    Q_PROPERTY(QString image READ image WRITE setImage)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_PROPERTY(QToolButton *nativeWidget READ nativeWidget)
    Q_PROPERTY(QAction *action READ action WRITE setAction)

public:
    explicit ToolButton(QGraphicsWidget *parent = 0);
    ~ToolButton();

    /**
     * Sets if the toolbutton has an autoraise behaviour
     *
     * @arg raise
     */
    void setAutoRaise(bool raise);

    /**
     * @return true if the toolbutton has an autoraise behaviour
     */
    bool autoRaise() const;

    /**
     * Sets the display text for this ToolButton
     *
     * @arg text the text to display; should be translated.
     */
    void setText(const QString &text);

    /**
     * @return the display text
     */
    QString text() const;

    /**
     * Sets the path to an image to display.
     *
     * @arg path the path to the image; if a relative path, then a themed image will be loaded.
     */
    void setImage(const QString &path);

    /**
     * Sets the path to an svg image to display and the id of the used svg element, if necessary.
     *
     * @arg path the path to the image; if a relative path, then a themed image will be loaded.
     * @arg elementid the id of a svg element.
     *
     * @since 4.4
     */
    void setImage(const QString &path, const QString &elementid);

    /**
     * @return the image path being displayed currently, or an empty string if none.
     */
    QString image() const;

    /**
     * @return true if the button is pressed down
     * @since 4.4
     */
    bool isDown() const;

    /**
     * Sets the stylesheet used to control the visual display of this ToolButton
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet();

    /**
     * Associate an action with this IconWidget
     * this makes the button follow the state of the action, using its icon, text, etc.
     * when the button is clicked, it will also trigger the action.
     *
     * @since 4.3
     */
    void setAction(QAction *action);

    /**
     * @return the currently associated action, if any.
     *
     * @since 4.3
     */
    QAction *action() const;

    /**
     * sets the icon for this toolbutton
     *
     * @arg icon the icon we want to use
     *
     * @since 4.3
     */
    void setIcon(const QIcon &icon);

    /**
     * @return the icon of this button
     *
     * @since 4.3
     */
    QIcon icon() const;

    /**
     * @return the native widget wrapped by this ToolButton
     */
    QToolButton *nativeWidget() const;

Q_SIGNALS:
    void clicked();
    /**
     * @since 4.4
     */
    void pressed();
    /**
     * @since 4.4
     */
    void released();

protected:
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void changeEvent(QEvent *event);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint) const;

private:
    Q_PRIVATE_SLOT(d, void syncBorders())
    Q_PRIVATE_SLOT(d, void animationUpdate(qreal progress))
    Q_PRIVATE_SLOT(d, void syncToAction())
    Q_PRIVATE_SLOT(d, void clearAction())
    Q_PRIVATE_SLOT(d, void setPixmap())

    friend class ToolButtonPrivate;
    ToolButtonPrivate *const d;
};

} // namespace Plasma

#endif // multiple inclusion guard
