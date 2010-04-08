/*
 *   Copyright 2009 Mehmet Ali Akmanalp <makmanalp@wpi.edu>
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
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

/**
 * @file This file contains the definition for the PixmapTransition effect.
 */

#ifndef PLASMA_ANIMATIONS_PIXMAPTRANSITION_P_H
#define PLASMA_ANIMATIONS_PIXMAPTRANSITION_P_H

#include <plasma/animations/easinganimation_p.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class Fade plasma/animations/pixmaptransition.h
 * @short PixmapTransition effect
 *
 * Effect that paints a transition between two pixmaps
 */
class PixmapTransition : public EasingAnimation
{
    Q_OBJECT
    Q_PROPERTY(QPixmap startPixmap READ startPixmap WRITE setStartPixmap)
    Q_PROPERTY(QPixmap targetPixmap READ targetPixmap WRITE setTargetPixmap)
    Q_PROPERTY(QPixmap currentPixmap READ currentPixmap)

public:
    explicit PixmapTransition(QObject *parent = 0);

    virtual ~PixmapTransition();

    /**
     * The first pixmap of the animation
     */
    QPixmap startPixmap() const;

    /**
     * Set the first pixmap of the animation
     */
    void setStartPixmap(const QPixmap &);

    /**
     * The pixmap the animation will evolve to
     */
    QPixmap targetPixmap() const;

    /**
     * Set the pixmap the animation will evolve to
     */
    void setTargetPixmap(const QPixmap &);

    /**
     * Returns the current pixmap
     */
    QPixmap currentPixmap() const;

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateEffectiveTime(int currentTime);

private:
    QPixmap m_startPixmap;
    QPixmap m_targetPixmap;
    QPixmap m_currentPixmap;
};

}

#endif // PLASMA_ANIMATIONS_PIXMAPTRANSITION_P_H
