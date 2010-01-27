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

#ifndef PLASMA_FRAME_H
#define PLASMA_FRAME_H

#include <QtGui/QGraphicsWidget>

#include <plasma/plasma_export.h>
#include <plasma/framesvg.h>

class QFrame;

namespace Plasma
{

class FramePrivate;

/**
 * @class Frame plasma/widgets/frame.h <Plasma/Widgets/Frame>
 *
 * @short A widget that provides a simple frame
 *
 * A simple frame to group widgets, it can have a plain, sunken or raise aspect
 * the default aspect is plain
 */
class PLASMA_EXPORT Frame : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(Shadow frameShadow READ frameShadow WRITE setFrameShadow)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString image READ image WRITE setImage)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_ENUMS(Shadow)

public:
    enum Shadow {
        Plain = 1,
        Raised = 2,
        Sunken = 3
    };

    /**
     * Constructs a new Frame
     *
     * @arg parent the parent of this widget
     */
    explicit Frame(QGraphicsWidget *parent = 0);
    ~Frame();

   /**
    * Sets the Frame's shadow style
    *
    * @arg shadow plain, raised or sunken
    */
    void setFrameShadow(Shadow shadow);

   /**
    * @return the Frame's shadow style
    */
    Shadow frameShadow() const;

    /**
     * Sets what borders should be painted
     * @arg flags borders we want to paint
     */
    void setEnabledBorders(const FrameSvg::EnabledBorders borders);

    /**
     * Convenience method to get the enabled borders
     * @return what borders are painted
     * @since 4.4
     */
    FrameSvg::EnabledBorders enabledBorders() const;

   /**
    * Set the text to display by this Frame
    *
    * @arg text the text
    * @since 4.4
    */
    void setText(QString text);

   /**
    * @return text displayed from this Frame
    */
    QString text() const;

    /**
     * Sets the path to an image to display.
     *
     * @arg path the path to the image; if a relative path, then a themed image will be loaded.
     */
    void setImage(const QString &path);

    /**
     * @return the image path being displayed currently, or an empty string if none.
     */
    QString image() const;

    /**
     * Sets the stylesheet used to control the visual display of this Frame
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet() const;

    /**
     * @return the native widget wrapped by this Frame
     */
    QWidget *nativeWidget() const;

protected:
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void resizeEvent(QGraphicsSceneResizeEvent *event);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint) const;
    void changeEvent(QEvent *event);

private:
    FramePrivate * const d;

    Q_PRIVATE_SLOT(d, void syncBorders())
};

} // namespace Plasma

#endif // multiple inclusion guard
