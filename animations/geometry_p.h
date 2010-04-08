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
 * @file This file contains the definition for the Geometry effect.
 */

#ifndef PLASMA_ANIMATIONS_GEOMETRY_P_H
#define PLASMA_ANIMATIONS_GEOMETRY_P_H

#include <plasma/animations/easinganimation_p.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class GeometryAnimation plasma/animations/geo_p.h
 * @short Geometry Animation
 * Use this class when you want to change the geometry of an QGraphicsWidget
 * in an animated way (you should at least set the target geometry).
 */
class GeometryAnimation : public EasingAnimation
{
    Q_OBJECT
    Q_PROPERTY(QRectF startGeometry READ startGeometry WRITE setStartGeometry)
    Q_PROPERTY(QRectF targetGeometry READ targetGeometry WRITE setTargetGeometry)

public:
    /** Default constructor */
    explicit GeometryAnimation(QObject *parent = 0);

    /** Destructor */
    virtual ~GeometryAnimation();

    /**
     * Access the initial geometry of animated widget.
     *
     * If no geometry is set, it will use the widget current geometry
     * when the animation is first run).
     * @return Start geometry.
     */
    QRectF startGeometry() const;
    /**
     * Set the initial geometry of animated widget.
     *
     * If no geometry is set, it will use the widget current geometry
     * when the animation is first run).
     * @arg geometry The initial geometry.
     */
    void setStartGeometry(const QRectF &geometry);

    /**
     * Access the final geometry of animated widget.
     *
     * \todo: check if it was set and case negative, handle the error.
     * @return Target geometry.
     */
    QRectF targetGeometry() const;
    /**
     * Set the final geometry of animated widget.
     *
     * See also \ref setStartGeometry.
     * @param geometry Returns the target geometry of animated widget.
     */
    void setTargetGeometry(const QRectF &geometry);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateEffectiveTime(int currentTime);

private:
    /** Initial geometry */
    QRectF m_startGeometry;
    /** Final geometry */
    QRectF m_targetGeometry;
};

} // PLASMA_ANIMATIONS_GEOMETRY_P_H

#endif
