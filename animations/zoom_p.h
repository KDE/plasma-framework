/*
 *   Copyright 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>
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
 * @file This file contains the definition for the Zoom animation.
 */

#ifndef PLASMA_ANIMATIONS_ZOOM_P_H
#define PLASMA_ANIMATIONS_ZOOM_P_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class ZoomAnimation plasma/animations/zoom_p.h
 * @short Zoom Animation
 *
 */
class ZoomAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom)

public:
    explicit ZoomAnimation(QObject *parent = 0);
    virtual ~ZoomAnimation();

    qreal zoom() const;
    void setZoom(qreal);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateCurrentTime(int currentTime);

private:
    qreal m_zoom;
};

}

#endif // PLASMA_ANIMATIONS_ZOOM_P_H
