/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *                 2007 Alexis Ménard <darktears31@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef PHASE_H
#define PHASE_H

#include <QImage>
#include <QObject>

#include <plasma/plasma_export.h>

class QGraphicsItem;
class QTimeLine;

namespace Plasma
{

/**
 * @short A system for applying effects to Plasma elements
 */
class PLASMA_EXPORT Phase : public QObject
{
    Q_OBJECT

public:
    enum Animation
    {
        Appear = 0 /*<< When some appears in the Corona */,
        Disappear /*<< When something is about to disappear */,
        Activate /*<< When something is activated or launched, such as an app icon being clicked */,
        FrameAppear /*<< Make a frame appear around an object */
    };

    enum ElementAnimation
    {
        ElementAppear = 0 /*<< Animate the appearance of an element */,
        ElementDisappear /*<< Animate the disappearance of an element */
    };

    enum RenderOp
    {
        RenderBackground = 0 /*<< Render the background of an item */
    };

    enum CurveShape
    {
        EaseInCurve = 0,
        EaseOutCurve,
        EaseInOutCurve,
        LinearCurve
    };

    enum Movement
    {
        SlideIn = 0,
        SlideOut
    };

    typedef int AnimId;

    /**
     * Singleton accessor
     **/
    static Phase* self();

    explicit Phase(QObject * parent = 0);
    ~Phase();

    void animateItem(QGraphicsItem* item, Animation anim);
    void moveItem(QGraphicsItem* item, Movement movement, QPoint destination);
    void render(QGraphicsItem* item, QImage& image, RenderOp op);

    AnimId animateElement(QGraphicsItem *obj, ElementAnimation);
    void stopElementAnimation(AnimId id);
    void setAnimationPixmap(AnimId id, const QPixmap &pixmap);
    QPixmap animationResult(AnimId id);

Q_SIGNALS:
    void animationComplete(QGraphicsItem *item, Animation anim);

protected:
    void timerEvent(QTimerEvent *event);

protected Q_SLOTS:
    void appletDestroyed(QObject*);

private:
    void init();

    class Private;
    Private * const d;
};

} // namespace Plasma

#endif

