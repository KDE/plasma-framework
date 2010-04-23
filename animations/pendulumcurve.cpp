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

#include "pendulumcurve_p.h"

/**
 * This static method is used to create a custom easing curve type.
 * @param progress animation progress value
 * @return pendulum easing curve progress value
 */
static qreal pendulumFunction(qreal progress)
{
    if (progress <= 0.25) {
        progress *= 4;
    } else if (progress <= 0.50) {
        progress -= 0.25;
        progress *= 4;
        progress = 1 - progress;
    } else if (progress <= 0.75) {
        progress -= 0.50;
        progress *= -4;
    } else {
        progress -= 0.75;
        progress *= 4;
        progress = 1 - progress;
        progress *= -1;
    }
    return progress;
}

namespace Plasma
{

PendulumCurve::PendulumCurve()
    : QEasingCurve()
{
    setCustomType(pendulumFunction);
}

} // namespace Plasma
