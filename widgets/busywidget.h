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

#ifndef PLASMA_BUSYWIDGET_H
#define PLASMA_BUSYWIDGET_H

#include <QtGui/QGraphicsWidget>

#include <plasma/plasma_export.h>

class QFrame;

namespace Plasma
{

class BusyWidgetPrivate;

/**
 * @class BusyWidget plasma/widgets/spinner.h <Plasma/Widgets/BusyWidget>
 *
 * @short A widget that provides a waiting spinner
 *
 * A simple spinner widget that can be used to represent a wait of unknown length
 */
class PLASMA_EXPORT BusyWidget : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning WRITE setRunning)
    Q_PROPERTY(QString label READ label WRITE setLabel)

public:
    /**
     * Constructs a new BusyWidget
     *
     * @arg parent the parent of this widget
     */
    explicit BusyWidget(QGraphicsWidget *parent = 0);
    ~BusyWidget();

    /**
     * @param running whether or not the spinner has to be animated. defaults to true.
     */
    void setRunning(bool running);

    /**
     * @return whether or not the spinner is animated.
     */
    bool isRunning() const;

    /**
     * @param label a string to be shown in front of the icon.
     */
    void setLabel(const QString &label);

    /**
     * @param label the string that is shown in front of the icon.
     */
    QString label() const;

Q_SIGNALS:
    void clicked();

protected:
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

protected Q_SLOTS:
    void timerEvent(QTimerEvent *event);

private:
    BusyWidgetPrivate * const d;

    Q_PRIVATE_SLOT(d, void themeChanged())
};

} // namespace Plasma

#endif // multiple inclusion guard
