/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Davide Bettio <davide.bettio@kdemail.net>
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

#ifndef PLASMA_SPINBOX_H
#define PLASMA_SPINBOX_H

#include <QtGui/QGraphicsProxyWidget>

#include <plasma/plasma_export.h>

class KIntSpinBox;

namespace Plasma
{

class SpinBoxPrivate;

/**
 * @class SpinBox plasma/widgets/slider.h <Plasma/Widgets/SpinBox>
 *
 * @short Provides a plasma-themed KIntSpinBox.
 */
class PLASMA_EXPORT SpinBox : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsWidget *parentWidget READ parentWidget)
    Q_PROPERTY(int maximum READ maximum WRITE setMinimum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_PROPERTY(KIntSpinBox *nativeWidget READ nativeWidget)

public:
    explicit SpinBox(QGraphicsWidget *parent = 0);
    ~SpinBox();

    /**
     * @return the maximum value
     */
    int maximum() const;

    /**
     * @return the minimum value
     */
    int minimum() const;

    /**
     * @return the current value
     */
    int value() const;

    /**
     * Sets the stylesheet used to control the visual display of this SpinBox
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet();

    /**
     * @return the native widget wrapped by this SpinBox
     */
    KIntSpinBox *nativeWidget() const;

protected:
    void changeEvent(QEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public Q_SLOTS:
    /**
     * Sets the maximum value the slider can take.
     */
    void setMaximum(int maximum);

    /**
     * Sets the minimum value the slider can take.
     */
    void setMinimum(int minimum);

    /**
     * Sets the minimum and maximum values the slider can take.
     */
    void setRange(int minimum, int maximum);

    /**
     * Sets the value of the slider.
     *
     * If it is outside the range specified by minimum() and maximum(),
     * it will be adjusted to fit.
     */
    void setValue(int value);

Q_SIGNALS:
    /**
     * This signal is emitted when the user drags the slider.
     *
     * In fact, it is emitted whenever the sliderMoved(int) signal
     * of KIntSpinBox would be emitted.  See the Qt documentation for
     * more information.
     */
    void sliderMoved(int value);

    /**
     * This signal is emitted when the slider value has changed,
     * with the new slider value as argument.
     */
    void valueChanged(int value);

    /**
     * This signal is emitted when editing is finished. 
     * This happens when the spinbox loses focus and when enter is pressed.
     */
    void editingFinished();

private:
    Q_PRIVATE_SLOT(d, void setPalette())

    SpinBoxPrivate * const d;
};

} // namespace Plasma

#endif // multiple inclusion guard
