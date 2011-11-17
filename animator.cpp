/*
    Copyright (C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "animator.h"

#include <kdebug.h>

#include "animations/animation.h"
#include "animations/fade_p.h"
#include "animations/grow_p.h"
#include "animations/pulser_p.h"
#include "animations/rotation_p.h"
#include "animations/slide_p.h"
#include "animations/rotationstacked_p.h"
#include "animations/geometry_p.h"
#include "animations/zoom_p.h"
#include "animations/pixmaptransition_p.h"
#include "animations/pendulumcurve_p.h"
#include "theme.h"

namespace Plasma
{

Animator::Animator(QObject *parent)
    : QObject(parent),
      d(0)
{
}

Plasma::Animation* Animator::create(Animator::Animation type, QObject *parent)
{
    Plasma::Animation *result = 0;

    switch (type) {
    case FadeAnimation:
        result = new Plasma::FadeAnimation(parent);
        break;

    case GrowAnimation:
        result = new Plasma::GrowAnimation(parent);
        break;

    case PulseAnimation:
        result = new Plasma::PulseAnimation(parent);
        break;

    case RotationAnimation:
        result = new Plasma::RotationAnimation(parent);
        break;

    case RotationStackedAnimation:
        result = new Plasma::RotationStackedAnimation(parent);
        break;

    case SlideAnimation:
        result = new Plasma::SlideAnimation(parent);
        break;

    case GeometryAnimation:
        result = new Plasma::GeometryAnimation(parent);
        break;

    case ZoomAnimation:
        result = new Plasma::ZoomAnimation(parent);
        break;

    case PixmapTransitionAnimation:
        result = new Plasma::PixmapTransition(parent);
        break;

    default:
        //kDebug() << "Unsupported animation type.";
        break;
    }

    return result;
}

QEasingCurve Animator::create(Animator::CurveShape type)
{
    QEasingCurve result;

    switch (type) {
    case EaseInCurve:
        result.setType(QEasingCurve::InQuad);
        break;

    case EaseOutCurve:
        result.setType(QEasingCurve::OutQuad);
        break;

    case EaseInOutCurve:
        result.setType(QEasingCurve::InOutQuad);
        break;

    case LinearCurve:
        result.setType(QEasingCurve::Linear);
        break;

    case PendularCurve:
        result = PendulumCurve();
        break;

    default:
#ifndef NDEBUG
        kDebug() << "Unsupported easing curve type.";
#endif
        break;
    }

    return result;
}

} // namespace Plasma

#include <animator.moc>

