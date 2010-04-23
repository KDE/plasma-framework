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

#ifndef PLASMA_ANIMATIONS_PENDULUMCURVE_P_H
#define PLASMA_ANIMATIONS_PENDULUMCURVE_P_H

#include <QtCore/QEasingCurve>

namespace Plasma
{

/*
 * @class PendulumCurve plasma/animations/pendulumcurve.h
 * @shot Pendulum Easing Curve
 *
 * This easing curve provides values which are split in 4 parts:
 * from 0 to 1, from 1 to 0, from 0 to -1, and from -1 to 0, in a linear way.
 */
class PendulumCurve : public QEasingCurve
{
public:
    /**
     * default constructor
     */
    PendulumCurve();
};

} // namespace Plasma

#endif // PLASMA_ANIMATIONS_PENDULUMCURVE_P_H
