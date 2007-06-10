/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
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

    enum RenderOp
    {
        RenderBackground = 0 /*<< Render the background of an item */
    };

    explicit Phase(QObject * parent = 0);
    ~Phase();

Q_SIGNALS:
    void animationComplete(QGraphicsItem* item, Animation anim);

public Q_SLOTS:
    void animate(QGraphicsItem* item, Animation anim);
    void render(QGraphicsItem* item, QImage& image, RenderOp op);

protected Q_SLOTS:
    void appletDestroyed(QObject*);

    /**
     * NEVER call this method directly, as it relies on sender()
     */
    void advanceFrame(int frame);

    /**
     * NEVER call this method directly, as it relies on sender()
     */
    void animationComplete();

private:
    void init();

    class Private;
    Private * const d;
};

} // namespace Plasma

#endif

