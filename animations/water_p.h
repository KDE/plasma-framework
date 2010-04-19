/*
 * Copyright (C) 2010 Bruno Abinader <bruno.abinader@indt.org.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLASMA_ANIMATIONS_WATER_P_H
#define PLASMA_ANIMATIONS_WATER_P_H

#include <plasma/animations/easinganimation_p.h>

namespace Plasma
{

/*
 * @class Water plasma/animations/water_p.h
 * @short Water animation using ripple effect
 *
 * Simulates a water animation using ripple effect
 */
class WaterAnimation : public EasingAnimation
{
    Q_OBJECT
    Q_PROPERTY(qint8 offset READ offset WRITE setOffset)

public:
    /**
     * Default constructor
     * @param parent Animation object parent
     */
    explicit WaterAnimation(QObject *parent = 0);

    /**
     * Returns the ripple offset. The ripple offset is the distance between neighbour pixels used to
     * calculate the wave propagation.
     * @return ripple offset
     */
    qint8 offset() const;

public slots:
    /**
     * Sets the ripple offset
     * @param offset ripple offset
     */
    void setOffset(qint8 offset);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateEffectiveTime(int currentTime);

private:
    qint8 m_offset; /** Ripple offset (default is 1) */
};

} // namespace Plasma

#endif // PLASMA_ANIMATIONS_WATER_P_H
