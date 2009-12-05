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
 * @file This file contains the definition for the Fade effect.
 */

#ifndef PLASMA_ANIMATIONS_GEO_H
#define PLASMA_ANIMATIONS_GEO_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class ZoomAnimationPrivate;

/**
 * @class GeoAnimation plasma/animations/geo_p.h
 * @short Geometry Animation
 *
 */
class GeoAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(QRectF startGeometry READ startGeometry WRITE setStartGeometry)
    Q_PROPERTY(QRectF targetGeometry READ targetGeometry WRITE setTargetGeometry)

public:
    GeoAnimation(QObject *parent = 0);
    virtual ~GeoAnimation();

    QRectF startGeometry() const;
    void setStartGeometry(const QRectF &);

    QRectF targetGeometry() const;
    void setTargetGeometry(const QRectF &);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateCurrentTime(int currentTime);

private:
    QRectF m_startGeometry;
    QRectF m_targetGeometry;
};

}

#endif
